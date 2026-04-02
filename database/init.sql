-- ============================================================================
-- Database Initialization Script for User Management System
-- ============================================================================
-- Description: This script creates the database schema for the user
--              management system with proper indexing and constraints.
-- ============================================================================

-- Create database if it doesn't exist
CREATE DATABASE IF NOT EXISTS hello_app CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- Use the database
USE hello_app;

-- ============================================================================
-- Users Table
-- ============================================================================
-- Purpose: Store user account information with authentication credentials
-- ============================================================================
CREATE TABLE IF NOT EXISTS users (
    -- Primary key: auto-incrementing user ID
    id INT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT 'User ID (Primary Key)',

    -- Username: unique, required, max 50 characters
    username VARCHAR(50) NOT NULL UNIQUE COMMENT 'Username (Unique)',

    -- Email: unique, required, valid email format
    email VARCHAR(100) NOT NULL UNIQUE COMMENT 'Email address (Unique)',

    -- Password hash: bcrypt/SHA256 hashed password, never plain text
    password_hash VARCHAR(255) NOT NULL COMMENT 'Password hash (bcrypt/SHA256)',

    -- Account creation timestamp
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Account created timestamp',

    -- Account last updated timestamp
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Last update timestamp',

    -- Account status: active or inactive
    status ENUM('active', 'inactive', 'banned') DEFAULT 'active' COMMENT 'User account status',

    -- Login attempt counter for security
    login_attempts INT UNSIGNED DEFAULT 0 COMMENT 'Failed login attempts counter',

    -- Last login timestamp
    last_login TIMESTAMP NULL COMMENT 'Last successful login timestamp'
) ENGINE=InnoDB AUTO_INCREMENT=1000 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='User accounts table';

-- ============================================================================
-- Indexes
-- ============================================================================
-- Username index for efficient lookups by username
CREATE UNIQUE INDEX idx_username ON users(username);

-- Email index for efficient lookups by email
CREATE UNIQUE INDEX idx_email ON users(email);

-- Status index for filtering active users
CREATE INDEX idx_status ON users(status);

-- Created_at index for chronological queries
CREATE INDEX idx_created_at ON users(created_at);

-- Last_login index for user activity tracking
CREATE INDEX idx_last_login ON users(last_login);

-- ============================================================================
-- Sample Data (Optional - Remove in production)
-- ============================================================================
-- These are example accounts with hashed passwords (placeholder hashes)
-- In production, use proper password hashing (bcrypt, Argon2, etc.)
INSERT INTO users (username, email, password_hash, status) VALUES
('admin', 'admin@example.com', '$2b$12$placeholder_bcrypt_hash_admin123456789', 'active'),
('testuser', 'testuser@example.com', '$2b$12$placeholder_bcrypt_hash_test1234567890', 'active')
ON DUPLICATE KEY UPDATE username=username;

-- ============================================================================
-- Game Statistics Table
-- ============================================================================
-- Purpose: Store game statistics for each user and game combination
-- ============================================================================
CREATE TABLE IF NOT EXISTS game_statistics (
    -- Primary key: auto-incrementing ID
    id INT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT 'Statistic ID (Primary Key)',

    -- Foreign key: references users.id
    user_id INT UNSIGNED NOT NULL COMMENT 'User ID (Foreign Key)',

    -- Game type: fishing, tetris, fighter, etc.
    game_type VARCHAR(50) NOT NULL COMMENT 'Game type',

    -- Highest score achieved
    highest_score INT DEFAULT 0 COMMENT 'Highest score achieved',

    -- Total number of times played
    times_played INT DEFAULT 0 COMMENT 'Total times played',

    -- Total score across all games
    total_score INT DEFAULT 0 COMMENT 'Total score',

    -- Average score per game
    average_score FLOAT DEFAULT 0 COMMENT 'Average score per game',

    -- Last time this game was played
    last_played TIMESTAMP NULL COMMENT 'Last played timestamp',

    -- Record creation timestamp
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Record created timestamp',

    -- Record update timestamp
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Last updated timestamp',

    -- Foreign key constraint
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

    -- Unique constraint: one record per user-game pair
    UNIQUE KEY unique_user_game (user_id, game_type),

    -- Indexes for efficient queries
    INDEX idx_user_id (user_id),
    INDEX idx_game_type (game_type),
    INDEX idx_last_played (last_played)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Game statistics table';

-- Initialize statistics for existing users
INSERT INTO game_statistics (user_id, game_type, highest_score, times_played, total_score)
SELECT id, 'fishing', 0, 0, 0 FROM users
ON DUPLICATE KEY UPDATE user_id=user_id;

INSERT INTO game_statistics (user_id, game_type, highest_score, times_played, total_score)
SELECT id, 'tetris', 0, 0, 0 FROM users
ON DUPLICATE KEY UPDATE user_id=user_id;

INSERT INTO game_statistics (user_id, game_type, highest_score, times_played, total_score)
SELECT id, 'fighter', 0, 0, 0 FROM users
ON DUPLICATE KEY UPDATE user_id=user_id;

-- ============================================================================
-- End of Database Initialization Script
-- ============================================================================
