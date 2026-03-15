<h1 align="center">libraryManagementSystem</h1>

<p align="center">
  <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cplusplus/cplusplus-original.svg" alt="C++" width="50" height="50"/>
  &nbsp;&nbsp;&nbsp;
  <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/sqlite/sqlite-original.svg" alt="SQLite" width="50" height="50"/>
  &nbsp;&nbsp;&nbsp;
  <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cmake/cmake-original.svg" alt="CMake" width="50" height="50"/>
</p>

<p align="center">A cross-platform library management system based on C++20 and SQLite</p>

<p align="center">
  <a href="#table-of-contents">Overview</a> •
  <a href="#features">Features</a> •
  <a href="#building">Building</a> •
  <a href="#usage">Usage</a> •
  <a href="#project-structure">Structure</a>
</p>

<p align="center">
  <strong>🇺🇸 <a href="README.md">English</a></strong> | 
  <strong>🇨🇳 <a href="docs/README_zh-CN.md">简体中文</a></strong>
</p>


---

## Overview

**libraryManagementSystem** is a terminal-based library management system built with SQLite database. Developed using C++20 standard and CMake build system, it supports Windows, Linux, and macOS operating systems.

The system implements role-based access control with two user types:

| Role | Functions |
|------|-----------|
| **Administrator** | Book management (CRUD), user management, borrowing record queries |
| **Student** | Book search, borrow/return/renew books, personal account management |

---

## Features

| Feature | Description |
|---------|-------------|
| Cross-Platform | Native support for Windows, Linux, macOS |
| Zero Dependencies | Built-in SQLite, no external database installation required |
| Secure Passwords | SHA-256 encrypted password storage |
| Security Token | Support for password recovery security tokens |
| Overdue Alerts | Automatic detection and notification of overdue books |
| Adaptive UI | Tables automatically adjust to terminal width |

---

## Building

### Prerequisites

| Dependency | Version |
|------------|---------|
| CMake | >= 3.10 |
| C++ Compiler | C++20 support |

Compiler options by platform:
- **Windows**: MinGW-w64 or Visual Studio 2019+
- **Linux**: g++ >= 9.0
- **macOS**: Apple Clang or gcc

Check CMake installation:
```bash
cmake --version
```

---

### Method 1: CMake Command Line

#### Linux / macOS

```bash
# Navigate to project directory
cd /path/to/libraryManagementSystem

# Create build directory and compile
cmake -B build && cmake --build build

# Run the program
cd build
./lib
```

#### Windows (MinGW-w64)

```bash
# Navigate to project directory
cd /d /path/to/libraryManagementSystem

# Create build directory and compile
cmake -B build -G "MinGW Makefiles" && cmake --build build

# Run the program
cd build
lib.exe
```

#### Windows (Visual Studio)

```bash
# Create Visual Studio solution
cmake -B build -G "Visual Studio 17 2022"

# Build (or open build directory in IDE)
cmake --build build --config Release

# Run the program
build\Release\lib.exe
```

---

### Method 2: IDE

| IDE | Steps |
|-----|-------|
| **CLion** | Open project → Auto-detect CMake → Click Run |
| **Visual Studio** | Open folder → Select `lib.exe` as startup → Run |

> **Note**: For legacy IDEs like VC++ 6.0, consider using CMake or modern IDEs

---

### Method 3: Cross-Compilation

Compile Windows executables on macOS/Linux:

#### macOS

```bash
# Install dependencies
brew install cmake mingw-w64

# Create build directory
mkdir build && cd build

# Configure cross-compilation
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Windows-x86.cmake

# Build
cmake --build .
```

#### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y cmake g++-mingw-w64-x86-64 gcc-mingw-w64-x86-64

# Create build directory
mkdir build && cd build

# Configure cross-compilation
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Windows-x86.cmake

# Build
cmake --build .
```

---

## Program Logic

### Startup Flow

```
┌─────────────────────────────────────────────────────────────┐
│                      Program Start                           │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              Initialize Database (library.db)                │
│              - Create table structures                       │
│              - Check for administrator account               │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Main Menu                                 │
│         1. Login  2. Register  3. Forgot Password  0. Exit  │
└─────────────────────────────────────────────────────────────┘
```

### User Roles

```
┌─────────────────────────────────────────────────────────────────────┐
│                           User Login                                 │
└─────────────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              │                               │
              ▼                               ▼
    ┌──────────────────┐            ┌──────────────────┐
    │   Admin (ADMIN)  │            │  Student (STUDENT)│
    └──────────────────┘            └──────────────────┘
              │                               │
              ▼                               ▼
    ┌──────────────────┐            ┌──────────────────┐
    │   Admin Menu     │            │  Student Menu    │
    │ 1. Book Mgmt     │            │ 1. Search Books  │
    │    - Add/Edit/   │            │ 2. Borrow Book   │
    │      Delete/Find │            │ 3. Return Book   │
    │ 2. User Mgmt     │            │ 4. Renew Book    │
    │    - Add User    │            │ 5. My Borrowings │
    │    - Change Pass │            │ 6. Account Mgmt  │
    │ 3. Borrow Mgmt   │            │    - View Info   │
    │    - Query Student│           │    - Edit Info   │
    │    - All Records │            │    - Change Pass │
    └──────────────────┘            │    - Set Token   │
                                    └──────────────────┘
