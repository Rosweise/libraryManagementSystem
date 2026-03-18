import axios, { AxiosInstance, AxiosResponse } from 'axios';
import { Book, User, BorrowRecord, FullBorrowRecord, StudentBorrowRecord, LoginCredentials, RegisterData, LogEntry, ApiResponse } from '@/types';

class ApiService {
  private api: AxiosInstance;

  constructor() {
    this.api = axios.create({
      baseURL: '/api', // This will be proxied to the backend
      timeout: 10000,
      headers: {
        'Content-Type': 'application/json',
      },
    });

    // Add request interceptor to include token
    this.api.interceptors.request.use(
      (config) => {
        const token = localStorage.getItem('token');
        if (token) {
          config.headers.Authorization = `Bearer ${token}`;
        }
        return config;
      },
      (error) => Promise.reject(error)
    );

    // Add response interceptor to handle errors globally
    this.api.interceptors.response.use(
      (response) => response,
      (error) => {
        if (error.response?.status === 401) {
          // Clear token and redirect to login
          localStorage.removeItem('token');
          window.location.href = '/login';
        }
        return Promise.reject(error);
      }
    );
  }

  // Authentication endpoints
  async login(credentials: LoginCredentials): Promise<ApiResponse<{ user: User; token: string }>> {
    try {
      const response: AxiosResponse<{ user: User; token: string }> = await this.api.post('/auth/login', credentials);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Login failed' };
    }
  }

  async register(userData: RegisterData): Promise<ApiResponse<User>> {
    try {
      const response: AxiosResponse<User> = await this.api.post('/auth/register', userData);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Registration failed' };
    }
  }

  async forgotPassword(username: string, token: string, newPassword: string): Promise<ApiResponse<void>> {
    try {
      await this.api.post('/auth/forgot-password', { username, token, newPassword });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Password reset failed' };
    }
  }

  // Book management endpoints
  async getBooks(searchKeyword: string = '', sortBy: string = 'title', page: number = 0, size: number = 10): Promise<ApiResponse<{ books: Book[]; totalPages: number; totalCount: number }>> {
    try {
      const response: AxiosResponse<{ books: Book[]; totalPages: number; totalCount: number }> = await this.api.get('/books', {
        params: { keyword: searchKeyword, sortBy, page, size }
      });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch books' };
    }
  }

  async getBookByIsbn(isbn: string): Promise<ApiResponse<Book>> {
    try {
      const response: AxiosResponse<Book> = await this.api.get(`/books/${isbn}`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch book' };
    }
  }

  async addBook(book: Book): Promise<ApiResponse<Book>> {
    try {
      const response: AxiosResponse<Book> = await this.api.post('/books', book);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to add book' };
    }
  }

  async updateBook(book: Book): Promise<ApiResponse<Book>> {
    try {
      const response: AxiosResponse<Book> = await this.api.put(`/books/${book.isbn}`, book);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to update book' };
    }
  }

  async deleteBook(isbn: string): Promise<ApiResponse<void>> {
    try {
      await this.api.delete(`/books/${isbn}`);
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to delete book' };
    }
  }

  // User management endpoints
  async getUsers(role?: string, page: number = 0, size: number = 10): Promise<ApiResponse<{ users: User[]; totalPages: number; totalCount: number }>> {
    try {
      const response: AxiosResponse<{ users: User[]; totalPages: number; totalCount: number }> = await this.api.get('/users', {
        params: { role, page, size }
      });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch users' };
    }
  }

  async getUserById(userId: string): Promise<ApiResponse<User>> {
    try {
      const response: AxiosResponse<User> = await this.api.get(`/users/${userId}`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch user' };
    }
  }

  async updateUser(user: User): Promise<ApiResponse<User>> {
    try {
      const response: AxiosResponse<User> = await this.api.put(`/users/${user.id}`, user);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to update user' };
    }
  }

  async changePassword(userId: string, oldPassword: string, newPassword: string): Promise<ApiResponse<void>> {
    try {
      await this.api.put(`/users/${userId}/password`, { oldPassword, newPassword });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to change password' };
    }
  }

  async setRecoveryToken(userId: string, token: string): Promise<ApiResponse<void>> {
    try {
      await this.api.put(`/users/${userId}/recovery-token`, { token });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to set recovery token' };
    }
  }

  // Borrowing management endpoints
  async getBorrowedBooks(userId: string): Promise<ApiResponse<BorrowRecord[]>> {
    try {
      const response: AxiosResponse<BorrowRecord[]> = await this.api.get(`/borrowings/user/${userId}`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch borrowed books' };
    }
  }

  async getAllBorrowings(userId: string): Promise<ApiResponse<BorrowRecord[]>> {
    try {
      const response: AxiosResponse<BorrowRecord[]> = await this.api.get(`/borrowings/user/${userId}/all`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch all borrowings' };
    }
  }

  async getOverdueBooks(userId: string): Promise<ApiResponse<BorrowRecord[]>> {
    try {
      const response: AxiosResponse<BorrowRecord[]> = await this.api.get(`/borrowings/user/${userId}/overdue`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch overdue books' };
    }
  }

  async borrowBook(userId: string, isbn: string, days: number): Promise<ApiResponse<BorrowRecord>> {
    try {
      const response: AxiosResponse<BorrowRecord> = await this.api.post('/borrowings', { userId, isbn, days });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to borrow book' };
    }
  }

  async returnBook(recordId: number, userId: string): Promise<ApiResponse<void>> {
    try {
      await this.api.put(`/borrowings/${recordId}/return`, { userId });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to return book' };
    }
  }

  async renewBook(recordId: number, userId: string): Promise<ApiResponse<BorrowRecord>> {
    try {
      const response: AxiosResponse<BorrowRecord> = await this.api.put(`/borrowings/${recordId}/renew`, { userId });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to renew book' };
    }
  }

  // Admin-specific endpoints
  async getFullBorrowRecords(sortBy: string = 'studentId'): Promise<ApiResponse<FullBorrowRecord[]>> {
    try {
      const response: AxiosResponse<FullBorrowRecord[]> = await this.api.get('/borrowings/full', { params: { sortBy } });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch borrow records' };
    }
  }

  async getStudentBorrowRecords(studentId: string): Promise<ApiResponse<StudentBorrowRecord[]>> {
    try {
      const response: AxiosResponse<StudentBorrowRecord[]> = await this.api.get(`/borrowings/student/${studentId}`);
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch student borrow records' };
    }
  }

  async getLogs(startTime?: string, endTime?: string, user?: string, level?: string): Promise<ApiResponse<LogEntry[]>> {
    try {
      const response: AxiosResponse<LogEntry[]> = await this.api.get('/logs', { params: { startTime, endTime, user, level } });
      return { success: true, data: response.data };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to fetch logs' };
    }
  }

  // Batch operations
  async importBooksFromCSV(filePath: string): Promise<ApiResponse<void>> {
    try {
      await this.api.post('/books/import', { filePath });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to import books' };
    }
  }

  async exportBooksToCSV(filePath: string): Promise<ApiResponse<void>> {
    try {
      await this.api.post('/books/export', { filePath });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to export books' };
    }
  }

  // System operations
  async backupDatabase(backupPath: string): Promise<ApiResponse<void>> {
    try {
      await this.api.post('/system/backup', { backupPath });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to backup database' };
    }
  }

  async restoreDatabase(restorePath: string): Promise<ApiResponse<void>> {
    try {
      await this.api.post('/system/restore', { restorePath });
      return { success: true };
    } catch (error: any) {
      return { success: false, error: error.response?.data?.message || 'Failed to restore database' };
    }
  }
}

export default new ApiService();
