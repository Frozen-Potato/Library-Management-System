TRUNCATE TABLE teachers, students, members, borrow, media, users RESTART IDENTITY CASCADE;

INSERT INTO users (name, user_type)
VALUES 
('Alice', 'MEMBER'),
('Bob', 'MEMBER');

INSERT INTO members (id, role, borrow_limit)
SELECT id, 'STUDENT', 3 FROM users WHERE name = 'Alice'
UNION ALL
SELECT id, 'TEACHER', 5 FROM users WHERE name = 'Bob';

INSERT INTO students (id, grade_level)
SELECT id, 'Sophomore' FROM users WHERE name = 'Alice';

INSERT INTO teachers (id, department)
SELECT id, 'Computer Science' FROM users WHERE name = 'Bob';

INSERT INTO media (title, author, media_type, is_available)
VALUES
('The C++ Programming Language', 'Bjarne Stroustrup', 'BOOK', TRUE),
('Effective Modern C++', 'Scott Meyers', 'BOOK', TRUE),
('National Geographic - May 2023', NULL, 'MAGAZINE', TRUE),
('Time Magazine - June 2023', NULL, 'MAGAZINE', TRUE);

-- BORROW
INSERT INTO borrow (user_id, media_id, borrow_date)
SELECT id, 1, CURRENT_DATE FROM users WHERE name = 'Alice';

INSERT INTO borrow (user_id, media_id, borrow_date)
SELECT id, 3, CURRENT_DATE FROM users WHERE name = 'Bob';

-- Update borrowed media to unavailable
UPDATE media
SET is_available = FALSE
WHERE id IN (
    SELECT media_id FROM borrow
);