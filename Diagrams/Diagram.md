# Library Diagram

## Class Diagram v1 (End of Day 1)

```mermaid
classDiagram
    class Library {
        - books : unordered_map<int, shared_ptr<Book>>
        - members : unordered_map<int, shared_ptr<Member>>
        + Library()
        + ~Library()
        + addBook(id:int, title:string, author:string) void
        + findBook(id:int) shared_ptr<Book>
        + removeBook(id:int) void
        + addMember<T>(id:int, args...) void
        + findMember(id:int) shared_ptr<Member>
        + removeMember(id:int) void
        + borrowBook(bookId:int, memberId:int) void
        + returnBook(bookId:int, memberId:int) void
        + listBooks() void
        + listMembers() void
    }

    class Member {
        + id : int
        + name : string
        + borrowedBooks : vector<int>
        + Member()
        + Member(id:int, name:string)
        + ~Member()
        + getBorrowLimit() int*
        + borrowBook(book:shared_ptr<Book>) void
        + returnBook(book:shared_ptr<Book>) void
        + hasBorrowed(bookId:int) bool
        + allBorrowedUnavailable(LibBooks:unordered_map<int,shared_ptr<Book>>) bool
        + getName() string
        + operator==(Member&) bool
    }

    class Student {
        + getBorrowLimit() int
    }

    class Teacher {
        + getBorrowLimit() int
    }

    class Book {
        - id : int
        - title : string
        - author : string
        - isAvailable : bool
        + count : static size_t
        + Book()
        + Book(id:int, title:string, author:string)
        + Book(other:Book)
        + ~Book()
        + getId() size_t
        + getName() string
        + getAvailability() bool
        + setAvailability(newStatus:bool) void
        + operator==(rhs:Book) bool
        + operator<(rhs:Book) bool
    }

    %% Inheritance
    Member <|-- Student
    Member <|-- Teacher

    %% Associations
    Library "1" --> "0..*" Book : manages
    Library "1" --> "0..*" Member : manages
    Member "1" --> "0..*" Book : borrows
```

## Sequence Diagrams

### borrowBook

```mermaid
sequenceDiagram
    participant Client
    participant Library
    participant Member
    participant Book

    Client->>Library: borrowBook(bookId, memberId)
    Library->>Library: findBookById(bookId)
    Library->>Library: findMemberById(memberId)
    Library->>Member: borrowBook(book)
    Member->>Book: getAvailability()
    alt Book available AND limit not reached
        Member->>Book: setAvailability(false)
        Member->>Member: borrowedBooks.push_back(bookId)
        Member-->>Library: success
    else Error
        Member-->>Library: throw exception
    end
    Library-->>Client: result/exception
```

### returnBook

```mermaid
sequenceDiagram
    participant Client
    participant Library
    participant Member
    participant Book

    Client->>Library: returnBook(bookId, memberId)
    Library->>Library: findBookById(bookId)
    Library->>Library: findMemberById(memberId)
    Library->>Member: returnBook(book)
    Member->>Member: find bookId in borrowedBooks
    alt Found
        Member->>Member: erase(bookId)
        Member->>Book: setAvailability(true)
        Member-->>Library: success
    else Not Found
        Member-->>Library: throw GeneralFailure
    end
    Library-->>Client: result/exception
```
## Class Diagram (Media + Magazine)
```mermaid
classDiagram
    class Media {
        <<interface>>
        +getId() size_t
        +getName() string
        +getAvailability() bool
        +setAvailability(bool) void
        +print(out) void
    }

    class Magazine {
        -id : int
        -title : string
        -issueNumber : int
        -available : bool
        +getId() size_t
        +getName() string
        +getAvailability() bool
        +setAvailability(bool) void
        +getIssueNumber() int
        +print(out) void
    }

    Media <|.. Magazine
```
## Updated class diagram
```mermaid
classDiagram
    class Media {
        <<interface>>
        +getId() size_t
        +getName() string
        +setName(string) void
        +getAvailability() bool
        +setAvailability(bool) void
        +print(out) void
    }

    class Book {
        -id : int
        -title : string
        -author : string
        -isAvailable : bool
        +count : static size_t
        +Book()
        +Book(id:int, title:string, author:string)
        +getId() size_t
        +getName() string
        +setName(string) void
        +getAuthor() string
        +getAvailability() bool
        +setAvailability(bool) void
        +operator== (Book) bool
        +operator< (Book) bool
        +print(out) void
    }

    class Magazine {
        -id : int
        -title : string
        -issueNumber : int
        -available : bool
        +count : static size_t
        +Magazine()
        +Magazine(id:int, title:string, issueNumber:int)
        +getId() size_t
        +getName() string
        +setName(string) void
        +getAvailability() bool
        +setAvailability(bool) void
        +getIssueNumber() int
        +operator== (Magazine) bool
        +operator< (Magazine) bool
        +print(out) void
    }

    class User {
        <<Interface>>
        +getId() size_t
        +getName() string
        +setName(string) void
        +print(out) void
    }

    class Member {
        +id : int
        +name : string
        +borrowedItems : vector<int>
        +Member()
        +Member(id:int, name:string)
        +getBorrowLimit() : int
        +borrowItem(Media)
        +returnItem(Media)
        +hasBorrowed(int) bool
        +allBorrowedUnavailable(map<int,Media>) bool
        +getName() string
        +operator==(Member) bool
    }

    class Student {
        -grade : int
        +getBorrowLimit() int
    }

    class Teacher {
        -department : string
        +getBorrowLimit() int
    }

    class Library {
        -items : map<int, Media>
        -members : map<int, Member>
        +addBook(id:int, title:string, author:string)
        +addMagazine(id:int, title:string, issueNumber:int)
        +findItemById(int) Media
        +findItemByName(string) Media
        +removeItem(int)
        +addMember<T>(id:int, args...)
        +findMemberById(int) Member
        +findMemberByName(string) Member
        +removeMember(int)
        +borrowItem(int,int)
        +returnItem(int,int)
        +removeUnavailableItems()
        +upperCasified()
        +countAvailableItems() size_t
        +getSortedByTitle() vector<Media>
        +listItems()
        +listMembers()
    }

    %% Inheritance (is-a)
    Media <|.. Book
    Media <|.. Magazine
    Member <|-- Student
    Member <|-- Teacher

    User <|.. Member

    %% Aggregation (unfilled diamond)
    Library o-- Media : manages
    Library o-- Member : manages

    %% Association (straight line)
    Member -- Media : borrows
```

## State Diagram for Book

```mermaid
stateDiagram-v2
    [*] --> Available : Created

    Available --> Borrowed : borrowItem()
    Borrowed --> Returned : returnItem()
    Returned --> Available : setAvailability(true)
    Returned --> Archived : archive()

    Archived --> [*]
```

## Deployment Diagram (Library server + Members as clients)

```mermaid
graph TD
    subgraph Server
        Library[Library System]
    end

    subgraph Clients
        StudentClient[Student: Alice]
        TeacherClient[Teacher: Prof. Bob]
    end

    StudentClient <-->|Borrow/Return Requests| Library
    TeacherClient <-->|Borrow/Return Requests| Library
```

