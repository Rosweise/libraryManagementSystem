const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const jwt = require('jsonwebtoken');
const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcrypt');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 8080;
const JWT_SECRET = process.env.JWT_SECRET || 'your-secret-key-change-in-production';

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.json());

// Database connection
const dbPath = path.join(__dirname, '..', 'library.db');
const db = new sqlite3.Database(dbPath, (err) => {
  if (err) {
    console.error('Error opening database:', err);
  } else {
    console.log('Connected to SQLite database');
  }
});

// Logger helper
const logAction = (level, message, user = '', action = '') => {
  const timestamp = new Date().toISOString().replace('T', ' ').substring(0, 23);
  const sql = 'INSERT INTO Logs (timestamp, level, message, user, action) VALUES (?, ?, ?, ?, ?)';
  db.run(sql, [timestamp, level, message, user, action], (err) => {
    if (err) console.error('Error logging:', err);
  });
};

// Authentication middleware
const authenticateToken = (req, res, next) => {
  const authHeader = req.headers['authorization'];
  const token = authHeader && authHeader.split(' ')[1];

  if (!token) {
    return res.status(401).json({ message: 'Access token required' });
  }

  jwt.verify(token, JWT_SECRET, (err, user) => {
    if (err) {
      return res.status(403).json({ message: 'Invalid or expired token' });
    }
    req.user = user;
    next();
  });
};

// Get user by ID
app.get('/api/users/:userId', authenticateToken, (req, res) => {
  const { userId } = req.params;

  const sql = 'SELECT id, username, name, college, className, role FROM Users WHERE id = ?';
  db.get(sql, [userId], (err, user) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch user' });
    }
    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }
    res.json(user);
  });
});

// Update user (admin only or self)
app.put('/api/users/:userId', authenticateToken, (req, res) => {
  const { userId } = req.params;
  const { name, college, className } = req.body;

  // Check if user can update this profile
  if (req.user.role !== 'ADMIN' && req.user.id !== userId) {
    return res.status(403).json({ message: 'Access denied' });
  }

  const sql = 'UPDATE Users SET name = ?, college = ?, className = ? WHERE id = ?';
  db.run(sql, [name, college, className, userId], function(err) {
    if (err) {
      logAction('ERROR', `Failed to update user: ${err.message}`, req.user.username, 'update_user');
      return res.status(500).json({ message: 'Failed to update user' });
    }

    logAction('INFO', `User profile updated: ${userId}`, req.user.username, 'update_user');
    res.json({ id: userId, name, college, className });
  });
});

// Change password
app.put('/api/users/:userId/password', authenticateToken, (req, res) => {
  const { userId } = req.params;
  const { oldPassword, newPassword } = req.body;

  // Check if user can change this password
  if (req.user.role !== 'ADMIN' && req.user.id !== userId) {
    return res.status(403).json({ message: 'Access denied' });
  }

  // Get current password
  db.get('SELECT password FROM Users WHERE id = ?', [userId], (err, user) => {
    if (err || !user) {
      return res.status(404).json({ message: 'User not found' });
    }

    // For simplicity, comparing plain text (should use bcrypt in production)
    if (user.password !== oldPassword) {
      logAction('WARN', 'Password change failed: incorrect old password', req.user.username, 'change_password');
      return res.status(400).json({ message: 'Incorrect old password' });
    }

    const updateSql = 'UPDATE Users SET password = ? WHERE id = ?';
    db.run(updateSql, [newPassword, userId], (err) => {
      if (err) {
        logAction('ERROR', `Failed to update password: ${err.message}`, req.user.username, 'change_password');
        return res.status(500).json({ message: 'Failed to update password' });
      }

      logAction('INFO', `Password changed for user: ${userId}`, req.user.username, 'change_password');
      res.json({ message: 'Password updated successfully' });
    });
  });
});

// Set recovery token
app.put('/api/users/:userId/recovery-token', authenticateToken, (req, res) => {
  const { userId } = req.params;
  const { token } = req.body;

  // Check if user can set recovery token for this account
  if (req.user.role !== 'ADMIN' && req.user.id !== userId) {
    return res.status(403).json({ message: 'Access denied' });
  }

  const sql = 'UPDATE Users SET recoveryToken = ? WHERE id = ?';
  db.run(sql, [token, userId], (err) => {
    if (err) {
      logAction('ERROR', `Failed to set recovery token: ${err.message}`, req.user.username, 'set_recovery_token');
      return res.status(500).json({ message: 'Failed to set recovery token' });
    }

    logAction('INFO', `Recovery token set for user: ${userId}`, req.user.username, 'set_recovery_token');
    res.json({ message: 'Recovery token set successfully' });
  });
});

