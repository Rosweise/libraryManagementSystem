//  MIT License
//
//  Copyright (c) 2025 Dianna
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "./header/database.h"
#include "./header/sha256.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

DatabaseManager::DatabaseManager(std::string db_path) : db_path_(std::move(db_path)) {
    // to do noting
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

std::string DatabaseManager::generateSalt() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
        oss << std::setw(16) << dis(gen);
    }
    return oss.str(); // 64-character hex salt
}

std::string DatabaseManager::hashPassword(const std::string &password, const std::string &salt) {
    if (salt.empty()) {
        return SHA256::hash(password); // Legacy: no salt (backward compat)
    }
    return SHA256::hash(salt + ":" + password);
}

bool DatabaseManager::initialize() {
    if (sqlite3_open(db_path_.c_str(), &db_)) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const auto create_users_table = R"(
        CREATE TABLE IF NOT EXISTS Users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            password_salt TEXT NOT NULL DEFAULT '',
            name TEXT,
            college TEXT,
            className TEXT,
            role TEXT NOT NULL CHECK(role IN ('ADMIN', 'STUDENT')),
            recovery_token_hash TEXT,
            password_needs_change INTEGER NOT NULL DEFAULT 0
        );
    )";

    const auto create_books_table = R"(
        CREATE TABLE IF NOT EXISTS Books (
            isbn TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            author TEXT,
            publisher TEXT,
            category TEXT,
            totalCopies INTEGER NOT NULL,
            availableCopies INTEGER NOT NULL
        );
    )";

    const auto create_records_table = R"(
        CREATE TABLE IF NOT EXISTS BorrowingRecords (
            recordId INTEGER PRIMARY KEY AUTOINCREMENT,
            userId TEXT NOT NULL,
            bookIsbn TEXT NOT NULL,
            borrowDate TEXT NOT NULL,
            dueDate TEXT NOT NULL,
            returnDate TEXT,
            FOREIGN KEY(userId) REFERENCES Users(id),
            FOREIGN KEY(bookIsbn) REFERENCES Books(isbn)
        );
    )";

    const auto create_audit_table = R"(
        CREATE TABLE IF NOT EXISTS AuditLog (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            userId TEXT NOT NULL,
            action TEXT NOT NULL,
            detail TEXT
        );
    )";

    const auto create_login_attempts_table = R"(
        CREATE TABLE IF NOT EXISTS LoginAttempts (
            username TEXT PRIMARY KEY,
            failed_count INTEGER NOT NULL DEFAULT 0,
            locked_until TEXT
        );
    )";

    // Define indexes to improve query performance
    const auto create_users_username_index = R"(CREATE INDEX IF NOT EXISTS idx_users_username ON Users(username);)";
    const auto create_books_title_index = R"(CREATE INDEX IF NOT EXISTS idx_books_title ON Books(title);)";
    const auto create_books_author_index = R"(CREATE INDEX IF NOT EXISTS idx_books_author ON Books(author);)";
    const auto create_books_isbn_index = R"(CREATE INDEX IF NOT EXISTS idx_books_isbn ON Books(isbn);)";
    const auto create_records_user_index = R"(CREATE INDEX IF NOT EXISTS idx_borrowing_records_user ON BorrowingRecords(userId);)";
    const auto create_records_book_index = R"(CREATE INDEX IF NOT EXISTS idx_borrowing_records_book ON BorrowingRecords(bookIsbn);)";
    const auto create_users_name_index = R"(CREATE INDEX IF NOT EXISTS idx_users_name ON Users(name);)";
    const auto create_users_college_index = R"(CREATE INDEX IF NOT EXISTS idx_users_college ON Users(college);)";
    const auto create_audit_time_index = R"(CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON AuditLog(timestamp);)";

    char *err_msg = nullptr;
    if (sqlite3_exec(db_, create_users_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_books_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_records_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_audit_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_login_attempts_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error creating tables: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    // Schema migrations: add new columns to Users if they don't exist yet (for existing databases)
    sqlite3_exec(db_, "ALTER TABLE Users ADD COLUMN password_salt TEXT NOT NULL DEFAULT '';", nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "ALTER TABLE Users ADD COLUMN password_needs_change INTEGER NOT NULL DEFAULT 0;", nullptr, nullptr, nullptr);

    // Create indexes
    if (sqlite3_exec(db_, create_users_username_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_books_title_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_books_author_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_books_isbn_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_records_user_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_records_book_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_users_name_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_users_college_index, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_audit_time_index, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error creating indexes: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool DatabaseManager::addUser(const User &user, const std::string &password, bool needsPasswordChange) const {
    const std::string sql =
            "INSERT INTO Users (id, username, password_hash, password_salt, name, college, className, role, recovery_token_hash, password_needs_change) VALUES (?, ?, ?, ?, ?, ?, ?, ?, NULL, ?);";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const std::string salt = generateSalt();
    const std::string hashedPassword = hashPassword(password, salt);
    sqlite3_bind_text(stmt.get(), 1, user.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, user.username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 4, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 5, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 6, user.college.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 7, user.className.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 8, user.role.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 9, needsPasswordChange ? 1 : 0);

    bool success = (sqlite3_step(stmt.get()) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db_) << std::endl;
    }
    return success;
}

bool DatabaseManager::userExists(const std::string &username) const {
    const std::string sql = "SELECT 1 FROM Users WHERE username = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) {
        std::cerr << "Failed to prepare statement in userExists: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_STATIC);
    bool exists = (sqlite3_step(stmt.get()) == SQLITE_ROW);
    return exists;
}


User DatabaseManager::authenticateUser(const std::string &username, const std::string &password) const {
    User user;
    user.role = ""; // Default: authentication failed

    // First, fetch salt and stored hash for the user
    const std::string salt_sql =
            "SELECT id, name, college, className, role, recovery_token_hash, password_hash, password_salt, password_needs_change FROM Users WHERE username = ?;";
    
    SQLiteStatement stmt(db_, salt_sql);
    if (!stmt) {
        std::cerr << "Failed to prepare statement in authenticateUser: " << sqlite3_errmsg(db_) << std::endl;
        return user;
    }

    sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        const char *stored_hash = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 6));
        const char *salt_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 7));
        const std::string salt = salt_text ? salt_text : "";
        const std::string computedHash = hashPassword(password, salt);

        if (stored_hash && computedHash == std::string(stored_hash)) {
            user.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 0));
            user.username = username;
            const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 1));
            user.name = name_text ? name_text : "";
            const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 2));
            user.college = college_text ? college_text : "";
            const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 3));
            user.className = class_text ? class_text : "";
            user.role = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 4));
            user.hasRecoveryToken = (sqlite3_column_type(stmt.get(), 5) != SQLITE_NULL);
            user.passwordNeedsChange = (sqlite3_column_int(stmt.get(), 8) == 1);
        }
    }

    return user;
}

