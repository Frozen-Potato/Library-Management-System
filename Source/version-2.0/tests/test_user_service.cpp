#include <gtest/gtest.h>
#include "../src/application/services/UserService.h"
#include "../src/data/PostgresAdapter.h"
#include <pqxx/pqxx>
#include <memory>

const std::string TEST_DB_URL = "postgresql://postgres:password@postgres:5432/librarydb";

class UserServiceIntegrationTest : public ::testing::Test {
protected:
    std::shared_ptr<pqxx::connection> conn;
    std::shared_ptr<PostgresAdapter> db;
    std::unique_ptr<UserService> service;

    void SetUp() override {
        conn = std::make_shared<pqxx::connection>(TEST_DB_URL);
        db = std::make_shared<PostgresAdapter>(conn);
        service = std::make_unique<UserService>(db);

        pqxx::work txn(*conn);
        txn.exec("DELETE FROM students WHERE id IN (SELECT id FROM users WHERE email LIKE 'test_%');");
        txn.exec("DELETE FROM teachers WHERE id IN (SELECT id FROM users WHERE email LIKE 'test_%');");
        txn.exec("DELETE FROM members WHERE id IN (SELECT id FROM users WHERE email LIKE 'test_%');");
        txn.exec("DELETE FROM users WHERE email LIKE 'test_%';");
        txn.commit();
    }
};

// MEMBER
TEST_F(UserServiceIntegrationTest, CreateAndFetchMember) {
    int id = service->createUser("test_member", "test_member@example.com",
                                 "pass123", "MEMBER", std::nullopt, std::nullopt);
    ASSERT_GT(id, 0);
    auto userOpt = db->getUserByName("test_member");
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->role, "MEMBER");
}

// STUDENT
TEST_F(UserServiceIntegrationTest, CreateAndFetchStudent) {
    int id = service->createUser("test_student", "test_student@example.com",
                                 "pass123", "STUDENT", "Grade 10", std::nullopt);
    ASSERT_GT(id, 0);
    auto userOpt = db->getUserByName("test_student");
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->role, "STUDENT");
}

// TEACHER
TEST_F(UserServiceIntegrationTest, CreateAndFetchTeacher) {
    int id = service->createUser("test_teacher", "test_teacher@example.com",
                                 "pass123", "TEACHER", std::nullopt, "Mathematics");
    ASSERT_GT(id, 0);
    auto userOpt = db->getUserByName("test_teacher");
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->role, "TEACHER");
}