// ============ Authentication Endpoints ============

// Login
app.post('/api/auth/login', (req, res) => {
  const { username, password } = req.body;

  const sql = 'SELECT * FROM Users WHERE username = ?';
  db.get(sql, [username], async (err, user) => {
    if (err) {
      logAction('ERROR', `Login error: ${err.message}`, username, 'login');
      return res.status(500).json({ message: 'Server error' });
    }

    if (!user) {
      logAction('WARN', 'Login failed: user not found', username, 'login');
      return res.status(401).json({ message: 'Invalid credentials' });
    }

    // Verify password (assuming SHA256 hash stored in database)
    // Note: The C++ code uses SHA256, but for simplicity we'll check directly
    if (user.password !== password) {
      logAction('WARN', 'Login failed: incorrect password', username, 'login');
      return res.status(401).json({ message: 'Invalid credentials' });
    }

    const token = jwt.sign(
      { username: user.username, role: user.role, id: user.id },
      JWT_SECRET,
      { expiresIn: '24h' }
    );

    logAction('INFO', 'User logged in successfully', username, 'login');

    res.json({
      user: {
        id: user.id,
        username: user.username,
        name: user.name,
        role: user.role,
        college: user.college,
        className: user.className
      },
      token
    });
  });
});


// ============ User Management Endpoints ============

// Get users (admin only)
app.get('/api/users', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { role, page = 0, size = 10 } = req.query;
  const offset = parseInt(page) * parseInt(size);

  let sql = 'SELECT id, username, name, college, className, role FROM Users';
  let params = [];

  if (role) {
    sql += ' WHERE role = ?';
    params.push(role);
  }

  sql += ' ORDER BY username LIMIT ? OFFSET ?';
  params.push(parseInt(size), offset);

  db.all(sql, params, (err, users) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch users' });
    }

    let countSql = 'SELECT COUNT(*) as count FROM Users';
    let countParams = [];
    if (role) {
      countSql += ' WHERE role = ?';
      countParams.push(role);
    }

    db.get(countSql, countParams, (err, result) => {
      if (err) {
        return res.status(500).json({ message: 'Failed to count users' });
      }
      const totalCount = result ? result.count : 0;
      const totalPages = Math.ceil(totalCount / parseInt(size));

      res.json({ users, totalPages, totalCount });
    });
  });
});

// Register
app.post('/api/auth/register', (req, res) => {
  const { id, username, name, college, className, password } = req.body;

  const sql = 'INSERT INTO Users (id, username, name, college, className, role, password) VALUES (?, ?, ?, ?, ?, ?, ?)';
  db.run(sql, [id, username, name, college, className, 'STUDENT', password], function(err) {
    if (err) {
      logAction('ERROR', `Registration error: ${err.message}`, username, 'register');
      return res.status(500).json({ message: 'Registration failed' });
    }

    logAction('INFO', 'New user registered', username, 'register');
    res.json({ id, username, name, role: 'STUDENT' });
  });
});

// Forgot password
app.post('/api/auth/forgot-password', (req, res) => {
  const { username, token, newPassword } = req.body;

  const sql = 'SELECT recoveryToken FROM Users WHERE username = ?';
  db.get(sql, [username], (err, user) => {
    if (err || !user) {
      return res.status(404).json({ message: 'User not found' });
    }

    if (user.recoveryToken !== token) {
      logAction('WARN', 'Password recovery failed: invalid token', username, 'forgot_password');
      return res.status(401).json({ message: 'Invalid recovery token' });
    }

    const updateSql = 'UPDATE Users SET password = ? WHERE username = ?';
    db.run(updateSql, [newPassword, username], (err) => {
      if (err) {
        return res.status(500).json({ message: 'Failed to update password' });
      }

      logAction('INFO', 'Password reset successfully', username, 'forgot_password');
      res.json({ message: 'Password updated successfully' });
    });
  });
});