bool DatabaseManager::updateStudentInfo(const User &user) const {
    const std::string sql = "UPDATE Users SET name = ?, college = ?, className = ? WHERE id = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    sqlite3_bind_text(stmt.get(), 1, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, user.college.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, user.className.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 4, user.id.c_str(), -1, SQLITE_STATIC);

    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

bool DatabaseManager::updatePassword(const std::string &username, const std::string &newPassword) const {
    const std::string sql = "UPDATE Users SET password_hash = ?, password_salt = ?, password_needs_change = 0 WHERE username = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    const std::string salt = generateSalt();
    const std::string hashedPassword = hashPassword(newPassword, salt);
    sqlite3_bind_text(stmt.get(), 1, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, username.c_str(), -1, SQLITE_STATIC);

    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

bool DatabaseManager::updateRecoveryToken(const std::string &username, const std::string &token) const {
    const std::string sql = "UPDATE Users SET recovery_token_hash = ? WHERE username = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    const std::string hashedToken = SHA256::hash(token);
    sqlite3_bind_text(stmt.get(), 1, hashedToken.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, username.c_str(), -1, SQLITE_STATIC);

    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

bool DatabaseManager::recoverPassword(const std::string &username, const std::string &token,
                                      const std::string &newPassword) const {
    const std::string sql = "UPDATE Users SET password_hash = ?, password_salt = ? WHERE username = ? AND recovery_token_hash = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    const std::string newSalt = generateSalt();
    const std::string newHashedPassword = hashPassword(newPassword, newSalt);
    const std::string hashedToken = SHA256::hash(token);

    sqlite3_bind_text(stmt.get(), 1, newHashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, newSalt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 4, hashedToken.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt.get()) == SQLITE_DONE);
    const int changes = sqlite3_changes(db_);

    return success && (changes > 0);
}


bool DatabaseManager::addBook(const Book &book) const {
    const  std::string sql =
            "INSERT INTO Books (isbn, title, author, publisher, category, totalCopies, availableCopies) VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    sqlite3_bind_text(stmt.get(), 1, book.isbn.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, book.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, book.author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 4, book.publisher.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 5, book.category.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 6, book.totalCopies);
    sqlite3_bind_int(stmt.get(), 7, book.availableCopies);

    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

bool DatabaseManager::updateBook(const Book &book) const {
    const std::string sql =
            "UPDATE Books SET title = ?, author = ?, publisher = ?, category = ?, totalCopies = ?, availableCopies = ? WHERE isbn = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;

    sqlite3_bind_text(stmt.get(), 1, book.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, book.author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, book.publisher.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 4, book.category.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 5, book.totalCopies);
    sqlite3_bind_int(stmt.get(), 6, book.availableCopies);
    sqlite3_bind_text(stmt.get(), 7, book.isbn.c_str(), -1, SQLITE_STATIC);

    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

bool DatabaseManager::deleteBook(const std::string &isbn) const {
    const std::string sql = "DELETE FROM Books WHERE isbn = ?;";
    
    SQLiteStatement stmt(db_, sql);
    if (!stmt) return false;
    sqlite3_bind_text(stmt.get(), 1, isbn.c_str(), -1, SQLITE_STATIC);
    return (sqlite3_step(stmt.get()) == SQLITE_DONE);
}

std::vector<Book> DatabaseManager::findBooks(const std::string &keyword, const std::string &sortBy) const {
    std::vector<Book> books;

    // Whitelist allowed sort fields to prevent SQL injection
    std::string safeSortBy = "title"; // Default to a safe value
    if (sortBy == "title") {
        safeSortBy = "title";
    } else if (sortBy == "author") {
        safeSortBy = "author";
    } else if (sortBy == "isbn") {
        safeSortBy = "isbn";
    } else if (sortBy == "publisher") {
        safeSortBy = "publisher";
    } else if (sortBy == "category") {
        safeSortBy = "category";
    } else if (sortBy == "totalCopies") {
        safeSortBy = "totalCopies";
    } else if (sortBy == "availableCopies") {
        safeSortBy = "availableCopies";
    }

    const std::string sql = "SELECT * FROM Books WHERE title LIKE ? OR author LIKE ? OR isbn LIKE ? ORDER BY " + safeSortBy + " COLLATE NOCASE;";
    SQLiteStatement stmt(db_, sql);

    if (!stmt) {
        std::cerr << "Failed to prepare statement in findBooks: " << sqlite3_errmsg(db_) << std::endl;
        return books;
    }

    const std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt.get(), 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, like_pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        Book b;
        b.isbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 0));
        b.title = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 1));
        b.author = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 2));
        b.publisher = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 3));
        b.category = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 4));
        b.totalCopies = sqlite3_column_int(stmt.get(), 5);
        b.availableCopies = sqlite3_column_int(stmt.get(), 6);
        books.push_back(b);
    }
    return books;
}

