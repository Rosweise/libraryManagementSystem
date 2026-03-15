# 图书馆管理系统 API 文档

本文档描述了图书馆管理系统 (libraryManagementSystem) 的核心 API 接口和数据结构。

## 目录

- [数据结构](#数据结构)
- [DatabaseManager 类](#databasemanager-类)
- [SHA256 类](#sha256-类)
- [工具函数](#工具函数)

---

## 数据结构

### Book (图书)

图书信息结构体，用于存储和管理图书数据。

```cpp
struct Book {
    std::string isbn;           // ISBN 编号
    std::string title;          // 书名
    std::string author;         // 作者
    std::string publisher;      // 出版社
    std::string category;       // 分类
    int totalCopies;            // 总藏书量
    int availableCopies;        // 可借数量
};
```

### User (用户)

用户信息结构体，支持管理员和学生两种角色。

```cpp
struct User {
    std::string id;             // 学号/管理员 ID
    std::string username;       // 登录用户名
    std::string name;           // 真实姓名
    std::string college;        // 学院
    std::string className;      // 班级
    std::string role;           // "ADMIN" 或 "STUDENT"
    bool hasRecoveryToken;      // 是否设置了安全口令
};
```

### BorrowRecord (借阅记录)

基础借阅记录结构体。

```cpp
struct BorrowRecord {
    int recordId;               // 记录 ID
    std::string userId;         // 用户 ID
    std::string bookIsbn;       // 图书 ISBN
    std::string bookTitle;      // 书名
    std::string borrowDate;     // 借阅日期 (YYYY-MM-DD)
    std::string dueDate;        // 应还日期 (YYYY-MM-DD)
    std::string returnDate;     // 归还日期 (YYYY-MM-DD)
};
```

### FullBorrowRecord (完整借阅记录)

包含完整学生信息和逾期状态的借阅记录。

```cpp
struct FullBorrowRecord {
    int recordId;               // 记录 ID
    std::string studentId;      // 学号
    std::string studentName;    // 学生姓名
    std::string studentCollege; // 学院
    std::string studentClass;   // 班级
    std::string bookTitle;      // 书名
    std::string borrowDate;     // 借阅日期
    std::string dueDate;        // 应还日期
    bool isOverdue;             // 是否逾期
};
```

---

## DatabaseManager 类

数据库管理类，负责所有数据库操作。

### 构造与析构

#### DatabaseManager(std::string db_path)

构造函数，初始化数据库管理器。

**参数:**
- `db_path`: 数据库文件路径

**示例:**
```cpp
DatabaseManager db("library.db");
```

#### ~DatabaseManager()

析构函数，关闭数据库连接。

---

### 初始化

#### initialize()

初始化数据库，创建必要的表结构。

**返回值:**
- `true`: 初始化成功
- `false`: 初始化失败

**功能:**
- 创建 `Users` 表 (用户表)
- 创建 `Books` 表 (图书表)
- 创建 `BorrowingRecords` 表 (借阅记录表)
- 如果不存在管理员账户，创建默认管理员 (admin/admin)

**示例:**
```cpp
if (!db.initialize()) {
    std::cerr << "数据库初始化失败" << std::endl;
    return 1;
}
```

---

### 用户管理 API

#### addUser(const User &user, const std::string &password)

添加新用户。

**参数:**
- `user`: 用户信息结构体
- `password`: 用户密码 (将使用 SHA-256 加密存储)

**返回值:**
- `true`: 添加成功
- `false`: 添加失败 (用户名或学号已存在)

**示例:**
```cpp
User newUser;
newUser.id = "2024001";
newUser.username = "2024001";
newUser.name = "张三";
newUser.college = "计算机学院";
newUser.className = "计算机 1 班";
newUser.role = "STUDENT";
db.addUser(newUser, "password123");
```

#### userExists(const std::string &username)

检查用户是否存在。

**参数:**
- `username`: 用户名

**返回值:**
- `true`: 用户存在
- `false`: 用户不存在

#### authenticateUser(const std::string &username, const std::string &password)

用户登录认证。

**参数:**
- `username`: 用户名
- `password`: 密码

**返回值:**
- `User`: 认证成功的用户信息 (role 不为空)
- `User`: 认证失败 (role 为空字符串)

**示例:**
```cpp
User user = db.authenticateUser("admin", "admin");
if (user.role == "ADMIN") {
    // 显示管理员菜单
} else if (user.role == "STUDENT") {
    // 显示学生菜单
} else {
    std::cout << "用户名或密码错误" << std::endl;
}
```

#### updateStudentInfo(const User &user)

更新学生信息。

**参数:**
- `user`: 包含新信息的用户结构体

**返回值:**
- `true`: 更新成功
- `false`: 更新失败

#### updatePassword(const std::string &username, const std::string &newPassword)

更新用户密码。

**参数:**
- `username`: 用户名
- `newPassword`: 新密码

**返回值:**
- `true`: 更新成功
- `false`: 更新失败

#### updateRecoveryToken(const std::string &username, const std::string &token)

更新用户安全口令。

**参数:**
- `username`: 用户名
- `token`: 安全口令 (将使用 SHA-256 加密存储)

**返回值:**
- `true`: 更新成功
- `false`: 更新失败

#### recoverPassword(const std::string &username, const std::string &token, const std::string &newPassword)

通过安全口令找回密码。

**参数:**
- `username`: 用户名
- `token`: 安全口令
- `newPassword`: 新密码

**返回值:**
- `true`: 密码重置成功
- `false`: 重置失败 (用户名或口令错误)

---

### 图书管理 API

#### addBook(const Book &book)

添加新图书。

**参数:**
- `book`: 图书信息结构体

**返回值:**
- `true`: 添加成功
- `false`: 添加失败 (ISBN 已存在)

**示例:**
```cpp
Book book;
book.isbn = "978-7-111-40701-0";
book.title = "计算机系统基础";
book.author = "Randal E. Bryant";
book.publisher = "机械工业出版社";
book.category = "计算机";
book.totalCopies = 5;
book.availableCopies = 5;
db.addBook(book);
```

#### updateBook(const Book &book)

更新图书信息。

**参数:**
- `book`: 包含新信息的图书结构体 (通过 ISBN 匹配)

**返回值:**
- `true`: 更新成功
- `false`: 更新失败

#### deleteBook(const std::string &isbn)

删除图书。

**参数:**
- `isbn`: 图书 ISBN

**返回值:**
- `true`: 删除成功
- `false`: 删除失败 (图书不存在或仍有借阅记录)

#### findBooks(const std::string &keyword, const std::string &sortBy)

查找图书。

**参数:**
- `keyword`: 搜索关键词 (支持书名/作者/ISBN)
- `sortBy`: 排序方式 ("title", "author", "isbn")

**返回值:**
- `std::vector<Book>`: 匹配的图书列表

**示例:**
```cpp
auto books = db.findBooks("C++", "title");
```

#### getAllBooks(const std::string &sortBy)

获取所有图书。

**参数:**
- `sortBy`: 排序方式 ("title", "author", "isbn")

**返回值:**
- `std::vector<Book>`: 所有图书列表

---

### 借阅管理 API

#### borrowBook(const std::string &userId, const std::string &isbn, int daysToBorrow)

借阅图书。

**参数:**
- `userId`: 用户 ID (学号)
- `isbn`: 图书 ISBN
- `daysToBorrow`: 借阅天数 (1-90 天)

**返回值:**
- `true`: 借阅成功
- `false`: 借阅失败 (图书不可借或 ISBN 错误)

**功能:**
- 检查图书可借数量
- 减少可借数量
- 创建借阅记录

#### returnBook(int recordId, const std::string &userId)

归还图书。

**参数:**
- `recordId`: 借阅记录 ID
- `userId`: 用户 ID

**返回值:**
- `true`: 归还成功
- `false`: 归还失败 (记录无效或用户不匹配)

**功能:**
- 设置归还日期
- 增加图书可借数量

#### renewBook(int recordId, const std::string &userId)

续借图书。

**参数:**
- `recordId`: 借阅记录 ID
- `userId`: 用户 ID

**返回值:**
- `true`: 续借成功
- `false`: 续借失败 (记录无效)

**功能:**
- 将应还日期延长 30 天

#### getBorrowedBooksByUser(const std::string &userId)

获取用户当前借阅的图书。

**参数:**
- `userId`: 用户 ID

**返回值:**
- `std::vector<BorrowRecord>`: 借阅记录列表 (未归还)

#### getOverdueBooksByUser(const std::string &userId)

获取用户逾期图书。

**参数:**
- `userId`: 用户 ID

**返回值:**
- `std::vector<BorrowRecord>`: 逾期借阅记录列表

---

### 管理员查询 API

#### getAllStudents()

获取所有学生用户。

**返回值:**
- `std::vector<User>`: 所有学生列表

#### findStudents(const std::string &keyword)

搜索学生。

**参数:**
- `keyword`: 搜索关键词 (学号/用户名/姓名)

**返回值:**
- `std::vector<User>`: 匹配的学生列表

#### getFullBorrowRecordsForUser(const std::string &userId)

获取特定学生的完整借阅记录。

**参数:**
- `userId`: 学生 ID

**返回值:**
- `std::vector<FullBorrowRecord>`: 完整借阅记录列表

#### getAllFullBorrowRecords(const std::string &sortBy)

获取所有借阅记录。

**参数:**
- `sortBy`: 排序方式 ("studentId", "dueDate")

**返回值:**
- `std::vector<FullBorrowRecord>`: 所有借阅记录列表

---

## SHA256 类

SHA-256 加密工具类。

### hash(const std::string& input)

计算输入字符串的 SHA-256 哈希值。

**参数:**
- `input`: 输入字符串

**返回值:**
- `std::string`: 64 位十六进制哈希字符串

**示例:**
```cpp
std::string hashed = SHA256::hash("password123");
// 输出：ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f
```

**用途:**
- 密码加密存储
- 安全口令加密存储

---

## 工具函数

### getTerminalWidth()

获取终端窗口宽度。

**返回值:**
- `int`: 终端宽度 (字符数)

**平台支持:**
- Windows: 使用 Windows API
- Linux/macOS: 使用 ioctl

### clearScreen()

清理终端屏幕。

**平台支持:**
- Windows: 执行 `cls`
- Linux/macOS: 执行 `clear`

### getDisplayWidth(const std::string& str)

获取 UTF-8 字符串的显示宽度。

**参数:**
- `str`: 输入字符串

**返回值:**
- `int`: 显示宽度 (ASCII 字符=1, CJK 字符=2)

### formatCell(const std::string& str, int width)

格式化单元格字符串，用于表格显示。

**参数:**
- `str`: 输入字符串
- `width`: 目标宽度

**返回值:**
- `std::string`: 格式化后的字符串 (超出截断并添加"...", 不足则填充空格)

---

## 数据库表结构

### Users 表

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | TEXT | PRIMARY KEY | 学号/管理员 ID |
| username | TEXT | UNIQUE NOT NULL | 登录用户名 |
| password_hash | TEXT | NOT NULL | SHA-256 加密的密码 |
| name | TEXT | | 真实姓名 |
| college | TEXT | | 学院 |
| className | TEXT | | 班级 |
| role | TEXT | NOT NULL | "ADMIN" 或 "STUDENT" |
| recovery_token_hash | TEXT | | SHA-256 加密的安全口令 |

### Books 表

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| isbn | TEXT | PRIMARY KEY | ISBN 编号 |
| title | TEXT | NOT NULL | 书名 |
| author | TEXT | | 作者 |
| publisher | TEXT | | 出版社 |
| category | TEXT | | 分类 |
| totalCopies | INTEGER | NOT NULL | 总藏书量 |
| availableCopies | INTEGER | NOT NULL | 可借数量 |

### BorrowingRecords 表

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| recordId | INTEGER | PRIMARY KEY AUTOINCREMENT | 记录 ID |
| userId | TEXT | NOT NULL | 用户 ID (外键) |
| bookIsbn | TEXT | NOT NULL | 图书 ISBN (外键) |
| borrowDate | TEXT | NOT NULL | 借阅日期 |
| dueDate | TEXT | NOT NULL | 应还日期 |
| returnDate | TEXT | | 归还日期 (NULL 表示未归还) |

---

## 使用示例

### 完整使用流程

```cpp
#include "header/database.h"
#include "header/sha256.h"
#include <iostream>

int main() {
    // 1. 创建数据库管理器
    DatabaseManager db("library.db");
    
    // 2. 初始化数据库
    if (!db.initialize()) {
        std::cerr << "初始化失败" << std::endl;
        return 1;
    }
    
    // 3. 管理员登录
    User admin = db.authenticateUser("admin", "admin");
    if (admin.role != "ADMIN") {
        std::cerr << "登录失败" << std::endl;
        return 1;
    }
    
    // 4. 添加图书
    Book book;
    book.isbn = "978-7-111-40701-0";
    book.title = "计算机系统基础";
    book.author = "Randal E. Bryant";
    book.publisher = "机械工业出版社";
    book.category = "计算机";
    book.totalCopies = 5;
    book.availableCopies = 5;
    db.addBook(book);
    
    // 5. 添加学生用户
    User student;
    student.id = "2024001";
    student.username = "2024001";
    student.name = "张三";
    student.college = "计算机学院";
    student.className = "计算机 1 班";
    student.role = "STUDENT";
    db.addUser(student, "password123");
    
    // 6. 学生借阅图书
    db.borrowBook("2024001", "978-7-111-40701-0", 30);
    
    // 7. 查询学生借阅记录
    auto records = db.getBorrowedBooksByUser("2024001");
    for (const auto& rec : records) {
        std::cout << "书名：" << rec.bookTitle 
                  << ", 应还日期：" << rec.dueDate << std::endl;
    }
    
    // 8. 归还图书
    if (!records.empty()) {
        db.returnBook(records[0].recordId, "2024001");
    }
    
    return 0;
}
```

---

## 注意事项

1. **线程安全**: 当前实现不支持多线程并发访问
2. **事务处理**: 借阅和归还操作使用事务保证数据一致性
3. **密码安全**: 所有密码和安全口令均使用 SHA-256 加密存储
4. **外键约束**: 借阅记录依赖用户和图书表，删除前需检查关联记录
5. **日期格式**: 所有日期均使用 `YYYY-MM-DD` 格式

---

## 版本信息

- **项目版本**: 1.0
- **C++ 标准**: C++20
- **数据库**: SQLite3
- **加密算法**: SHA-256

---

<p align="center">Made with ❤️ for <a href="https://github.com/Rouxiv/libraryManagementSystem">libraryManagementSystem</a></p>