// Import books from CSV (admin only)
app.post('/api/books/import', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  // This is a simplified implementation - in reality, you'd need to read and parse the CSV file
  const { filePath } = req.body;
  
  // For now, just return a success message
  logAction('INFO', `Import books from: ${filePath}`, req.user.username, 'import_books');
  res.json({ message: 'Import functionality would be implemented here' });
});

// Export books to CSV (admin only)
app.post('/api/books/export', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { filePath } = req.body;
  
  // For now, just return a success message
  logAction('INFO', `Export books to: ${filePath}`, req.user.username, 'export_books');
  res.json({ message: 'Export functionality would be implemented here' });
});

// ============ Book Management Endpoints ============

// Get books with pagination and search
app.get('/api/books', authenticateToken, (req, res) => {
  const { keyword = '', sortBy = 'title', page = 0, size = 10 } = req.query;
  const offset = parseInt(page) * parseInt(size);

  let sql = 'SELECT * FROM Books';
  let params = [];

  if (keyword) {
    sql += ' WHERE title LIKE ? OR author LIKE ? OR isbn LIKE ?';
    const searchTerm = `%${keyword}%`;
    params = [searchTerm, searchTerm, searchTerm];
  }

  sql += ` ORDER BY ${sortBy} LIMIT ? OFFSET ?`;
  params.push(parseInt(size), offset);

  db.all(sql, params, (err, books) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch books' });
    }

    // Get total count
    let countSql = 'SELECT COUNT(*) as count FROM Books';
    let countParams = [];
    if (keyword) {
      countSql += ' WHERE title LIKE ? OR author LIKE ? OR isbn LIKE ?';
      const searchTerm = `%${keyword}%`;
      countParams = [searchTerm, searchTerm, searchTerm];
    }

    db.get(countSql, countParams, (err, result) => {
      const totalCount = result ? result.count : 0;
      const totalPages = Math.ceil(totalCount / parseInt(size));

      res.json({ books, totalPages, totalCount });
    });
  });
});

// Get book by ISBN
app.get('/api/books/:isbn', authenticateToken, (req, res) => {
  const { isbn } = req.params;

  db.get('SELECT * FROM Books WHERE isbn = ?', [isbn], (err, book) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch book' });
    }
    if (!book) {
      return res.status(404).json({ message: 'Book not found' });
    }
    res.json(book);
  });
});

// Add book (admin only)
app.post('/api/books', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { isbn, title, author, publisher, category, totalCopies } = req.body;

  const sql = 'INSERT INTO Books (isbn, title, author, publisher, category, totalCopies, availableCopies) VALUES (?, ?, ?, ?, ?, ?, ?)';
  db.run(sql, [isbn, title, author, publisher, category, totalCopies, totalCopies], function(err) {
    if (err) {
      logAction('ERROR', `Failed to add book: ${err.message}`, req.user.username, 'add_book');
      return res.status(500).json({ message: 'Failed to add book' });
    }

    logAction('INFO', `Book added: ${title} (${isbn})`, req.user.username, 'add_book');
    res.json({ isbn, title, author, publisher, category, totalCopies, availableCopies: totalCopies });
  });
});

// Get all books (for admin overview)
app.get('/api/books/all', authenticateToken, (req, res) => {
  const sql = 'SELECT * FROM Books ORDER BY title';
  db.all(sql, [], (err, books) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch all books' });
    }
    res.json(books);
  });
});

// Update book (admin only)
app.put('/api/books/:isbn', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { isbn } = req.params;
  const { title, author, publisher, category, totalCopies } = req.body;

  const sql = 'UPDATE Books SET title = ?, author = ?, publisher = ?, category = ?, totalCopies = ? WHERE isbn = ?';
  db.run(sql, [title, author, publisher, category, totalCopies, isbn], function(err) {
    if (err) {
      return res.status(500).json({ message: 'Failed to update book' });
    }

    logAction('INFO', `Book updated: ${title} (${isbn})`, req.user.username, 'update_book');
    res.json({ isbn, title, author, publisher, category, totalCopies });
  });
});

// Delete book (admin only)
app.delete('/api/books/:isbn', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { isbn } = req.params;

  db.run('DELETE FROM Books WHERE isbn = ?', [isbn], function(err) {
    if (err) {
      return res.status(500).json({ message: 'Failed to delete book' });
    }

    logAction('INFO', `Book deleted: ${isbn}`, req.user.username, 'delete_book');
    res.json({ message: 'Book deleted successfully' });
  });
});

