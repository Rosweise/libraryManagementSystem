<template>
  <div class="book-management">
    <div class="header-section">
      <h2>{{ t('book_management') }}</h2>
      <el-button type="primary" @click="addNewBook" class="add-book-btn">
        <el-icon><Plus /></el-icon>
        {{ t('add_new_book') }}
      </el-button>
    </div>
    
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
      <el-table-column :label="t('actions')" width="200" fixed="right">
        <template #default="scope">
          <el-button size="small" @click="editBook(scope.row)" type="primary">
            {{ t('edit') }}
          </el-button>
          <el-button size="small" @click="viewBookDetails(scope.row)" type="info">
            {{ t('details') }}
          </el-button>
          <el-popconfirm
            :title="t('delete_confirm', { title: scope.row.title })"
            @confirm="deleteBook(scope.row.isbn)"
          >
            <template #reference>
              <el-button size="small" type="danger">
                {{ t('delete') }}
              </el-button>
            </template>
          </el-popconfirm>
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
    
    <!-- Book Dialog -->
    <el-dialog 
      :title="dialogTitle" 
      v-model="dialogVisible" 
      width="50%" 
      :before-close="handleClose"
    >
      <el-form 
        :model="currentBook" 
        :rules="bookRules" 
        ref="bookFormRef"
        label-width="120px"
      >
        <el-form-item :label="t('isbn')" prop="isbn">
          <el-input 
            v-model="currentBook.isbn" 
            :disabled="isEdit"
            :placeholder="t('isbn_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('title')" prop="title">
          <el-input 
            v-model="currentBook.title" 
            :placeholder="t('title_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('author')" prop="author">
          <el-input 
            v-model="currentBook.author" 
            :placeholder="t('author_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('publisher')" prop="publisher">
          <el-input 
            v-model="currentBook.publisher" 
            :placeholder="t('publisher_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('category')" prop="category">
          <el-select v-model="currentBook.category" style="width: 100%">
            <el-option :label="t('fiction')" value="Fiction" />
            <el-option :label="t('non_fiction')" value="Non-Fiction" />
            <el-option :label="t('science')" value="Science" />
            <el-option :label="t('technology')" value="Technology" />
            <el-option :label="t('literature')" value="Literature" />
            <el-option :label="t('history')" value="History" />
            <el-option :label="t('biography')" value="Biography" />
            <el-option :label="t('other')" value="Other" />
          </el-select>
        </el-form-item>
        
        <el-form-item :label="t('total_copies')" prop="totalCopies">
          <el-input-number 
            v-model="currentBook.totalCopies" 
            :min="1"
            :max="9999"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="handleClose">{{ t('cancel') }}</el-button>
          <el-button type="primary" @click="saveBook" :loading="saving">
            {{ isEdit ? t('update') : t('create') }}
          </el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { Book } from '@/types';

// Icons
import { Plus } from '@element-plus/icons-vue';

const { t } = useI18n();

interface SearchForm {
  keyword: string;
  sortBy: string;
}

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

const books = ref<Book[]>([]);
const loading = ref(false);
const saving = ref(false);
const dialogVisible = ref(false);
const isEdit = ref(false);

const searchForm = ref<SearchForm>({
  keyword: '',
  sortBy: 'title'
});

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

const currentBook = ref<Book>({
  isbn: '',
  title: '',
  author: '',
  publisher: '',
  category: 'Other',
  totalCopies: 1,
  availableCopies: 1
});

const bookFormRef = ref();

const dialogTitle = computed(() => {
  return isEdit.value ? t('edit_book') : t('add_new_book');
});

const bookRules = {
  isbn: [
    { required: true, message: t('isbn_required'), trigger: 'blur' },
    { min: 10, max: 20, message: t('isbn_length'), trigger: 'blur' }
  ],
  title: [
    { required: true, message: t('title_required'), trigger: 'blur' }
  ],
  author: [
    { required: true, message: t('author_required'), trigger: 'blur' }
  ],
  publisher: [
    { required: true, message: t('publisher_required'), trigger: 'blur' }
  ],
  totalCopies: [
    { required: true, message: t('total_copies_required'), trigger: 'blur' },
    { type: 'number', min: 1, message: t('total_copies_min'), trigger: 'blur' }
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

const addNewBook = () => {
  isEdit.value = false;
  currentBook.value = {
    isbn: '',
    title: '',
    author: '',
    publisher: '',
    category: 'Other',
    totalCopies: 1,
    availableCopies: 1
  };
  dialogVisible.value = true;
};

const editBook = (book: Book) => {
  isEdit.value = true;
  currentBook.value = { ...book };
  dialogVisible.value = true;
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

const saveBook = async () => {
  try {
    const valid = await bookFormRef.value.validate();
    if (!valid) return;
    
    saving.value = true;
    
    if (isEdit.value) {
      // Update existing book
      const response = await apiService.updateBook(currentBook.value);
      if (response.success) {
        ElMessage.success(t('book_updated_success'));
        dialogVisible.value = false;
        fetchBooks();
      } else {
        ElMessage.error(response.error || t('book_update_error'));
      }
    } else {
      // Add new book
      const response = await apiService.addBook(currentBook.value);
      if (response.success) {
        ElMessage.success(t('book_added_success'));
        dialogVisible.value = false;
        fetchBooks();
      } else {
        ElMessage.error(response.error || t('book_add_error'));
      }
    }
  } catch (error) {
    console.error('Error saving book:', error);
    ElMessage.error(t('save_error'));
  } finally {
    saving.value = false;
  }
};

const deleteBook = async (isbn: string) => {
  try {
    const response = await apiService.deleteBook(isbn);
    if (response.success) {
      ElMessage.success(t('book_deleted_success'));
      fetchBooks();
    } else {
      ElMessage.error(response.error || t('book_delete_error'));
    }
  } catch (error) {
    console.error('Error deleting book:', error);
    ElMessage.error(t('delete_error'));
  }
};

const handleClose = () => {
  dialogVisible.value = false;
};
</script>

<style scoped>
.book-management {
  padding: 20px;
}

.header-section {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.add-book-btn {
  margin-left: 20px;
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