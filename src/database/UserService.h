// ============================================================================
// UserService.h - User Database Service Layer
// ============================================================================
// Purpose: Provides CRUD operations and user authentication methods
//          for the user management system using SqlPool connection pool.
// ============================================================================
// Key Features:
//  - Thread-safe singleton pattern
//  - Connection pooling for database access
//  - SQL injection prevention using mysql_real_escape_string
//  - Password hashing using SHA256
//  - Comprehensive CRUD operations
//  - User authentication and validation
// ============================================================================

#ifndef _USERSERVICE_H_
#define _USERSERVICE_H_

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include <mysql/mysql.h>

// Forward declarations
class Connection;
class ConnectionPool;

namespace yoyo {
namespace database {

// ============================================================================
// User Data Structure
// ============================================================================
/// Represents a user entity with all relevant information
struct User {
    unsigned int id;                // User ID (auto-incremented primary key)
    std::string username;           // Username (3-50 chars, alphanumeric + underscore)
    std::string email;              // Email address (max 100 chars, valid format)
    std::string password_hash;      // Password hash (SHA256, never plain text)
    std::string status;             // Account status: active, inactive, banned
    int login_attempts;             // Failed login attempts counter (0-255)
    std::string created_at;         // Creation timestamp (MySQL TIMESTAMP)
    std::string updated_at;         // Last update timestamp (MySQL TIMESTAMP)
    std::string last_login;         // Last successful login timestamp (nullable)

    User() : id(0), login_attempts(0) {}
};

// ============================================================================
// User Service Result Type
// ============================================================================
/// Encapsulates the result of a service operation with optional user data
struct ServiceResult {
    bool success;                   // Operation success flag
    std::string message;            // Error or success message
    std::optional<User> data;       // Optional user data returned by query

    ServiceResult(bool s, const std::string& msg)
        : success(s), message(msg), data(std::nullopt) {}

    ServiceResult(bool s, const std::string& msg, const User& u)
        : success(s), message(msg), data(u) {}
};

// ============================================================================
// UserService Class
// ============================================================================
class UserService {
public:
    // Singleton pattern - get the only instance
    static UserService* getInstance();

    // Destructor
    ~UserService();

    // ========================================================================
    // User Query Operations
    // ========================================================================

    /// Get user by ID
    /// @param id User ID
    /// @return ServiceResult with user data if found
    ServiceResult getUserById(unsigned int id);

    /// Get user by username
    /// @param username Username to search for
    /// @return ServiceResult with user data if found
    ServiceResult getUserByUsername(const std::string& username);

    /// Get user by email
    /// @param email Email to search for
    /// @return ServiceResult with user data if found
    ServiceResult getUserByEmail(const std::string& email);

    /// Get all users (limited pagination support)
    /// @param offset Number of records to skip
    /// @param limit Maximum number of records to return
    /// @return Vector of users
    std::vector<User> getAllUsers(int offset = 0, int limit = 100);

    // ========================================================================
    // User Creation and Authentication
    // ========================================================================

    /// Register a new user
    /// @param username Username (must be unique)
    /// @param email Email address (must be unique, valid format)
    /// @param password Plain password (will be hashed)
    /// @return ServiceResult indicating success or failure
    ServiceResult registerUser(const std::string& username,
                               const std::string& email,
                               const std::string& password);

    /// Authenticate user with credentials
    /// @param username Username or email
    /// @param password Plain password to verify
    /// @return ServiceResult with user data if authentication successful
    ServiceResult authenticateUser(const std::string& username,
                                   const std::string& password);

    // ========================================================================
    // User Update Operations
    // ========================================================================

    /// Update user profile information
    /// @param id User ID
    /// @param email New email (optional)
    /// @return ServiceResult indicating success or failure
    ServiceResult updateUserProfile(unsigned int id,
                                    const std::string& email);