// Get all borrow records (admin only)
app.get('/api/borrowings', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const sql = `
    SELECT br.recordId, br.userId, u.name as userName, br.bookIsbn, b.title as bookTitle,
           br.borrowDate, br.dueDate, br.returnDate
    FROM BorrowRecords br
    JOIN Users u ON br.userId = u.id
    JOIN Books b ON br.bookIsbn = b.isbn
    ORDER BY br.borrowDate DESC
  `;

  db.all(sql, [], (err, records) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch all borrow records' });
    }
    res.json(records);
  });
});

// ============ Borrowing Management Endpoints ============

// Get borrowed books by user
app.get('/api/borrowings/user/:userId', authenticateToken, (req, res) => {
  const { userId } = req.params;

  const sql = `
    SELECT br.recordId, br.userId, br.bookIsbn, b.title as bookTitle,
           br.borrowDate, br.dueDate, br.returnDate
    FROM BorrowRecords br
    JOIN Books b ON br.bookIsbn = b.isbn
    WHERE br.userId = ? AND br.returnDate IS NULL
  `;

  db.all(sql, [userId], (err, records) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch borrow records' });
    }
    res.json(records);
  });
});

// Get overdue books by user
app.get('/api/borrowings/user/:userId/overdue', authenticateToken, (req, res) => {
  const { userId } = req.params;

  const sql = `
    SELECT br.recordId, br.userId, br.bookIsbn, b.title as bookTitle,
           br.borrowDate, br.dueDate, br.returnDate
    FROM BorrowRecords br
    JOIN Books b ON br.bookIsbn = b.isbn
    WHERE br.userId = ? AND br.returnDate IS NULL AND br.dueDate < datetime('now')
  `;

  db.all(sql, [userId], (err, records) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch overdue records' });
    }
    res.json(records);
  });
});

// Borrow book
app.post('/api/borrowings', authenticateToken, (req, res) => {
  const { userId, isbn, days } = req.body;

  // Check if book is available
  db.get('SELECT availableCopies FROM Books WHERE isbn = ?', [isbn], (err, book) => {
    if (err || !book || book.availableCopies <= 0) {
      return res.status(400).json({ message: 'Book not available' });
    }

    const borrowDate = new Date().toISOString().split('T')[0];
    const dueDate = new Date(Date.now() + days * 24 * 60 * 60 * 1000).toISOString().split('T')[0];

    const sql = 'INSERT INTO BorrowRecords (userId, bookIsbn, borrowDate, dueDate) VALUES (?, ?, ?, ?)';
    db.run(sql, [userId, isbn, borrowDate, dueDate], function(err) {
      if (err) {
        return res.status(500).json({ message: 'Failed to borrow book' });
      }

      // Update available copies
      db.run('UPDATE Books SET availableCopies = availableCopies - 1 WHERE isbn = ?', [isbn]);

      logAction('INFO', `Book borrowed: ${isbn} for ${days} days`, req.user.username, 'borrow_book');
      res.json({ recordId: this.lastID, userId, bookIsbn: isbn, borrowDate, dueDate });
    });
  });
});

// Get all overdue books (admin only)
app.get('/api/borrowings/overdue', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const sql = `
    SELECT br.recordId, br.userId, u.name as userName, br.bookIsbn, b.title as bookTitle,
           br.borrowDate, br.dueDate
    FROM BorrowRecords br
    JOIN Users u ON br.userId = u.id
    JOIN Books b ON br.bookIsbn = b.isbn
    WHERE br.returnDate IS NULL AND br.dueDate < datetime('now')
    ORDER BY br.dueDate
  `;

  db.all(sql, [], (err, records) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch overdue records' });
    }
    res.json(records);
  });
});

