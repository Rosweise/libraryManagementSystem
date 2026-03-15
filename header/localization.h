#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>

enum class Language {
    ENGLISH,
    CHINESE
};

class Localization {
public:
    static Localization& getInstance() {
        static Localization instance;
        return instance;
    }

    void loadConfig(const std::string& configFile = "config.ini") {
        configFilePath = configFile;
        
        // Load existing config or create default
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.substr(0, 8) == "language") {
                    size_t pos = line.find('=');
                    if (pos != std::string::npos) {
                        std::string lang = line.substr(pos + 1);
                        if (lang == "en") {
                            currentLanguage = Language::ENGLISH;
                        } else {
                            currentLanguage = Language::CHINESE;
                        }
                    }
                }
            }
        } else {
            // Default to Chinese if no config exists
            currentLanguage = Language::CHINESE;
            saveConfig();
        }
    }

    void saveConfig() const {
        std::ofstream file(configFilePath);
        if (file.is_open()) {
            file << "[settings]\n";
            file << "language=" << (currentLanguage == Language::ENGLISH ? "en" : "zh") << "\n";
        }
    }

    void setLanguage(Language lang) {
        currentLanguage = lang;
        saveConfig();
    }

    Language getCurrentLanguage() const {
        return currentLanguage;
    }

    std::string getText(const std::string& key) const {
        if (currentLanguage == Language::ENGLISH) {
            auto it = englishTexts.find(key);
            return it != englishTexts.end() ? it->second : key;
        } else {
            auto it = chineseTexts.find(key);
            return it != chineseTexts.end() ? it->second : key;
        }
    }

