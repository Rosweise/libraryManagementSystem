#include "./header/logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <ctime>

// Get current time as string
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// Convert level to string
static std::string levelToString(Logger::Level level) {
    switch (level) {
        case Logger::Level::DEBUG: return "DEBUG";
        case Logger::Level::INFO: return "INFO";
        case Logger::Level::WARN: return "WARN";
        case Logger::Level::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& db_path) {
    // Try to set up SQLite logging first
    if (!db_path.empty()) {
        setupSQLiteLogging(db_path);
    }
    
    // If SQLite setup failed or wasn't provided, fall back to file logging
    if (!use_sqlite_) {
        setupFileLogging();
    }
}

void Logger::setupSQLiteLogging(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &sqlite_db_) != SQLITE_OK) {
        std::cerr << "Error opening SQLite database for logging: " << sqlite3_errmsg(sqlite_db_) << std::endl;
        sqlite3_close(sqlite_db_);
        sqlite_db_ = nullptr;
        use_sqlite_ = false;
        return;
    }

    // Create logs table if it doesn't exist
    const char* create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS Logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            level TEXT NOT NULL,
            message TEXT NOT NULL,
            user TEXT,
            action TEXT
        );
        
        CREATE INDEX IF NOT EXISTS idx_logs_timestamp ON Logs(timestamp);
        CREATE INDEX IF NOT EXISTS idx_logs_user ON Logs(user);
        CREATE INDEX IF NOT EXISTS idx_logs_level ON Logs(level);
        CREATE INDEX IF NOT EXISTS idx_logs_action ON Logs(action);
    )";

    char* err_msg = nullptr;
    if (sqlite3_exec(sqlite_db_, create_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "Error creating logs table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(sqlite_db_);
        sqlite_db_ = nullptr;
        use_sqlite_ = false;
        return;
    }

    use_sqlite_ = true;
}

void Logger::setupFileLogging() {
    // Just set up the file path, we'll write to it when logging
    use_sqlite_ = false;
}

void Logger::log(Level level, const std::string& message, const std::string& user, const std::string& action) {
    if (use_sqlite_) {
        logToSQLite(level, message, user, action);
    } else {
        logToFile(level, message, user, action);
    }
}

void Logger::logToSQLite(Level level, const std::string& message, const std::string& user, const std::string& action) {
    if (!sqlite_db_) return;

    const char* sql = "INSERT INTO Logs (timestamp, level, message, user, action) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for logging: " << sqlite3_errmsg(sqlite_db_) << std::endl;
        return;
    }

    std::string timestamp = getCurrentTimestamp();
    std::string level_str = levelToString(level);

    sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, level_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, message.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, action.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to insert log entry: " << sqlite3_errmsg(sqlite_db_) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void Logger::logToFile(Level level, const std::string& message, const std::string& user, const std::string& action) {
    std::ofstream logfile(log_file_path_, std::ios_base::app);
    if (!logfile.is_open()) {
        std::cerr << "Could not open log file: " << log_file_path_ << std::endl;
        return;
    }

    std::string timestamp = getCurrentTimestamp();
    std::string level_str = levelToString(level);
    
    logfile << "[" << timestamp << "] [" << level_str << "] ";
    if (!user.empty()) {
        logfile << "[User: " << user << "] ";
    }
    if (!action.empty()) {
        logfile << "[Action: " << action << "] ";
    }
    logfile << message << std::endl;
    
    logfile.close();
}

void Logger::debug(const std::string& message, const std::string& user, const std::string& action) {
    log(Level::DEBUG, message, user, action);
}

void Logger::info(const std::string& message, const std::string& user, const std::string& action) {
    log(Level::INFO, message, user, action);
}

void Logger::warn(const std::string& message, const std::string& user, const std::string& action) {
    log(Level::WARN, message, user, action);
}

void Logger::error(const std::string& message, const std::string& user, const std::string& action) {
    log(Level::ERROR, message, user, action);
}

std::vector<Logger::LogEntry> Logger::queryLogsByTime(const std::string& start_time, const std::string& end_time) {
    if (!use_sqlite_ || !sqlite_db_) {
        std::cerr << "Cannot query logs: SQLite not initialized" << std::endl;
        return {};
    }

    std::string sql = "SELECT id, timestamp, level, message, user, action FROM Logs WHERE timestamp BETWEEN ? AND ? ORDER BY timestamp DESC;";
    return executeQuery(sql, {start_time, end_time});
}

std::vector<Logger::LogEntry> Logger::queryLogsByUser(const std::string& username) {
    if (!use_sqlite_ || !sqlite_db_) {
        std::cerr << "Cannot query logs: SQLite not initialized" << std::endl;
        return {};
    }

    std::string sql = "SELECT id, timestamp, level, message, user, action FROM Logs WHERE user = ? ORDER BY timestamp DESC;";
    return executeQuery(sql, {username});
}

std::vector<Logger::LogEntry> Logger::queryLogsByLevel(Level level) {
    if (!use_sqlite_ || !sqlite_db_) {
        std::cerr << "Cannot query logs: SQLite not initialized" << std::endl;
        return {};
    }

    std::string level_str = levelToString(level);
    std::string sql = "SELECT id, timestamp, level, message, user, action FROM Logs WHERE level = ? ORDER BY timestamp DESC;";
    return executeQuery(sql, {level_str});
}

std::vector<Logger::LogEntry> Logger::queryLogsByAction(const std::string& action) {
    if (!use_sqlite_ || !sqlite_db_) {
        std::cerr << "Cannot query logs: SQLite not initialized" << std::endl;
        return {};
    }

    std::string sql = "SELECT id, timestamp, level, message, user, action FROM Logs WHERE action = ? ORDER BY timestamp DESC;";
    return executeQuery(sql, {action});
}

std::vector<Logger::LogEntry> Logger::executeQuery(const std::string& sql, const std::vector<std::string>& params) {
    std::vector<LogEntry> results;
    
    if (!sqlite_db_) return results;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(sqlite_db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for query: " << sqlite3_errmsg(sqlite_db_) << std::endl;
        return results;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry entry;
        entry.id = sqlite3_column_int(stmt, 0);
        entry.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        std::string level_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (level_str == "DEBUG") entry.level = Level::DEBUG;
        else if (level_str == "INFO") entry.level = Level::INFO;
        else if (level_str == "WARN") entry.level = Level::WARN;
        else if (level_str == "ERROR") entry.level = Level::ERROR;
        else entry.level = Level::INFO; // default
        
        entry.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* user_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        entry.user = user_ptr ? user_ptr : "";
        const char* action_ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        entry.action = action_ptr ? action_ptr : "";
        
        results.push_back(entry);
    }

    sqlite3_finalize(stmt);
    return results;
}

Logger::~Logger() {
    if (sqlite_db_) {
        sqlite3_close(sqlite_db_);
        sqlite_db_ = nullptr;
    }
}