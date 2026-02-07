-- Extended Sample Data for Library Management System
-- Run AFTER seed.sql to add additional media, users, and borrow records
-- Usage: docker exec -i library_postgres psql -U postgres -d librarydb < db/sample_data.sql

BEGIN;

-- ===== ADDITIONAL USERS =====
-- password_hash is bcrypt hash of 'password123'
INSERT INTO users (name, email, password_hash) VALUES
('Karen Librarian', 'karen.lib@library.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy'),
('Leo Student', 'leo.s@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy'),
('Mia Student', 'mia.s@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy'),
('Noah Teacher', 'noah.t@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy'),
('Olivia Student', 'olivia.s@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy'),
('Paul Member', 'paul.m@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy');

-- Karen (id 11) -> Librarian
INSERT INTO librarians (id) VALUES (11);

-- Leo (12), Mia (13), Olivia (15) -> Students
INSERT INTO members (id, borrow_limit) VALUES (12, 3), (13, 3), (15, 3), (16, 3);
INSERT INTO students (id, grade_level) VALUES (12, 'Grade 9'), (13, 'Grade 10'), (15, 'Grade 12');

-- Noah (14) -> Teacher
INSERT INTO members (id, borrow_limit) VALUES (14, 5);
INSERT INTO teachers (id, department) VALUES (14, 'History');

-- Paul (16) -> regular member (no student/teacher subtype)

-- User-Role mappings for new users
INSERT INTO user_roles (user_id, role_id) VALUES
(11, 2),          -- Karen: Librarian
(12, 3), (12, 5), -- Leo: Member + Student
(13, 3), (13, 5), -- Mia: Member + Student
(14, 3), (14, 4), -- Noah: Member + Teacher
(15, 3), (15, 5), -- Olivia: Member + Student
(16, 3);          -- Paul: Member


-- ===== ADDITIONAL BOOKS =====
INSERT INTO media (title, media_type_id, is_available) VALUES
('Moby-Dick', 1, TRUE),
('War and Peace', 1, TRUE),
('Jane Eyre', 1, TRUE),
('Wuthering Heights', 1, TRUE),
('The Odyssey', 1, TRUE),
('Crime and Punishment', 1, TRUE),
('Great Expectations', 1, TRUE),
('Don Quixote', 1, TRUE),
('The Count of Monte Cristo', 1, TRUE),
('Les Miserables', 1, TRUE),
('Dracula', 1, TRUE),
('Frankenstein', 1, TRUE),
('The Picture of Dorian Gray', 1, TRUE),
('A Tale of Two Cities', 1, TRUE),
('The Grapes of Wrath', 1, TRUE);

INSERT INTO book (media_id, author, isbn) VALUES
(28, 'Herman Melville', '978-0-14-243724-7'),
(29, 'Leo Tolstoy', '978-0-14-044793-4'),
(30, 'Charlotte Bronte', '978-0-14-144114-6'),
(31, 'Emily Bronte', '978-0-14-143955-6'),
(32, 'Homer', '978-0-14-026886-7'),
(33, 'Fyodor Dostoevsky', '978-0-14-044913-6'),
(34, 'Charles Dickens', '978-0-14-143956-3'),
(35, 'Miguel de Cervantes', '978-0-06-093434-7'),
(36, 'Alexandre Dumas', '978-0-14-044926-6'),
(37, 'Victor Hugo', '978-0-14-044430-8'),
(38, 'Bram Stoker', '978-0-14-143984-6'),
(39, 'Mary Shelley', '978-0-14-143947-1'),
(40, 'Oscar Wilde', '978-0-14-143957-0'),
(41, 'Charles Dickens', '978-0-14-143960-0'),
(42, 'John Steinbeck', '978-0-14-018640-2');


-- ===== ADDITIONAL MAGAZINES =====
INSERT INTO media (title, media_type_id, is_available) VALUES
('Nature - February 2025', 2, TRUE),
('The Economist - January 2025', 2, TRUE),
('Popular Science - March 2025', 2, TRUE),
('IEEE Spectrum - December 2024', 2, TRUE);

