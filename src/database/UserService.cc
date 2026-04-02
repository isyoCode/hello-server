// ============================================================================
// UserService.cc - User Database Service Layer Implementation
// ============================================================================
// This file implements user CRUD operations, authentication, and password
// management using the ConnectionPool for database access.
//
// Implementation Details:
//  - Uses SqlPool (connectionPool.h) for thread-safe connection management
//  - Retrieves connections via
//  ConnectionPool::getConnectionPool()->getConnection()
//  - Each connection is a shared_ptr<Connection> for automatic resource cleanup
//  - SQL queries use mysql_real_escape_string for SQL injection prevention
//  - Query results (MYSQL_RES*) are properly freed with mysql_free_result()
//  - Database operations include proper error handling and logging
// ============================================================================

#include "../utils/logger.hpp"
#include "UserService.h"

// SqlPool connection pool includes
#include <commonConnection.h>
#include <connectionPool.h>
#include <mysql/mysql.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include <cctype>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

namespace yoyo {
namespace database {

// ============================================================================
// Singleton Instance Initialization
// ============================================================================
UserService* UserService::m_instance = nullptr;
std::mutex UserService::m_mutex;

// ============================================================================
// Constructor & Destructor
// ============================================================================
UserService::UserService() { LOG("UserService initialized"); }

UserService::~UserService() { LOG("UserService destroyed"); }

// ============================================================================
// Singleton Pattern Implementation
// ============================================================================
UserService* UserService::getInstance() {
  if (m_instance == nullptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_instance == nullptr) {
      m_instance = new UserService();
    }
  }
  return m_instance;
}

// ============================================================================
// Helper Methods for Connection Pool Management
// ============================================================================

/// Get a connection from the connection pool
/// @return shared_ptr<Connection> pointing to a pooled connection, or nullptr
/// on failure
std::shared_ptr<Connection> UserService::getConnection() {
  ConnectionPool* pool = ConnectionPool::getConnectionPool();
  if (!pool) {
    LOG("ERROR: [getConnection] Failed to get ConnectionPool instance");
    return nullptr;
  }
  auto conn = pool->getConnection();
  if (!conn) {
    LOG("ERROR: [getConnection] Failed to obtain connection from pool (pool "
        "may be exhausted)");
    return nullptr;
  }
  return conn;
}

/// Log errors with operation context
/// @param operation The database operation that failed
/// @param error The error message or reason
void UserService::logError(const std::string& operation,
                           const std::string& error) {
  std::ostringstream oss;
  oss << "UserService ERROR [" << operation << "]: " << error;
  LOG(oss.str());
}

/// Parse user data from a MySQL query result row
/// Converts a MYSQL_ROW into a User struct with proper null checking and type
/// conversion Row format: (id, username, email, password_hash, status,
/// login_attempts, created_at, updated_at, last_login)
/// @param row The MySQL result row from a SELECT query
/// @return User struct populated with data from the row
User UserService::parseUserFromResult(MYSQL_ROW row) {
  User user;
  if (!row) {
    return user;  // Return empty user with default values
  }

  try {
    // Parse each field with null safety checks
    user.id = row[0] ? std::stoul(row[0]) : 0;
    user.username = row[1] ? row[1] : "";
    user.email = row[2] ? row[2] : "";
    user.password_hash = row[3] ? row[3] : "";
    user.status = row[4] ? row[4] : "active";
    user.login_attempts = row[5] ? std::stoi(row[5]) : 0;
    user.created_at = row[6] ? row[6] : "";
    user.updated_at = row[7] ? row[7] : "";
    user.last_login = row[8] ? row[8] : "";
  } catch (const std::exception& e) {
    logError("parseUserFromResult", std::string(e.what()));
    return User();  // Return empty user on parse error
  }
  return user;
}

// ============================================================================
// Password Hashing and Security
// ============================================================================

/// Hash a plain password using SHA256
/// Note: For production systems, consider using bcrypt or Argon2 instead
/// SHA256 is acceptable for this implementation but less secure than bcrypt
/// Uses OpenSSL EVP API (OpenSSL 3.0+ recommended method)
/// @param password The plain text password to hash
/// @return Hexadecimal string representation of the SHA256 hash
std::string UserService::hashPassword(const std::string& password) {
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len = 0;

  EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
  if (!mdctx) {
    LOG("ERROR: [hashPassword] Failed to create EVP_MD_CTX");
    return "";
  }

  // Initialize and perform hashing using EVP API (OpenSSL 3.0+ recommended)
  if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
    LOG("ERROR: [hashPassword] EVP_DigestInit_ex failed");
    EVP_MD_CTX_free(mdctx);
    return "";
  }

