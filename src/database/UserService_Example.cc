// ============================================================================
// UserService_Example.cc - Usage Examples for UserService Class
// ============================================================================
// This file demonstrates how to use the UserService class for various
// database operations. These are example implementations that can be
// integrated into your Handler classes or other parts of the application.
// ============================================================================

#include "UserService.h"
#include "../logger/logger.hpp"
#include <iostream>

using namespace yoyo::database;

// ============================================================================
// Example 1: User Registration with Error Handling
// ============================================================================
void example_user_registration() {
    std::cout << "\n=== Example 1: User Registration ===" << std::endl;

    auto userService = UserService::getInstance();

    // Registration data
    std::string username = "john_doe";
    std::string email = "john.doe@example.com";
    std::string password = "SecurePassword123";

    // Validate inputs before registration
    if (!UserService::isValidUsername(username)) {
        std::cout << "Invalid username format" << std::endl;
        return;
    }

    if (!UserService::isValidEmail(email)) {
        std::cout << "Invalid email format" << std::endl;
        return;
    }

    if (!UserService::isValidPassword(password)) {
        std::cout << "Password does not meet security requirements" << std::endl;
        return;
    }

    // Register user
    auto result = userService->registerUser(username, email, password);

    if (result.success) {
        std::cout << "Success: " << result.message << std::endl;
    } else {
        std::cout << "Error: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 2: User Authentication
// ============================================================================
void example_user_authentication() {
    std::cout << "\n=== Example 2: User Authentication ===" << std::endl;

    auto userService = UserService::getInstance();

    // Authentication attempt
    std::string username_or_email = "john_doe";
    std::string password = "SecurePassword123";

    auto result = userService->authenticateUser(username_or_email, password);

    if (result.success) {
        std::cout << "Authentication successful!" << std::endl;

        const auto& user = result.data.value();
        std::cout << "User ID: " << user.id << std::endl;
        std::cout << "Username: " << user.username << std::endl;
        std::cout << "Email: " << user.email << std::endl;
        std::cout << "Status: " << user.status << std::endl;
        std::cout << "Last Login: " << user.last_login << std::endl;
    } else {
        std::cout << "Authentication failed: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 3: Get User Information
// ============================================================================
void example_get_user_info() {
    std::cout << "\n=== Example 3: Get User Information ===" << std::endl;

    auto userService = UserService::getInstance();

    // Method 1: Get by username
    std::cout << "\nMethod 1: Get user by username" << std::endl;
    auto result_username = userService->getUserByUsername("john_doe");

    if (result_username.success) {
        const auto& user = result_username.data.value();
        std::cout << "Found user: " << user.username << " (" << user.email << ")" << std::endl;
    } else {
        std::cout << "User not found: " << result_username.message << std::endl;
    }

    // Method 2: Get by email
    std::cout << "\nMethod 2: Get user by email" << std::endl;
    auto result_email = userService->getUserByEmail("john.doe@example.com");

    if (result_email.success) {
        const auto& user = result_email.data.value();
        std::cout << "Found user: " << user.username << " (ID: " << user.id << ")" << std::endl;
    } else {
        std::cout << "User not found: " << result_email.message << std::endl;
    }

    // Method 3: Get by ID
    std::cout << "\nMethod 3: Get user by ID" << std::endl;
    auto result_id = userService->getUserById(1000);  // ID from users table

    if (result_id.success) {
        const auto& user = result_id.data.value();
        std::cout << "Found user: " << user.username << std::endl;
        std::cout << "Created at: " << user.created_at << std::endl;
        std::cout << "Updated at: " << user.updated_at << std::endl;
    } else {
        std::cout << "User not found: " << result_id.message << std::endl;
    }
}

// ============================================================================
// Example 4: List All Users (Pagination)
// ============================================================================
void example_list_all_users() {
    std::cout << "\n=== Example 4: List All Users ===" << std::endl;

    auto userService = UserService::getInstance();

    // Get first 10 users
    auto users = userService->getAllUsers(0, 10);

    std::cout << "Total users retrieved: " << users.size() << std::endl;
    std::cout << "\nUser List:" << std::endl;
    std::cout << "ID\tUsername\tEmail\t\t\tStatus\t\tCreated At" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;

    for (const auto& user : users) {
        std::cout << user.id << "\t" << user.username << "\t"
                  << user.email << "\t" << user.status << "\t"
                  << user.created_at << std::endl;
    }

    // Get next page
    std::cout << "\nNext page (offset 10, limit 10):" << std::endl;
    auto users_page2 = userService->getAllUsers(10, 10);
    std::cout << "Users on page 2: " << users_page2.size() << std::endl;
}

// ============================================================================
// Example 5: Update User Password
// ============================================================================
void example_update_password() {
    std::cout << "\n=== Example 5: Update User Password ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1000;  // Example user ID

    // Prepare new password
    std::string old_password = "SecurePassword123";
    std::string new_password = "NewSecurePassword456";

    // Validate new password
    if (!UserService::isValidPassword(new_password)) {
        std::cout << "New password does not meet security requirements" << std::endl;
        return;
    }

    // Update password
    auto result = userService->updatePassword(user_id, old_password, new_password);

    if (result.success) {
        std::cout << "Success: " << result.message << std::endl;
    } else {
        std::cout << "Error: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 6: Update User Profile
// ============================================================================
void example_update_profile() {
    std::cout << "\n=== Example 6: Update User Profile ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1000;  // Example user ID

    // New email
    std::string new_email = "john.newemail@example.com";

    // Validate email
    if (!UserService::isValidEmail(new_email)) {
        std::cout << "Invalid email format" << std::endl;
        return;
    }

    // Update profile
    auto result = userService->updateUserProfile(user_id, new_email);

    if (result.success) {
        std::cout << "Success: " << result.message << std::endl;
    } else {
        std::cout << "Error: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 7: Update User Status
// ============================================================================
void example_update_status() {
    std::cout << "\n=== Example 7: Update User Status ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1000;

    // Change status to inactive
    auto result_inactive = userService->updateUserStatus(user_id, "inactive");

    if (result_inactive.success) {
        std::cout << "User marked as inactive" << std::endl;
    }

    // Change status back to active
    auto result_active = userService->updateUserStatus(user_id, "active");

    if (result_active.success) {
        std::cout << "User marked as active" << std::endl;
    }

    // Ban a user
    auto result_ban = userService->updateUserStatus(user_id, "banned");

    if (result_ban.success) {
        std::cout << "User has been banned" << std::endl;
    }
}

// ============================================================================
// Example 8: Check User Existence
// ============================================================================
void example_check_existence() {
    std::cout << "\n=== Example 8: Check User Existence ===" << std::endl;

    auto userService = UserService::getInstance();

    // Check username
    std::string username = "john_doe";
    if (userService->usernameExists(username)) {
        std::cout << "Username '" << username << "' exists" << std::endl;
    } else {
        std::cout << "Username '" << username << "' does not exist" << std::endl;
    }

    // Check email
    std::string email = "john.doe@example.com";
    if (userService->emailExists(email)) {
        std::cout << "Email '" << email << "' exists" << std::endl;
    } else {
        std::cout << "Email '" << email << "' does not exist" << std::endl;
    }
}

// ============================================================================
// Example 9: Delete User (Soft Delete)
// ============================================================================
void example_delete_user_soft() {
    std::cout << "\n=== Example 9: Soft Delete User ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1000;

    // Soft delete (marks as inactive)
    auto result = userService->deleteUser(user_id);

    if (result.success) {
        std::cout << "User marked as inactive (soft delete)" << std::endl;
    } else {
        std::cout << "Error: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 10: Permanent Delete User (Hard Delete)
// ============================================================================
void example_delete_user_permanent() {
    std::cout << "\n=== Example 10: Permanent Delete User ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1001;  // Different user ID

    // WARNING: This is permanent and cannot be undone
    auto result = userService->permanentlyDeleteUser(user_id);

    if (result.success) {
        std::cout << "User permanently deleted from database" << std::endl;
    } else {
        std::cout << "Error: " << result.message << std::endl;
    }
}

// ============================================================================
// Example 11: Handle Login Attempts and Account Lockout
// ============================================================================
void example_login_attempts() {
    std::cout << "\n=== Example 11: Handle Login Attempts ===" << std::endl;

    auto userService = UserService::getInstance();
    unsigned int user_id = 1000;

    // Simulate failed login attempts
    for (int i = 0; i < 3; i++) {
        std::cout << "Failed login attempt " << (i + 1) << std::endl;
        auto result = userService->incrementLoginAttempts(user_id);
    }

    // Check user status - should be near lockout
    auto user_result = userService->getUserById(user_id);
    if (user_result.success) {
        std::cout << "Current login attempts: " << user_result.data.value().login_attempts << std::endl;
    }

    // After successful login, reset attempts
    std::cout << "\nAfter successful authentication:" << std::endl;
    auto reset_result = userService->resetLoginAttempts(user_id);

    if (reset_result.success) {
        std::cout << "Login attempts reset to 0" << std::endl;
    }
}

// ============================================================================
// Example 12: Input Validation Examples
// ============================================================================
void example_input_validation() {
    std::cout << "\n=== Example 12: Input Validation ===" << std::endl;

    // Username validation
    std::cout << "\nUsername Validation:" << std::endl;
    std::vector<std::string> usernames = {"john_doe", "j", "john-doe", "john@doe"};

    for (const auto& username : usernames) {
        bool valid = UserService::isValidUsername(username);
        std::cout << "'" << username << "': " << (valid ? "VALID" : "INVALID") << std::endl;
    }

    // Email validation
    std::cout << "\nEmail Validation:" << std::endl;
    std::vector<std::string> emails = {
        "john@example.com",
        "invalid.email",
        "test@domain",
        "user+tag@example.co.uk"
    };

    for (const auto& email : emails) {
        bool valid = UserService::isValidEmail(email);
        std::cout << "'" << email << "': " << (valid ? "VALID" : "INVALID") << std::endl;
    }

    // Password validation
    std::cout << "\nPassword Validation:" << std::endl;
    std::vector<std::string> passwords = {
        "SecurePass123",      // Valid
        "weak",               // Too short
        "nouppercase123",     // No uppercase
        "NOLOWERCASE123",     // No lowercase
        "NoDigits"            // No digits
    };

    for (const auto& password : passwords) {
        bool valid = UserService::isValidPassword(password);
        std::cout << "'" << password << "': " << (valid ? "VALID" : "INVALID") << std::endl;
    }
}

// ============================================================================
// Example 13: Error Handling Pattern
// ============================================================================
void example_error_handling() {
    std::cout << "\n=== Example 13: Error Handling Pattern ===" << std::endl;

    auto userService = UserService::getInstance();

    // Try various operations and handle errors gracefully
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"invalid_username", "test@example.com"},     // Username too short
        {"test.user", "invalid-email"},                // Invalid email
        {"valid_user", "valid@example.com"}            // Valid
    };

    for (const auto& [username, email] : test_cases) {
        std::cout << "\nAttempting registration with:" << std::endl;
        std::cout << "  Username: " << username << std::endl;
        std::cout << "  Email: " << email << std::endl;

        auto result = userService->registerUser(
            username, email, "TempPassword123"
        );

        if (result.success) {
            std::cout << "  Status: SUCCESS" << std::endl;
        } else {
            std::cout << "  Status: FAILED" << std::endl;
            std::cout << "  Reason: " << result.message << std::endl;
        }
    }
}

// ============================================================================
// Main Function - Run All Examples
// ============================================================================
/*
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "UserService Examples" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // Run all examples
        example_user_registration();
        example_user_authentication();
        example_get_user_info();
        example_list_all_users();
        example_update_password();
        example_update_profile();
        example_update_status();
        example_check_existence();
        example_delete_user_soft();
        // example_delete_user_permanent();  // Uncomment with caution
        example_login_attempts();
        example_input_validation();
        example_error_handling();

    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "All examples completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
*/
