#ifndef API_SERVER_H
#define API_SERVER_H

#include <string>
#include <memory>
#include "database.h"
#include "logger.h"

// Forward declaration for httplib
namespace httplib {
    class Server;
}

class ApiServer {
public:
    explicit ApiServer(DatabaseManager* db, int port = 8080);
    ~ApiServer();

    void start();
    void stop();

private:
    void setupRoutes();

    // Authentication endpoints
    void handleLogin();
    void handleRegister();
    void handleForgotPassword();

    // Book management endpoints
    void handleGetBooks();
    void handleGetBookByIsbn();
    void handleAddBook();
    void handleUpdateBook();
    void handleDeleteBook();

    // User management endpoints
    void handleGetUsers();
    void handleGetUserById();
    void handleUpdateUser();
    void handleChangePassword();
    void handleSetRecoveryToken();

    // Borrowing management endpoints
    void handleGetBorrowedBooks();
    void handleGetOverdueBooks();
    void handleBorrowBook();
    void handleReturnBook();
    void handleRenewBook();
    void handleGetFullBorrowRecords();
    void handleGetStudentBorrowRecords();

    // Log management endpoints
    void handleGetLogs();

    // Helper methods
    std::string generateToken(const std::string& username);
    bool verifyToken(const std::string& token, std::string& username);

    DatabaseManager* db_;
    std::unique_ptr<httplib::Server> server_;
    int port_;
    bool running_;
};

#endif // API_SERVER_H