  if (EVP_DigestUpdate(mdctx, password.c_str(), password.length()) != 1) {
    LOG("ERROR: [hashPassword] EVP_DigestUpdate failed");
    EVP_MD_CTX_free(mdctx);
    return "";
  }

  if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
    LOG("ERROR: [hashPassword] EVP_DigestFinal_ex failed");
    EVP_MD_CTX_free(mdctx);
    return "";
  }

  EVP_MD_CTX_free(mdctx);

  // Convert binary hash to hexadecimal string representation
  std::ostringstream oss;
  for (unsigned int i = 0; i < hash_len; i++) {
    oss << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(hash[i]);
  }
  return oss.str();
}

/// Verify a plain password against a stored hash
/// Uses the same hashing algorithm (SHA256) to compare passwords
/// @param password The plain text password to verify
/// @param hash The stored password hash to compare against
/// @return True if password matches the hash, false otherwise
bool UserService::verifyPassword(const std::string& password,
                                 const std::string& hash) {
  return hashPassword(password) == hash;
}

// ============================================================================
// Input Validation Methods
// ============================================================================

/// Validate username format and constraints
/// Requirements: 3-50 characters, alphanumeric and underscore only
/// @param username The username to validate
/// @return True if username is valid, false otherwise
bool UserService::isValidUsername(const std::string& username) {
  // Check length constraints (3-50 characters)
  if (username.length() < 3 || username.length() > 50) {
    return false;
  }

  // Check character set: alphanumeric and underscore only
  // Pattern: ^[a-zA-Z0-9_]+$
  std::regex username_regex("^[a-zA-Z0-9_]+$");
  return std::regex_match(username, username_regex);
}

/// Validate email format
/// Requirements: Valid email format, max 100 characters
/// Uses regex pattern for basic email validation
/// @param email The email address to validate
/// @return True if email is valid, false otherwise
bool UserService::isValidEmail(const std::string& email) {
  // Check maximum length
  if (email.length() > 100) {
    return false;
  }

  // Basic email validation regex
  // Pattern: ^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$
  std::regex email_regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
  return std::regex_match(email, email_regex);
}

/// Validate password security requirements
/// Requirements:
///  - Minimum 8 characters, maximum 255 characters
///  - At least one uppercase letter (A-Z)
///  - At least one lowercase letter (a-z)
///  - At least one digit (0-9)
/// @param password The password to validate
/// @return True if password meets security requirements, false otherwise
bool UserService::isValidPassword(const std::string& password) {
  // Check length constraints (8-255 characters)
  if (password.length() < 8 || password.length() > 255) {
    return false;
  }

  // Check for required character types
  bool hasUpper = false;  // At least one uppercase letter
  bool hasLower = false;  // At least one lowercase letter
  bool hasDigit = false;  // At least one digit

  for (char c : password) {
    if (std::isupper(c)) hasUpper = true;
    if (std::islower(c)) hasLower = true;
    if (std::isdigit(c)) hasDigit = true;
  }

  // All three requirements must be met
  return hasUpper && hasLower && hasDigit;
}

// ============================================================================
// User Query Operations (Read)
// ============================================================================

