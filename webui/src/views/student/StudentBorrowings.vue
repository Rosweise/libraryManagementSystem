<template>
  <div class="student-borrowings">
    <h2>{{ t('my_borrowings') }}</h2>
    
    <el-tabs v-model="activeTab" class="borrowings-tabs">
      <el-tab-pane :label="t('current_borrowings')" name="current">
        <el-table 
          :data="currentBorrowings" 
          v-loading="loading"
          class="borrowings-table"
        >
          <el-table-column prop="bookTitle" :label="t('book_title')" width="200" show-overflow-tooltip />
          <el-table-column prop="bookIsbn" :label="t('isbn')" width="150" />
          <el-table-column prop="borrowDate" :label="t('borrow_date')" width="120" />
          <el-table-column prop="dueDate" :label="t('due_date')" width="120" />
          <el-table-column prop="returnDate" :label="t('return_date')" width="120" />
          <el-table-column :label="t('status')" width="100">
            <template #default="scope">
              <el-tag :type="getStatusType(scope.row)">
                {{ getStatusText(scope.row) }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column :label="t('actions')" width="200">
            <template #default="scope">
              <el-button 
                size="small" 
                @click="handleReturn(scope.row)"
                :disabled="isReturned(scope.row)"
                type="danger"
              >
                {{ t('return_book') }}
              </el-button>
              <el-button 
                size="small" 
                @click="handleRenew(scope.row)"
                :disabled="isReturned(scope.row) || isOverdue(scope.row)"
                type="primary"
              >
                {{ t('renew_book') }}
              </el-button>
            </template>
          </el-table-column>
        </el-table>
      </el-tab-pane>
      
      <el-tab-pane :label="t('borrow_history')" name="history">
        <el-table 
          :data="borrowHistory" 
          v-loading="loading"
          class="borrowings-table"
        >
          <el-table-column prop="bookTitle" :label="t('book_title')" width="200" show-overflow-tooltip />
          <el-table-column prop="bookIsbn" :label="t('isbn')" width="150" />
          <el-table-column prop="borrowDate" :label="t('borrow_date')" width="120" />
          <el-table-column prop="dueDate" :label="t('due_date')" width="120" />
          <el-table-column prop="returnDate" :label="t('return_date')" width="120" />
          <el-table-column :label="t('status')" width="100">
            <template #default="scope">
              <el-tag type="success">
                {{ t('returned') }}
              </el-tag>
            </template>
          </el-table-column>
        </el-table>
      </el-tab-pane>
    </el-tabs>
    
    <!-- Renew Dialog -->
    <el-dialog 
      :title="t('renew_book')" 
      v-model="renewDialogVisible" 
      width="400px" 
      :before-close="closeRenewDialog"
    >
      <p>{{ t('renew_confirmation', { title: selectedRecord?.bookTitle }) }}</p>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="closeRenewDialog">{{ t('cancel') }}</el-button>
          <el-button type="primary" @click="confirmRenew" :loading="renewing">
            {{ t('confirm_renew') }}
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
import { BorrowRecord } from '@/types';

const { t } = useI18n();
const userStore = useUserStore();

const activeTab = ref('current');
const currentBorrowings = ref<BorrowRecord[]>([]);
const borrowHistory = ref<BorrowRecord[]>([]);
const loading = ref(false);
const renewing = ref(false);
const renewDialogVisible = ref(false);
const selectedRecord = ref<BorrowRecord | null>(null);

onMounted(() => {
  if (userStore.user?.id) {
    fetchBorrowings(userStore.user.id);
  }
});

const fetchBorrowings = async (userId: string) => {
  loading.value = true;
  try {
    const response = await apiService.getBorrowedBooks(userId);
    
    if (response.success && response.data) {
      // Separate current borrowings and history
      currentBorrowings.value = response.data.filter(record => !record.returnDate);
      borrowHistory.value = response.data.filter(record => record.returnDate);
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

const getStatusType = (record: BorrowRecord): string => {
  if (record.returnDate) return 'success';
  if (isOverdue(record)) return 'danger';
  return 'warning';
};

const getStatusText = (record: BorrowRecord): string => {
  if (record.returnDate) return t('returned');
  if (isOverdue(record)) return t('overdue');
  return t('borrowed');
};

const isReturned = (record: BorrowRecord): boolean => {
  return !!record.returnDate;
};

const isOverdue = (record: BorrowRecord): boolean => {
  if (record.returnDate) return false;
  return new Date(record.dueDate) < new Date();
};

const handleReturn = async (record: BorrowRecord) => {
  try {
    await ElMessageBox.confirm(
      t('confirm_return', { title: record.bookTitle }),
      t('confirm'),
      {
        confirmButtonText: t('confirm'),
        cancelButtonText: t('cancel'),
        type: 'warning'
      }
    );
    
    if (!userStore.user?.id) return;
    
    const response = await apiService.returnBook(record.recordId, userStore.user.id);
    if (response.success) {
      ElMessage.success(t('return_success'));
      // Refresh the data
      if (userStore.user.id) {
        fetchBorrowings(userStore.user.id);
      }
    } else {
      ElMessage.error(response.error || t('return_failed'));
    }
  } catch (error) {
    // User cancelled
  }
};

const handleRenew = async (record: BorrowRecord) => {
  if (!userStore.user?.id) return;
  
  selectedRecord.value = record;
  renewDialogVisible.value = true;
};

const confirmRenew = async () => {
  if (!selectedRecord.value || !userStore.user?.id) return;
  
  try {
    renewing.value = true;
    
    const response = await apiService.renewBook(selectedRecord.value.recordId, userStore.user.id);
    if (response.success) {
      ElMessage.success(t('renew_success'));
      renewDialogVisible.value = false;
      // Refresh the data
      if (userStore.user.id) {
        fetchBorrowings(userStore.user.id);
      }
    } else {
      ElMessage.error(response.error || t('renew_failed'));
    }
  } catch (error) {
    console.error('Error renewing book:', error);
    ElMessage.error(t('renew_error'));
  } finally {
    renewing.value = false;
  }
};

const closeRenewDialog = () => {
  renewDialogVisible.value = false;
  selectedRecord.value = null;
};
</script>

<style scoped>
.student-borrowings {
  padding: 20px;
}

.borrowings-tabs {
  margin-top: 20px;
}

.borrowings-table {
  margin-top: 20px;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}
</style>