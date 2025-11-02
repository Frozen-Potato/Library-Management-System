-- Library Database Schema 
-- PostgreSQL 15+

-- Reference Table: Media Type ===
CREATE TABLE IF NOT EXISTS media_type (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    description TEXT
);

-- Core Abstract Media ===
CREATE TABLE IF NOT EXISTS media (
    id BIGSERIAL PRIMARY KEY,
    title TEXT NOT NULL,
    media_type_id INT REFERENCES media_type(id) ON DELETE CASCADE,
    is_available BOOLEAN DEFAULT TRUE
);

CREATE INDEX IF NOT EXISTS idx_media_type_id ON media(media_type_id);
CREATE INDEX IF NOT EXISTS idx_media_title ON media USING gin (to_tsvector('english', title));

-- Subtypes ===
CREATE TABLE IF NOT EXISTS book (
    media_id BIGINT PRIMARY KEY REFERENCES media(id) ON DELETE CASCADE,
    author TEXT,
    isbn TEXT
);

CREATE INDEX IF NOT EXISTS idx_book_isbn ON book(isbn);

CREATE TABLE IF NOT EXISTS magazine (
    media_id BIGINT PRIMARY KEY REFERENCES media(id) ON DELETE CASCADE,
    issue_number INT,
    publisher TEXT
);

CREATE TABLE IF NOT EXISTS dvd (
    media_id BIGINT PRIMARY KEY REFERENCES media(id) ON DELETE CASCADE,
    director TEXT
);

CREATE TABLE IF NOT EXISTS audiobook (
    media_id BIGINT PRIMARY KEY REFERENCES media(id) ON DELETE CASCADE,
    narrator TEXT
);

-- Physical Copies ===
CREATE TABLE IF NOT EXISTS media_copy (
    copy_id BIGSERIAL PRIMARY KEY,
    media_id BIGINT NOT NULL REFERENCES media(id) ON DELETE CASCADE,
    condition TEXT DEFAULT 'GOOD',
    is_available BOOLEAN DEFAULT TRUE
);

CREATE INDEX IF NOT EXISTS idx_copy_media_id ON media_copy(media_id);
CREATE INDEX IF NOT EXISTS idx_copy_availability ON media_copy(is_available);

-- Users ===
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_name ON users(name);

CREATE TABLE IF NOT EXISTS members (
    id INT PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    borrow_limit INT DEFAULT 3
);

CREATE TABLE IF NOT EXISTS students (
    id INT PRIMARY KEY REFERENCES members(id) ON DELETE CASCADE,
    grade_level TEXT
);

CREATE TABLE IF NOT EXISTS teachers (
    id INT PRIMARY KEY REFERENCES members(id) ON DELETE CASCADE,
    department TEXT
);

