```mermaid
erDiagram

    %% === Core User Entity ===
    USER {
        int id PK
        string name
        string user_type      "ENUM: 'MEMBER' | 'ADMIN' | 'GUEST' etc."
        timestamp created_at  "Time of account creation"
    }

    %% === Member Entity (inherits from User) ===
    MEMBER {
        int id PK 
        string role           "ENUM: 'STUDENT' | 'TEACHER'"
        int borrow_limit      "Max number of items allowed to borrow"
    }

    %% === Specialized Member Roles ===
    STUDENT {
        int id PK 
        string grade_level
    }

    TEACHER {
        int id PK 
        string department
    }

    %% === Core Media Entity ===
    MEDIA {
        int id PK
        string title
        string author          "NULLABLE — used if media_type = 'BOOK'"
        int issue_number       "NULLABLE — used if media_type = 'MAGAZINE'"
        bool is_available
        string media_type      "ENUM: 'BOOK' | 'MAGAZINE'"
    }

    %% === Borrow Relationship ===
    BORROW {
        int borrow_id PK
        int user_id FK          "References USER(id)"
        int media_id FK         "References MEDIA(id)"
        date borrow_date
        date return_date
    }

    %% === Relationships ===
    USER   ||--|| MEMBER : "can be a"
    MEMBER ||--|{ STUDENT : "is a"
    MEMBER ||--|{ TEACHER : "is a"
    USER   ||--o{ BORROW  : "makes"
    MEDIA  ||--o{ BORROW  : "is borrowed in"

```

```sql
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    user_type TEXT DEFAULT 'MEMBER' CHECK (user_type IN ('MEMBER','ADMIN','GUEST')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE members (
    id INT PRIMARY KEY REFERENCES users(id),
    role TEXT CHECK (role IN ('STUDENT','TEACHER')),
    borrow_limit INT DEFAULT 3
);

CREATE TABLE students (
    id INT PRIMARY KEY REFERENCES members(id),
    grade_level TEXT
);

CREATE TABLE teachers (
    id INT PRIMARY KEY REFERENCES members(id),
    department TEXT
);

CREATE TABLE media (
    id SERIAL PRIMARY KEY,
    title TEXT NOT NULL,
    author TEXT,
    issue_number INT,
    is_available BOOLEAN DEFAULT TRUE,
    media_type TEXT CHECK (media_type IN ('BOOK','MAGAZINE'))
);

CREATE TABLE borrow (
    borrow_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id),
    media_id INT REFERENCES media(id),
    borrow_date DATE DEFAULT CURRENT_DATE,
    return_date DATE
);
```