/// Get a user by their ID
/// Retrieves user information from database excluding banned users
/// @param id The user ID to search for
/// @return ServiceResult containing user data if found, error message otherwise
ServiceResult UserService::getUserById(unsigned int id) {
  auto conn = getConnection();
  if (!conn) {
    logError("getUserById", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Build SQL query - numeric ID doesn't need escaping
  std::string sql =
      "SELECT id, username, email, password_hash, status, "
      "login_attempts, created_at, updated_at, last_login "
      "FROM users WHERE id = " +
      std::to_string(id) + " AND status != 'banned'";

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("getUserById",
             "Query execution failed for ID: " + std::to_string(id));
    return ServiceResult(false, "Database query failed");
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return ServiceResult(false, "User not found");
  }

  User user = parseUserFromResult(row);
  mysql_free_result(result);  // Always free result to prevent memory leaks

  return ServiceResult(true, "User found successfully", user);
}

/// Get a user by their username
/// Retrieves user information by username, excluding banned users
/// Uses mysql_real_escape_string for SQL injection prevention
/// @param username The username to search for
/// @return ServiceResult containing user data if found, error message otherwise
ServiceResult UserService::getUserByUsername(const std::string& username) {
  auto conn = getConnection();
  if (!conn) {
    logError("getUserByUsername",
             "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Escape the username string to prevent SQL injection
  // mysql_real_escape_string requires a buffer 2x the input length + 1
  char escaped_username[username.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_username,
                           username.c_str(), username.length());

  std::string sql =
      std::string("SELECT id, username, email, password_hash, ") +
      "status, login_attempts, created_at, updated_at, last_login " +
      "FROM users WHERE username = '" + escaped_username +
      "' AND status != 'banned'";

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("getUserByUsername",
             "Query execution failed for username: " + username);
    return ServiceResult(false, "Database query failed");
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return ServiceResult(false, "User not found");
  }

  User user = parseUserFromResult(row);
  mysql_free_result(result);  // Always free result to prevent memory leaks

  return ServiceResult(true, "User found successfully", user);
}

/// Get a user by their email address
/// Retrieves user information by email, excluding banned users
/// Uses mysql_real_escape_string for SQL injection prevention
/// @param email The email address to search for
/// @return ServiceResult containing user data if found, error message otherwise
ServiceResult UserService::getUserByEmail(const std::string& email) {
  auto conn = getConnection();
  if (!conn) {
    logError("getUserByEmail", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Escape the email string to prevent SQL injection
  // mysql_real_escape_string requires a buffer 2x the input length + 1
  char escaped_email[email.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_email, email.c_str(),
                           email.length());

  std::string sql =
      std::string("SELECT id, username, email, password_hash, ") +
      "status, login_attempts, created_at, updated_at, last_login " +
      "FROM users WHERE email = '" + escaped_email + "' AND status != 'banned'";

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("getUserByEmail", "Query execution failed for email: " + email);
    return ServiceResult(false, "Database query failed");
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return ServiceResult(false, "User not found");
  }

  User user = parseUserFromResult(row);
  mysql_free_result(result);  // Always free result to prevent memory leaks

  return ServiceResult(true, "User found successfully", user);
}

/// Get all active users with pagination support
/// Retrieves multiple users excluding banned accounts
/// Supports pagination via offset and limit
/// @param offset Number of records to skip (default 0)
/// @param limit Maximum number of records to return (default 100)
/// @return Vector of User objects, empty vector if query fails
std::vector<User> UserService::getAllUsers(int offset, int limit) {
  std::vector<User> users;

  auto conn = getConnection();
  if (!conn) {
    logError("getAllUsers", "Failed to get database connection from pool");
    return users;  // Return empty vector on connection failure
  }

  // Clamp limit to reasonable values (1-1000)
  if (limit < 1) limit = 1;
  if (limit > 1000) limit = 1000;
  if (offset < 0) offset = 0;

  std::string sql =
      "SELECT id, username, email, password_hash, status, "
      "login_attempts, created_at, updated_at, last_login "
      "FROM users WHERE status != 'banned' " +
      std::string("ORDER BY id ASC LIMIT ") + std::to_string(limit) +
      " OFFSET " + std::to_string(offset);

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("getAllUsers",
             "Query execution failed (offset=" + std::to_string(offset) +
                 ", limit=" + std::to_string(limit) + ")");
    return users;  // Return empty vector on query failure
  }

  // Fetch all rows and parse them into User objects
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != nullptr) {
    User user = parseUserFromResult(row);
    if (user.id > 0) {  // Only add valid users with non-zero ID
      users.push_back(user);
    }
  }

  mysql_free_result(result);  // Always free result to prevent memory leaks
  return users;
}

// ============================================================================
// User Registration and Authentication (Create & Verify)
// ============================================================================

/// Register a new user account
/// Validates all input, checks for duplicates, hashes password, and inserts
/// user
/// @param username Username (3-50 alphanumeric + underscore)
/// @param email Email address (valid format, max 100 chars)
/// @param password Plain password (will be hashed with SHA256)
/// @return ServiceResult indicating success or failure with message
ServiceResult UserService::registerUser(const std::string& username,
                                        const std::string& email,
                                        const std::string& password) {
  // Step 1: Validate input format
  if (!isValidUsername(username)) {
    return ServiceResult(
        false, "Invalid username format (3-50 chars, alphanumeric only)");
  }
  if (!isValidEmail(email)) {
    return ServiceResult(false, "Invalid email format");
  }
  if (!isValidPassword(password)) {
    return ServiceResult(false,
                         "Password too weak (min 8 chars, needs uppercase, "
                         "lowercase, digit)");
  }

  // Step 2: Get database connection
  auto conn = getConnection();
  if (!conn) {
    logError("registerUser", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Step 3: Check for duplicate username
  if (usernameExists(username)) {
    logError("registerUser", "Username already exists: " + username);
    return ServiceResult(false, "Username already exists");
  }

  // Step 4: Check for duplicate email
  if (emailExists(email)) {
    logError("registerUser", "Email already exists: " + email);
    return ServiceResult(false, "Email already exists");
  }

  // Step 5: Hash the password
  std::string password_hash = hashPassword(password);

  // Step 6: Escape all string inputs to prevent SQL injection
  // mysql_real_escape_string requires a buffer 2x the input length + 1
  char escaped_username[username.length() * 2 + 1];
  char escaped_email[email.length() * 2 + 1];
  char escaped_hash[password_hash.length() * 2 + 1];

  mysql_real_escape_string(conn->getRawConn(), escaped_username,
                           username.c_str(), username.length());
  mysql_real_escape_string(conn->getRawConn(), escaped_email, email.c_str(),
                           email.length());
  mysql_real_escape_string(conn->getRawConn(), escaped_hash,
                           password_hash.c_str(), password_hash.length());

  // Step 7: Build and execute INSERT query
  std::string sql = std::string("INSERT INTO users (username, email, ") +
                    "password_hash, status) VALUES ('" + escaped_username +
                    "', '" + escaped_email + "', '" + escaped_hash +
                    "', 'active')";

  if (!conn->update(sql)) {
    logError("registerUser", "Failed to insert new user: " + username);
    return ServiceResult(false, "Failed to register user");
  }

  LOG("User registered successfully: " + username);
  return ServiceResult(true, "User registered successfully");
}

/// Authenticate a user with username/email and password
/// Attempts to find user by username first, then by email
/// Verifies password, checks account status, and manages login attempts
/// On success, resets failed login attempts and updates last login timestamp
/// @param username Username or email address
/// @param password Plain password to verify
/// @return ServiceResult with user data if authentication succeeds
ServiceResult UserService::authenticateUser(const std::string& username,
                                            const std::string& password) {
  // Step 1: Try to get user by username first
  ServiceResult result = getUserByUsername(username);

  // Step 2: If not found by username, try by email
  if (!result.success) {
    result = getUserByEmail(username);
  }

  // Step 3: If user not found in either lookup
  if (!result.success) {
    logError("authenticateUser", "User not found: " + username);
    return ServiceResult(false, "Invalid username or password");
  }

  User& user = result.data.value();

  // Step 4: Check if account is active
  if (user.status != "active") {
    logError("authenticateUser", "Account is " + user.status + ": " + username);
    return ServiceResult(false, "Account is " + user.status);
  }

  // Step 5: Check if account is locked due to failed login attempts (brute
  // force protection)
  const int MAX_LOGIN_ATTEMPTS = 5;
  if (user.login_attempts >= MAX_LOGIN_ATTEMPTS) {
    logError("authenticateUser",
             "Account locked (too many failed attempts): " + username);
    return ServiceResult(false,
                         "Account locked due to too many failed login "
                         "attempts. Please contact support.");
  }

  // Step 6: Verify password hash
  if (!verifyPassword(password, user.password_hash)) {
    // Password is incorrect - increment failed attempt counter
    incrementLoginAttempts(user.id);
    logError("authenticateUser", "Password verification failed: " + username);
    return ServiceResult(false, "Invalid username or password");
  }

  // Step 7: Password verified - reset login attempts and update last login
  resetLoginAttempts(user.id);
  updateLastLogin(user.id);

  LOG("User authenticated successfully: " + username);
  return ServiceResult(true, "Authentication successful", user);
}

// ============================================================================
// User Update Operations (Update)
// ============================================================================

/// Update user email address
/// Validates email format and checks for duplicates before updating
/// @param id User ID to update
/// @param email New email address
/// @return ServiceResult indicating success or failure
ServiceResult UserService::updateUserProfile(unsigned int id,
                                             const std::string& email) {
  // Step 1: Validate email format
  if (!isValidEmail(email)) {
    return ServiceResult(false, "Invalid email format");
  }

  // Step 2: Get database connection
  auto conn = getConnection();
  if (!conn) {
    logError("updateUserProfile",
             "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Step 3: Check if email already exists for another user
  char escaped_email[email.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_email, email.c_str(),
                           email.length());

  std::string check_sql = "SELECT id FROM users WHERE email = '" +
                          std::string(escaped_email) +
                          "' AND id != " + std::to_string(id) + " LIMIT 1";

  MYSQL_RES* check_result = conn->query(check_sql);
  if (!check_result) {
    logError("updateUserProfile", "Duplicate check query failed");
    return ServiceResult(false, "Database query failed");
  }

  if (mysql_fetch_row(check_result) != nullptr) {
    mysql_free_result(check_result);
    logError("updateUserProfile", "Email already in use: " + email);
    return ServiceResult(false, "Email already exists");
  }
  mysql_free_result(check_result);

  // Step 4: Update the email
  std::string sql = "UPDATE users SET email = '" + std::string(escaped_email) +
                    "' WHERE id = " + std::to_string(id);

  if (!conn->update(sql)) {
    logError("updateUserProfile",
             "Failed to update email for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to update profile");
  }

  LOG("User profile updated: ID " + std::to_string(id));
  return ServiceResult(true, "Profile updated successfully");
}

/// Update user password
/// Verifies current password, validates new password, and updates hash
/// @param id User ID to update
/// @param currentPassword Current password for verification
/// @param newPassword New password (must meet security requirements)
/// @return ServiceResult indicating success or failure
ServiceResult UserService::updatePassword(unsigned int id,
                                          const std::string& currentPassword,
                                          const std::string& newPassword) {
  // Step 1: Validate new password strength
  if (!isValidPassword(newPassword)) {
    return ServiceResult(false,
                         "New password too weak (min 8 chars, needs "
                         "uppercase, lowercase, digit)");
  }

  // Step 2: Get database connection
  auto conn = getConnection();
  if (!conn) {
    logError("updatePassword", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Step 3: Retrieve current password hash from database
  std::string query_sql =
      "SELECT password_hash FROM users WHERE id = " + std::to_string(id);

  MYSQL_RES* result = conn->query(query_sql);
  if (!result) {
    logError("updatePassword",
             "Query failed for user ID: " + std::to_string(id));
    return ServiceResult(false, "Database query failed");
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    logError("updatePassword", "User not found: ID " + std::to_string(id));
    return ServiceResult(false, "User not found");
  }

  std::string stored_hash = row[0] ? row[0] : "";
  mysql_free_result(result);

  // Step 4: Verify current password matches
  if (!verifyPassword(currentPassword, stored_hash)) {
    logError("updatePassword", "Current password verification failed for ID: " +
                                   std::to_string(id));
    return ServiceResult(false, "Current password is incorrect");
  }

  // Step 5: Hash the new password
  std::string new_hash = hashPassword(newPassword);

  // Step 6: Escape the new hash for SQL
  char escaped_hash[new_hash.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_hash, new_hash.c_str(),
                           new_hash.length());

  // Step 7: Update password in database
  std::string update_sql = "UPDATE users SET password_hash = '" +
                           std::string(escaped_hash) +
                           "' WHERE id = " + std::to_string(id);

  if (!conn->update(update_sql)) {
    logError("updatePassword",
             "Failed to update password for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to update password");
  }

  LOG("User password updated: ID " + std::to_string(id));
  return ServiceResult(true, "Password updated successfully");
}

/// Update user account status
/// Validates status value and updates in database
/// Valid statuses: active, inactive, banned
/// @param id User ID to update
/// @param status New status value
/// @return ServiceResult indicating success or failure
ServiceResult UserService::updateUserStatus(unsigned int id,
                                            const std::string& status) {
  // Step 1: Validate status value against allowed values
  if (status != "active" && status != "inactive" && status != "banned") {
    return ServiceResult(
        false, "Invalid status value. Allowed: active, inactive, banned");
  }

  // Step 2: Get database connection
  auto conn = getConnection();
  if (!conn) {
    logError("updateUserStatus", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Step 3: Update status in database
  std::string sql = "UPDATE users SET status = '" + status +
                    "' WHERE id = " + std::to_string(id);

  if (!conn->update(sql)) {
    logError("updateUserStatus",
             "Failed to update status for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to update status");
  }

  LOG("User status updated: ID " + std::to_string(id) + " -> " + status);
  return ServiceResult(true, "Status updated successfully");
}

/// Update user's last login timestamp
/// Records the current timestamp as the user's last successful login
/// Called automatically after successful authentication
/// @param id User ID to update
/// @return ServiceResult indicating success or failure
ServiceResult UserService::updateLastLogin(unsigned int id) {
  auto conn = getConnection();
  if (!conn) {
    logError("updateLastLogin", "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Use MySQL CURRENT_TIMESTAMP function for server-side timestamp
  std::string sql =
      "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = " +
      std::to_string(id);

  if (!conn->update(sql)) {
    logError("updateLastLogin",
             "Failed to update last login for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to update last login");
  }

  return ServiceResult(true, "Last login updated successfully");
}

// ============================================================================
// User Deletion Operations (Delete)
// ============================================================================

/// Soft delete a user account
/// Marks user as inactive rather than removing from database
/// Preserves user history and data while preventing login
/// @param id User ID to delete
/// @return ServiceResult indicating success or failure
ServiceResult UserService::deleteUser(unsigned int id) {
  // Soft delete - mark account as inactive
  // This preserves data while preventing login
  return updateUserStatus(id, "inactive");
}

/// Permanently delete a user account
/// Completely removes user from database (hard delete)
/// WARNING: This operation cannot be undone
/// @param id User ID to permanently remove
/// @return ServiceResult indicating success or failure
ServiceResult UserService::permanentlyDeleteUser(unsigned int id) {
  auto conn = getConnection();
  if (!conn) {
    logError("permanentlyDeleteUser",
             "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  // Execute DELETE query - removes user completely
  std::string sql = "DELETE FROM users WHERE id = " + std::to_string(id);

  if (!conn->update(sql)) {
    logError("permanentlyDeleteUser",
             "Failed to delete user ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to delete user");
  }

  LOG("User permanently deleted: ID " + std::to_string(id));
  return ServiceResult(true, "User deleted successfully");
}

// ============================================================================
// Utility Methods for Login Management and Existence Checks
// ============================================================================

/// Reset login attempt counter
/// Called after successful login to clear failed attempt count
/// Part of brute force protection mechanism
/// @param id User ID to reset
/// @return ServiceResult indicating success or failure
ServiceResult UserService::resetLoginAttempts(unsigned int id) {
  auto conn = getConnection();
  if (!conn) {
    logError("resetLoginAttempts",
             "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  std::string sql =
      "UPDATE users SET login_attempts = 0 WHERE id = " + std::to_string(id);

  if (!conn->update(sql)) {
    logError("resetLoginAttempts",
             "Failed to reset login attempts for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to reset login attempts");
  }

  return ServiceResult(true, "Login attempts reset");
}

/// Increment login attempt counter
/// Called after failed login attempt
/// When attempts reach threshold (5), account is locked
/// Part of brute force protection mechanism
/// @param id User ID to increment
/// @return ServiceResult indicating success or failure
ServiceResult UserService::incrementLoginAttempts(unsigned int id) {
  auto conn = getConnection();
  if (!conn) {
    logError("incrementLoginAttempts",
             "Failed to get database connection from pool");
    return ServiceResult(false, "Failed to get database connection");
  }

  std::string sql =
      "UPDATE users SET login_attempts = login_attempts + 1 "
      "WHERE id = " +
      std::to_string(id);

  if (!conn->update(sql)) {
    logError(
        "incrementLoginAttempts",
        "Failed to increment login attempts for ID: " + std::to_string(id));
    return ServiceResult(false, "Failed to increment login attempts");
  }

  return ServiceResult(true, "Login attempts incremented");
}

/// Check if username already exists in database
/// Used for duplicate checking during registration and profile updates
/// Uses mysql_real_escape_string for SQL injection prevention
/// @param username The username to check
/// @return True if username exists, false otherwise
bool UserService::usernameExists(const std::string& username) {
  auto conn = getConnection();
  if (!conn) {
    logError("usernameExists", "Failed to get database connection from pool");
    return false;  // Assume doesn't exist on connection error
  }

  // Escape username to prevent SQL injection
  char escaped_username[username.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_username,
                           username.c_str(), username.length());

  std::string sql = "SELECT id FROM users WHERE username = '" +
                    std::string(escaped_username) + "' LIMIT 1";

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("usernameExists", "Query failed for username: " + username);
    return false;  // Assume doesn't exist on query error
  }

  // Check if any row was returned
  bool exists = mysql_fetch_row(result) != nullptr;
  mysql_free_result(result);  // Always free result
  return exists;
}

/// Check if email already exists in database
/// Used for duplicate checking during registration and profile updates
/// Uses mysql_real_escape_string for SQL injection prevention
/// @param email The email address to check
/// @return True if email exists, false otherwise
bool UserService::emailExists(const std::string& email) {
  auto conn = getConnection();
  if (!conn) {
    logError("emailExists", "Failed to get database connection from pool");
    return false;  // Assume doesn't exist on connection error
  }

  // Escape email to prevent SQL injection
  char escaped_email[email.length() * 2 + 1];
  mysql_real_escape_string(conn->getRawConn(), escaped_email, email.c_str(),
                           email.length());

  std::string sql = "SELECT id FROM users WHERE email = '" +
                    std::string(escaped_email) + "' LIMIT 1";

  MYSQL_RES* result = conn->query(sql);
  if (!result) {
    logError("emailExists", "Query failed for email: " + email);
    return false;  // Assume doesn't exist on query error
  }

  // Check if any row was returned
  bool exists = mysql_fetch_row(result) != nullptr;
  mysql_free_result(result);  // Always free result
  return exists;
}

// ============================================================================
// Game Statistics Operations
// ============================================================================

ServiceResult UserService::updateGameStats(unsigned int userId,
                                           const std::string& gameType,
                                           int score) {
  auto conn = getConnection();
  if (!conn) {
    return ServiceResult(false, "数据库连接失败");
  }

  try {
    // 1. 验证分数合理性
    if (score < 0) {
      return ServiceResult(false, "分数不能为负数");
    }

    if (gameType == "fishing" && score > 5000) {
      return ServiceResult(false, "钓鱼游戏分数异常");
    } else if (gameType == "tetris" && score > 1000000) {
      return ServiceResult(false, "俄罗斯方块分数异常");
    } else if (gameType == "fighter" && score > 50000) {
      return ServiceResult(false, "格斗游戏分数异常");
    }

    // 2. 获取当前统计
    std::string selectSql =
        "SELECT highest_score, times_played, total_score FROM game_statistics "
        "WHERE user_id = " +
        std::to_string(userId) +
        " "
        "AND game_type = '" +
        gameType + "'";

    MYSQL_RES* result = conn->query(selectSql);

    int highestScore = 0, timesPlayed = 0, totalScore = 0;

    if (result && mysql_num_rows(result) > 0) {
      MYSQL_ROW row = mysql_fetch_row(result);
      if (row[0]) highestScore = std::stoi(row[0]);
      if (row[1]) timesPlayed = std::stoi(row[1]);
      if (row[2]) totalScore = std::stoi(row[2]);
      mysql_free_result(result);
    } else {
      if (result) mysql_free_result(result);
      return ServiceResult(false, "游戏统计记录不存在");
    }

    // 3. 更新数据
    if (score > highestScore) {
      highestScore = score;
    }
    timesPlayed++;
    totalScore += score;
    float averageScore = (float)totalScore / timesPlayed;

    // 4. 构建 UPDATE SQL
    std::string updateSql =
        "UPDATE game_statistics SET "
        "highest_score = " +
        std::to_string(highestScore) +
        ", "
        "times_played = " +
        std::to_string(timesPlayed) +
        ", "
        "total_score = " +
        std::to_string(totalScore) +
        ", "
        "average_score = " +
        std::to_string(averageScore) +
        ", "
        "last_played = NOW() "
        "WHERE user_id = " +
        std::to_string(userId) +
        " "
        "AND game_type = '" +
        gameType + "'";

    if ((int)(conn->update(updateSql)) < 0) {
      return ServiceResult(false, "更新游戏统计失败");
    }

    LOGI("Game stats updated: user_id=" + std::to_string(userId) +
         " game_type=" + gameType + " score=" + std::to_string(score));

    return ServiceResult(true, "游戏统计已更新");

  } catch (const std::exception& e) {
    logError("updateGameStats", e.what());
    return ServiceResult(false, std::string("错误: ") + e.what());
  }
}

ServiceResult UserService::getGameStats(unsigned int userId) {
  auto conn = getConnection();
  if (!conn) {
    return ServiceResult(false, "数据库连接失败");
  }

  try {
    std::string sql =
        "SELECT game_type, highest_score, times_played, total_score, "
        "average_score, last_played "
        "FROM game_statistics WHERE user_id = " +
        std::to_string(userId) +
        " "
        "ORDER BY game_type";

    MYSQL_RES* result = conn->query(sql);

    if (!result) {
      return ServiceResult(false, "查询游戏统计失败");
    }

    // 构建 JSON 返回
    std::string jsonResult = "[";
    bool first = true;

    while (MYSQL_ROW row = mysql_fetch_row(result)) {
      if (!first) jsonResult += ",";

      std::string lastPlayed = row[5] ? std::string(row[5]) : "null";

      jsonResult += "{\"gameType\":\"" + std::string(row[0]) +
                    "\","
                    "\"highestScore\":" +
                    std::string(row[1]) +
                    ","
                    "\"timesPlayed\":" +
                    std::string(row[2]) +
                    ","
                    "\"totalScore\":" +
                    std::string(row[3]) +
                    ","
                    "\"averageScore\":" +
                    std::string(row[4]) + "}";
      first = false;
    }

    jsonResult += "]";
    mysql_free_result(result);

    LOGI("Retrieved game stats for user_id=" + std::to_string(userId));
    return ServiceResult(true, jsonResult);

  } catch (const std::exception& e) {
    logError("getGameStats", e.what());
    return ServiceResult(false, std::string("错误: ") + e.what());
  }
}

ServiceResult UserService::getGameStatsForGame(unsigned int userId,
                                               const std::string& gameType) {
  auto conn = getConnection();
  if (!conn) {
    return ServiceResult(false, "数据库连接失败");
  }

  try {
    std::string sql =
        "SELECT highest_score, times_played, total_score, average_score, "
        "last_played "
        "FROM game_statistics WHERE user_id = " +
        std::to_string(userId) +
        " "
        "AND game_type = '" +
        gameType + "'";

    MYSQL_RES* result = conn->query(sql);

    if (!result || mysql_num_rows(result) == 0) {
      if (result) mysql_free_result(result);
      return ServiceResult(false, "未找到该游戏的统计记录");
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    std::string jsonResult = "{\"gameType\":\"" + gameType +
                             "\","
                             "\"highestScore\":" +
                             std::string(row[0]) +
                             ","
                             "\"timesPlayed\":" +
                             std::string(row[1]) +
                             ","
                             "\"totalScore\":" +
                             std::string(row[2]) +
                             ","
                             "\"averageScore\":" +
                             std::string(row[3]) + "}";

    mysql_free_result(result);

    return ServiceResult(true, jsonResult);

  } catch (const std::exception& e) {
    logError("getGameStatsForGame", e.what());
    return ServiceResult(false, std::string("错误: ") + e.what());
  }
}

}  // namespace database
}  // namespace yoyo
