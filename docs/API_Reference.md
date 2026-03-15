# Library Management System API Documentation

This document describes the core API interfaces and data structures of the Library Management System (libraryManagementSystem).

## Table of Contents

- [Data Structures](#data-structures)
- [DatabaseManager Class](#databasemanager-class)
- [SHA256 Class](#sha256-class)
- [Utility Functions](#utility-functions)

---

## Data Structures

### Book

Book structure for storing and managing book data.

```cpp
struct Book {
    std::string isbn;           // ISBN number
    std::string title;          // Book title
    std::string author;         // Author
    std::string publisher;      // Publisher
    std::string category;       // Category
    int totalCopies;            // Total copies
    int availableCopies;        // Available copies for borrowing
};
```

### User

User information structure supporting administrator and student roles.

```cpp
struct User {
    std::string id;             // Student ID / Admin ID
    std::string username;       // Login username
    std::string name;           // Real name
    std::string college;        // College/School
    std::string className;      // Class
    std::string role;           // "ADMIN" or "STUDENT"
    bool hasRecoveryToken;      // Whether recovery token is set
};
```

### BorrowRecord

Basic borrowing record structure.

```cpp
struct BorrowRecord {
    int recordId;               // Record ID
    std::string userId;         // User ID
    std::string bookIsbn;       // Book ISBN
    std::string bookTitle;      // Book title
    std::string borrowDate;     // Borrow date (YYYY-MM-DD)
    std::string dueDate;        // Due date (YYYY-MM-DD)
    std::string returnDate;     // Return date (YYYY-MM-DD)
};
```

### FullBorrowRecord

Complete borrowing record with student information and overdue status.

```cpp
struct FullBorrowRecord {
    int recordId;               // Record ID
    std::string studentId;      // Student ID
    std::string studentName;    // Student name
    std::string studentCollege; // College/School
    std::string studentClass;   // Class
    std::string bookTitle;      // Book title
    std::string borrowDate;     // Borrow date
    std::string dueDate;        // Due date
    bool isOverdue;             // Is overdue
};
```

---

## DatabaseManager Class

Database management class responsible for all database operations.

### Constructor and Destructor

#### DatabaseManager(std::string db_path)

Constructor that initializes the database manager.

**Parameters:**
- `db_path`: Path to the database file

**Example:**
```cpp
DatabaseManager db("library.db");
```

#### ~DatabaseManager()

Destructor that closes the database connection.

---

### Initialization

#### initialize()

Initializes the database and creates necessary table structures.

**Returns:**
- `true`: Initialization successful
- `false`: Initialization failed

**Functionality:**
- Creates `Users` table
- Creates `Books` table
- Creates `BorrowingRecords` table
- Creates default admin account if not exists (admin/admin)

**Example:**
```cpp
if (!db.initialize()) {
    std::cerr << "Database initialization failed" << std::endl;
    return 1;
}
```

---

### User Management API

#### addUser(const User &user, const std::string &password)

Adds a new user.

**Parameters:**
- `user`: User information structure
- `password`: User password (will be SHA-256 encrypted)

**Returns:**
- `true`: Successfully added
- `false`: Failed (username or student ID already exists)

**Example:**
```cpp
User newUser;
newUser.id = "2024001";
newUser.username = "2024001";
newUser.name = "Zhang San";
newUser.college = "School of Computer Science";
newUser.className = "CS Class 1";
newUser.role = "STUDENT";
db.addUser(newUser, "password123");
```

#### userExists(const std::string &username)

Checks if a user exists.

**Parameters:**
- `username`: Username

**Returns:**
- `true`: User exists
- `false`: User does not exist

#### authenticateUser(const std::string &username, const std::string &password)

Authenticates user login.

**Parameters:**
- `username`: Username
- `password`: Password

**Returns:**
- `User`: User information on success (role is not empty)
- `User`: Authentication failed (role is empty string)

**Example:**
```cpp
User user = db.authenticateUser("admin", "admin");
if (user.role == "ADMIN") {
    // Show admin menu
} else if (user.role == "STUDENT") {
    // Show student menu
} else {
    std::cout << "Invalid username or password" << std::endl;
}
```

#### updateStudentInfo(const User &user)

Updates student information.

**Parameters:**
- `user`: User structure with new information

**Returns:**
- `true`: Update successful
- `false`: Update failed

#### updatePassword(const std::string &username, const std::string &newPassword)

Updates user password.

**Parameters:**
- `username`: Username
- `newPassword`: New password

**Returns:**
- `true`: Update successful
- `false`: Update failed

#### updateRecoveryToken(const std::string &username, const std::string &token)

Updates user recovery token.

**Parameters:**
- `username`: Username
- `token`: Recovery token (will be SHA-256 encrypted)

**Returns:**
- `true`: Update successful
- `false`: Update failed

#### recoverPassword(const std::string &username, const std::string &token, const std::string &newPassword)

Recovers password using recovery token.

**Parameters:**
- `username`: Username
- `token`: Recovery token
- `newPassword`: New password

**Returns:**
- `true`: Password reset successful
- `false`: Reset failed (invalid username or token)

---

### Book Management API

#### addBook(const Book &book)

Adds a new book.

**Parameters:**
- `book`: Book information structure

**Returns:**
- `true`: Successfully added
- `false`: Failed (ISBN already exists)

**Example:**
```cpp
Book book;
book.isbn = "978-7-111-40701-0";
book.title = "Computer Systems: A Programmer's Perspective";
book.author = "Randal E. Bryant";
book.publisher = "Pearson";
book.category = "Computer Science";
book.totalCopies = 5;
book.availableCopies = 5;
db.addBook(book);
```

#### updateBook(const Book &book)

Updates book information.

**Parameters:**
- `book`: Book structure with new information (matched by ISBN)

**Returns:**
- `true`: Update successful
- `false`: Update failed

#### deleteBook(const std::string &isbn)

Deletes a book.

**Parameters:**
- `isbn`: Book ISBN

**Returns:**
- `true`: Deletion successful
- `false`: Deletion failed (book doesn't exist or has active borrowings)

#### findBooks(const std::string &keyword, const std::string &sortBy)

Searches for books.

**Parameters:**
- `keyword`: Search keyword (supports title/author/ISBN)
- `sortBy`: Sort order ("title", "author", "isbn")

**Returns:**
- `std::vector<Book>`: List of matching books

**Example:**
```cpp
auto books = db.findBooks("C++", "title");
```

#### getAllBooks(const std::string &sortBy)

Gets all books.

**Parameters:**
- `sortBy`: Sort order ("title", "author", "isbn")

**Returns:**
- `std::vector<Book>`: List of all books

---

### Borrowing Management API

#### borrowBook(const std::string &userId, const std::string &isbn, int daysToBorrow)

Borrows a book.

**Parameters:**
- `userId`: User ID (student ID)
- `isbn`: Book ISBN
- `daysToBorrow`: Borrowing days (1-90 days)

**Returns:**
- `true`: Borrowing successful
- `false`: Borrowing failed (book unavailable or invalid ISBN)

**Functionality:**
- Checks book availability
- Decrements available copies
- Creates borrowing record

#### returnBook(int recordId, const std::string &userId)

Returns a borrowed book.

**Parameters:**
- `recordId`: Borrowing record ID
- `userId`: User ID

**Returns:**
- `true`: Return successful
- `false`: Return failed (invalid record or user mismatch)

**Functionality:**
- Sets return date
- Increments book available copies

#### renewBook(int recordId, const std::string &userId)

Renews a borrowed book.

**Parameters:**
- `recordId`: Borrowing record ID
- `userId`: User ID

**Returns:**
- `true`: Renewal successful
- `false`: Renewal failed (invalid record)

**Functionality:**
- Extends due date by 30 days

#### getBorrowedBooksByUser(const std::string &userId)

Gets books currently borrowed by a user.

**Parameters:**
- `userId`: User ID

**Returns:**
- `std::vector<BorrowRecord>`: List of borrowing records (not returned)

#### getOverdueBooksByUser(const std::string &userId)

Gets overdue books for a user.

**Parameters:**
- `userId`: User ID

**Returns:**
- `std::vector<BorrowRecord>`: List of overdue borrowing records

---

### Administrator Query API

#### getAllStudents()

Gets all student users.

**Returns:**
- `std::vector<User>`: List of all students

#### findStudents(const std::string &keyword)

Searches for students.

**Parameters:**
- `keyword`: Search keyword (student ID/username/name)

**Returns:**
- `std::vector<User>`: List of matching students

#### getFullBorrowRecordsForUser(const std::string &userId)

Gets complete borrowing records for a specific student.

**Parameters:**
- `userId`: Student ID

**Returns:**
- `std::vector<FullBorrowRecord>`: List of complete borrowing records

#### getAllFullBorrowRecords(const std::string &sortBy)

Gets all borrowing records.

**Parameters:**
- `sortBy`: Sort order ("studentId", "dueDate")

**Returns:**
- `std::vector<FullBorrowRecord>`: List of all borrowing records

---

## SHA256 Class

SHA-256 encryption utility class.

### hash(const std::string& input)

Computes SHA-256 hash of the input string.

**Parameters:**
- `input`: Input string

**Returns:**
- `std::string`: 64-character hexadecimal hash string

**Example:**
```cpp
std::string hashed = SHA256::hash("password123");
// Output: ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f
```

**Usage:**
- Password encryption for storage
- Recovery token encryption

---

## Utility Functions

### getTerminalWidth()

Gets the terminal window width.

**Returns:**
- `int`: Terminal width in characters

**Platform Support:**
- Windows: Uses Windows API
- Linux/macOS: Uses ioctl

### clearScreen()

Clears the terminal screen.

**Platform Support:**
- Windows: Executes `cls`
- Linux/macOS: Executes `clear`

### getDisplayWidth(const std::string& str)

Gets the display width of a UTF-8 string.

**Parameters:**
- `str`: Input string

**Returns:**
- `int`: Display width (ASCII chars=1, CJK chars=2)

### formatCell(const std::string& str, int width)

Formats a cell string for table display.

**Parameters:**
- `str`: Input string
- `width`: Target width

**Returns:**
- `std::string`: Formatted string (truncated with "..." if too long, padded with spaces if too short)

---

## Database Schema

### Users Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| id | TEXT | PRIMARY KEY | Student ID / Admin ID |
| username | TEXT | UNIQUE NOT NULL | Login username |
| password_hash | TEXT | NOT NULL | SHA-256 encrypted password |
| name | TEXT | | Real name |
| college | TEXT | | College/School |
| className | TEXT | | Class |
| role | TEXT | NOT NULL | "ADMIN" or "STUDENT" |
| recovery_token_hash | TEXT | | SHA-256 encrypted recovery token |

### Books Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| isbn | TEXT | PRIMARY KEY | ISBN number |
| title | TEXT | NOT NULL | Book title |
| author | TEXT | | Author |
| publisher | TEXT | | Publisher |
| category | TEXT | | Category |
| totalCopies | INTEGER | NOT NULL | Total copies |
| availableCopies | INTEGER | NOT NULL | Available copies |

### BorrowingRecords Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| recordId | INTEGER | PRIMARY KEY AUTOINCREMENT | Record ID |
| userId | TEXT | NOT NULL | User ID (foreign key) |
| bookIsbn | TEXT | NOT NULL | Book ISBN (foreign key) |
| borrowDate | TEXT | NOT NULL | Borrow date |
| dueDate | TEXT | NOT NULL | Due date |
| returnDate | TEXT | | Return date (NULL if not returned) |

---

## Usage Example

### Complete Workflow

```cpp
#include "header/database.h"
#include "header/sha256.h"
#include <iostream>

int main() {
    // 1. Create database manager
    DatabaseManager db("library.db");
    
    // 2. Initialize database
    if (!db.initialize()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // 3. Admin login
    User admin = db.authenticateUser("admin", "admin");
    if (admin.role != "ADMIN") {
        std::cerr << "Login failed" << std::endl;
        return 1;
    }
    
    // 4. Add a book
    Book book;
    book.isbn = "978-7-111-40701-0";
    book.title = "Computer Systems: A Programmer's Perspective";
    book.author = "Randal E. Bryant";
    book.publisher = "Pearson";
    book.category = "Computer Science";
    book.totalCopies = 5;
    book.availableCopies = 5;
    db.addBook(book);
    
    // 5. Add a student user
    User student;
    student.id = "2024001";
    student.username = "2024001";
    student.name = "Zhang San";
    student.college = "School of Computer Science";
    student.className = "CS Class 1";
    student.role = "STUDENT";
    db.addUser(student, "password123");
    
    // 6. Student borrows a book
    db.borrowBook("2024001", "978-7-111-40701-0", 30);
    
    // 7. Query student's borrowing records
    auto records = db.getBorrowedBooksByUser("2024001");
    for (const auto& rec : records) {
        std::cout << "Title: " << rec.bookTitle 
                  << ", Due Date: " << rec.dueDate << std::endl;
    }
    
    // 8. Return the book
    if (!records.empty()) {
        db.returnBook(records[0].recordId, "2024001");
    }
    
    return 0;
}
```

---

## Notes

1. **Thread Safety**: Current implementation does not support multi-threaded concurrent access
2. **Transaction Handling**: Borrowing and return operations use transactions to ensure data consistency
3. **Password Security**: All passwords and recovery tokens are SHA-256 encrypted before storage
4. **Foreign Key Constraints**: Borrowing records depend on users and books tables; check associations before deletion
5. **Date Format**: All dates use `YYYY-MM-DD` format

---

## Version Information

- **Project Version**: 1.0
- **C++ Standard**: C++20
- **Database**: SQLite3
- **Encryption Algorithm**: SHA-256

---

<p align="center">Made with ❤️ for <a href="https://github.com/Rouxiv/libraryManagementSystem">libraryManagementSystem</a></p>
