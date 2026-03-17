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

#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>
#include <map>
#include <chrono>
#include "../header/database.h"
#include "../header/utils.h"
#include "../header/localization.h"
#include "../header/logger.h"


void handleAddBook(const DatabaseManager &db);  // 添加图书
void handleFindBook(const DatabaseManager &db); // 查找图书
void handleUpdateBook(const DatabaseManager &db); // 更新图书信息
void handleDeleteBook(const DatabaseManager &db);  // 删除图书
void handleListAllBooks(const DatabaseManager &db);  // 列出所有图书
void handleListAllBooksWithPagination(const DatabaseManager &db);  // 列出所有图书（带分页）

void handleBorrowBook(const DatabaseManager &db, const User &currentUser);  // 借阅图书
void handleReturnBook(const DatabaseManager &db, const User &currentUser);  // 归还图书
void handleRenewBook(const DatabaseManager &db, const User &currentUser);   // 续借图书
void handleMyBorrowedBooks(const DatabaseManager &db, const User &currentUser);  // 普通用户查看借阅信息

void handleAddUser(const DatabaseManager &db, const User &adminUser);  // 管理员添加用户
void handleStudentManagement(const DatabaseManager &db);  // 学生管理
void handleListAllBorrowRecords(const DatabaseManager &db);  // 列出所有借阅记录
void handleRegister(const DatabaseManager &db);  // 登记信息
bool handleUpdateMyInfo(const DatabaseManager &db, User &currentUser);  // 普通用户更新自己的登记信息

void handleForgotPassword(const DatabaseManager &db);    // 忘记密码/找回密码
void handleAdminChangePassword(const DatabaseManager &db, const User &adminUser);  // 管理员用户帮助修改密码
void handleStudentChangePassword(const DatabaseManager &db, const User &currentUser);  // 普通用户修改密码
void handleSetRecoveryToken(const DatabaseManager &db, User &currentUser);  // 安全口令
void handleViewMyInfo(const User &currentUser);  // 查看自己的信息


// Log management functions
void handleQueryLogsByTime(const DatabaseManager &db);
void handleQueryLogsByUser(const DatabaseManager &db);
void handleQueryLogsByLevel(const DatabaseManager &db);
void handleQueryLogsByAction(const DatabaseManager &db);
void handleViewAllLogs(const DatabaseManager &db);
void handleViewAuditLog(const DatabaseManager &db);

void displayBooks(const std::vector<Book> &books);  //  显示图书信息
void displayBorrowRecords(const std::vector<BorrowRecord> &records);  // 显示借阅记录
void displayStudents(const std::vector<User> &students);  // 显示普通永固/学生用户信息
void displayFullBorrowRecords(const std::vector<FullBorrowRecord> &records);  // 显示全部借阅记录

// ------ 安全：登录频率限制 ------
struct LoginAttemptInfo {
    int failedCount = 0;
    std::chrono::steady_clock::time_point lockedUntil = std::chrono::steady_clock::now();
};
static std::map<std::string, LoginAttemptInfo> g_loginAttempts;
constexpr int MAX_LOGIN_ATTEMPTS = 5;        // 最多5次连续失败
constexpr int LOCKOUT_SECONDS = 300;         // 锁定5分钟

// 验证密码是否满足最低安全要求（至少8个字符）
static bool isValidPassword(const std::string &pwd) {
    return pwd.length() >= 8;
}


int pause() {
    std::cout << "\n" << _("press_enter_continue");
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    return 0;
}

