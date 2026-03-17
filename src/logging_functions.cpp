#include "../header/database.h"
#include "../header/utils.h"
#include "../header/localization.h"
#include "../header/logger.h"
#include <iostream>
#include <vector>
#include <iomanip>

// Display logs in a formatted table
void displayLogs(const std::vector<Logger::LogEntry> &logs) {
    if (logs.empty()) {
        std::cout << _("no_logs_found") << "\n";
        return;
    }

    // Calculate column widths
    int termWidth = getTerminalWidth();
    int idWidth = 6;
    int timeWidth = 20;
    int levelWidth = 8;
    int userWidth = 15;
    int actionWidth = 20;
    int messageWidth = termWidth - idWidth - timeWidth - levelWidth - userWidth - actionWidth - 7;

    if (messageWidth < 20) messageWidth = 20;

    // Print header
    std::cout << std::string(termWidth, '-') << "\n";
    std::cout << std::left
              << std::setw(idWidth) << "ID"
              << " " << std::setw(timeWidth) << _("time")
              << " " << std::setw(levelWidth) << _("level")
              << " " << std::setw(userWidth) << _("user")
              << " " << std::setw(actionWidth) << _("action")
              << " " << std::setw(messageWidth) << _("message") << "\n";
    std::cout << std::string(termWidth, '-') << "\n";

    // Print log entries
    for (const auto &log : logs) {
        std::string levelStr;
        switch (log.level) {
            case Logger::Level::DEBUG: levelStr = "DEBUG"; break;
            case Logger::Level::INFO: levelStr = "INFO"; break;
            case Logger::Level::WARN: levelStr = "WARN"; break;
            case Logger::Level::ERROR: levelStr = "ERROR"; break;
        }

        std::cout << std::left
                  << std::setw(idWidth) << log.id
                  << " " << std::setw(timeWidth) << log.timestamp
                  << " " << std::setw(levelWidth) << levelStr
                  << " " << std::setw(userWidth) << (log.user.empty() ? "-" : log.user)
                  << " " << std::setw(actionWidth) << (log.action.empty() ? "-" : log.action)
                  << " " << std::setw(messageWidth) << log.message << "\n";
    }
    std::cout << std::string(termWidth, '-') << "\n";
    std::cout << _("total_logs") << ": " << logs.size() << "\n";
}

void handleQueryLogsByTime(const DatabaseManager &db) {
    std::string startTime, endTime;
    std::cout << "--- " << _("query_logs_by_time") << " ---\n";
    std::cout << _("start_time_prompt") << " (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, startTime);
    std::cout << _("end_time_prompt") << " (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, endTime);

    auto logs = Logger::getInstance().queryLogsByTime(startTime, endTime);
    displayLogs(logs);
    pause();
}

void handleQueryLogsByUser(const DatabaseManager &db) {
    std::string username;
    std::cout << "--- " << _("query_logs_by_user") << " ---\n";
    std::cout << _("username_prompt") << ": ";
    std::getline(std::cin, username);

    auto logs = Logger::getInstance().queryLogsByUser(username);
    displayLogs(logs);
    pause();
}

void handleQueryLogsByLevel(const DatabaseManager &db) {
    int levelChoice;
    std::cout << "--- " << _("query_logs_by_level") << " ---\n";
    std::cout << "1. DEBUG\n2. INFO\n3. WARN\n4. ERROR\n";
    std::cout << _("enter_choice");
    std::cin >> levelChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Logger::Level level;
    switch (levelChoice) {
        case 1: level = Logger::Level::DEBUG; break;
        case 2: level = Logger::Level::INFO; break;
        case 3: level = Logger::Level::WARN; break;
        case 4: level = Logger::Level::ERROR; break;
        default:
            std::cout << _("invalid_choice") << "\n";
            pause();
            return;
    }

    auto logs = Logger::getInstance().queryLogsByLevel(level);
    displayLogs(logs);
    pause();
}

void handleQueryLogsByAction(const DatabaseManager &db) {
    std::string action;
    std::cout << "--- " << _("query_logs_by_action") << " ---\n";
    std::cout << _("action_prompt") << ": ";
    std::getline(std::cin, action);

    auto logs = Logger::getInstance().queryLogsByAction(action);
    displayLogs(logs);
    pause();
}

void handleViewAllLogs(const DatabaseManager &db) {
    std::cout << "--- " << _("view_all_logs") << " ---\n";

    // Query all logs by getting logs from a very old date to far future
    auto logs = Logger::getInstance().queryLogsByTime("2000-01-01 00:00:00", "2099-12-31 23:59:59");
    displayLogs(logs);
    pause();
}