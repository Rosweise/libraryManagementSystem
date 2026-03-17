<template>
  <div class="borrow-management">
    <h2>{{ t('borrow_management') }}</h2>

    <el-card class="controls-card">
      <div class="controls-row">
        <div class="controls-left">
          <el-button type="primary" @click="fetchAllBorrowings" class="refresh-btn">
            <el-icon><Refresh /></el-icon>
            {{ t('refresh') }}
          </el-button>

          <el-select v-model="sortBy" @change="fetchAllBorrowings" class="sort-select">
            <el-option :label="t('sort_by_student_id')" value="studentId" />
            <el-option :label="t('sort_by_due_date')" value="dueDate" />
          </el-select>
        </div>

        <div class="controls-right">
          <el-input
            v-model="studentQueryId"
            :placeholder="t('student_id_placeholder')"
            clearable
            class="query-input"
            @keyup.enter="queryStudentBorrowings()"
          />
          <el-button type="primary" @click="queryStudentBorrowings()">
            {{ t('query_student') }}
          </el-button>
          <el-button @click="resetToAllBorrowings">{{ t('reset') }}</el-button>
        </div>
      </div>
    </el-card>

    <div v-if="isStudentQueryMode" class="query-state">
      {{ t('query_student') }}: {{ studentQueryId }}
    </div>

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
      <el-table-column prop="returnDate" :label="t('return_date')" width="120">
        <template #default="scope">
          {{ scope.row.returnDate || '-' }}
        </template>
      </el-table-column>
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
        <el-table-column prop="returnDate" :label="t('return_date')" width="120">
          <template #default="scope">
            {{ scope.row.returnDate || '-' }}
          </template>
        </el-table-column>
        <el-table-column :label="t('status')" width="100">
          <template #default="scope">
            <el-tag :type="scope.row.returnDate ? 'success' : (isOverdue(scope.row) ? 'danger' : 'warning')">
              {{ scope.row.returnDate ? t('returned') : (isOverdue(scope.row) ? t('overdue') : t('borrowed')) }}
            </el-tag>
          </template>
        </el-table-column>
      </el-table>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { ElMessage } from 'element-plus';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { FullBorrowRecord, StudentBorrowRecord } from '@/types';
import { Refresh } from '@element-plus/icons-vue';

const { t } = useI18n();

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

type BorrowingTableRecord = FullBorrowRecord & {
  bookIsbn?: string;
  returnDate?: string | null;
};

const borrowings = ref<BorrowingTableRecord[]>([]);
const loading = ref(false);
const studentBorrowings = ref<StudentBorrowRecord[]>([]);
const studentBorrowingsLoading = ref(false);
const studentBorrowingsDialogVisible = ref(false);
const studentQueryId = ref('');
const isStudentQueryMode = ref(false);

const sortBy = ref('studentId');

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

const normalizeRecords = (records: Array<FullBorrowRecord | StudentBorrowRecord>): BorrowingTableRecord[] => {
  return records.map((record) => ({
    ...record,
    isOverdue: Boolean(record.isOverdue),
    returnDate: 'returnDate' in record ? record.returnDate ?? null : null
  }));
};

onMounted(() => {
  fetchAllBorrowings();
});

const fetchAllBorrowings = async () => {
  loading.value = true;
  try {
    const response = await apiService.getFullBorrowRecords(sortBy.value);

    if (response.success && response.data) {
      borrowings.value = normalizeRecords(response.data);
      pagination.value.total = response.data.length;
      isStudentQueryMode.value = false;
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

const queryStudentBorrowings = async (studentId?: string) => {
  const targetStudentId = (studentId ?? studentQueryId.value).trim();
  if (!targetStudentId) {
    ElMessage.warning(t('student_id_required'));
    return;
  }

  loading.value = true;
  try {
    const response = await apiService.getStudentBorrowRecords(targetStudentId);
    if (response.success && response.data) {
      borrowings.value = normalizeRecords(response.data);
      pagination.value.total = response.data.length;
      studentQueryId.value = targetStudentId;
      isStudentQueryMode.value = true;
    } else {
      ElMessage.error(response.error || t('fetch_student_borrowings_error'));
    }
  } catch (error) {
    console.error('Error querying student borrowings:', error);
    ElMessage.error(t('fetch_student_borrowings_error'));
  } finally {
    loading.value = false;
  }
};

const resetToAllBorrowings = () => {
  studentQueryId.value = '';
  isStudentQueryMode.value = false;
  fetchAllBorrowings();
};

const handleSizeChange = (size: number) => {
  pagination.value.pageSize = size;
  pagination.value.currentPage = 1;
  if (isStudentQueryMode.value) {
    queryStudentBorrowings();
    return;
  }
  fetchAllBorrowings();
};

const handleCurrentChange = (page: number) => {
  pagination.value.currentPage = page;
  if (isStudentQueryMode.value) {
    queryStudentBorrowings();
    return;
  }
  fetchAllBorrowings();
};

const viewStudentDetails = async (studentId: string) => {
  studentBorrowingsLoading.value = true;
  try {
    const response = await apiService.getStudentBorrowRecords(studentId);
    if (response.success && response.data) {
      studentBorrowings.value = response.data.map((record) => ({
        ...record,
        isOverdue: Boolean(record.isOverdue),
        returnDate: record.returnDate ?? null
      }));
      studentBorrowingsDialogVisible.value = true;
      studentQueryId.value = studentId;
    } else {
      ElMessage.error(response.error || t('fetch_student_borrowings_error'));
    }
  } catch (error) {
    console.error('Error fetching student borrowings:', error);
    ElMessage.error(t('fetch_student_borrowings_error'));
  } finally {
    studentBorrowingsLoading.value = false;
  }
};

const viewBookDetails = (bookTitle: string) => {
  ElMessage.info(`${t('viewing_book')}: ${bookTitle}`);
};

const isOverdue = (record: StudentBorrowRecord): boolean => {
  if (record.returnDate) return false;
  return Boolean(record.isOverdue);
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
}

.controls-row {
  display: flex;
  justify-content: space-between;
  gap: 12px;
  flex-wrap: wrap;
}

.controls-left,
.controls-right {
  display: flex;
  align-items: center;
  gap: 12px;
  flex-wrap: wrap;
}

.sort-select {
  width: 200px;
}

.query-input {
  width: 220px;
}

.query-state {
  margin-bottom: 12px;
  color: #606266;
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