int getIntInput() {
    int choice;
    while (!(std::cin >> choice)) {
        std::cout << "输入无效。请输入一个数字: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

void showAdminMenu(const DatabaseManager &db, const User &currentUser) {
    int choice;
    do {
        clearScreen();
        std::cout << "--- " << _("admin_menu_title") << " (" << currentUser.username << ") ---\n";
        std::cout << _("book_management") << "\n";
        std::cout << _("user_management") << "\n";
        std::cout << _("borrow_management") << "\n";
        std::cout << "4. " << _("log_management") << "\n";
        std::cout << _("logout") << "\n";
        std::cout << "---------------------------------\n";
        std::cout << _("enter_choice");
        choice = getIntInput();

        switch (choice) {
            case 1: {
                int bookChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("book_management") << " ---\n";
                    std::cout << _("add_new_book") << "\n" << _("modify_book_info") << "\n" << _("delete_book") << "\n" << _("search_book") << "\n" << _("list_all_books") << "\n" << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    bookChoice = getIntInput();
                    switch (bookChoice) {
                        case 1: handleAddBook(db);
                            break;
                        case 2: handleUpdateBook(db);
                            break;
                        case 3: handleDeleteBook(db);
                            break;
                        case 4: handleFindBook(db);
                            break;
                        case 5: handleListAllBooks(db);
                            break;
                        default: ;
                    }
                } while (bookChoice != 0);
                break;
            }
            case 2: {
                int userChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("user_management") << " ---\n";
                    std::cout << _("add_new_user") << "\n" << _("change_student_password") << "\n" << _("view_audit_log") << "\n" << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    userChoice = getIntInput();
                    switch (userChoice) {
                        case 1: handleAddUser(db, currentUser);
                            break;
                        case 2: handleAdminChangePassword(db, currentUser);
                            break;
                        case 3: handleViewAuditLog(db);
                            break;
                        default: ;
                    }
                } while (userChoice != 0);
                break;
            }
            case 3: {
                int recordChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("borrow_management") << " ---\n";
                    std::cout << _("query_specific_student") << "\n" << _("list_all_borrow_records") << "\n" << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    recordChoice = getIntInput();
                    switch (recordChoice) {
                        case 1: handleStudentManagement(db);
                            break;
                        case 2: handleListAllBorrowRecords(db);
                            break;
                        default: ;
                    }
                } while (recordChoice != 0);
                break;
            }

            case 4: {
                int logChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("log_management") << " ---\n";
                    std::cout << "1. " << _("query_logs_by_time") << "\n";
                    std::cout << "2. " << _("query_logs_by_user") << "\n";
                    std::cout << "3. " << _("query_logs_by_level") << "\n";
                    std::cout << "4. " << _("query_logs_by_action") << "\n";
                    std::cout << "5. " << _("view_all_logs") << "\n";
                    std::cout << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    logChoice = getIntInput();
                    switch (logChoice) {
                        case 1: handleQueryLogsByTime(db);
                            break;
                        case 2: handleQueryLogsByUser(db);
                            break;
                        case 3: handleQueryLogsByLevel(db);
                            break;
                        case 4: handleQueryLogsByAction(db);
                            break;
                        case 5: handleViewAllLogs(db);
                            break;
                        default: ;
                    }
                } while (logChoice != 0);
                break;
            }

            case 0: std::cout << _("logging_out") << "\n";
                break;
            default: std::cout << _("invalid_choice") << "\n";
                pause();
                break;
        }
    } while (choice != 0);
}

void showStudentMenu(const DatabaseManager &db, User &currentUser) {
    int choice;

    if (currentUser.name.empty() || currentUser.college.empty() || currentUser.className.empty()) {
        clearScreen();
        std::cout << _("welcome_user") << currentUser.username << "!\n";
        std::cout << _("complete_personal_info") << "\n";
        pause();
        if (!handleUpdateMyInfo(db, currentUser)) {
            std::cout << _("info_update_failed") << "\n";
            pause();
            return;
        }
    }

    clearScreen();
    std::cout << _("welcome_user") << currentUser.name << "!\n";
    if (const auto overdueBooks = db.getOverdueBooksByUser(currentUser.id); !overdueBooks.empty()) {
        std::cout << "\n" << _("overdue_notice") << "\n";
        displayBorrowRecords(overdueBooks);
    }
    pause();

    do {
        clearScreen();
        std::cout << "--- " << _("student_menu_title") << " (" << currentUser.name << ") ---\n";
        std::cout << _("search_book_prompt") << "\n";
        std::cout << _("borrow_book") << "\n";
        std::cout << _("return_book") << "\n";
        std::cout << _("renew_book") << "\n";
        std::cout << _("view_my_borrowed") << "\n";
        std::cout << _("account_management") << "\n";
        std::cout << _("logout") << "\n";
        std::cout << "-----------------------------\n";
        std::cout << _("enter_choice");
        choice = getIntInput();

        switch (choice) {
            case 1: handleFindBook(db);
                break;
            case 2: handleBorrowBook(db, currentUser);
                break;
            case 3: handleReturnBook(db, currentUser);
                break;
            case 4: handleRenewBook(db, currentUser);
                break;
            case 5: handleMyBorrowedBooks(db, currentUser);
                break;
            case 6: {
                int accountChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("account_management") << " ---\n";
                    std::cout << _("view_my_info") << "\n" << _("update_my_info") << "\n" << _("change_password") << "\n" << _("set_recovery_token") << "\n" << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    accountChoice = getIntInput();
                    switch (accountChoice) {
                        case 1: handleViewMyInfo(currentUser);
                            break;
                        case 2: handleUpdateMyInfo(db, currentUser);
                            break;
                        case 3: handleStudentChangePassword(db, currentUser);
                            break;
                        case 4: handleSetRecoveryToken(db, currentUser);
                            break;
                        default: ;
                    }
                } while (accountChoice != 0);
                break;
            }
            case 0: std::cout << _("logging_out") << "\n";
                break;
            default: std::cout << _("invalid_choice") << "\n";
                pause();
                break;
        }
    } while (choice != 0);
}