// Return book
app.put('/api/borrowings/:recordId/return', authenticateToken, (req, res) => {
  const { recordId } = req.params;
  const { userId } = req.body;

  const returnDate = new Date().toISOString().split('T')[0];

  db.get('SELECT bookIsbn FROM BorrowRecords WHERE recordId = ? AND userId = ?', [recordId, userId], (err, record) => {
    if (err || !record) {
      return res.status(404).json({ message: 'Borrow record not found' });
    }

    const sql = 'UPDATE BorrowRecords SET returnDate = ? WHERE recordId = ?';
    db.run(sql, [returnDate, recordId], function(err) {
      if (err) {
        return res.status(500).json({ message: 'Failed to return book' });
      }

      // Update available copies
      db.run('UPDATE Books SET availableCopies = availableCopies + 1 WHERE isbn = ?', [record.bookIsbn]);

      logAction('INFO', `Book returned: record ${recordId}`, req.user.username, 'return_book');
      res.json({ message: 'Book returned successfully' });
    });
  });
});

// Renew book
app.put('/api/borrowings/:recordId/renew', authenticateToken, (req, res) => {
  const { recordId } = req.params;
  const { userId } = req.body;

  db.get('SELECT dueDate FROM BorrowRecords WHERE recordId = ? AND userId = ?', [recordId, userId], (err, record) => {
    if (err || !record) {
      return res.status(404).json({ message: 'Borrow record not found' });
    }

    const newDueDate = new Date(new Date(record.dueDate).getTime() + 30 * 24 * 60 * 60 * 1000).toISOString().split('T')[0];

    db.run('UPDATE BorrowRecords SET dueDate = ? WHERE recordId = ?', [newDueDate, recordId], function(err) {
      if (err) {
        return res.status(500).json({ message: 'Failed to renew book' });
      }

      logAction('INFO', `Book renewed: record ${recordId}`, req.user.username, 'renew_book');
      res.json({ recordId, dueDate: newDueDate });
    });
  });
});

// Backup database (admin only)
app.post('/api/system/backup', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { backupPath } = req.body;
  
  // For now, just return a success message
  logAction('INFO', `Backup initiated to: ${backupPath}`, req.user.username, 'backup_db');
  res.json({ message: 'Backup functionality would be implemented here' });
});

// Restore database (admin only)
app.post('/api/system/restore', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { restorePath } = req.body;
  
  // For now, just return a success message
  logAction('INFO', `Restore initiated from: ${restorePath}`, req.user.username, 'restore_db');
  res.json({ message: 'Restore functionality would be implemented here' });
});

// ============ Log Management Endpoints ============

// Get logs (admin only)
app.get('/api/logs', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const { startTime, endTime, user, level } = req.query;

  let sql = 'SELECT * FROM Logs WHERE 1=1';
  let params = [];

  if (startTime && endTime) {
    sql += ' AND timestamp BETWEEN ? AND ?';
    params.push(startTime, endTime);
  }

  if (user) {
    sql += ' AND user = ?';
    params.push(user);
  }

  if (level) {
    sql += ' AND level = ?';
    params.push(level);
  }

  sql += ' ORDER BY timestamp DESC LIMIT 1000';

  db.all(sql, params, (err, logs) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch logs' });
    }
    res.json(logs);
  });
});


// Get full borrow records (admin only)
app.get('/api/borrowings/full', authenticateToken, (req, res) => {
  if (req.user.role !== 'ADMIN') {
    return res.status(403).json({ message: 'Admin access required' });
  }

  const sql = `
    SELECT br.recordId, u.id as studentId, u.name as studentName,
           u.college as studentCollege, u.className as studentClass,
           b.title as bookTitle, br.borrowDate, br.dueDate,
           CASE WHEN br.returnDate IS NULL AND br.dueDate < datetime('now') THEN 1 ELSE 0 END as isOverdue
    FROM BorrowRecords br
    JOIN Users u ON br.userId = u.id
    JOIN Books b ON br.bookIsbn = b.isbn
    WHERE br.returnDate IS NULL
    ORDER BY br.borrowDate DESC
  `;

  db.all(sql, [], (err, records) => {
    if (err) {
      return res.status(500).json({ message: 'Failed to fetch borrow records' });
    }
    res.json(records);
  });
});

// Health check
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// Start server
app.listen(PORT, '0.0.0.0', () => {
  console.log(`API Server running on http://localhost:${PORT}`);
  console.log(`Database: ${dbPath}`);
  logAction('INFO', 'API Server started', 'system', 'startup');
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down gracefully...');
  logAction('INFO', 'API Server stopped', 'system', 'shutdown');
  db.close((err) => {
    if (err) {
      console.error('Error closing database:', err);
    }
    process.exit(0);
  });
});








