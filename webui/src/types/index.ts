// Types for the library management system

export interface Book {
  isbn: string;
  title: string;
  author: string;
  publisher: string;
  category: string;
  totalCopies: number;
  availableCopies: number;
}

export interface User {
  id: string;
  username: string;
  name: string;
  college: string;
  className: string;
  role: 'ADMIN' | 'STUDENT';
  permissionLevel?: PermissionLevel;
  hasRecoveryToken: boolean;
}

export enum PermissionLevel {
  SUPER_ADMIN = 0,
  BOOK_MANAGER = 1,
  USER_MANAGER = 2,
  LOG_MANAGER = 3,
  BASIC_ADMIN = 4
}

export interface BorrowRecord {
  recordId: number;
  userId: string;
  bookIsbn: string;
  bookTitle: string;
  borrowDate: string;
  dueDate: string;
  returnDate?: string;
}

export interface FullBorrowRecord {
  recordId: number;
  studentId: string;
  studentName: string;
  studentCollege: string;
  studentClass: string;
  bookTitle: string;
  borrowDate: string;
  dueDate: string;
  isOverdue: boolean;
}

export interface StudentBorrowRecord extends FullBorrowRecord {
  bookIsbn: string;
  returnDate?: string | null;
}

export interface LoginCredentials {
  username: string;
  password: string;
}

export interface RegisterData {
  id: string;
  username: string;
  name: string;
  college: string;
  className: string;
  password: string;
}

export interface LogEntry {
  id: number;
  timestamp: string;
  level: 'DEBUG' | 'INFO' | 'WARN' | 'ERROR';
  message: string;
  user: string;
  action: string;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  message?: string;
  error?: string;
}
