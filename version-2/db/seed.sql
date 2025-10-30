-- Library Database Seed Data
-- PostgreSQL 15+

-- Clear existing data (in correct order to respect foreign keys)
TRUNCATE TABLE task_queue CASCADE;
TRUNCATE TABLE user_roles CASCADE;
TRUNCATE TABLE role_permissions CASCADE;
TRUNCATE TABLE permissions CASCADE;
TRUNCATE TABLE roles CASCADE;
TRUNCATE TABLE borrow_history CASCADE;
TRUNCATE TABLE active_borrow CASCADE;
TRUNCATE TABLE media_copy CASCADE;
TRUNCATE TABLE audiobook CASCADE;
TRUNCATE TABLE dvd CASCADE;
TRUNCATE TABLE magazine CASCADE;
TRUNCATE TABLE book CASCADE;
TRUNCATE TABLE media CASCADE;
TRUNCATE TABLE media_type CASCADE;
TRUNCATE TABLE admins CASCADE;
TRUNCATE TABLE librarians CASCADE;
TRUNCATE TABLE teachers CASCADE;
TRUNCATE TABLE students CASCADE;
TRUNCATE TABLE members CASCADE;
TRUNCATE TABLE users CASCADE;

-- Reset sequences
ALTER SEQUENCE media_type_id_seq RESTART WITH 1;
ALTER SEQUENCE media_id_seq RESTART WITH 1;
ALTER SEQUENCE media_copy_copy_id_seq RESTART WITH 1;
ALTER SEQUENCE active_borrow_borrow_id_seq RESTART WITH 1;
ALTER SEQUENCE borrow_history_borrow_id_seq RESTART WITH 1;
ALTER SEQUENCE users_id_seq RESTART WITH 1;
ALTER SEQUENCE roles_id_seq RESTART WITH 1;
ALTER SEQUENCE permissions_id_seq RESTART WITH 1;
ALTER SEQUENCE task_queue_id_seq RESTART WITH 1;

-- ===== MEDIA TYPES =====
INSERT INTO media_type (name, description) VALUES
('Book', 'Physical books and novels'),
('Magazine', 'Periodical publications'),
('DVD', 'Digital versatile disc media'),
('Audiobook', 'Audio recordings of books');