void login(const DatabaseManager &db) {
    std::string username, password;
    std::cout << "--- " << _("admin_menu_title") << " ---\n";  // Using admin menu title as generic login title
    std::cout << _("username_prompt");
    std::getline(std::cin, username);

    // Check if this account is currently locked out
    auto &info = g_loginAttempts[username];
    auto now = std::chrono::steady_clock::now();
    if (info.failedCount >= MAX_LOGIN_ATTEMPTS && now < info.lockedUntil) {
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(info.lockedUntil - now).count();
        int remainingMinutes = static_cast<int>((remaining + 59) / 60); // ceil: round up to next whole minute
        std::cout << _("account_locked") << remainingMinutes << _("account_locked_minutes") << "\n";
        pause();
        return;
    }

    std::cout << _("password_prompt");
    std::getline(std::cin, password);

    User user = db.authenticateUser(username, password);


    if (user.role == "ADMIN") {
        Logger::getInstance().info("Admin user logged in successfully", username, "login");
        showAdminMenu(db, user);
    } else if (user.role == "STUDENT") {
        Logger::getInstance().info("Student user logged in successfully", username, "login");
        showStudentMenu(db, user);
    } else {
        Logger::getInstance().warn("Failed login attempt", username, "login");
        std::cout << _("login_failed") << "\n";
        pause();
    }
}


int main() {
    // Initialize localization and load config
    Localization::getInstance().loadConfig();

    // Initialize logger
    Logger::getInstance().initialize("library.db");
    Logger::getInstance().info("Application started", "system", "startup");

    DatabaseManager db("library.db");
    if (!db.initialize()) {
        Logger::getInstance().error("Failed to initialize database", "system", "startup");
        return 1;
    }
    Logger::getInstance().info("Database initialized successfully", "system", "startup");

    if (!db.userExists("admin")) {
        std::cout << _("first_run_setup") << "\n";
        std::cout << _("creating_default_admin") << "\n";
        User adminUser;
        adminUser.id = "admin";
        adminUser.username = "admin";
        adminUser.name = "管理员";
        adminUser.role = "ADMIN";
        db.addUser(adminUser, "admin", true);  // Force password change on first login
        pause();
    }

    int choice;
    do {
        clearScreen();
        std::cout << "--- " << _("welcome_main_menu") << " ---\n";
        std::cout << _("login_option") << "\n";
        std::cout << _("register_option") << "\n";
        std::cout << _("forgot_password_option") << "\n";
        std::cout << _("choose_language") << "\n";
        std::cout << _("exit_option") << "\n";
        std::cout << "--------------------------------\n";
        std::cout << _("enter_choice");
        choice = getIntInput();

        switch (choice) {
            case 1: login(db);
                break;
            case 2: handleRegister(db);
                break;
            case 3: handleForgotPassword(db);
                break;
            case 4: {
                // Language selection
                int langChoice;
                do {
                    clearScreen();
                    std::cout << "--- " << _("choose_language") << " ---\n";
                    std::cout << _("english_option") << "\n";
                    std::cout << _("chinese_option") << "\n";
                    std::cout << _("return_to_main") << "\n";
                    std::cout << _("enter_choice");
                    langChoice = getIntInput();
                    
                    switch (langChoice) {
                        case 1: 
                            Localization::getInstance().setLanguage(Language::ENGLISH);
                            std::cout << _("language_set_to_english") << "\n";
                            pause();
                            break;
                        case 2:
                            Localization::getInstance().setLanguage(Language::CHINESE);
                            std::cout << _("language_set_to_chinese") << "\n";
                            pause();
                            break;
                        case 0:
                            break; // Return to main menu
                        default:
                            std::cout << _("invalid_choice") << "\n";
                            pause();
                            break;
                    }
                } while (langChoice != 0);
                
                // Refresh the main menu after language change to show updated language
                choice = -1; // Force re-display of main menu with new language
            }
            case 0: break;
            default: std::cout << _("invalid_choice") << "\n";
                pause();
                break;
        }
    } while (choice != 0);

    std::cout << _("exiting") << "\n";
    return 0;
}


void displayBooks(const std::vector<Book> &books) {
    if (books.empty()) {
        std::cout << _("no_books_found") << "\n";
        return;
    }

    const int termWidth = getTerminalWidth();
    const int availableWidth = termWidth - 12;

    const int isbnWidth = static_cast<int>(availableWidth > 20 ? 20 : availableWidth * 0.20);
    constexpr int availWidth = 6;
    constexpr int totalWidth = 6;
    const int authorWidth = static_cast<int>((availableWidth - isbnWidth - availWidth - totalWidth) * 0.4);
    const int titleWidth = static_cast<int>((availableWidth - isbnWidth - availWidth - totalWidth) * 0.6);

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell(_("book_table_header_isbn"), isbnWidth)
            << "| " << formatCell(_("book_table_header_title"), titleWidth)
            << "| " << formatCell(_("book_table_header_author"), authorWidth)
            << "| " << formatCell(_("book_table_header_available"), availWidth)
            << "| " << formatCell(_("book_table_header_total"), totalWidth) << "|\n";

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";

    for (const auto &book: books) {
        std::cout << "| " << formatCell(book.isbn, isbnWidth)
                << "| " << formatCell(book.title, titleWidth)
                << "| " << formatCell(book.author, authorWidth)
                << "| " << formatCell(std::to_string(book.availableCopies), availWidth)
                << "| " << formatCell(std::to_string(book.totalCopies), totalWidth) << "|\n";
    }

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";
}