std::vector<Book> DatabaseManager::getAllBooks(const std::string &sortBy) const {
    return findBooks("", sortBy);
}

std::vector<Book> DatabaseManager::findBooksWithPagination(const std::string &keyword, const std::string &sortBy, int offset, int limit) const {
    std::vector<Book> books;

    // Whitelist allowed sort fields to prevent SQL injection
    std::string safeSortBy = "title"; // Default to a safe value
    if (sortBy == "title") {
        safeSortBy = "title";
    } else if (sortBy == "author") {
        safeSortBy = "author";
    } else if (sortBy == "isbn") {
        safeSortBy = "isbn";
    } else if (sortBy == "publisher") {
        safeSortBy = "publisher";
    } else if (sortBy == "category") {
        safeSortBy = "category";
    } else if (sortBy == "totalCopies") {
        safeSortBy = "totalCopies";
    } else if (sortBy == "availableCopies") {
        safeSortBy = "availableCopies";
    }

    const std::string sql = "SELECT * FROM Books WHERE title LIKE ? OR author LIKE ? OR isbn LIKE ? ORDER BY " + safeSortBy + " COLLATE NOCASE LIMIT ? OFFSET ?;";
    SQLiteStatement stmt(db_, sql);

    if (!stmt) {
        std::cerr << "Failed to prepare statement in findBooksWithPagination: " << sqlite3_errmsg(db_) << std::endl;
        return books;
    }

    const std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt.get(), 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 3, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt.get(), 4, limit);
    sqlite3_bind_int(stmt.get(), 5, offset);

    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        Book b;
        b.isbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 0));
        b.title = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 1));
        b.author = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 2));
        b.publisher = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 3));
        b.category = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 4));
        b.totalCopies = sqlite3_column_int(stmt.get(), 5);
        b.availableCopies = sqlite3_column_int(stmt.get(), 6);
        books.push_back(b);
    }
    return books;
}