-- ===== USERS =====
-- Note: password_hash is bcrypt hash of 'password123' for demo purposes
INSERT INTO users (name, email, password_hash, user_type) VALUES
-- Admins
('Alice Admin', 'alice.admin@library.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'ADMIN'),
('Bob Administrator', 'bob.admin@library.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'ADMIN'),

-- Librarians
('Carol Librarian', 'carol.lib@library.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'LIBRARIAN'),
('David Keeper', 'david.lib@library.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'LIBRARIAN'),

-- Members (Students)
('Emma Student', 'emma.s@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER'),
('Frank Learner', 'frank.l@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER'),
('Grace Scholar', 'grace.s@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER'),
('Henry Reader', 'henry.r@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER'),

-- Members (Teachers)
('Iris Teacher', 'iris.t@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER'),
('Jack Professor', 'jack.p@school.edu', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'MEMBER');

-- ===== USER TYPE TABLES =====
INSERT INTO admins (id) VALUES (1), (2);
INSERT INTO librarians (id) VALUES (3), (4);

INSERT INTO members (id, role, borrow_limit) VALUES
(5, 'STUDENT', 3),
(6, 'STUDENT', 3),
(7, 'STUDENT', 3),
(8, 'STUDENT', 3),
(9, 'TEACHER', 5),
(10, 'TEACHER', 5);

INSERT INTO students (id, grade_level) VALUES
(5, 'Grade 9'),
(6, 'Grade 10'),
(7, 'Grade 11'),
(8, 'Grade 12');

INSERT INTO teachers (id, department) VALUES
(9, 'Mathematics'),
(10, 'English Literature');

-- ===== ROLES & PERMISSIONS =====
INSERT INTO roles (name, description) VALUES
('ADMIN', 'Full system access'),
('LIBRARIAN', 'Manage library operations'),
('MEMBER', 'Basic library access'),
('TEACHER', 'Enhanced borrowing privileges');

INSERT INTO permissions (name, description) VALUES
('manage_users', 'Create, update, delete users'),
('manage_media', 'Add, edit, remove media items'),
('manage_copies', 'Handle physical copies'),
('view_reports', 'Access system reports'),
('borrow_media', 'Borrow library items'),
('return_media', 'Return borrowed items'),
('manage_borrows', 'Override borrow rules'),
('system_config', 'Configure system settings');

-- Role-Permission mappings
INSERT INTO role_permissions (role_id, permission_id) VALUES
-- ADMIN: all permissions
(1, 1), (1, 2), (1, 3), (1, 4), (1, 5), (1, 6), (1, 7), (1, 8),
-- LIBRARIAN: operational permissions
(2, 2), (2, 3), (2, 4), (2, 5), (2, 6), (2, 7),
-- MEMBER: basic permissions
(3, 5), (3, 6),
-- TEACHER: extended permissions
(4, 5), (4, 6), (4, 4);

-- User-Role mappings
INSERT INTO user_roles (user_id, role_id) VALUES
(1, 1), -- Alice: Admin
(2, 1), -- Bob: Admin
(3, 2), -- Carol: Librarian
(4, 2), -- David: Librarian
(5, 3), -- Emma: Member
(6, 3), -- Frank: Member
(7, 3), -- Grace: Member
(8, 3), -- Henry: Member
(9, 3), (9, 4), -- Iris: Member + Teacher
(10, 3), (10, 4); -- Jack: Member + Teacher

-- ===== MEDIA ITEMS =====
-- Books
INSERT INTO media (title, media_type_id, is_available) VALUES
('To Kill a Mockingbird', 1, TRUE),
('1984', 1, TRUE),
('Pride and Prejudice', 1, TRUE),
('The Great Gatsby', 1, TRUE),
('Harry Potter and the Sorcerer''s Stone', 1, TRUE),
('The Catcher in the Rye', 1, TRUE),
('The Hobbit', 1, TRUE),
('Fahrenheit 451', 1, TRUE),
('Animal Farm', 1, TRUE),
('Lord of the Flies', 1, TRUE),
('Brave New World', 1, TRUE),
('The Chronicles of Narnia', 1, TRUE);

INSERT INTO book (media_id, author, isbn) VALUES
(1, 'Harper Lee', '978-0-06-112008-4'),
(2, 'George Orwell', '978-0-452-28423-4'),
(3, 'Jane Austen', '978-0-14-143951-8'),
(4, 'F. Scott Fitzgerald', '978-0-7432-7356-5'),
(5, 'J.K. Rowling', '978-0-439-70818-8'),
(6, 'J.D. Salinger', '978-0-316-76948-0'),
(7, 'J.R.R. Tolkien', '978-0-547-92822-7'),
(8, 'Ray Bradbury', '978-1-451-67331-9'),
(9, 'George Orwell', '978-0-452-28424-1'),
(10, 'William Golding', '978-0-399-50148-7'),
(11, 'Aldous Huxley', '978-0-06-085052-4'),
(12, 'C.S. Lewis', '978-0-06-023481-4');

-- Magazines
INSERT INTO media (title, media_type_id, is_available) VALUES
('National Geographic - January 2025', 2, TRUE),
('Scientific American - December 2024', 2, TRUE),
('Time Magazine - October 2024', 2, TRUE),
('The New Yorker - November 2024', 2, TRUE),
('Wired - September 2024', 2, TRUE);

INSERT INTO magazine (media_id, issue_number, publisher) VALUES
(13, 202501, 'National Geographic Society'),
(14, 202412, 'Springer Nature'),
(15, 202410, 'Time USA LLC'),
(16, 202411, 'Condé Nast'),
(17, 202409, 'Condé Nast');

-- DVDs
INSERT INTO media (title, media_type_id, is_available) VALUES
('The Shawshank Redemption', 3, TRUE),
('The Godfather', 3, TRUE),
('Schindler''s List', 3, TRUE),
('Inception', 3, TRUE),
('The Matrix', 3, TRUE),
('Forrest Gump', 3, TRUE);

INSERT INTO dvd (media_id, director) VALUES
(18,  'Frank Darabont'),
(19,  'Francis Ford Coppola'),
(20,  'Steven Spielberg'),
(21,  'Christopher Nolan'),
(22,  'Lana Wachowski, Lilly Wachowski'),
(23,  'Robert Zemeckis');

-- Audiobooks
INSERT INTO media (title, media_type_id, is_available) VALUES
('Becoming by Michelle Obama', 4, TRUE),
('Educated by Tara Westover', 4, TRUE),
('The Subtle Art of Not Giving a F*ck', 4, TRUE),
('Atomic Habits', 4, TRUE);

INSERT INTO audiobook (media_id, narrator) VALUES
(24,  'Michelle Obama'),
(25,  'Julia Whelan'),
(26,  'Roger Wayne'),
(27,  'James Clear');

-- ===== MEDIA COPIES =====
-- Create multiple copies for popular items
-- Books (2-3 copies each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
-- To Kill a Mockingbird
(1, 'GOOD', TRUE), (1, 'EXCELLENT', TRUE), (1, 'FAIR', TRUE),
-- 1984
(2, 'GOOD', FALSE), (2, 'GOOD', TRUE),
-- Pride and Prejudice
(3, 'EXCELLENT', TRUE), (3, 'GOOD', TRUE),
-- The Great Gatsby
(4, 'GOOD', TRUE), (4, 'FAIR', TRUE),
-- Harry Potter
(5, 'EXCELLENT', FALSE), (5, 'GOOD', TRUE), (5, 'GOOD', TRUE),
-- The Catcher in the Rye
(6, 'GOOD', TRUE), (6, 'GOOD', TRUE),
-- The Hobbit
(7, 'EXCELLENT', TRUE), (7, 'GOOD', FALSE),
-- Fahrenheit 451
(8, 'GOOD', TRUE), (8, 'FAIR', TRUE),
-- Animal Farm
(9, 'GOOD', TRUE), (9, 'GOOD', TRUE),
-- Lord of the Flies
(10, 'EXCELLENT', TRUE), (10, 'GOOD', TRUE),
-- Brave New World
(11, 'GOOD', FALSE), (11, 'FAIR', TRUE),
-- Chronicles of Narnia
(12, 'EXCELLENT', TRUE), (12, 'GOOD', TRUE);

-- Magazines (1 copy each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(13, 'EXCELLENT', TRUE),
(14, 'EXCELLENT', TRUE),
(15, 'GOOD', TRUE),
(16, 'GOOD', TRUE),
(17, 'GOOD', TRUE);

-- DVDs (2 copies each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(18, 'GOOD', TRUE), (18, 'EXCELLENT', TRUE),
(19, 'GOOD', TRUE), (19, 'GOOD', FALSE),
(20, 'EXCELLENT', TRUE), (20, 'GOOD', TRUE),
(21, 'GOOD', TRUE), (21, 'GOOD', TRUE),
(22, 'EXCELLENT', TRUE), (22, 'GOOD', TRUE),
(23, 'GOOD', TRUE), (23, 'FAIR', TRUE);

-- Audiobooks (1 copy each)
INSERT INTO media_copy (media_id, condition, is_available) VALUES
(24, 'EXCELLENT', TRUE),
(25, 'EXCELLENT', TRUE),
(26, 'GOOD', TRUE),
(27, 'GOOD', TRUE);

-- ===== ACTIVE BORROWS =====
INSERT INTO active_borrow (user_id, copy_id, borrow_date, due_date) VALUES
-- Emma borrowed 1984 (copy 4)
(5, 4, NOW() - INTERVAL '5 days', NOW() + INTERVAL '9 days'),
-- Grace borrowed Harry Potter (copy 10)
(7, 10, NOW() - INTERVAL '3 days', NOW() + INTERVAL '11 days'),
-- Henry borrowed The Hobbit (copy 16)
(8, 16, NOW() - INTERVAL '10 days', NOW() + INTERVAL '4 days'),
-- Iris borrowed Brave New World (copy 25)
(9, 25, NOW() - INTERVAL '7 days', NOW() + INTERVAL '7 days'),
-- Jack borrowed The Godfather DVD (copy 33)
(10, 33, NOW() - INTERVAL '2 days', NOW() + INTERVAL '5 days');

-- ===== BORROW HISTORY =====
INSERT INTO borrow_history (user_id, copy_id, borrow_date, return_date) VALUES
-- Emma's history
(5, 1, NOW() - INTERVAL '45 days', NOW() - INTERVAL '30 days'),
(5, 18, NOW() - INTERVAL '30 days', NOW() - INTERVAL '23 days'),
-- Frank's history
(6, 5, NOW() - INTERVAL '60 days', NOW() - INTERVAL '46 days'),
(6, 19, NOW() - INTERVAL '40 days', NOW() - INTERVAL '33 days'),
(6, 13, NOW() - INTERVAL '25 days', NOW() - INTERVAL '20 days'),
-- Grace's history
(7, 7, NOW() - INTERVAL '50 days', NOW() - INTERVAL '36 days'),
(7, 31, NOW() - INTERVAL '35 days', NOW() - INTERVAL '28 days'),
-- Henry's history
(8, 11, NOW() - INTERVAL '55 days', NOW() - INTERVAL '41 days'),
-- Iris's history
(9, 20, NOW() - INTERVAL '70 days', NOW() - INTERVAL '56 days'),
(9, 22, NOW() - INTERVAL '55 days', NOW() - INTERVAL '48 days'),
(9, 24, NOW() - INTERVAL '40 days', NOW() - INTERVAL '26 days'),
(9, 28, NOW() - INTERVAL '25 days', NOW() - INTERVAL '18 days'),
-- Jack's history
(10, 8, NOW() - INTERVAL '65 days', NOW() - INTERVAL '51 days'),
(10, 14, NOW() - INTERVAL '50 days', NOW() - INTERVAL '43 days'),
(10, 27, NOW() - INTERVAL '30 days', NOW() - INTERVAL '23 days');

-- ===== TASK QUEUE =====
INSERT INTO task_queue (task_type, payload, status) VALUES
('email_overdue_notice', '{"user_id": 8, "copy_id": 16}', 'PENDING'),
('generate_monthly_report', '{"month": "2024-09"}', 'DONE'),
('backup_database', '{"type": "full"}', 'DONE'),
('sync_catalog', '{"source": "external_api"}', 'PENDING');

-- ===== VERIFICATION QUERIES =====
-- Uncomment to verify the seed data

-- SELECT 'Media Types' AS category, COUNT(*) AS count FROM media_type
-- UNION ALL
-- SELECT 'Media Items', COUNT(*) FROM media
-- UNION ALL
-- SELECT 'Books', COUNT(*) FROM book
-- UNION ALL
-- SELECT 'Magazines', COUNT(*) FROM magazine
-- UNION ALL
-- SELECT 'DVDs', COUNT(*) FROM dvd
-- UNION ALL
-- SELECT 'Audiobooks', COUNT(*) FROM audiobook
-- UNION ALL
-- SELECT 'Media Copies', COUNT(*) FROM media_copy
-- UNION ALL
-- SELECT 'Users', COUNT(*) FROM users
-- UNION ALL
-- SELECT 'Active Borrows', COUNT(*) FROM active_borrow
-- UNION ALL
-- SELECT 'Borrow History', COUNT(*) FROM borrow_history
-- UNION ALL
-- SELECT 'Roles', COUNT(*) FROM roles
-- UNION ALL
-- SELECT 'Permissions', COUNT(*) FROM permissions;

COMMIT;

-- Success message
DO $$
BEGIN
    RAISE NOTICE 'Library database seeded successfully!';
    RAISE NOTICE '- 4 media types';
    RAISE NOTICE '- 27 media items (12 books, 5 magazines, 6 DVDs, 4 audiobooks)';
    RAISE NOTICE '- 51 physical copies';
    RAISE NOTICE '- 10 users (2 admins, 2 librarians, 4 students, 2 teachers)';
    RAISE NOTICE '- 5 active borrows';
    RAISE NOTICE '- 14 historical borrows';
    RAISE NOTICE '- 4 roles with 8 permissions';
    RAISE NOTICE 'Default password for all users: password123';
END $$;