void displayBorrowRecords(const std::vector<BorrowRecord> &records) {
    if (records.empty()) {
        std::cout << _("no_borrow_records") << "\n";
        return;
    }

    const int termWidth = getTerminalWidth();
    int availableWidth = termWidth - 12;
    constexpr int idWidth = 8;
    const int isbnWidth = static_cast<int>(availableWidth > 20 ? 20 : availableWidth * 0.20);
    constexpr int borrowDateWidth = 12;
    constexpr int dueDateWidth = 12;
    const int titleWidth = availableWidth - idWidth - isbnWidth - borrowDateWidth - dueDateWidth;

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell(_("borrow_table_header_record_id"), idWidth)
            << "| " << formatCell(_("book_table_header_isbn"), isbnWidth)
            << "| " << formatCell(_("book_table_header_title"), titleWidth)
            << "| " << formatCell(_("borrow_table_header_borrow_date"), borrowDateWidth)
            << "| " << formatCell(_("borrow_table_header_due_date"), dueDateWidth) << "|\n";

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";

    for (const auto &rec: records) {
        std::cout << "| " << formatCell(std::to_string(rec.recordId), idWidth)
                << "| " << formatCell(rec.bookIsbn, isbnWidth)
                << "| " << formatCell(rec.bookTitle, titleWidth)
                << "| " << formatCell(rec.borrowDate, borrowDateWidth)
                << "| " << formatCell(rec.dueDate, dueDateWidth) << "|\n";
    }

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";
}

void displayStudents(const std::vector<User> &students) {
    if (students.empty()) {
        std::cout << _("no_students") << "\n";
        return;
    }
    const int termWidth = getTerminalWidth();
    const int idWidth = static_cast<int>((termWidth - 6) * 0.20);
    const int nameWidth = static_cast<int>((termWidth - 6) * 0.20);
    const int collegeWidth = static_cast<int>((termWidth - 6) * 0.30);
    const int classWidth = static_cast<int>((termWidth - 6) * 0.30);

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
    std::cout << "| " << formatCell(_("student_table_header_id"), idWidth) << "| " << formatCell(_("student_table_header_name"), nameWidth) << "| " <<
            formatCell(_("student_table_header_college"), collegeWidth) << "| " << formatCell(_("student_table_header_class"), classWidth) << "|\n";
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
    for (const auto &s: students) {
        std::cout << "| " << formatCell(s.id, idWidth) << "| " << formatCell(s.name, nameWidth) << "| " <<
                formatCell(s.college, collegeWidth) << "| " << formatCell(s.className, classWidth) << "|\n";
    }
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
}

void displayFullBorrowRecords(const std::vector<FullBorrowRecord> &records) {
    if (records.empty()) {
        std::cout << _("no_borrow_records_found") << "\n";
        return;
    }
    const int termWidth = getTerminalWidth();
    const int availableWidth = termWidth - 10;
    const int idWidth = static_cast<int>(termWidth > 120 ? 15 : availableWidth * 0.12);
    const int nameWidth = static_cast<int>(termWidth > 120 ? 10 : availableWidth * 0.10);
    const int collegeWidth = static_cast<int>(termWidth > 120 ? 20 : availableWidth * 0.20);
    const int titleWidth = static_cast<int>(termWidth > 120 ? 30 : availableWidth * 0.30);
    constexpr int borrowDateWidth = 12;
    constexpr int dueDateWidth = 12;
    constexpr int overdueWidth = 8;

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell(_("full_borrow_table_header_student_id"), idWidth) << "| " << formatCell(_("full_borrow_table_header_name"), nameWidth)
            << "| " << formatCell(_("full_borrow_table_header_college"), collegeWidth) << "| " << formatCell(_("full_borrow_table_header_title"), titleWidth)
            << "| " << formatCell(_("borrow_table_header_borrow_date"), borrowDateWidth) << "| " << formatCell(_("borrow_table_header_due_date"), dueDateWidth)
            << "| " << formatCell(_("full_borrow_table_header_overdue"), overdueWidth) << "|\n";

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";

    for (const auto &rec: records) {
        std::cout << "| " << formatCell(rec.studentId, idWidth)
                << "| " << formatCell(rec.studentName, nameWidth)
                << "| " << formatCell(rec.studentCollege, collegeWidth)
                << "| " << formatCell(rec.bookTitle, titleWidth)
                << "| " << formatCell(rec.borrowDate, borrowDateWidth)
                << "| " << formatCell(rec.dueDate, dueDateWidth)
                << "| " << formatCell(rec.isOverdue ? _("yes") : _("no"), overdueWidth) << "|\n";
    }
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";
}