INSERT INTO magazine (media_id, issue_number, publisher) VALUES
(43, 202502, 'Springer Nature'),
(44, 202501, 'The Economist Group'),
(45, 202503, 'Bonnier Corp'),
(46, 202412, 'IEEE');


-- ===== ADDITIONAL DVDS =====
INSERT INTO media (title, media_type_id, is_available) VALUES
('Pulp Fiction', 3, TRUE),
('The Dark Knight', 3, TRUE),
('Interstellar', 3, TRUE),
('Gladiator', 3, TRUE);

INSERT INTO dvd (media_id, director) VALUES
(47, 'Quentin Tarantino'),
(48, 'Christopher Nolan'),
(49, 'Christopher Nolan'),
(50, 'Ridley Scott');


-- ===== ADDITIONAL AUDIOBOOKS =====
INSERT INTO media (title, media_type_id, is_available) VALUES
('Sapiens by Yuval Noah Harari', 4, TRUE),
('Thinking, Fast and Slow', 4, TRUE),
('The Power of Habit', 4, TRUE);

INSERT INTO audiobook (media_id, narrator) VALUES
(51, 'Derek Perkins'),
(52, 'Patrick Egan'),
(53, 'Mike Chamberlain');


-- ===== COPIES FOR NEW MEDIA =====
-- Books (2 copies each for the first 10, 1 copy for the rest)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(28, 'GOOD', TRUE), (28, 'EXCELLENT', TRUE),
(29, 'GOOD', TRUE), (29, 'FAIR', TRUE),
(30, 'EXCELLENT', TRUE), (30, 'GOOD', TRUE),
(31, 'GOOD', TRUE), (31, 'GOOD', TRUE),
(32, 'EXCELLENT', TRUE), (32, 'GOOD', TRUE),
(33, 'GOOD', TRUE), (33, 'FAIR', TRUE),
(34, 'GOOD', TRUE), (34, 'EXCELLENT', TRUE),
(35, 'GOOD', TRUE), (35, 'GOOD', TRUE),
(36, 'EXCELLENT', TRUE), (36, 'GOOD', TRUE),
(37, 'GOOD', TRUE), (37, 'FAIR', TRUE),
(38, 'GOOD', TRUE),
(39, 'EXCELLENT', TRUE),
(40, 'GOOD', TRUE),
(41, 'GOOD', TRUE),
(42, 'GOOD', TRUE);

