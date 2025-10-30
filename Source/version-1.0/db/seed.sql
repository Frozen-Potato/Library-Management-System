-- Reset all relevant tables
TRUNCATE TABLE teachers, students, members, borrow, media_copy, media, users RESTART IDENTITY CASCADE;

-- USERS
INSERT INTO users (name, user_type)
VALUES 
('Alice', 'MEMBER'),
('Bob', 'MEMBER');

-- MEMBERS
INSERT INTO members (id, role, borrow_limit)
SELECT id, 'STUDENT', 3 FROM users WHERE name = 'Alice'
UNION ALL
SELECT id, 'TEACHER', 5 FROM users WHERE name = 'Bob';

-- STUDENTS
INSERT INTO students (id, grade_level)
SELECT id, 'Sophomore' FROM users WHERE name = 'Alice';

-- TEACHERS
INSERT INTO teachers (id, department)
SELECT id, 'Computer Science' FROM users WHERE name = 'Bob';

-- MEDIA
INSERT INTO media (title, author, media_type, is_available)
VALUES
('The C++ Programming Language', 'Bjarne Stroustrup', 'BOOK', TRUE),
('Effective Modern C++', 'Scott Meyers', 'BOOK', TRUE),
('National Geographic - May 2023', NULL, 'MAGAZINE', TRUE),
('Time Magazine - June 2023', NULL, 'MAGAZINE', TRUE);

-- MEDIA COPIES
INSERT INTO media_copy (media_id, condition, is_available, location)
VALUES
(1, 'GOOD', TRUE, 'Shelf A1'),
(1, 'GOOD', TRUE, 'Shelf A2'),
(2, 'GOOD', TRUE, 'Shelf B1'),
(3, 'GOOD', TRUE, 'Shelf C1'),
(4, 'GOOD', TRUE, 'Shelf C2');

-- BORROW (note: using copy_id now, not media_id)
-- Alice borrows a copy of "The C++ Programming Language"
INSERT INTO borrow (user_id, copy_id, borrow_date)
SELECT u.id, c.copy_id, CURRENT_DATE
FROM users u
JOIN media_copy c ON c.media_id = 1
WHERE u.name = 'Alice'
LIMIT 1;

-- Bob borrows a copy of "National Geographic - May 2023"
INSERT INTO borrow (user_id, copy_id, borrow_date)
SELECT u.id, c.copy_id, CURRENT_DATE
FROM users u
JOIN media_copy c ON c.media_id = 3
WHERE u.name = 'Bob'
LIMIT 1;

-- Update borrowed copies and corresponding media as unavailable
UPDATE media_copy
SET is_available = FALSE
WHERE copy_id IN (SELECT copy_id FROM borrow);

UPDATE media
SET is_available = FALSE
WHERE id IN (
    SELECT media_id 
    FROM media_copy 
    WHERE copy_id IN (SELECT copy_id FROM borrow)
);