void handleAddBook(const DatabaseManager &db) {
    Book b;
    std::cout << "--- " << _("add_new_book") << " ---\n";
    std::cout << _("isbn_prompt");
    std::getline(std::cin, b.isbn);
    std::cout << _("title_prompt");
    std::getline(std::cin, b.title);
    std::cout << _("author_prompt");
    std::getline(std::cin, b.author);
    std::cout << _("publisher_prompt");
    std::getline(std::cin, b.publisher);
    std::cout << _("category_prompt");
    std::getline(std::cin, b.category);
    std::cout << _("total_copies_prompt");
    b.totalCopies = getIntInput();
    b.availableCopies = b.totalCopies;

    if (db.addBook(b)) {
        Logger::getInstance().info("Book added: " + b.title + " (ISBN: " + b.isbn + ")", "admin", "add_book");
        std::cout << _("book_added_success") << "\n";
    } else {
        Logger::getInstance().error("Failed to add book: " + b.title + " (ISBN: " + b.isbn + ")", "admin", "add_book");
        std::cout << _("book_add_failed") << "\n";
    }
    pause();
}

void handleFindBook(const DatabaseManager &db) {
    std::string keyword;
    std::cout << _("search_keyword_prompt");
    std::getline(std::cin, keyword);
    auto books = db.findBooks(keyword, "title");
    displayBooks(books);
    pause();
}

void handleListAllBooksWithPagination(const DatabaseManager &db) {
    std::cout << _("sort_by_prompt");
    int choice = getIntInput();
    std::string sortBy = "title";
    if (choice == 2) sortBy = "author";
    if (choice == 3) sortBy = "isbn";
    
    std::cout << _("page_size_prompt") << ": ";
    int pageSize = getIntInput();
    if (pageSize <= 0) pageSize = 10; // Default page size
    
    int offset = 0;
    bool hasNextPage = true;
    
    while (hasNextPage) {
        auto books = db.getAllBooksWithPagination(sortBy, offset, pageSize);
        
        if (offset == 0) {
            std::cout << "\n--- " << _("all_books_list") << " ---\n";
        }
        
        if (books.empty()) {
            if (offset == 0) {
                std::cout << _("no_books_found") << "\n";
            } else {
                std::cout << _("no_more_books") << "\n";
            }
            break;
        }
        
        displayBooks(books);
        
        // Check if there are more books
        auto nextPageBooks = db.getAllBooksWithPagination(sortBy, offset + pageSize, 1);
        hasNextPage = !nextPageBooks.empty();
        
        if (hasNextPage) {
            std::cout << "\n" << _("show_next_page_or_exit") << "\n";
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "q" || input == "Q") {
                break;
            }
            
            offset += pageSize;
        } else {
            std::cout << _("end_of_list") << "\n";
        }
    }
    pause();
}

void handleListAllBooks(const DatabaseManager &db) {
    std::cout << _("pagination_option_prompt") << "\n";
    std::cout << "1. " << _("show_all_at_once") << "\n";
    std::cout << "2. " << _("show_with_pagination") << "\n";
    int paginationChoice = getIntInput();
    
    if (paginationChoice == 2) {
        handleListAllBooksWithPagination(db);
        return;
    }
    
    std::cout << _("sort_by_prompt");
    int choice = getIntInput();
    std::string sortBy = "title";
    if (choice == 2) sortBy = "author";
    if (choice == 3) sortBy = "isbn";

    auto books = db.getAllBooks(sortBy);
    displayBooks(books);
    pause();
}


void handleUpdateBook(const DatabaseManager &db) {
    std::string isbn;
    std::cout << _("modify_isbn_prompt");
    std::getline(std::cin, isbn);

    auto books = db.findBooks(isbn, "isbn");
    if (books.empty() || books[0].isbn != isbn) {
        std::cout << _("book_not_found") << "\n";
        pause();
        return;
    }

    Book b = books[0];
    std::cout << _("modifying_book") << b.title << std::endl;

    std::cout << _("new_title_prompt") << b.title << "'): ";
    std::string new_val;
    std::getline(std::cin, new_val);
    if (!new_val.empty()) b.title = new_val;

    std::cout << _("new_author_prompt") << b.author << "'): ";
    std::getline(std::cin, new_val);
    if (!new_val.empty()) b.author = new_val;

    std::cout << _("current_total_prompt") << b.totalCopies << "): ";
    new_val.clear();
    std::getline(std::cin, new_val);
    if (!new_val.empty()) {
        try {
            int new_total = std::stoi(new_val);
            int diff = new_total - b.totalCopies;
            b.totalCopies = new_total;
            b.availableCopies += diff;
        } catch (...) {
            std::cout << _("invalid_choice") << "\n";
        }
    }


    if (db.updateBook(b)) {
        std::cout << _("book_update_success") << "\n";
    } else {
        std::cout << _("book_update_failed") << "\n";
    }
    pause();
}

void handleDeleteBook(const DatabaseManager &db) {
    std::string isbn;
    std::cout << _("delete_isbn_prompt");
    std::getline(std::cin, isbn);
    if (db.deleteBook(isbn)) {
        std::cout << _("book_delete_success") << "\n";
    } else {
        std::cout << _("book_delete_failed") << "\n";
    }
    pause();
}