-- Magazines (1 copy each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(43, 'EXCELLENT', TRUE),
(44, 'EXCELLENT', TRUE),
(45, 'GOOD', TRUE),
(46, 'GOOD', TRUE);

-- DVDs (2 copies each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(47, 'GOOD', TRUE), (47, 'EXCELLENT', TRUE),
(48, 'GOOD', TRUE), (48, 'GOOD', TRUE),
(49, 'EXCELLENT', TRUE), (49, 'GOOD', TRUE),
(50, 'GOOD', TRUE), (50, 'FAIR', TRUE);

-- Audiobooks (1 copy each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(51, 'EXCELLENT', TRUE),
(52, 'GOOD', TRUE),
(53, 'GOOD', TRUE);


-- ===== ADDITIONAL ACTIVE BORROWS =====
-- Using copy IDs that start at 52 (seed used 1-51)
INSERT INTO active_borrow (user_id, copy_id, borrow_date, due_date) VALUES
-- Leo borrowed Moby-Dick (copy 52)
(12, 52, NOW() - INTERVAL '4 days', NOW() + INTERVAL '10 days'),
-- Mia borrowed War and Peace (copy 54)
(13, 54, NOW() - INTERVAL '6 days', NOW() + INTERVAL '8 days'),
-- Noah borrowed Crime and Punishment (copy 62)
(14, 62, NOW() - INTERVAL '1 day', NOW() + INTERVAL '13 days'),
-- Olivia borrowed The Odyssey (copy 60)
(15, 60, NOW() - INTERVAL '8 days', NOW() + INTERVAL '6 days'),
-- Paul borrowed Pulp Fiction DVD (copy 82)
(16, 82, NOW() - INTERVAL '3 days', NOW() + INTERVAL '4 days');

-- Mark those copies as unavailable
UPDATE media_copy SET is_available = FALSE WHERE copy_id IN (52, 54, 60, 62, 82);


-- ===== ADDITIONAL BORROW HISTORY =====
INSERT INTO borrow_history (user_id, copy_id, borrow_date, return_date) VALUES
-- Leo's history
(12, 1, NOW() - INTERVAL '90 days', NOW() - INTERVAL '76 days'),
(12, 11, NOW() - INTERVAL '60 days', NOW() - INTERVAL '45 days'),
(12, 30, NOW() - INTERVAL '30 days', NOW() - INTERVAL '18 days'),
-- Mia's history
(13, 6, NOW() - INTERVAL '80 days', NOW() - INTERVAL '66 days'),
(13, 37, NOW() - INTERVAL '50 days', NOW() - INTERVAL '35 days'),
-- Noah's history
(14, 8, NOW() - INTERVAL '70 days', NOW() - INTERVAL '56 days'),
(14, 34, NOW() - INTERVAL '45 days', NOW() - INTERVAL '31 days'),
(14, 27, NOW() - INTERVAL '20 days', NOW() - INTERVAL '8 days'),
-- Olivia's history
(15, 13, NOW() - INTERVAL '85 days', NOW() - INTERVAL '71 days'),
(15, 7, NOW() - INTERVAL '55 days', NOW() - INTERVAL '41 days'),
(15, 20, NOW() - INTERVAL '25 days', NOW() - INTERVAL '12 days'),
-- Paul's history
(16, 31, NOW() - INTERVAL '75 days', NOW() - INTERVAL '61 days'),
(16, 38, NOW() - INTERVAL '40 days', NOW() - INTERVAL '26 days');

-- Emma borrows more
INSERT INTO borrow_history (user_id, copy_id, borrow_date, return_date) VALUES
(5, 15, NOW() - INTERVAL '20 days', NOW() - INTERVAL '10 days'),
(5, 38, NOW() - INTERVAL '15 days', NOW() - INTERVAL '7 days');

-- Frank borrows more
INSERT INTO borrow_history (user_id, copy_id, borrow_date, return_date) VALUES
(6, 34, NOW() - INTERVAL '18 days', NOW() - INTERVAL '8 days'),
(6, 27, NOW() - INTERVAL '12 days', NOW() - INTERVAL '5 days');

-- Iris borrows more
INSERT INTO borrow_history (user_id, copy_id, borrow_date, return_date) VALUES
(9, 6, NOW() - INTERVAL '15 days', NOW() - INTERVAL '3 days'),
(9, 40, NOW() - INTERVAL '10 days', NOW() - INTERVAL '2 days');


-- ===== ADDITIONAL TASK QUEUE ENTRIES =====
INSERT INTO task_queue (task_type, payload, status) VALUES
('email_overdue_notice', '{"user_id": 15, "copy_id": 60}', 'PENDING'),
('generate_monthly_report', '{"month": "2025-01"}', 'PENDING'),
('sync_catalog', '{"source": "external_api"}', 'PROCESSING'),
('email_due_reminder', '{"user_id": 13, "copy_id": 54}', 'PENDING'),
('backup_database', '{"type": "incremental"}', 'DONE');

COMMIT;

-- Verification
DO $$
DECLARE
    media_count INT;
    user_count INT;
    copy_count INT;
    active_count INT;
    history_count INT;
BEGIN
    SELECT COUNT(*) INTO media_count FROM media;
    SELECT COUNT(*) INTO user_count FROM users;
    SELECT COUNT(*) INTO copy_count FROM media_copy;
    SELECT COUNT(*) INTO active_count FROM active_borrow;
    SELECT COUNT(*) INTO history_count FROM borrow_history;

    RAISE NOTICE 'Sample data loaded successfully!';
    RAISE NOTICE '  Total media items: %', media_count;
    RAISE NOTICE '  Total users: %', user_count;
    RAISE NOTICE '  Total copies: %', copy_count;
    RAISE NOTICE '  Active borrows: %', active_count;
    RAISE NOTICE '  Borrow history: %', history_count;
END $$;
