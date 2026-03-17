#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>
#include <vector>
#include "sqlite3.h"

// Forward declaration
struct LogEntry;

class Logger {
public:
    enum class Level {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };

    struct LogEntry {
        int id;
        std::string timestamp;
        Level level;
        std::string message;
        std::string user;  // Username of the user who performed the action
        std::string action; // Action that was logged
    };

    static Logger& getInstance();
    
    void initialize(const std::string& db_path = "");
    void log(Level level, const std::string& message, const std::string& user = "", const std::string& action = "");
    
    // Convenience methods
    void debug(const std::string& message, const std::string& user = "", const std::string& action = "");
    void info(const std::string& message, const std::string& user = "", const std::string& action = "");
    void warn(const std::string& message, const std::string& user = "", const std::string& action = "");
    void error(const std::string& message, const std::string& user = "", const std::string& action = "");

    // Query methods for administrators
    std::vector<LogEntry> queryLogsByTime(const std::string& start_time, const std::string& end_time);
    std::vector<LogEntry> queryLogsByUser(const std::string& username);
    std::vector<LogEntry> queryLogsByLevel(Level level);
    std::vector<LogEntry> queryLogsByAction(const std::string& action);

private:
    Logger() = default;
    ~Logger();
    
    void setupSQLiteLogging(const std::string& db_path);
    void setupFileLogging();
    void logToSQLite(Level level, const std::string& message, const std::string& user, const std::string& action);
    void logToFile(Level level, const std::string& message, const std::string& user, const std::string& action);
    
    sqlite3* sqlite_db_ = nullptr;
    bool use_sqlite_ = false;
    std::string log_file_path_ = "application.log";
    
    // Helper methods for SQLite queries
    std::vector<LogEntry> executeQuery(const std::string& sql, const std::vector<std::string>& params = {});
};

#endif // LOGGER_H