void handleBorrowBook(const DatabaseManager &db, const User &currentUser) {
    std::string isbn;
    std::cout << _("isbn_prompt");
    std::getline(std::cin, isbn);

    std::cout << _("borrow_days_prompt");
    int days = getIntInput();
    if (days < 1 || days > 90) {
        std::cout << _("invalid_days") << "\n";
        pause();
        return;
    }

    // Pre-checks to give the user a specific error message.
    // Note: The actual enforcement also runs inside borrowBook's transaction,
    // so these checks are safe even though they precede the atomic operation.
    if (db.isBookAlreadyBorrowedByUser(currentUser.id, isbn)) {
        std::cout << _("duplicate_borrow_error") << "\n";
        pause();
        return;
    }
    if (db.getActiveBorrowCount(currentUser.id) >= MAX_BORROW_LIMIT) {
        std::cout << _("borrow_limit_exceeded") << "\n";
        pause();
        return;
    }

    if (db.borrowBook(currentUser.id, isbn, days)) {
        Logger::getInstance().info("Book borrowed (ISBN: " + isbn + ") for " + std::to_string(days) + " days", currentUser.username, "borrow_book");
        std::cout << _("borrow_success") << "\n";
        db.logAction(currentUser.id, "BORROW", "Borrowed ISBN: " + isbn + " for " + std::to_string(days) + " days");
    } else {
        Logger::getInstance().warn("Failed to borrow book (ISBN: " + isbn + ")", currentUser.username, "borrow_book");
        std::cout << _("borrow_failed") << "\n";
    }
    pause();
}

void handleReturnBook(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- " << _("my_current_books") << " ---\n";
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    if (records.empty()) {
        pause();
        return;
    }

    std::cout << "\n" << _("return_record_id_prompt");
    int recordId = getIntInput();

    if (db.returnBook(recordId, currentUser.id)) {
        Logger::getInstance().info("Book returned (Record ID: " + std::to_string(recordId) + ")", currentUser.username, "return_book");
        std::cout << _("return_success") << "\n";
        db.logAction(currentUser.id, "RETURN", "Returned record ID: " + std::to_string(recordId));
    } else {
        Logger::getInstance().warn("Failed to return book (Record ID: " + std::to_string(recordId) + ")", currentUser.username, "return_book");
        std::cout << _("return_failed") << "\n";
    }
    pause();
}

void handleRenewBook(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- " << _("my_current_books") << " ---\n";
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    if (records.empty()) {
        pause();
        return;
    }

    std::cout << "\n" << _("return_record_id_prompt");  // Note: same prompt as return book

    if (const int recordId = getIntInput(); db.renewBook(recordId, currentUser.id)) {
        std::cout << _("renew_success") << "\n";
        db.logAction(currentUser.id, "RENEW", "Renewed record ID: " + std::to_string(recordId));
    } else {
        std::cout << _("renew_failed") << "\n";
    }
    pause();
}


void handleMyBorrowedBooks(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    pause();
}

void handleAddUser(const DatabaseManager &db, const User &adminUser) {
    User newUser;
    std::string password;
    std::cout << "--- " << _("add_new_user") << " ---\n";
    std::cout << _("add_username_prompt");
    std::getline(std::cin, newUser.username);
    std::cout << _("add_password_prompt");
    std::getline(std::cin, password);
    std::cout << _("add_role_prompt");
    std::getline(std::cin, newUser.role);

    if (newUser.role == "STUDENT") {
        std::cout << _("student_id_prompt");
        std::getline(std::cin, newUser.id);
        std::cout << _("student_name_prompt");
        std::getline(std::cin, newUser.name);
        std::cout << _("college_prompt");
        std::getline(std::cin, newUser.college);
        std::cout << _("class_prompt");
        std::getline(std::cin, newUser.className);
    } else if (newUser.role == "ADMIN") {
        newUser.id = newUser.username; // 管理员的ID就是其用户名
    } else {
        std::cout << _("invalid_role") << "\n";
        pause();
        return;
    }

    if (!isValidPassword(password)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return;
    }

    if (db.addUser(newUser, password)) {
        std::cout << _("user_add_success") << newUser.username << _("user_add_success_part2") << "\n";
        db.logAction(adminUser.username, "ADD_USER", "Added user: " + newUser.username + " role: " + newUser.role);
    } else {
        std::cout << _("user_add_failed") << "\n";
    }
    pause();
}

void handleStudentManagement(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- " << _("query_specific_student") << " ---\n";
    std::string keyword;
    std::cout << _("student_query_prompt");
    std::getline(std::cin, keyword);
    auto students = db.findStudents(keyword);

    if (students.empty()) {
        std::cout << _("student_not_found") << "\n";
    } else if (students.size() == 1) {
        const auto records = db.getFullBorrowRecordsForUser(students[0].id);
        std::cout << "\n" << _("student_borrow_records") << students[0].name << " (" << _("student_id_label") << students[0].id << ") " << _("borrow_table_header_borrow_date") << ":\n";
        displayFullBorrowRecords(records);
    } else {
        std::cout << _("multiple_students_found") << "\n";
        displayStudents(students);
        std::cout << _("select_student_id_prompt");
        std::string selectedId;
        std::getline(std::cin, selectedId);
        auto records = db.getFullBorrowRecordsForUser(selectedId);
        displayFullBorrowRecords(records);
    }
    pause();
}