private:
    Localization() {
        initializeTexts();
    }

    void initializeTexts() {
        // English texts
        englishTexts["welcome_main_menu"] = "Welcome to Library Management System";
        englishTexts["login_option"] = "1. Login";
        englishTexts["register_option"] = "2. Student Registration";
        englishTexts["forgot_password_option"] = "3. Forgot Password";
        englishTexts["exit_option"] = "0. Exit";
        englishTexts["choose_language"] = "4. Choose Language / 选择语言";
        englishTexts["english_option"] = "1. English";
        englishTexts["chinese_option"] = "2. 中文";
        englishTexts["enter_choice"] = "Enter your choice: ";
        englishTexts["invalid_choice"] = "Invalid choice, please try again.";
        englishTexts["exiting"] = "Thank you for using, goodbye!";
        englishTexts["admin_menu_title"] = "Admin Menu";
        englishTexts["student_menu_title"] = "Student Menu";
        englishTexts["book_management"] = "1. Book Management - Add/Edit/Delete/Search";
        englishTexts["user_management"] = "2. User Management - Add/Change Password";
        englishTexts["borrow_management"] = "3. Borrow Management - Query Students/All Records";
        englishTexts["logout"] = "0. Logout";
        englishTexts["add_new_book"] = "1. Add New Book";
        englishTexts["modify_book_info"] = "2. Modify Book Info";
        englishTexts["delete_book"] = "3. Delete Book";
        englishTexts["search_book"] = "4. Search Book";
        englishTexts["list_all_books"] = "5. List All Books";
        englishTexts["return_to_main"] = "0. Return";
        englishTexts["add_new_user"] = "1. Add New User";
        englishTexts["change_student_password"] = "2. Change Student Password";
        englishTexts["query_specific_student"] = "1. Query Specific Student";
        englishTexts["list_all_borrow_records"] = "2. List All Borrow Records";
        englishTexts["search_book_prompt"] = "1. Search Book";
        englishTexts["borrow_book"] = "2. Borrow Book";
        englishTexts["return_book"] = "3. Return Book";
        englishTexts["renew_book"] = "4. Renew Book";
        englishTexts["view_my_borrowed"] = "5. View My Borrowed Books";
        englishTexts["account_management"] = "6. Account Management (View Info/Change Password/Set Token)";
        englishTexts["view_my_info"] = "1. View My Info";
        englishTexts["update_my_info"] = "2. Update My Info";
        englishTexts["change_password"] = "3. Change Password";
        englishTexts["set_recovery_token"] = "4. Set/Update Recovery Token";
        englishTexts["username_prompt"] = "Username (Admin) or Student ID (Student): ";
        englishTexts["password_prompt"] = "Password: ";
        englishTexts["login_failed"] = "Incorrect username or password.";
        englishTexts["welcome_user"] = "Welcome, ";
        englishTexts["overdue_notice"] = "!!! Notice: You have overdue books! !!!";
        englishTexts["press_enter_continue"] = "Press Enter to continue...";
        englishTexts["isbn_prompt"] = "ISBN: ";
        englishTexts["title_prompt"] = "Title: ";
        englishTexts["author_prompt"] = "Author: ";
        englishTexts["publisher_prompt"] = "Publisher: ";
        englishTexts["category_prompt"] = "Category: ";
        englishTexts["total_copies_prompt"] = "Total Copies: ";
        englishTexts["book_added_success"] = "Book added successfully!";
        englishTexts["book_add_failed"] = "Failed to add book. ISBN may already exist.";
        englishTexts["search_keyword_prompt"] = "Enter search keyword (Title/Author/ISBN): ";
        englishTexts["sort_by_prompt"] = "Choose sort method (1:Title, 2:Author, 3:ISBN): ";
        englishTexts["modify_isbn_prompt"] = "Enter ISBN of book to modify: ";
        englishTexts["book_not_found"] = "Book with this ISBN not found.";
        englishTexts["modifying_book"] = "Modifying book: ";
        englishTexts["leave_blank_hint"] = "Leave blank to not modify '";
        englishTexts["new_title_prompt"] = "New Title (leave blank to keep '";
        englishTexts["new_author_prompt"] = "New Author (leave blank to keep '";
        englishTexts["current_total_prompt"] = "New Total (current: ";
        englishTexts["book_update_success"] = "Book information updated successfully!";
        englishTexts["book_update_failed"] = "Update failed.";
        englishTexts["delete_isbn_prompt"] = "Enter ISBN of book to delete: ";
        englishTexts["book_delete_success"] = "Book deleted successfully.";
        englishTexts["book_delete_failed"] = "Deletion failed. Check if the book still has borrowing records.";
        englishTexts["borrow_days_prompt"] = "How many days do you want to borrow (1-90 days): ";
        englishTexts["invalid_days"] = "Invalid number of days.";
        englishTexts["borrow_success"] = "Borrowing successful!";
        englishTexts["borrow_failed"] = "Borrowing failed.";
        englishTexts["my_current_books"] = "My Currently Borrowed Books";
        englishTexts["return_record_id_prompt"] = "Enter Record ID of book to return: ";
        englishTexts["return_success"] = "Return successful!";
        englishTexts["return_failed"] = "Return failed.";
        englishTexts["renew_success"] = "Renewal successful! New due date has been updated.";
        englishTexts["renew_failed"] = "Renewal failed.";
        englishTexts["add_username_prompt"] = "Enter username: ";
        englishTexts["add_password_prompt"] = "Enter password: ";
        englishTexts["add_role_prompt"] = "Enter role (ADMIN or STUDENT): ";
        englishTexts["student_id_prompt"] = "Enter Student ID: ";
        englishTexts["student_name_prompt"] = "Enter Name: ";
        englishTexts["college_prompt"] = "Enter College: ";
        englishTexts["class_prompt"] = "Enter Class: ";
        englishTexts["invalid_role"] = "Invalid role. Must be ADMIN or STUDENT.";
        englishTexts["user_add_success"] = "User '";
        englishTexts["user_add_success_part2"] = "' added successfully.";
        englishTexts["user_add_failed"] = "Addition failed, username or student ID may already exist.";
        englishTexts["student_query_prompt"] = "Enter student name or ID to search: ";
        englishTexts["student_not_found"] = "Student not found.";
        englishTexts["student_borrow_records"] = "Borrowing records for student ";
        englishTexts["multiple_students_found"] = "Found multiple students, please select one:";
        englishTexts["select_student_id_prompt"] = "Enter the ID to query: ";
        englishTexts["student_registration"] = "Student Registration";
        englishTexts["student_id_login_prompt"] = "Please enter your student ID (will be your login name): ";
        englishTexts["id_already_exists"] = "Error: This student ID is already registered.";
        englishTexts["real_name_prompt"] = "Please enter your real name: ";
        englishTexts["registration_success"] = "Registration successful! Please log in with your student ID.";
        englishTexts["registration_failed"] = "Registration failed, please try again later.";
        englishTexts["complete_personal_info"] = "Your personal information is incomplete, please complete it first to use borrowing functions.";
        englishTexts["info_update_failed"] = "Information update failed, logging out.";
        englishTexts["update_personal_info"] = "Complete/Modify Personal Information";
        englishTexts["current_name_prompt"] = "Your name (current: ";
        englishTexts["current_college_prompt"] = "Your college (current: ";
        englishTexts["current_class_prompt"] = "Your class (current: ";
        englishTexts["info_update_success"] = "Information updated successfully!";
        englishTexts["info_update_failed_msg"] = "Information update failed.";
        englishTexts["sort_method_prompt"] = "Choose sort method (1:by ID, 2:by Due Date): ";
        englishTexts["recovery_token_prompt"] = "Enter your recovery token: ";
        englishTexts["new_password_prompt"] = "Enter new password: ";
        englishTexts["confirm_password_prompt"] = "Enter new password again: ";
        englishTexts["password_mismatch"] = "Passwords do not match.";
        englishTexts["password_reset_success"] = "Password reset successful!";
        englishTexts["password_reset_failed"] = "Password reset failed. Please check your student ID and recovery token.";
        englishTexts["student_change_password"] = "Change Student Password";
        englishTexts["target_student_id_prompt"] = "Enter the student ID whose password to change: ";
        englishTexts["student_id_not_exist"] = "Error: This student ID does not exist.";
        englishTexts["password_change_success"] = "Password changed successfully!";
        englishTexts["password_change_failed"] = "Password change failed.";
        englishTexts["current_password_prompt"] = "Enter current password for verification: ";
        englishTexts["current_password_wrong"] = "Current password is incorrect!";
        englishTexts["student_password_change"] = "Change My Password";
        englishTexts["set_recovery_token"] = "Set/Update Recovery Token";
        englishTexts["token_already_set"] = "You have already set a recovery token. Updating will overwrite the old token.";
        englishTexts["enter_new_token"] = "Please enter a new recovery token (remember it well): ";
        englishTexts["token_empty_error"] = "Recovery token cannot be empty.";
        englishTexts["token_set_success"] = "Recovery token set successfully!";
        englishTexts["token_set_failed"] = "Recovery token setting failed.";
        englishTexts["my_personal_info"] = "My Personal Information";
        englishTexts["student_id_label"] = "Student ID: ";
        englishTexts["name_label"] = "Name: ";
        englishTexts["college_label"] = "College: ";
        englishTexts["class_label"] = "Class: ";
        englishTexts["role_label"] = "Role: ";
        englishTexts["recovery_token_status"] = "Recovery Token: ";
        englishTexts["token_set"] = "Set";
        englishTexts["token_not_set"] = "Not Set";
        englishTexts["no_books_found"] = "No books found.";
        englishTexts["no_borrow_records"] = "No borrowing records.";
        englishTexts["no_students"] = "No student users in the system.";
        englishTexts["no_borrow_records_found"] = "No borrowing records found.";
        englishTexts["first_run_setup"] = "First run setup: Admin account not found.";
        englishTexts["creating_default_admin"] = "Creating default admin account (username: admin, password: admin).";
        englishTexts["logging_out"] = "Logging out...";
        englishTexts["exiting_app"] = "Exiting...";
        englishTexts["sort_by_title"] = "1.Title";
        englishTexts["sort_by_author"] = "2.Author";
        englishTexts["sort_by_isbn"] = "3.ISBN";
        englishTexts["sort_by_id"] = "1.by ID";
        englishTexts["sort_by_due_date"] = "2.by Due Date";
        englishTexts["book_table_header_isbn"] = "ISBN";
        englishTexts["book_table_header_title"] = "Title";
        englishTexts["book_table_header_author"] = "Author";
        englishTexts["book_table_header_available"] = "Available";
        englishTexts["book_table_header_total"] = "Total";
        englishTexts["borrow_table_header_record_id"] = "Record ID";
        englishTexts["borrow_table_header_isbn"] = "ISBN";
        englishTexts["borrow_table_header_title"] = "Title";
        englishTexts["borrow_table_header_borrow_date"] = "Borrow Date";
        englishTexts["borrow_table_header_due_date"] = "Due Date";
        englishTexts["student_table_header_id"] = "ID";
        englishTexts["student_table_header_name"] = "Name";
        englishTexts["student_table_header_college"] = "College";
        englishTexts["student_table_header_class"] = "Class";
        englishTexts["full_borrow_table_header_student_id"] = "Student ID";
        englishTexts["full_borrow_table_header_name"] = "Name";
        englishTexts["full_borrow_table_header_college"] = "College";
        englishTexts["full_borrow_table_header_title"] = "Title";
        englishTexts["full_borrow_table_header_borrow_date"] = "Borrow Date";
        englishTexts["full_borrow_table_header_due_date"] = "Due Date";
        englishTexts["full_borrow_table_header_overdue"] = "Overdue";
        englishTexts["yes"] = "Yes";
        englishTexts["no"] = "No";
        
        // Chinese texts
        chineseTexts["welcome_main_menu"] = "欢迎使用图书管理系统";
        chineseTexts["login_option"] = "1. 登录";
        chineseTexts["register_option"] = "2. 学生注册";
        chineseTexts["forgot_password_option"] = "3. 忘记密码";
        chineseTexts["exit_option"] = "0. 退出";
        chineseTexts["choose_language"] = "4. 选择语言 / Choose Language";
        chineseTexts["english_option"] = "1. English";
        chineseTexts["chinese_option"] = "2. 中文";
        chineseTexts["enter_choice"] = "请输入您的选择: ";
        chineseTexts["invalid_choice"] = "无效的选择，请重试。";
        chineseTexts["exiting"] = "感谢使用，再见!";
        chineseTexts["admin_menu_title"] = "管理员菜单";
        chineseTexts["student_menu_title"] = "学生菜单";
        chineseTexts["book_management"] = "1. 图书管理 - 录入/修改/删除/查找";
        chineseTexts["user_management"] = "2. 用户管理 - 添加/修改密码";
        chineseTexts["borrow_management"] = "3. 借阅管理 - 查询学生/所有记录";
        chineseTexts["logout"] = "0. 退出登录";
        chineseTexts["add_new_book"] = "1. 录入新图书";
        chineseTexts["modify_book_info"] = "2. 修改图书信息";
        chineseTexts["delete_book"] = "3. 删除图书";
        chineseTexts["search_book"] = "4. 查找图书";
        chineseTexts["list_all_books"] = "5. 列出所有图书";
        chineseTexts["return_to_main"] = "0. 返回";
        chineseTexts["add_new_user"] = "1. 添加新用户";
        chineseTexts["change_student_password"] = "2. 修改学生密码";
        chineseTexts["query_specific_student"] = "1. 查询特定学生";
        chineseTexts["list_all_borrow_records"] = "2. 列出所有借阅记录";
        chineseTexts["search_book_prompt"] = "1. 查找图书";
        chineseTexts["borrow_book"] = "2. 借阅图书";
        chineseTexts["return_book"] = "3. 归还图书";
        chineseTexts["renew_book"] = "4. 续借图书";
        chineseTexts["view_my_borrowed"] = "5. 查看我的借阅";
        chineseTexts["account_management"] = "6. 账户管理 (查看信息/修改密码/设置口令)";
        chineseTexts["view_my_info"] = "1. 查看我的信息";
        chineseTexts["update_my_info"] = "2. 修改个人信息";
        chineseTexts["change_password"] = "3. 修改密码";
        chineseTexts["set_recovery_token"] = "4. 设置/更新找回密码口令";
        chineseTexts["username_prompt"] = "用户名 (管理员) 或 学号 (学生): ";
        chineseTexts["password_prompt"] = "密码: ";
        chineseTexts["login_failed"] = "用户名或密码错误。";
        chineseTexts["welcome_user"] = "欢迎, ";
        chineseTexts["overdue_notice"] = "!!! 注意: 您有已逾期的图书! !!!";
        chineseTexts["press_enter_continue"] = "按回车键继续...";
        chineseTexts["isbn_prompt"] = "ISBN: ";
        chineseTexts["title_prompt"] = "书名: ";
        chineseTexts["author_prompt"] = "作者: ";
        chineseTexts["publisher_prompt"] = "出版社: ";
        chineseTexts["category_prompt"] = "分类: ";
        chineseTexts["total_copies_prompt"] = "总数量: ";
        chineseTexts["book_added_success"] = "图书录入成功!";
        chineseTexts["book_add_failed"] = "录入失败。ISBN可能已存在。";
        chineseTexts["search_keyword_prompt"] = "输入查找关键词 (书名/作者/ISBN): ";
        chineseTexts["sort_by_prompt"] = "选择排序方式 (1:书名, 2:作者, 3:ISBN): ";
        chineseTexts["modify_isbn_prompt"] = "输入要修改图书的ISBN: ";
        chineseTexts["book_not_found"] = "未找到此ISBN的图书。";
        chineseTexts["modifying_book"] = "正在修改图书: ";
        chineseTexts["leave_blank_hint"] = "留空则不修改 '";
        chineseTexts["new_title_prompt"] = "新书名 (留空则不修改 '";
        chineseTexts["new_author_prompt"] = "新作者 (留空则不修改 '";
        chineseTexts["current_total_prompt"] = "新总数 (当前: ";
        chineseTexts["book_update_success"] = "图书信息更新成功!";
        chineseTexts["book_update_failed"] = "更新失败。";
        chineseTexts["delete_isbn_prompt"] = "输入要删除图书的ISBN: ";
        chineseTexts["book_delete_success"] = "图书删除成功。";
        chineseTexts["book_delete_failed"] = "删除失败。请检查该书是否仍有借阅记录。";
        chineseTexts["borrow_days_prompt"] = "您希望借阅多少天 (1-90天): ";
        chineseTexts["invalid_days"] = "无效的天数。";
        chineseTexts["borrow_success"] = "借阅成功!";
        chineseTexts["borrow_failed"] = "借阅失败。";
        chineseTexts["my_current_books"] = "您当前借阅的图书";
        chineseTexts["return_record_id_prompt"] = "输入要归还图书的记录ID: ";
        chineseTexts["return_success"] = "归还成功!";
        chineseTexts["return_failed"] = "归还失败。";
        chineseTexts["renew_success"] = "续借成功! 新的应还日期已更新。";
        chineseTexts["renew_failed"] = "续借失败。";
        chineseTexts["add_username_prompt"] = "输入用户名: ";
        chineseTexts["add_password_prompt"] = "输入密码: ";
        chineseTexts["add_role_prompt"] = "输入角色 (ADMIN 或 STUDENT): ";
        chineseTexts["student_id_prompt"] = "输入学号 (ID): ";
        chineseTexts["student_name_prompt"] = "输入姓名: ";
        chineseTexts["college_prompt"] = "输入学院: ";
        chineseTexts["class_prompt"] = "输入班级: ";
        chineseTexts["invalid_role"] = "无效的角色。必须是 ADMIN 或 STUDENT。";
        chineseTexts["user_add_success"] = "用户 '";
        chineseTexts["user_add_success_part2"] = "' 添加成功。";
        chineseTexts["user_add_failed"] = "添加失败，用户名或学号可能已存在。";
        chineseTexts["student_query_prompt"] = "输入学生姓名或学号进行查询: ";
        chineseTexts["student_not_found"] = "未找到该学生。";
        chineseTexts["student_borrow_records"] = "学生 ";
        chineseTexts["multiple_students_found"] = "找到多名学生，请选择一个:";
        chineseTexts["select_student_id_prompt"] = "请输入要查询的学号 (ID): ";
        chineseTexts["student_registration"] = "学生注册";
        chineseTexts["student_id_login_prompt"] = "请输入学号 (将作为您的登录名): ";
        chineseTexts["id_already_exists"] = "错误：该学号已被注册。";
        chineseTexts["real_name_prompt"] = "请输入您的真实姓名: ";
        chineseTexts["registration_success"] = "注册成功！请使用您的学号登录。";
        chineseTexts["registration_failed"] = "注册失败，请稍后重试。";
        chineseTexts["complete_personal_info"] = "您的个人信息不完整，请先完善信息以使用借阅功能。";
        chineseTexts["info_update_failed"] = "信息更新失败，将退出登录。";
        chineseTexts["update_personal_info"] = "完善/修改个人信息";
        chineseTexts["current_name_prompt"] = "您的姓名 (当前: ";
        chineseTexts["current_college_prompt"] = "您的学院 (当前: ";
        chineseTexts["current_class_prompt"] = "您的班级 (当前: ";
        chineseTexts["info_update_success"] = "信息更新成功！";
        chineseTexts["info_update_failed_msg"] = "信息更新失败。";
        chineseTexts["sort_method_prompt"] = "选择排序方式 (1:按学号, 2:按应还日期): ";
        chineseTexts["recovery_token_prompt"] = "请输入您设置的安全口令: ";
        chineseTexts["new_password_prompt"] = "请输入新密码: ";
        chineseTexts["confirm_password_prompt"] = "请再次输入新密码: ";
        chineseTexts["password_mismatch"] = "两次输入的密码不一致。";
        chineseTexts["password_reset_success"] = "密码重置成功！";
        chineseTexts["password_reset_failed"] = "密码重置失败。请检查您的学号和安全口令是否正确。";
        chineseTexts["student_change_password"] = "修改学生密码";
        chineseTexts["target_student_id_prompt"] = "请输入要修改密码的学生的学号: ";
        chineseTexts["student_id_not_exist"] = "错误：该学号不存在。";
        chineseTexts["password_change_success"] = "密码修改成功！";
        chineseTexts["password_change_failed"] = "密码修改失败。";
        chineseTexts["current_password_prompt"] = "请输入当前密码进行验证: ";
        chineseTexts["current_password_wrong"] = "当前密码错误！";
        chineseTexts["student_password_change"] = "修改我的密码";
        chineseTexts["set_recovery_token"] = "设置/更新找回密码口令";
        chineseTexts["token_already_set"] = "您已设置过安全口令。更新操作将覆盖旧口令。";
        chineseTexts["enter_new_token"] = "请输入新的安全口令 (请牢记): ";
        chineseTexts["token_empty_error"] = "安全口令不能为空。";
        chineseTexts["token_set_success"] = "安全口令设置成功！";
        chineseTexts["token_set_failed"] = "安全口令设置失败。";
        chineseTexts["my_personal_info"] = "我的个人信息";
        chineseTexts["student_id_label"] = "学号: ";
        chineseTexts["name_label"] = "姓名: ";
        chineseTexts["college_label"] = "学院: ";
        chineseTexts["class_label"] = "班级: ";
        chineseTexts["role_label"] = "角色: ";
        chineseTexts["recovery_token_status"] = "安全口令: ";
        chineseTexts["token_set"] = "已设置";
        chineseTexts["token_not_set"] = "未设置";
        chineseTexts["no_books_found"] = "未找到相关图书。";
        chineseTexts["no_borrow_records"] = "没有借阅记录。";
        chineseTexts["no_students"] = "系统中没有学生用户。";
        chineseTexts["no_borrow_records_found"] = "没有找到任何借阅记录。";
        chineseTexts["first_run_setup"] = "首次运行设置: 未找到管理员账户。";
        chineseTexts["creating_default_admin"] = "正在创建默认管理员账户 (用户名: admin, 密码: admin)。";
        chineseTexts["logging_out"] = "正在退出...";
        chineseTexts["exiting_app"] = "正在退出...";
        chineseTexts["sort_by_title"] = "1:书名";
        chineseTexts["sort_by_author"] = "2:作者";
        chineseTexts["sort_by_isbn"] = "3:ISBN";
        chineseTexts["sort_by_id"] = "1:按学号";
        chineseTexts["sort_by_due_date"] = "2:按应还日期";
        chineseTexts["book_table_header_isbn"] = "ISBN";
        chineseTexts["book_table_header_title"] = "书名";
        chineseTexts["book_table_header_author"] = "作者";
        chineseTexts["book_table_header_available"] = "可借";
        chineseTexts["book_table_header_total"] = "总数";
        chineseTexts["borrow_table_header_record_id"] = "记录ID";
        chineseTexts["borrow_table_header_isbn"] = "ISBN";
        chineseTexts["borrow_table_header_title"] = "书名";
        chineseTexts["borrow_table_header_borrow_date"] = "借阅日期";
        chineseTexts["borrow_table_header_due_date"] = "应还日期";
        chineseTexts["student_table_header_id"] = "学号";
        chineseTexts["student_table_header_name"] = "姓名";
        chineseTexts["student_table_header_college"] = "学院";
        chineseTexts["student_table_header_class"] = "班级";
        chineseTexts["full_borrow_table_header_student_id"] = "学号";
        chineseTexts["full_borrow_table_header_name"] = "姓名";
        chineseTexts["full_borrow_table_header_college"] = "学院";
        chineseTexts["full_borrow_table_header_title"] = "书名";
        chineseTexts["full_borrow_table_header_borrow_date"] = "借阅日期";
        chineseTexts["full_borrow_table_header_due_date"] = "应还日期";
        chineseTexts["full_borrow_table_header_overdue"] = "逾期";
        chineseTexts["yes"] = "是";
        chineseTexts["no"] = "否";
        englishTexts["language_set_to_english"] = "Language has been set to English.";
        englishTexts["language_set_to_chinese"] = "Language has been set to Chinese.";
        chineseTexts["language_set_to_english"] = "语言已设置为英文。";
        chineseTexts["language_set_to_chinese"] = "语言已设置为中文。";
        
        // Pagination related strings
        englishTexts["pagination_option_prompt"] = "Choose display option:";
        englishTexts["show_all_at_once"] = "1. Show all at once";
        englishTexts["show_with_pagination"] = "2. Show with pagination";
        englishTexts["page_size_prompt"] = "Enter page size";
        englishTexts["all_books_list"] = "All Books List";
        englishTexts["no_more_books"] = "No more books to display.";
        englishTexts["show_next_page_or_exit"] = "Press Enter for next page, or 'q' to quit: ";
        englishTexts["end_of_list"] = "End of list reached.";
        
        chineseTexts["pagination_option_prompt"] = "选择显示选项:";
        chineseTexts["show_all_at_once"] = "1. 一次性显示全部";
        chineseTexts["show_with_pagination"] = "2. 分页显示";
        chineseTexts["page_size_prompt"] = "输入每页大小";
        chineseTexts["all_books_list"] = "全部图书列表";
        chineseTexts["no_more_books"] = "没有更多图书可显示。";
        chineseTexts["show_next_page_or_exit"] = "按回车键查看下一页，或输入 'q' 退出: ";
        chineseTexts["end_of_list"] = "已到达列表末尾。";
    }

    Language currentLanguage = Language::CHINESE;
    std::string configFilePath = "config.ini";
    std::map<std::string, std::string> englishTexts;
    std::map<std::string, std::string> chineseTexts;
};

#define _(key) Localization::getInstance().getText(key)

#endif // LOCALIZATION_H