```

---

### Core Functions

#### 1. Book Management (Admin)

| Function | Description |
|----------|-------------|
| Add Book | Add new book with ISBN, title, author, publisher, category, quantity |
| Edit Book | Modify book information by ISBN |
| Delete Book | Remove book (requires no active borrowings) |
| Search Books | Search by keyword (title/author/ISBN) |
| List All Books | Display all books, sortable by title/author/ISBN |

#### 2. Borrowing Management (Student)

| Function | Description |
|----------|-------------|
| Borrow Book | Enter ISBN and borrowing days (1-90 days) |
| Return Book | Return borrowed book by record ID |
| Renew Book | Extend borrowing period |
| View Borrowings | View personal borrowing records |

#### 3. Account Security

| Function | Description |
|----------|-------------|
| Change Password | Admin can change any user's password; students can only change their own |
| Security Token | Set password recovery token for verification |
| Forgot Password | Reset password using username and security token |

#### 4. Overdue Management

- Automatic detection of overdue books
- Students are notified of overdue books upon login
- Admin can view all borrowing records with overdue status

---

## Screenshots

### Main Menu

```
--- Welcome to Library Management System ---
1. Login
2. Student Registration
3. Forgot Password
0. Exit
--------------------------------------------
Please enter your choice:
```

### Admin Menu

```
--- Admin Menu (admin) ---
1. Book Management - Add/Edit/Delete/Search
2. User Management - Add User/Change Password
3. Borrowing Management - Query Student/All Records
0. Logout
-----------------------------
Please enter your choice:
```

### Student Menu

```
--- Student Menu (Zhang San) ---
1. Search Books
2. Borrow Book
3. Return Book
4. Renew Book
5. View My Borrowings
6. Account Management (View Info/Change Password/Set Token)
0. Logout
---------------------------------
Please enter your choice:
```

### Book List

```
+----------------------+------------------------+----------------+--------+--------+
| ISBN                 | Title                  | Author         | Avail  | Total  |
+----------------------+------------------------+----------------+--------+--------+
| 978-7-111-40701-0    | Computer Systems       | Randal E. Bryant|   3    |   5    |
| 978-7-302-33227-9    | C++ Primer Plus        | Stephen Prata  |   2    |   3    |
+----------------------+------------------------+----------------+--------+--------+
```

### Borrowing Records

```
+--------+----------------------+------------------------+--------------+--------------+
| Rec ID | ISBN                 | Title                  | Borrow Date  | Due Date     |
+--------+----------------------+------------------------+--------------+--------------+
| 1      | 978-7-111-40701-0    | Computer Systems       | 2025-03-01   | 2025-03-31   |
+--------+----------------------+------------------------+--------------+--------------+
```

---

## Project Structure

```
libraryManagementSystem/
├── CMakeLists.txt          # CMake build configuration
├── cmake/
│   └── Windows-x86.cmake   # Windows cross-compilation toolchain
├── header/
│   ├── database.h          # Database manager class declaration
│   ├── sha256.h            # SHA-256 encryption header
│   └── utils.h             # Utility functions (clear screen, terminal width, etc.)
├── src/
│   ├── main.cpp            # Program entry point and UI logic
│   ├── database.cpp        # Database operations implementation
│   └── sha256.cpp          # SHA-256 encryption implementation
├── lib/
│   ├── sqlite3.c           # SQLite source code
│   └── sqlite3.h           # SQLite header file
├── Build/                  # Build output directory
└── docs/
    └── README_zh-CN.md     # Chinese documentation
    └── CMake 安装教程.md    # CMake installation guide (Chinese)
```

---

## Tech Stack

| Technology | Description |
|------------|-------------|
| **C++20** | Modern C++ features (structured bindings, smart pointers, etc.) |
| **SQLite3** | Lightweight embedded database, zero configuration |
| **CMake** | Cross-platform build system |
| **SHA-256** | Password encryption algorithm |

---

## Usage

### First Run

On first run, the program will automatically:

1. Create `library.db` database file
2. Initialize table structures (books, users, borrowing records)
3. Create default administrator account (username: `admin`, password: `admin`)

### Default Account

| Role | Username | Password |
|------|----------|----------|
| Admin | admin | admin |

### Student Registration

1. Select `2. Student Registration` from main menu
2. Enter student ID, name, college, class information
3. Set login username and password
4. Registration complete, ready to login

### Password Recovery

1. Select `3. Forgot Password` from main menu
2. Enter username
3. Enter previously set security token
4. Set new password

---

## Notes

| Item | Description |
|------|-------------|
| **Database File** | `library.db` is saved in the running directory, do not delete |
| **Terminal Encoding** | Use UTF-8 encoding (Windows: `chcp 65001`) |
| **Borrowing Limits** | 1-90 days borrowing period, book quantity cannot be negative |
| **Deletion Limits** | Books with active borrowings cannot be deleted |

---

## Use Cases

- Reference project for university C++ programming courses
- Database application development teaching case
- CMake cross-platform build practice
- Terminal UI design reference

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Contributing

Issues and Pull Requests are welcome!

---

<p align="center">Made with ❤️ by <a href="https://github.com/Rouxiv">@Rouxiv</a></p>