void handleRegister(const DatabaseManager &db) {
    User newUser;
    std::string password;
    newUser.role = "STUDENT";

    std::cout << "--- " << _("student_registration") << " ---\n";
    std::cout << _("student_id_login_prompt");
    std::getline(std::cin, newUser.id);
    newUser.username = newUser.id;

    if (db.userExists(newUser.username)) {
        std::cout << _("id_already_exists") << "\n";
        pause();
        return;
    }

    std::cout << _("real_name_prompt");
    std::getline(std::cin, newUser.name);
    std::cout << _("college_prompt");
    std::getline(std::cin, newUser.college);
    std::cout << _("class_prompt");
    std::getline(std::cin, newUser.className);
    std::cout << _("add_password_prompt");
    std::getline(std::cin, password);

    if (!isValidPassword(password)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return;
    }

    if (db.addUser(newUser, password)) {
        std::cout << _("registration_success") << "\n";
    } else {
        std::cout << _("registration_failed") << "\n";
    }
    pause();
}

bool handleUpdateMyInfo(const DatabaseManager &db, User &currentUser) {
    std::cout << "--- " << _("update_personal_info") << " ---\n";

    std::cout << _("current_name_prompt") << (currentUser.name.empty() ? _("token_not_set") : currentUser.name) << "): ";
    std::string newName;
    std::getline(std::cin, newName);
    if (!newName.empty()) currentUser.name = newName;

    std::cout << _("current_college_prompt") << (currentUser.college.empty() ? _("token_not_set") : currentUser.college) << "): ";
    std::string newCollege;
    std::getline(std::cin, newCollege);
    if (!newCollege.empty()) currentUser.college = newCollege;

    std::cout << _("current_class_prompt") << (currentUser.className.empty() ? _("token_not_set") : currentUser.className) << "): ";
    std::string newClass;
    std::getline(std::cin, newClass);
    if (!newClass.empty()) currentUser.className = newClass;

    if (db.updateStudentInfo(currentUser)) {
        std::cout << _("info_update_success") << "\n";
        pause();
        return true;
    } else {
        std::cout << _("info_update_failed_msg") << "\n";
        pause();
        return false;
    }
}

void handleListAllBorrowRecords(const DatabaseManager &db) {
    std::cout << _("sort_method_prompt");
    int choice = getIntInput();
    std::string sortBy = "studentId";
    if (choice == 2) sortBy = "dueDate";

    auto records = db.getAllFullBorrowRecords(sortBy);
    displayFullBorrowRecords(records);
    pause();
}

void handleForgotPassword(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- " << _("forgot_password_option") << " ---\n";
    std::string username, token, newPassword, confirmPassword;
    std::cout << _("student_id_login_prompt");
    std::getline(std::cin, username);
    std::cout << _("recovery_token_prompt");
    std::getline(std::cin, token);
    std::cout << _("new_password_prompt");
    std::getline(std::cin, newPassword);
    std::cout << _("confirm_password_prompt");
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << _("password_mismatch") << "\n";
        pause();
        return;
    }

    if (!isValidPassword(newPassword)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return;
    }

    if (db.recoverPassword(username, token, newPassword)) {
        std::cout << _("password_reset_success") << "\n";
    } else {
        std::cout << _("password_reset_failed") << "\n";
    }
    pause();
}

void handleAdminChangePassword(const DatabaseManager &db, const User &adminUser) {
    clearScreen();
    std::cout << "--- " << _("student_change_password") << " ---\n";
    std::string username, newPassword, confirmPassword;
    std::cout << _("target_student_id_prompt");
    std::getline(std::cin, username);

    if (!db.userExists(username)) {
        std::cout << _("student_id_not_exist") << "\n";
        pause();
        return;
    }

    std::cout << _("new_password_prompt");
    std::getline(std::cin, newPassword);
    std::cout << _("confirm_password_prompt");
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << _("password_mismatch") << "\n";
        pause();
        return;
    }

    if (!isValidPassword(newPassword)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return;
    }

    if (db.updatePassword(username, newPassword)) {
        std::cout << _("password_change_success") << "\n";
        db.logAction(adminUser.username, "CHANGE_PASSWORD", "Changed password for: " + username);
    } else {
        std::cout << _("password_change_failed") << "\n";
    }
    pause();
}

