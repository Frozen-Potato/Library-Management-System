#include <gtest/gtest.h>
#include "../src/application/services/AuthService.h"
#include "../src/application/services/UserService.h"
#include "../src/infrastructure/jwt/JwtHelper.h"
#include "../src/data/PostgresAdapter.h"
#include <pqxx/pqxx>
#include <memory>

const std::string TEST_DB_URL = "postgresql://postgres:password@postgres:5432/librarydb";

class AuthServiceIntegrationTest : public ::testing::Test {
protected:
    std::shared_ptr<pqxx::connection> conn;
    std::shared_ptr<PostgresAdapter> db;
    std::shared_ptr<JwtHelper> jwtHelper;
    std::unique_ptr<UserService> userService;
    std::unique_ptr<AuthService> authService;

    void SetUp() override {
        conn = std::make_shared<pqxx::connection>(TEST_DB_URL);
        db = std::make_shared<PostgresAdapter>(conn);
        jwtHelper = std::make_shared<JwtHelper>("super_secret_jwt_key", "library-system", 6000);
        userService = std::make_unique<UserService>(db);
        authService = std::make_unique<AuthService>(db, jwtHelper);

        pqxx::work txn(*conn);
        txn.exec("DELETE FROM students WHERE id IN (SELECT id FROM users WHERE email LIKE 'auth_test_%');");
        txn.exec("DELETE FROM teachers WHERE id IN (SELECT id FROM users WHERE email LIKE 'auth_test_%');");
        txn.exec("DELETE FROM members WHERE id IN (SELECT id FROM users WHERE email LIKE 'auth_test_%');");
        txn.exec("DELETE FROM users WHERE email LIKE 'auth_test_%';");
        txn.commit();
    }
};

// Test login & token verification
TEST_F(AuthServiceIntegrationTest, LoginAndVerifyToken) {
    int userId = userService->createUser("auth_test_user",
                                         "auth_test_user@example.com",
                                         "password123",
                                         "MEMBER",
                                         std::nullopt,
                                         std::nullopt);
    ASSERT_GT(userId, 0);

    auto tokenOpt = authService->login("auth_test_user", "password123");
    ASSERT_TRUE(tokenOpt.has_value());
    std::string token = tokenOpt.value();
    ASSERT_FALSE(token.empty());

    bool valid = authService->verifyToken(token);
    ASSERT_TRUE(valid);
}