    /// Update user password
    /// @param id User ID
    /// @param currentPassword Current password for verification
    /// @param newPassword New plain password (will be hashed)
    /// @return ServiceResult indicating success or failure
    ServiceResult updatePassword(unsigned int id,
                                 const std::string& currentPassword,
                                 const std::string& newPassword);

    /// Update user status
    /// @param id User ID
    /// @param status New status (active, inactive, banned)
    /// @return ServiceResult indicating success or failure
    ServiceResult updateUserStatus(unsigned int id,
                                   const std::string& status);

    /// Update last login timestamp
    /// @param id User ID
    /// @return ServiceResult indicating success or failure
    ServiceResult updateLastLogin(unsigned int id);

    // ========================================================================
    // User Deletion Operations
    // ========================================================================

    /// Delete a user account (soft delete - mark as inactive)
    /// @param id User ID
    /// @return ServiceResult indicating success or failure
    ServiceResult deleteUser(unsigned int id);

    /// Permanently remove a user from database (hard delete)
    /// @param id User ID
    /// @return ServiceResult indicating success or failure
    ServiceResult permanentlyDeleteUser(unsigned int id);

    // ========================================================================
    // Password and Security Operations
    // ========================================================================

    /// Hash a plain password using bcrypt or SHA256
    /// @param password Plain password
    /// @return Hashed password string
    static std::string hashPassword(const std::string& password);

    /// Verify a plain password against a hash
    /// @param password Plain password
    /// @param hash Stored password hash
    /// @return True if password matches hash
    static bool verifyPassword(const std::string& password,
                               const std::string& hash);

    /// Check if username format is valid
    /// @param username Username to validate
    /// @return True if valid
    static bool isValidUsername(const std::string& username);

    /// Check if email format is valid
    /// @param email Email to validate
    /// @return True if valid
    static bool isValidEmail(const std::string& email);

    /// Check if password meets security requirements
    /// @param password Password to validate
    /// @return True if meets requirements (min 8 chars, mix of types, etc.)
    static bool isValidPassword(const std::string& password);

    // ========================================================================
    // Utility Methods
    // ========================================================================

    /// Reset login attempts counter
    /// @param id User ID
    /// @return ServiceResult indicating success or failure
    ServiceResult resetLoginAttempts(unsigned int id);

    /// Increment login attempts counter
    /// @param id User ID
    /// @return ServiceResult indicating success or failure
    ServiceResult incrementLoginAttempts(unsigned int id);

    /// Check if username exists
    /// @param username Username to check
    /// @return True if username exists
    bool usernameExists(const std::string& username);

    /// Check if email exists
    /// @param email Email to check
    /// @return True if email exists
    bool emailExists(const std::string& email);

    // ========================================================================
    // Game Statistics Operations
    // ========================================================================

    /// Update game statistics after game completion
    /// @param userId User ID
    /// @param gameType Game type (fishing, tetris, fighter)
    /// @param score Score achieved in this game
    /// @return ServiceResult indicating success or failure
    ServiceResult updateGameStats(unsigned int userId,
                                 const std::string& gameType,
                                 int score);

    /// Get all game statistics for a user
    /// @param userId User ID
    /// @return ServiceResult with JSON array of game stats
    ServiceResult getGameStats(unsigned int userId);

    /// Get statistics for a specific game
    /// @param userId User ID
    /// @param gameType Game type
    /// @return ServiceResult with game statistics
    ServiceResult getGameStatsForGame(unsigned int userId,
                                     const std::string& gameType);

private:
    // Private constructor for singleton
    UserService();

    // Copy constructor and assignment operator (deleted)
    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;

    // Singleton instance
    static UserService* m_instance;
    static std::mutex m_mutex;

    // Helper methods
    std::shared_ptr<Connection> getConnection();
    User parseUserFromResult(MYSQL_ROW row);
    void logError(const std::string& operation, const std::string& error);
};

}  // namespace database
}  // namespace yoyo

#endif  // _USERSERVICE_H_