void handleStudentChangePassword(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- " << _("student_password_change") << " ---\n";
    std::string oldPassword, newPassword, confirmPassword;
    std::cout << _("current_password_prompt");
    std::getline(std::cin, oldPassword);

    // 验证旧密码是否正确
    if (db.authenticateUser(currentUser.username, oldPassword).role.empty()) {
        std::cout << _("current_password_wrong") << "\n";
        pause();
        return;
    }

    std::cout << _("new_password_prompt");
    std::getline(std::cin, newPassword);
    std::cout << _("confirm_password_prompt");
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << _("password_mismatch") << "\n";
        pause();
        return;
    }

    if (!isValidPassword(newPassword)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return;
    }

    if (db.updatePassword(currentUser.username, newPassword)) {
        std::cout << _("password_change_success") << "\n";
        db.logAction(currentUser.username, "CHANGE_PASSWORD", "Student changed own password");
    } else {
        std::cout << _("password_change_failed") << "\n";
    }
    pause();
}

void handleSetRecoveryToken(const DatabaseManager &db, User &currentUser) {
    clearScreen();
    std::cout << "--- " << _("set_recovery_token") << " ---\n";
    if (currentUser.hasRecoveryToken) {
        std::cout << _("token_already_set") << "\n";
    }
    std::cout << _("enter_new_token");
    std::string token;
    std::getline(std::cin, token);

    if (token.empty()) {
        std::cout << _("token_empty_error") << "\n";
        pause();
        return;
    }

    if (db.updateRecoveryToken(currentUser.username, token)) {
        std::cout << _("token_set_success") << "\n";
        currentUser.hasRecoveryToken = true;
    } else {
        std::cout << _("token_set_failed") << "\n";
    }
    pause();
}

void handleViewMyInfo(const User &currentUser) {
    clearScreen();
    std::cout << "--- " << _("my_personal_info") << " ---\n";
    std::cout << _("student_id_label") << "    " << currentUser.id << std::endl;
    std::cout << _("name_label") << "    " << currentUser.name << std::endl;
    std::cout << _("college_label") << "    " << currentUser.college << std::endl;
    std::cout << _("class_label") << "    " << currentUser.className << std::endl;
    std::cout << _("role_label") << "    " << currentUser.role << std::endl;
    std::cout << _("recovery_token_status") << " " << (currentUser.hasRecoveryToken ? _("token_set") : _("token_not_set")) << std::endl;
    pause();
}

void handleViewAuditLog(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- " << _("audit_log_title") << " ---\n";
    std::cout << _("audit_log_lines_prompt");
    std::string input;
    std::getline(std::cin, input);
    int limit = 20;
    if (!input.empty()) {
        try { limit = std::stoi(input); } catch (...) {}
    }
    if (limit < 1) limit = 20;

    auto records = db.getRecentAuditLogs(limit);
    if (records.empty()) {
        std::cout << _("no_audit_records") << "\n";
    } else {
        std::cout << std::left
                  << std::setw(6)  << _("audit_table_header_id")
                  << std::setw(20) << _("audit_table_header_time")
                  << std::setw(16) << _("audit_table_header_user")
                  << std::setw(20) << _("audit_table_header_action")
                  << _("audit_table_header_detail") << "\n";
        std::cout << std::string(90, '-') << "\n";
        for (const auto &r : records) {
            std::cout << std::left
                      << std::setw(6)  << r.id
                      << std::setw(20) << r.timestamp
                      << std::setw(16) << r.userId
                      << std::setw(20) << r.action
                      << r.detail << "\n";
        }
    }
    pause();
}

void handleViewStatistics(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- " << _("statistics_title") << " ---\n";
    const SystemStats s = db.getSystemStats();
    std::cout << _("stats_total_book_titles") << s.totalBookTitles << "\n";
    std::cout << _("stats_total_copies")      << s.totalCopies << "\n";
    std::cout << _("stats_available_copies")  << s.availableCopies << "\n";
    std::cout << _("stats_total_students")    << s.totalStudents << "\n";
    std::cout << _("stats_active_borrowings") << s.activeBorrowings << "\n";
    std::cout << _("stats_overdue_count")     << s.overdueCount << "\n";
    if (s.topBorrowedBookTitle.empty()) {
        std::cout << _("stats_top_borrowed") << _("stats_no_borrows_yet") << "\n";
    } else {
        std::cout << _("stats_top_borrowed") << s.topBorrowedBookTitle
                  << _("stats_borrow_count") << s.topBorrowedBookCount << _("stats_borrow_count_end") << "\n";
    }
    pause();
}

bool handleForcedPasswordChange(const DatabaseManager &db, User &currentUser) {
    std::string newPassword, confirmPassword;
    std::cout << _("new_password_prompt");
    std::getline(std::cin, newPassword);
    std::cout << _("confirm_password_prompt");
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << _("password_mismatch") << "\n";
        pause();
        return false;
    }
    if (!isValidPassword(newPassword)) {
        std::cout << _("password_too_short") << "\n";
        pause();
        return false;
    }
    if (db.updatePassword(currentUser.username, newPassword)) {
        std::cout << _("password_change_success") << "\n";
        currentUser.passwordNeedsChange = false;
        db.logAction(currentUser.username, "FORCED_PW_CHANGE", "Completed mandatory password change");
        pause();
        return true;
    }
    std::cout << _("password_change_failed") << "\n";
    pause();
    return false;
}
