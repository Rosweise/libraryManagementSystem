<template>
  <div class="search-books">
    <h2>{{ t('search_books') }}</h2>
    
    <el-card class="search-card">
      <el-form :model="searchForm" inline class="search-form">
        <el-form-item :label="t('search_keyword')">
          <el-input 
            v-model="searchForm.keyword" 
            :placeholder="t('search_placeholder')"
            @keyup.enter="searchBooks"
          />
        </el-form-item>
        <el-form-item :label="t('sort_by')">
          <el-select v-model="searchForm.sortBy" class="sort-select">
            <el-option :label="t('sort_by_title')" value="title" />
            <el-option :label="t('sort_by_author')" value="author" />
            <el-option :label="t('sort_by_isbn')" value="isbn" />
          </el-select>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="searchBooks">{{ t('search') }}</el-button>
          <el-button @click="resetSearch">{{ t('reset') }}</el-button>
        </el-form-item>
      </el-form>
    </el-card>
    
    <el-table 
      :data="books" 
      v-loading="loading"
      class="books-table"
      row-key="isbn"
    >
      <el-table-column prop="isbn" :label="t('isbn')" width="200" />
      <el-table-column prop="title" :label="t('title')" width="200" show-overflow-tooltip />
      <el-table-column prop="author" :label="t('author')" width="150" />
      <el-table-column prop="publisher" :label="t('publisher')" width="150" show-overflow-tooltip />
      <el-table-column prop="category" :label="t('category')" width="120" />
      <el-table-column prop="totalCopies" :label="t('total_copies')" width="100" />
      <el-table-column prop="availableCopies" :label="t('available_copies')" width="120">
        <template #default="scope">
          <el-tag :type="scope.row.availableCopies > 0 ? 'success' : 'danger'">
            {{ scope.row.availableCopies }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column :label="t('actions')" width="150" fixed="right">
        <template #default="scope">
          <el-button 
            size="small" 
            @click="borrowBook(scope.row)"
            :disabled="scope.row.availableCopies <= 0"
            type="primary"
          >
            {{ t('borrow_book') }}
          </el-button>
          <el-button size="small" @click="viewBookDetails(scope.row)" type="info">
            {{ t('details') }}
          </el-button>
        </template>
      </el-table-column>
    </el-table>
    
    <div class="pagination">
      <el-pagination
        @size-change="handleSizeChange"
        @current-change="handleCurrentChange"
        :current-page="pagination.currentPage"
        :page-sizes="[10, 20, 50, 100]"
        :page-size="pagination.pageSize"
        layout="total, sizes, prev, pager, next, jumper"
        :total="pagination.total"
      />
    </div>
    
    <!-- Borrow Dialog -->
    <el-dialog 
      :title="t('borrow_book')" 
      v-model="borrowDialogVisible" 
      width="400px" 
      :before-close="closeBorrowDialog"
    >
      <el-form :model="borrowForm" :rules="borrowRules" ref="borrowFormRef" label-width="120px">
        <el-form-item :label="t('book_title')">
          <el-input v-model="selectedBook.title" disabled />
        </el-form-item>
        
        <el-form-item :label="t('borrow_days')" prop="days">
          <el-input-number 
            v-model="borrowForm.days" 
            :min="1"
            :max="90"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="closeBorrowDialog">{{ t('cancel') }}</el-button>
          <el-button type="primary" @click="confirmBorrow" :loading="borrowing">
            {{ t('confirm_borrow') }}
          </el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { Book } from '@/types';

const { t } = useI18n();
const userStore = useUserStore();

interface SearchForm {
  keyword: string;
  sortBy: string;
}

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

interface BorrowForm {
  days: number;
}

const books = ref<Book[]>([]);
const loading = ref(false);
const borrowing = ref(false);
const borrowDialogVisible = ref(false);

const searchForm = ref<SearchForm>({
  keyword: '',
  sortBy: 'title'
});

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

const selectedBook = ref<Book | null>(null);
const borrowForm = ref<BorrowForm>({
  days: 30
});

const borrowFormRef = ref();

const borrowRules = {
  days: [
    { required: true, message: t('borrow_days_required'), trigger: 'blur' },
    { type: 'number', min: 1, max: 90, message: t('borrow_days_range'), trigger: 'blur' }
  ]
};

onMounted(() => {
  fetchBooks();
});

const fetchBooks = async () => {
  loading.value = true;
  try {
    const response = await apiService.getBooks(
      searchForm.value.keyword,
      searchForm.value.sortBy,
      pagination.value.currentPage - 1,
      pagination.value.pageSize
    );
    
    if (response.success && response.data) {
      books.value = response.data.books;
      pagination.value.total = response.data.totalCount;
    } else {
      ElMessage.error(response.error || t('fetch_books_error'));
    }
  } catch (error) {
    console.error('Error fetching books:', error);
    ElMessage.error(t('fetch_books_error'));
  } finally {
    loading.value = false;
  }
};

const searchBooks = () => {
  pagination.value.currentPage = 1;
  fetchBooks();
};

const resetSearch = () => {
  searchForm.value.keyword = '';
  searchForm.value.sortBy = 'title';
  pagination.value.currentPage = 1;
  fetchBooks();
};

const handleSizeChange = (size: number) => {
  pagination.value.pageSize = size;
  pagination.value.currentPage = 1;
  fetchBooks();
};

const handleCurrentChange = (page: number) => {
  pagination.value.currentPage = page;
  fetchBooks();
};

const borrowBook = (book: Book) => {
  if (!userStore.user?.id) {
    ElMessage.warning(t('login_required'));
    return;
  }
  
  if (book.availableCopies <= 0) {
    ElMessage.warning(t('book_unavailable'));
    return;
  }
  
  selectedBook.value = book;
  borrowForm.value.days = 30; // Default to 30 days
  borrowDialogVisible.value = true;
};

const confirmBorrow = async () => {
  try {
    const valid = await borrowFormRef.value.validate();
    if (!valid) return;
    
    if (!selectedBook.value || !userStore.user?.id) return;
    
    borrowing.value = true;
    
    const response = await apiService.borrowBook(
      userStore.user.id,
      selectedBook.value.isbn,
      borrowForm.value.days
    );
    
    if (response.success) {
      ElMessage.success(t('borrow_success'));
      borrowDialogVisible.value = false;
      // Refresh the book list to update availability
      fetchBooks();
    } else {
      ElMessage.error(response.error || t('borrow_failed'));
    }
  } catch (error) {
    console.error('Error borrowing book:', error);
    ElMessage.error(t('borrow_error'));
  } finally {
    borrowing.value = false;
  }
};

const closeBorrowDialog = () => {
  borrowDialogVisible.value = false;
  selectedBook.value = null;
};

const viewBookDetails = (book: Book) => {
  ElMessageBox.alert(`
    <div class="book-details">
      <p><strong>${t('isbn')}:</strong> ${book.isbn}</p>
      <p><strong>${t('title')}:</strong> ${book.title}</p>
      <p><strong>${t('author')}:</strong> ${book.author}</p>
      <p><strong>${t('publisher')}:</strong> ${book.publisher}</p>
      <p><strong>${t('category')}:</strong> ${book.category}</p>
      <p><strong>${t('total_copies')}:</strong> ${book.totalCopies}</p>
      <p><strong>${t('available_copies')}:</strong> ${book.availableCopies}</p>
    </div>
  `, t('book_details'), {
    dangerouslyUseHTMLString: true,
    confirmButtonText: t('ok'),
  });
};
</script>

<style scoped>
.search-books {
  padding: 20px;
}

.search-card {
  margin-bottom: 20px;
}

.search-form {
  display: flex;
  flex-wrap: wrap;
}

.search-form .el-form-item {
  margin-right: 20px;
  margin-bottom: 0;
}

.sort-select {
  width: 150px;
}

.books-table {
  margin-bottom: 20px;
}

.pagination {
  text-align: right;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}

.book-details p {
  margin: 5px 0;
}
</style>