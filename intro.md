# Library System – Overview

This is where most of the C++ feature practice will take place. As the project grows, some parts will be deleted and others added. A part of this project is done without a concrete plan, but in the later stages there is a more detailed plan located in [location](../../Additional%20stuffs/). The diagram for the project can be found in [location](./Diagram.md).
This project models a simple **Library Management System** in C++ using UML class diagrams.  
It demonstrates object-oriented principles such as **inheritance, polymorphism, aggregation, and association**.

## Purpose
The system allows a library to:
- Manage **media items** (books, magazines).
- Register and track **members** (students, teachers).
- Support operations like borrowing, returning, listing, and searching items.

## Main Components

### Media (Interface)
- Defines the common behavior of all library items.  
- Provides methods for **ID, name, availability, and printing details**.  

### Book & Magazine
- Concrete implementations of `Media`.  
- Each has unique attributes (`author` for books, `issueNumber` for magazines).  
- Support comparison operators for sorting and searching.  

### Member
- Represents a library user.  
- Tracks borrowed items and enforces borrowing rules.  
- Specialized by:
  - **Student** – limited borrowing capacity.  
  - **Teacher** – higher borrowing capacity.  

### Library
- Central manager of the system.  
- Stores all items (`map<int, Media>`) and members (`map<int, Member>`).  
- Provides core services:
  - Adding/removing items and members.  
  - Borrowing and returning items.  
  - Searching by ID or name.  
  - Counting and listing items.  

## Relationships
- **Inheritance**:  
  - `Book`, `Magazine` implement `Media`.  
  - `Student`, `Teacher` inherit from `Member`.  
- **Aggregation**:  
  - `Library` manages many `Media` and `Member` objects.  
- **Association**:  
  - `Member` borrows `Media`.  

## Notes
- Designed with **extensibility** in mind (easy to add new media types or member roles).  
- Demonstrates **C++ concepts** such as interfaces, polymorphism, operator overloading, and templates.  
