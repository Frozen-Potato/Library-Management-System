```cpp
#include "Library.h"
#include "Student.h"
#include "Teacher.h"
#include "Ultis.h"
#include "MongoLogger.h"

#include <mongocxx/instance.hpp>
#include <iostream>
#include <thread>
#include <cstdlib>

void borrowTask(Library& lib, int memberId, int itemId) {
    try {
        lib.borrowItem(itemId, memberId);
        std::cout << "[Thread] Member " << memberId << " borrowed item " << itemId << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[Thread Error] Member " << memberId << ": " << e.what() << "\n";
    }
}

int main() {
    try {
        mongocxx::instance instance{};
        EnvLoader env;
        MongoLogger logger(env.getMongoUrl());
        logger.logEvent("INFO", "Library system started");

        std::cout << "Initializing Library System...\n";
        Library lib; // Auto-connects to PostgreSQL via PostgresAdapter
        
        // --- Add Media ---
        std::cout << "\n[Setup] Adding Books and Magazines...\n";
        lib.addBook(1, "The C++ Programming Language", "Bjarne Stroustrup");
        lib.addBook(2, "Effective Modern C++", "Scott Meyers");
        lib.addBook(3, "Clean Code", "Robert C. Martin");
        lib.addMagazine(10, "C++ Weekly", 42);
        lib.addMagazine(11, "Software Design", 7);

        // --- Add Users ---
        std::cout << "\n[Setup] Adding Members...\n";
        lib.addUser<Student>(100, 100, "Alice");
        lib.addUser<Teacher>(200, 200, "Prof. Bob");

        std::cout << "\n--- Items in Library ---\n";
        lib.listItems();

        std::cout << "\n--- Members ---\n";
        lib.listUsers();

        // --- Borrowing ---
        std::cout << "\n--- Borrowing Items ---\n";
        lib.borrowItem(1, 100);   // Alice borrows book 1
        lib.borrowItem(2, 200);   // Bob borrows book 2
        lib.borrowItem(10, 200);  // Bob borrows magazine 10

        // --- Threaded Borrow Simulation ---
        std::cout << "\n--- Concurrent Borrow Simulation ---\n";
        std::vector<std::thread> threads;
        threads.emplace_back(borrowTask, std::ref(lib), 100, 5);
        threads.emplace_back(borrowTask, std::ref(lib), 200, 5); // Competing for same book
        threads.emplace_back(borrowTask, std::ref(lib), 100, 2);
        for (auto& t : threads) t.join();

        // --- Return Items ---
        std::cout << "\n--- Returning Items ---\n";
        lib.returnItem(1, 100);   // Alice returns book 1
        lib.returnItem(2, 200);   // Bob returns book 2

        // --- Reports ---
        std::cout << "\n--- Final Items ---\n";
        lib.listItems();

        std::cout << "\nAvailable count: " << lib.countAvailableItems() << "\n";
        std::cout << "\nSorted by Title:\n";
        auto sorted = lib.getSortedByTitle();
        Ultis::printContainer(sorted);

        std::cout << "\nSystem completed successfully.\n";
    } catch (const std::exception& ex) {
        std::cerr << "[Fatal] " << ex.what() << "\n";
    }

    return 0;
}
```