CREATE TABLE IF NOT EXISTS librarians (
    id INT PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS admins (
    id INT PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE
);

-- Roles & Permissions ===
CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS permissions (
    id SERIAL PRIMARY KEY,
    action TEXT NOT NULL,
    table_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS role_permissions (
    role_id INT REFERENCES roles(id) ON DELETE CASCADE,
    permission_id INT REFERENCES permissions(id) ON DELETE CASCADE,
    PRIMARY KEY (role_id, permission_id)
);

CREATE TABLE IF NOT EXISTS user_roles (
    user_id INT REFERENCES users(id) ON DELETE CASCADE,
    role_id INT REFERENCES roles(id) ON DELETE CASCADE,
    PRIMARY KEY (user_id, role_id)
);

CREATE INDEX IF NOT EXISTS idx_user_roles_user_id ON user_roles(user_id);
CREATE INDEX IF NOT EXISTS idx_role_permissions_role_id ON role_permissions(role_id);

CREATE TABLE IF NOT EXISTS task_queue (
    id BIGSERIAL PRIMARY KEY,
    task_type TEXT NOT NULL,
    payload JSONB NOT NULL,
    status TEXT DEFAULT 'PENDING' CHECK (status IN ('PENDING','PROCESSING','DONE','FAILED')),
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_task_status ON task_queue(status);

-- Borrowing ===
CREATE TABLE IF NOT EXISTS active_borrow (
    borrow_id BIGSERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    copy_id BIGINT NOT NULL REFERENCES media_copy(copy_id) ON DELETE CASCADE,
    borrow_date TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    due_date TIMESTAMPTZ
);

CREATE UNIQUE INDEX IF NOT EXISTS uq_active_copy_borrow ON active_borrow(copy_id) WHERE due_date IS NULL;
CREATE INDEX IF NOT EXISTS idx_borrow_user_id ON active_borrow(user_id);
CREATE INDEX IF NOT EXISTS idx_borrow_date ON active_borrow(borrow_date);

CREATE TABLE IF NOT EXISTS borrow_history (
    borrow_id BIGSERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    copy_id BIGINT NOT NULL REFERENCES media_copy(copy_id) ON DELETE CASCADE,
    borrow_date TIMESTAMPTZ,
    return_date TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_history_user_id ON borrow_history(user_id);
CREATE INDEX IF NOT EXISTS idx_history_copy_id ON borrow_history(copy_id);
CREATE INDEX IF NOT EXISTS idx_history_return_date ON borrow_history(return_date);

-- Utility Procedures (excluding move to history)


-- Create a new media copy for a given media
CREATE OR REPLACE PROCEDURE create_media_copy(p_media_id BIGINT, p_condition TEXT DEFAULT 'GOOD')
LANGUAGE plpgsql AS $$
BEGIN
    INSERT INTO media_copy (media_id, condition, is_available)
    VALUES (p_media_id, p_condition, TRUE);
END;
$$;

-- Mark a media copy as borrowed (set unavailable)
CREATE OR REPLACE PROCEDURE mark_copy_borrowed(p_copy_id BIGINT)
LANGUAGE plpgsql AS $$
BEGIN
    UPDATE media_copy
    SET is_available = FALSE
    WHERE copy_id = p_copy_id;
END;
$$;

-- Mark a media copy as available (returned)
CREATE OR REPLACE PROCEDURE mark_copy_returned(p_copy_id BIGINT)
LANGUAGE plpgsql AS $$
BEGIN
    UPDATE media_copy
    SET is_available = TRUE
    WHERE copy_id = p_copy_id;
END;
$$;

-- Insert an active borrow record
CREATE OR REPLACE PROCEDURE add_active_borrow(p_user_id INT, p_copy_id BIGINT)
LANGUAGE plpgsql AS $$
BEGIN
    INSERT INTO active_borrow (user_id, copy_id)
    VALUES (p_user_id, p_copy_id);
    CALL mark_copy_borrowed(p_copy_id);
END;
$$;

-- Mark a copy as lost (utility)
CREATE OR REPLACE PROCEDURE mark_copy_lost(p_copy_id BIGINT)
LANGUAGE plpgsql AS $$
BEGIN
    UPDATE media_copy
    SET is_available = FALSE, condition = 'LOST'
    WHERE copy_id = p_copy_id;
END;
$$;

-- Update media availability based on copies
CREATE OR REPLACE PROCEDURE sync_media_availability(p_media_id BIGINT)
LANGUAGE plpgsql AS $$
DECLARE
    available_count INT;
BEGIN
    SELECT COUNT(*) INTO available_count
    FROM media_copy
    WHERE media_id = p_media_id AND is_available = TRUE;

    UPDATE media
    SET is_available = (available_count > 0)
    WHERE id = p_media_id;
END;
$$;

CREATE OR REPLACE FUNCTION fn_sync_media_availability()
RETURNS TRIGGER AS $$
DECLARE
    target_media_id BIGINT;
BEGIN
    IF (TG_OP = 'INSERT') THEN
        target_media_id := NEW.media_id;

    ELSIF (TG_OP = 'UPDATE') THEN
        -- Only sync if availability changed or media_id changed
        IF NEW.is_available IS DISTINCT FROM OLD.is_available
           OR NEW.media_id IS DISTINCT FROM OLD.media_id THEN
            target_media_id := NEW.media_id;
        ELSE
            RETURN NEW;
        END IF;

    ELSIF (TG_OP = 'DELETE') THEN
        target_media_id := OLD.media_id;
    END IF;

    -- Call the sync procedure if we have a target media ID
    IF target_media_id IS NOT NULL THEN
        CALL sync_media_availability(target_media_id);
    END IF;

    -- Return appropriate record
    IF (TG_OP = 'DELETE') THEN
        RETURN OLD;
    ELSE
        RETURN NEW;
    END IF;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_sync_media_availability ON media_copy;

CREATE TRIGGER trg_sync_media_availability
AFTER INSERT OR UPDATE OR DELETE
ON media_copy
FOR EACH ROW
EXECUTE FUNCTION fn_sync_media_availability();

CREATE OR REPLACE FUNCTION notify_permission_change()
RETURNS trigger AS $$
BEGIN
  PERFORM pg_notify('permissions_changed', 'reload');
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER permission_change_trigger
AFTER INSERT OR UPDATE OR DELETE ON role_permissions
FOR EACH STATEMENT EXECUTE FUNCTION notify_permission_change();