std::vector<Book> DatabaseManager::getAllBooksWithPagination(const std::string &sortBy, int offset, int limit) const {
    return findBooksWithPagination("", sortBy, offset, limit);
}


bool DatabaseManager::borrowBook(const std::string &userId, const std::string &isbn, int daysToBorrow) const {
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    // Check for duplicate active borrow (same user, same book, not returned)
    sqlite3_stmt *dup_stmt;
    const std::string dup_sql = "SELECT COUNT(*) FROM BorrowingRecords WHERE userId = ? AND bookIsbn = ? AND returnDate IS NULL;";
    if (sqlite3_prepare_v2(db_, dup_sql.c_str(), -1, &dup_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(dup_stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(dup_stmt, 2, isbn.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(dup_stmt) == SQLITE_ROW && sqlite3_column_int(dup_stmt, 0) > 0) {
            sqlite3_finalize(dup_stmt);
            sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false; // caller will show appropriate message
        }
        sqlite3_finalize(dup_stmt);
    } else {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // Enforce maximum borrow limit per user
    sqlite3_stmt *limit_stmt;
    const std::string limit_sql = "SELECT COUNT(*) FROM BorrowingRecords WHERE userId = ? AND returnDate IS NULL;";
    if (sqlite3_prepare_v2(db_, limit_sql.c_str(), -1, &limit_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(limit_stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(limit_stmt) == SQLITE_ROW && sqlite3_column_int(limit_stmt, 0) >= MAX_BORROW_LIMIT) {
            sqlite3_finalize(limit_stmt);
            sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false; // caller will show appropriate message
        }
        sqlite3_finalize(limit_stmt);
    } else {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    std::string check_sql = "SELECT availableCopies FROM Books WHERE isbn = ?;";
    sqlite3_stmt *check_stmt;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(check_stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) != SQLITE_ROW || sqlite3_column_int(check_stmt, 0) <= 0) {
        sqlite3_finalize(check_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        std::cerr << "Book not available or ISBN is incorrect." << std::endl;
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::string update_sql = "UPDATE Books SET availableCopies = availableCopies - 1 WHERE isbn = ?;";
    sqlite3_stmt *update_stmt;
    if (sqlite3_prepare_v2(db_, update_sql.c_str(), -1, &update_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(update_stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(update_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_stmt);

    std::string insert_sql =
            "INSERT INTO BorrowingRecords (userId, bookIsbn, borrowDate, dueDate) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *insert_stmt;
    if (sqlite3_prepare_v2(db_, insert_sql.c_str(), -1, &insert_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto borrow_time = std::chrono::system_clock::to_time_t(now);
    auto due_time = std::chrono::system_clock::to_time_t(now + std::chrono::hours(24 * daysToBorrow));

    std::stringstream borrow_ss, due_ss;
    borrow_ss << std::put_time(std::localtime(&borrow_time), "%Y-%m-%d");
    due_ss << std::put_time(std::localtime(&due_time), "%Y-%m-%d");

    sqlite3_bind_text(insert_stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, isbn.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 3, borrow_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 4, due_ss.str().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(insert_stmt) != SQLITE_DONE) {
        sqlite3_finalize(insert_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(insert_stmt);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool DatabaseManager::returnBook(int recordId, const std::string &userId) const {
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string check_sql =
            "SELECT bookIsbn FROM BorrowingRecords WHERE recordId = ? AND userId = ? AND returnDate IS NULL;";
    sqlite3_stmt *check_stmt;
    std::string bookIsbn;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_int(check_stmt, 1, recordId);
    sqlite3_bind_text(check_stmt, 2, userId.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(check_stmt, 0));
    } else {
        sqlite3_finalize(check_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        std::cerr << "Invalid record ID or you are not the borrower." << std::endl;
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::string update_record_sql = "UPDATE BorrowingRecords SET returnDate = ? WHERE recordId = ?;";
    sqlite3_stmt *update_record_stmt;
    if (sqlite3_prepare_v2(db_, update_record_sql.c_str(), -1, &update_record_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto return_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream return_ss;
    return_ss << std::put_time(std::localtime(&return_time), "%Y-%m-%d");

    sqlite3_bind_text(update_record_stmt, 1, return_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(update_record_stmt, 2, recordId);

    if (sqlite3_step(update_record_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_record_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_record_stmt);

    std::string update_book_sql = "UPDATE Books SET availableCopies = availableCopies + 1 WHERE isbn = ?;";
    sqlite3_stmt *update_book_stmt;
    if (sqlite3_prepare_v2(db_, update_book_sql.c_str(), -1, &update_book_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(update_book_stmt, 1, bookIsbn.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(update_book_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_book_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_book_stmt);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool DatabaseManager::renewBook(int recordId, const std::string &userId) const {
    std::string check_sql =
            "SELECT dueDate FROM BorrowingRecords WHERE recordId = ? AND userId = ? AND returnDate IS NULL;";
    sqlite3_stmt *check_stmt;
    std::string currentDueDateStr;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(check_stmt, 1, recordId);
    sqlite3_bind_text(check_stmt, 2, userId.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        currentDueDateStr = reinterpret_cast<const char *>(sqlite3_column_text(check_stmt, 0));
    } else {
        sqlite3_finalize(check_stmt);
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::tm tm = {};
    std::stringstream ss(currentDueDateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    auto current_due_time_t = std::mktime(&tm);
    auto new_due_time_t = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::from_time_t(current_due_time_t) + std::chrono::hours(24 * 30));

    std::stringstream new_due_ss;
    new_due_ss << std::put_time(std::localtime(&new_due_time_t), "%Y-%m-%d");

    std::string update_sql = "UPDATE BorrowingRecords SET dueDate = ? WHERE recordId = ?;";
    sqlite3_stmt *update_stmt;
    if (sqlite3_prepare_v2(db_, update_sql.c_str(), -1, &update_stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(update_stmt, 1, new_due_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(update_stmt, 2, recordId);

    bool success = (sqlite3_step(update_stmt) == SQLITE_DONE);
    sqlite3_finalize(update_stmt);
    return success;
}

std::vector<BorrowRecord> DatabaseManager::getBorrowedBooksByUser(const std::string &userId) const {
    std::vector<BorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, r.userId, r.bookIsbn, b.title, r.borrowDate, r.dueDate, r.returnDate
        FROM BorrowingRecords r JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ? AND r.returnDate IS NULL;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return records;

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.userId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        const auto return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.returnDate = return_date ? return_date : "";
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<BorrowRecord> DatabaseManager::getOverdueBooksByUser(const std::string &userId) const {
    std::vector<BorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, r.userId, r.bookIsbn, b.title, r.borrowDate, r.dueDate, r.returnDate
        FROM BorrowingRecords r JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ? AND r.returnDate IS NULL AND r.dueDate < date('now');
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return records;

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.userId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        const char *return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.returnDate = return_date ? return_date : "";
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}


std::vector<User> DatabaseManager::getAllStudents() const {
    std::vector<User> students;
    sqlite3_stmt *stmt;
    if (const auto sql = "SELECT id, username, name, college, className FROM Users WHERE role = 'STUDENT' ORDER BY id;"; sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<User> DatabaseManager::findStudents(const std::string &keyword) const {
    std::vector<User> students;
    const auto sql =
            "SELECT id, username, name, college, className FROM Users WHERE (username LIKE ? OR id LIKE ? OR name LIKE ?) AND role = 'STUDENT' ORDER BY id COLLATE NOCASE;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, like_pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<User> DatabaseManager::findStudentsWithPagination(const std::string &keyword, int offset, int limit) const {
    std::vector<User> students;
    const auto sql =
            "SELECT id, username, name, college, className FROM Users WHERE (username LIKE ? OR id LIKE ? OR name LIKE ?) AND role = 'STUDENT' ORDER BY id COLLATE NOCASE LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, limit);
    sqlite3_bind_int(stmt, 5, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<User> DatabaseManager::getAllStudentsWithPagination(int offset, int limit) const {
    std::vector<User> students;
    const auto sql = "SELECT id, username, name, college, className FROM Users WHERE role = 'STUDENT' ORDER BY id COLLATE NOCASE LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<FullBorrowRecord> DatabaseManager::getFullBorrowRecordsForUser(const std::string &userId) const {
    std::vector<FullBorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, u.id, u.name, u.college, u.className, b.title, r.borrowDate, r.dueDate,
               (CASE WHEN r.returnDate IS NULL AND date('now') > r.dueDate THEN 1 ELSE 0 END) as is_overdue
        FROM BorrowingRecords r
        JOIN Users u ON r.userId = u.id
        JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for getFullBorrowRecordsForUser: " << sqlite3_errmsg(db_) <<
                std::endl;
        return records;
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FullBorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.studentId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.studentName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.studentCollege = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.studentClass = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        rec.isOverdue = sqlite3_column_int(stmt, 8) == 1;
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<FullBorrowRecord> DatabaseManager::getAllFullBorrowRecords(const std::string &sortBy) const {
    std::vector<FullBorrowRecord> records;
    
    // Whitelist allowed sort fields to prevent SQL injection
    std::string safeSortBy = "u.id"; // Default sort
    if (sortBy == "dueDate") {
        safeSortBy = "r.dueDate";
    } else if (sortBy == "studentId") {
        safeSortBy = "u.id";
    } else if (sortBy == "studentName") {
        safeSortBy = "u.name";
    } else if (sortBy == "studentCollege") {
        safeSortBy = "u.college";
    } else if (sortBy == "bookTitle") {
        safeSortBy = "b.title";
    } else if (sortBy == "borrowDate") {
        safeSortBy = "r.borrowDate";
    } else if (sortBy == "isOverdue") {
        safeSortBy = "is_overdue";
    }

    const std::string sql = R"(
        SELECT r.recordId, u.id, u.name, u.college, u.className, b.title, r.borrowDate, r.dueDate,
               (CASE WHEN r.returnDate IS NULL AND date('now') > r.dueDate THEN 1 ELSE 0 END) as is_overdue
        FROM BorrowingRecords r
        JOIN Users u ON r.userId = u.id
        JOIN Books b ON r.bookIsbn = b.isbn
        ORDER BY )" + safeSortBy + " COLLATE NOCASE;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for getAllFullBorrowRecords: " << sqlite3_errmsg(db_) << std::endl;
        return records;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FullBorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.studentId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.studentName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.studentCollege = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.studentClass = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        rec.isOverdue = sqlite3_column_int(stmt, 8) == 1;
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

bool DatabaseManager::isBookAlreadyBorrowedByUser(const std::string &userId, const std::string &isbn) const {
    const std::string sql = "SELECT COUNT(*) FROM BorrowingRecords WHERE userId = ? AND bookIsbn = ? AND returnDate IS NULL;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, isbn.c_str(), -1, SQLITE_STATIC);
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0) > 0;
    }
    sqlite3_finalize(stmt);
    return result;
}

int DatabaseManager::getActiveBorrowCount(const std::string &userId) const {
    const std::string sql = "SELECT COUNT(*) FROM BorrowingRecords WHERE userId = ? AND returnDate IS NULL;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

bool DatabaseManager::logAction(const std::string &userId, const std::string &action, const std::string &detail) const {
    const std::string sql = "INSERT INTO AuditLog (timestamp, userId, action, detail) VALUES (datetime('now'), ?, ?, ?);"; // UTC timestamp for consistency
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, action.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, detail.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<AuditRecord> DatabaseManager::getRecentAuditLogs(int limit) const {
    std::vector<AuditRecord> records;
    const std::string sql = "SELECT id, timestamp, userId, action, detail FROM AuditLog ORDER BY id DESC LIMIT ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return records;

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AuditRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        const char *ts = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.timestamp = ts ? ts : "";
        const char *uid = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.userId = uid ? uid : "";
        const char *act = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.action = act ? act : "";
        const char *det = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.detail = det ? det : "";
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

SystemStats DatabaseManager::getSystemStats() const {
    SystemStats stats;
    sqlite3_stmt *stmt;

    // Total book titles and copy counts
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*), COALESCE(SUM(totalCopies),0), COALESCE(SUM(availableCopies),0) FROM Books;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalBookTitles = sqlite3_column_int(stmt, 0);
            stats.totalCopies = sqlite3_column_int(stmt, 1);
            stats.availableCopies = sqlite3_column_int(stmt, 2);
        }
        sqlite3_finalize(stmt);
    }

    // Total students
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM Users WHERE role = 'STUDENT';", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalStudents = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Active (unreturned) borrowings
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM BorrowingRecords WHERE returnDate IS NULL;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.activeBorrowings = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Overdue borrowings
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM BorrowingRecords WHERE returnDate IS NULL AND dueDate < date('now');", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.overdueCount = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Most borrowed book
    const auto top_sql = R"(
        SELECT b.title, COUNT(*) as cnt
        FROM BorrowingRecords r JOIN Books b ON r.bookIsbn = b.isbn
        GROUP BY r.bookIsbn ORDER BY cnt DESC LIMIT 1;
    )";
    if (sqlite3_prepare_v2(db_, top_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            stats.topBorrowedBookTitle = title ? title : "";
            stats.topBorrowedBookCount = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    return stats;
}

// Login attempt management methods (persisted to database)
int DatabaseManager::getFailedLoginCount(const std::string &username) const {
    const std::string sql = "SELECT failed_count FROM LoginAttempts WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return 0;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

void DatabaseManager::recordFailedLogin(const std::string &username) const {
    // Insert or update failed count and lockout time
    const std::string sql = R"(
        INSERT INTO LoginAttempts (username, failed_count, locked_until) 
        VALUES (?, 1, datetime('now', '+5 minutes'))
        ON CONFLICT(username) DO UPDATE SET 
            failed_count = failed_count + 1,
            locked_until = CASE 
                WHEN failed_count + 1 >= 5 THEN datetime('now', '+5 minutes')
                ELSE locked_until 
            END
    ;)";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void DatabaseManager::resetFailedLoginCount(const std::string &username) const {
    const std::string sql = "DELETE FROM LoginAttempts WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

bool DatabaseManager::isAccountLocked(const std::string &username) const {
    const std::string sql = "SELECT locked_until FROM LoginAttempts WHERE username = ? AND locked_until IS NOT NULL;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    bool locked = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *lockedUntil = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        if (lockedUntil) {
            // Compare with current time using SQLite's datetime
            sqlite3_stmt *check_stmt;
            const std::string check_sql = "SELECT ? > datetime('now');";
            if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(check_stmt, 1, lockedUntil, -1, SQLITE_STATIC);
                if (sqlite3_step(check_stmt) == SQLITE_ROW) {
                    locked = sqlite3_column_int(check_stmt, 0) != 0;
                }
                sqlite3_finalize(check_stmt);
            }
        }
    }
    sqlite3_finalize(stmt);
    return locked;
}

int DatabaseManager::getLockoutRemainingMinutes(const std::string &username) const {
    const std::string sql = R"(
        SELECT CASE 
            WHEN locked_until > datetime('now') THEN 
                CAST((julianday(locked_until) - julianday('now')) * 24 * 60 AS INTEGER)
            ELSE 0 
        END as remaining_minutes
        FROM LoginAttempts 
        WHERE username = ?;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return 0;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    int minutes = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        minutes = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return minutes > 0 ? minutes : 0;
}
