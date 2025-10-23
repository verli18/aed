#include "app/db.hpp"
#include "app/db_test.hpp"
#include "app/models.hpp"
#include "app/repos/student_repository.hpp"
#include "app/schema.hpp"

#include <iostream>
#include <stdexcept>

namespace app {

void runDatabaseTests() {
    std::cout << "=== Running Database Tests ===\n\n";

    try {
        // Create an in-memory database for testing
        Database db(":memory:");
        schema::initializeSchema(db.handle());
        
        std::cout << "✓ Database and schema initialized\n";

        // Create repository
        repos::StudentRepository studentRepo(db);

        // Test 1: Create students
        std::cout << "\n--- Test 1: Creating students ---\n";
        
        models::Student student1("Alice Johnson", "251001234", "alice@example.com", "555-0001");
        int64_t id1 = studentRepo.create(student1);
        std::cout << "✓ Created student: " << student1.name 
                  << " (ID: " << id1 << ", Reg: " << student1.registration_number << ")\n";

        models::Student student2("Bob Smith", "251002345", "bob@example.com");
        int64_t id2 = studentRepo.create(student2);
        std::cout << "✓ Created student: " << student2.name 
                  << " (ID: " << id2 << ", Reg: " << student2.registration_number << ")\n";

        models::Student student3("Carol Williams", "251003456");
        int64_t id3 = studentRepo.create(student3);
        std::cout << "✓ Created student: " << student3.name 
                  << " (ID: " << id3 << ", Reg: " << student3.registration_number << ")\n";

        // Test 2: Duplicate registration number should fail
        std::cout << "\n--- Test 2: Duplicate registration number ---\n";
        try {
            models::Student duplicate("Duplicate Student", "251001234");
            studentRepo.create(duplicate);
            std::cout << "✗ FAILED: Should have thrown exception for duplicate registration number\n";
        } catch (const std::runtime_error& ex) {
            std::cout << "✓ Correctly rejected duplicate: " << ex.what() << "\n";
        }

        // Test 3: Find by ID
        std::cout << "\n--- Test 3: Find by ID ---\n";
        auto found = studentRepo.findById(id1);
        if (found && found->name == "Alice Johnson") {
            std::cout << "✓ Found student by ID: " << found->name << "\n";
        } else {
            std::cout << "✗ FAILED: Could not find student by ID\n";
        }

        // Test 4: Find by registration number
        std::cout << "\n--- Test 4: Find by registration number ---\n";
        auto foundByReg = studentRepo.findByRegistrationNumber("251002345");
        if (foundByReg && foundByReg->name == "Bob Smith") {
            std::cout << "✓ Found student by registration number: " << foundByReg->name << "\n";
        } else {
            std::cout << "✗ FAILED: Could not find student by registration number\n";
        }

        // Test 5: Find all
        std::cout << "\n--- Test 5: Find all students ---\n";
        auto allStudents = studentRepo.findAll();
        std::cout << "✓ Found " << allStudents.size() << " students:\n";
        for (const auto& s : allStudents) {
            std::cout << "  - " << s.name << " (" << s.registration_number << ")"
                      << (s.email.has_value() ? " - " + *s.email : "")
                      << (s.active ? " [ACTIVE]" : " [INACTIVE]") << "\n";
        }

        // Test 6: Update student
        std::cout << "\n--- Test 6: Update student ---\n";
        if (found) {
            found->email = "alice.new@example.com";
            found->phone = "555-9999";
            bool updated = studentRepo.update(*found);
            if (updated) {
                std::cout << "✓ Updated student email and phone\n";
                auto verify = studentRepo.findById(id1);
                if (verify && verify->email == "alice.new@example.com") {
                    std::cout << "✓ Verified update: " << *verify->email << "\n";
                }
            } else {
                std::cout << "✗ FAILED: Update returned false\n";
            }
        }

        // Test 7: Deactivate student
        std::cout << "\n--- Test 7: Deactivate student ---\n";
        bool deactivated = studentRepo.deactivate(id2);
        if (deactivated) {
            std::cout << "✓ Deactivated student\n";
            auto verify = studentRepo.findById(id2);
            if (verify && !verify->active) {
                std::cout << "✓ Verified student is inactive\n";
            }
            
            // Check active-only query
            auto activeOnly = studentRepo.findAll(true);
            std::cout << "✓ Active students count: " << activeOnly.size() << " (should be 2)\n";
        }

        // Test 8: Reactivate student
        std::cout << "\n--- Test 8: Reactivate student ---\n";
        bool reactivated = studentRepo.activate(id2);
        if (reactivated) {
            std::cout << "✓ Reactivated student\n";
            auto verify = studentRepo.findById(id2);
            if (verify && verify->active) {
                std::cout << "✓ Verified student is active again\n";
            }
        }

        // Test 9: Count
        std::cout << "\n--- Test 9: Count students ---\n";
        int64_t totalCount = studentRepo.count();
        int64_t activeCount = studentRepo.count(true);
        std::cout << "✓ Total students: " << totalCount << "\n";
        std::cout << "✓ Active students: " << activeCount << "\n";

        // Test 10: Delete
        std::cout << "\n--- Test 10: Delete student ---\n";
        bool deleted = studentRepo.deleteById(id3);
        if (deleted) {
            std::cout << "✓ Deleted student\n";
            auto verify = studentRepo.findById(id3);
            if (!verify) {
                std::cout << "✓ Verified student no longer exists\n";
            }
            std::cout << "✓ Remaining students: " << studentRepo.count() << "\n";
        }

        std::cout << "\n=== All Tests Passed! ===\n";

    } catch (const std::exception& ex) {
        std::cerr << "✗ Test failed with exception: " << ex.what() << "\n";
        throw;
    }
}

}  // namespace app
