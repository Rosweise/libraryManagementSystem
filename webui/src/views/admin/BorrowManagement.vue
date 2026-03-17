<template>
  <div class="borrow-management">
    <h2>{{ t('borrow_management') }}</h2>
    
    <el-card class="controls-card">
      <el-button type="primary" @click="fetchAllBorrowings" class="refresh-btn">
        <el-icon><Refresh /></el-icon>
        {{ t('refresh') }}
      </el-button>
      
      <el-select v-model="sortBy" @change="fetchAllBorrowings" class="sort-select">
        <el-option :label="t('sort_by_student_id')" value="studentId" />
        <el-option :label="t('sort_by_due_date')" value="dueDate" />
      </el-select>
    </el-card>
    
    <el-table 
      :data="borrowings" 
      v-loading="loading"
      class="borrowings-table"
      row-key="recordId"
    >
      <el-table-column prop="studentId" :label="t('student_id')" width="120" />
      <el-table-column prop="studentName" :label="t('student_name')" width="150" />
      <el-table-column prop="studentCollege" :label="t('college')" width="150" show-overflow-tooltip />
      <el-table-column prop="bookTitle" :label="t('book_title')" width="200" show-overflow-tooltip />
      <el-table-column prop="borrowDate" :label="t('borrow_date')" width="120" />
      <el-table-column prop="dueDate" :label="t('due_date')" width="120" />
      <el-table-column :label="t('overdue')" width="100">
        <template #default="scope">
          <el-tag :type="scope.row.isOverdue ? 'danger' : 'success'">
            {{ scope.row.isOverdue ? t('yes') : t('no') }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column :label="t('actions')" width="150" fixed="right">
        <template #default="scope">
          <el-button size="small" @click="viewStudentDetails(scope.row.studentId)" type="info">
            {{ t('view_student') }}
          </el-button>
          <el-button size="small" @click="viewBookDetails(scope.row.bookTitle)" type="primary">
            {{ t('view_book') }}
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
    
    <!-- Student Borrowings Dialog -->
    <el-dialog 
      :title="t('student_borrowings')" 
      v-model="studentBorrowingsDialogVisible" 
      width="80%" 
      :before-close="closeStudentBorrowingsDialog"
    >
      <el-table 
        :data="studentBorrowings" 
        v-loading="studentBorrowingsLoading"
        class="student-borrowings-table"
      >
        <el-table-column prop="bookTitle" :label="t('book_title')" width="200" show-overflow-tooltip />
        <el-table-column prop="bookIsbn" :label="t('isbn')" width="150" />
        <el-table-column prop="borrowDate" :label="t('borrow_date')" width="120" />
        <el-table-column prop="dueDate" :label="t('due_date')" width="120" />
        <el-table-column prop="returnDate" :label="t('return_date')" width="120" />
        <el-table-column :label="t('status')" width="100">
          <template #default="scope">
            <el-tag :type="scope.row.returnDate ? 'success' : (isOverdue(scope.row) ? 'danger' : 'warning')">
              {{ scope.row.returnDate ? t('returned') : (isOverdue(scope.row) ? t('overdue') : t('borrowed') )}}
            </el-tag>
          </template>
        </el-table-column>
      </el-table>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import { ElMessage } from 'element-plus';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { FullBorrowRecord } from '@/types';

// Icons
import { Refresh } from '@element-plus/icons-vue';

const { t } = useI18n();

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

const borrowings = ref<FullBorrowRecord[]>([]);
const loading = ref(false);
const studentBorrowings = ref<any[]>([]); // Using any for simplicity
const studentBorrowingsLoading = ref(false);
const studentBorrowingsDialogVisible = ref(false);

const sortBy = ref('studentId');

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

// Fetch all borrowings initially
fetchAllBorrowings();

const fetchAllBorrowings = async () => {
  loading.value = true;
  try {
    const response = await apiService.getFullBorrowRecords(sortBy.value);
    
    if (response.success && response.data) {
      borrowings.value = response.data;
      pagination.value.total = response.data.length; // Simplified for demo
    } else {
      ElMessage.error(response.error || t('fetch_borrowings_error'));
    }
  } catch (error) {
    console.error('Error fetching borrowings:', error);
    ElMessage.error(t('fetch_borrowings_error'));
  } finally {
    loading.value = false;
  }
};

const handleSizeChange = (size: number) => {
  pagination.value.pageSize = size;
  pagination.value.currentPage = 1;
  fetchAllBorrowings();
};

const handleCurrentChange = (page: number) => {
  pagination.value.currentPage = page;
  fetchAllBorrowings();
};

const viewStudentDetails = async (studentId: string) => {
  studentBorrowingsLoading.value = true;
  try {
    // In a real implementation, we would fetch the student's borrowings
    // For now, we'll simulate with mock data
    studentBorrowings.value = [
      {
        bookTitle: 'Introduction to Algorithms',
        bookIsbn: '978-0262033848',
        borrowDate: '2023-10-15',
        dueDate: '2023-11-15',
        returnDate: null
      },
      {
        bookTitle: 'Clean Code',
        bookIsbn: '978-0132350884',
        borrowDate: '2023-10-20',
        dueDate: '2023-11-20',
        returnDate: '2023-11-18'
      }
    ];
    studentBorrowingsDialogVisible.value = true;
  } catch (error) {
    console.error('Error fetching student borrowings:', error);
    ElMessage.error(t('fetch_student_borrowings_error'));
  } finally {
    studentBorrowingsLoading.value = false;
  }
};

const viewBookDetails = (bookTitle: string) => {
  // In a real implementation, we would navigate to the book details page
  ElMessage.info(`${t('viewing_book')}: ${bookTitle}`);
};

const isOverdue = (record: any): boolean => {
  if (record.returnDate) return false;
  return new Date(record.dueDate) < new Date();
};

const closeStudentBorrowingsDialog = () => {
  studentBorrowingsDialogVisible.value = false;
  studentBorrowings.value = [];
};
</script>

<style scoped>
.borrow-management {
  padding: 20px;
}

.controls-card {
  margin-bottom: 20px;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.refresh-btn {
  margin-right: 20px;
}

.sort-select {
  width: 200px;
}

.borrowings-table {
  margin-bottom: 20px;
}

.pagination {
  text-align: right;
}

.student-borrowings-table {
  margin-top: 20px;
}
</style>