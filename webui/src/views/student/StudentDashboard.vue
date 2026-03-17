<template>
  <div class="student-dashboard">
    <h2>{{ t('welcome_user', { name: userStore.user?.name }) }}</h2>
    
    <el-alert 
      v-if="overdueBooks.length > 0" 
      :title="t('overdue_notice')" 
      type="warning" 
      show-icon 
      :closable="false"
      class="overdue-alert"
    >
      <p>{{ t('overdue_count', { count: overdueBooks.length }) }}</p>
    </el-alert>
    
    <el-row :gutter="20" class="dashboard-cards">
      <el-col :span="8">
        <el-card class="dashboard-card">
          <div class="card-content">
            <div class="card-icon">
              <el-icon><Reading /></el-icon>
            </div>
            <div class="card-info">
              <h3>{{ stats.currentBorrowings }}</h3>
              <p>{{ t('current_borrowings') }}</p>
            </div>
          </div>
        </el-card>
      </el-col>
      
      <el-col :span="8">
        <el-card class="dashboard-card">
          <div class="card-content">
            <div class="card-icon">
              <el-icon><Clock /></el-icon>
            </div>
            <div class="card-info">
              <h3>{{ stats.pendingReturns }}</h3>
              <p>{{ t('pending_returns') }}</p>
            </div>
          </div>
        </el-card>
      </el-col>
      
      <el-col :span="8">
        <el-card class="dashboard-card">
          <div class="card-content">
            <div class="card-icon">
              <el-icon><DocumentChecked /></el-icon>
            </div>
            <div class="card-info">
              <h3>{{ stats.history }}</h3>
              <p>{{ t('borrow_history') }}</p>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>
    
    <el-row :gutter="20" class="dashboard-content">
      <el-col :span="16">
        <el-card class="recent-borrowings">
          <template #header>
            <div class="card-header">
              <span>{{ t('my_recent_borrowings') }}</span>
            </div>
          </template>
          <el-table :data="recentBorrowings" style="width: 100%">
            <el-table-column prop="bookTitle" :label="t('book_title')" width="200" />
            <el-table-column prop="borrowDate" :label="t('borrow_date')" width="120" />
            <el-table-column prop="dueDate" :label="t('due_date')" width="120" />
            <el-table-column prop="returnDate" :label="t('return_date')" width="120" />
            <el-table-column prop="status" :label="t('status')" width="100">
              <template #default="scope">
                <el-tag :type="getStatusType(scope.row)">
                  {{ getStatusText(scope.row) }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column :label="t('actions')" width="150">
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
        </el-card>
      </el-col>
      
      <el-col :span="8">
        <el-card class="quick-actions">
          <template #header>
            <div class="card-header">
              <span>{{ t('quick_actions') }}</span>
            </div>
          </template>
          <div class="actions-grid">
            <el-button type="primary" @click="goToSearch" size="large" class="action-btn">
              <el-icon><Search /></el-icon>
              {{ t('search_books') }}
            </el-button>
            <el-button type="success" @click="goToBorrowings" size="large" class="action-btn">
              <el-icon><Reading /></el-icon>
              {{ t('view_my_borrowings') }}
            </el-button>
            <el-button type="warning" @click="goToAccount" size="large" class="action-btn">
              <el-icon><Setting /></el-icon>
              {{ t('account_settings') }}
            </el-button>
          </div>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { BorrowRecord } from '@/types';

// Icons
import { 
  Reading, 
  Clock, 
  DocumentChecked, 
  Search, 
  Setting 
} from '@element-plus/icons-vue';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

interface Stats {
  currentBorrowings: number;
  pendingReturns: number;
  history: number;
}

const stats = ref<Stats>({
  currentBorrowings: 0,
  pendingReturns: 0,
  history: 0
});

const recentBorrowings = ref<BorrowRecord[]>([]);
const overdueBooks = ref<BorrowRecord[]>([]);

onMounted(async () => {
  if (!userStore.user?.id) return;
  
  try {
    // Fetch user's borrowed books
    const borrowResponse = await apiService.getBorrowedBooks(userStore.user.id);
    if (borrowResponse.success && borrowResponse.data) {
      recentBorrowings.value = borrowResponse.data;
      stats.value.currentBorrowings = borrowResponse.data.length;
      stats.value.pendingReturns = borrowResponse.data.filter(b => !b.returnDate).length;
    }
    
    // Fetch overdue books
    const overdueResponse = await apiService.getOverdueBooks(userStore.user.id);
    if (overdueResponse.success && overdueResponse.data) {
      overdueBooks.value = overdueResponse.data;
    }
    
    // Mock history count
    stats.value.history = 42;
  } catch (error) {
    console.error('Error fetching dashboard data:', error);
    ElMessage.error(t('fetch_data_error'));
  }
});

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
    
    const response = await apiService.returnBook(record.recordId, userStore.user!.id);
    if (response.success) {
      ElMessage.success(t('return_success'));
      // Refresh the data
      onMounted();
    } else {
      ElMessage.error(response.error || t('return_failed'));
    }
  } catch (error) {
    // User cancelled
  }
};

const handleRenew = async (record: BorrowRecord) => {
  try {
    await ElMessageBox.confirm(
      t('confirm_renew', { title: record.bookTitle }),
      t('confirm'),
      {
        confirmButtonText: t('confirm'),
        cancelButtonText: t('cancel'),
        type: 'info'
      }
    );
    
    const response = await apiService.renewBook(record.recordId, userStore.user!.id);
    if (response.success && response.data) {
      ElMessage.success(t('renew_success'));
      // Refresh the data
      onMounted();
    } else {
      ElMessage.error(response.error || t('renew_failed'));
    }
  } catch (error) {
    // User cancelled
  }
};

const goToSearch = () => {
  router.push('/student/search');
};

const goToBorrowings = () => {
  router.push('/student/borrowings');
};

const goToAccount = () => {
  router.push('/student/account');
};
</script>

<style scoped>
.student-dashboard {
  padding: 20px;
}

.overdue-alert {
  margin-bottom: 20px;
}

.dashboard-cards {
  margin-bottom: 20px;
}

.dashboard-card {
  text-align: center;
  cursor: pointer;
  transition: transform 0.3s ease;
}

.dashboard-card:hover {
  transform: translateY(-5px);
}

.card-content {
  display: flex;
  align-items: center;
  justify-content: space-around;
}

.card-icon {
  font-size: 24px;
  color: #67c23a;
}

.card-info h3 {
  margin: 0 0 5px 0;
  font-size: 24px;
  font-weight: bold;
}

.card-info p {
  margin: 0;
  color: #909399;
}

.dashboard-content {
  margin-top: 20px;
}

.recent-borrowings {
  height: 100%;
}

.quick-actions {
  height: 100%;
}

.actions-grid {
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.action-btn {
  width: 100%;
  justify-content: flex-start;
}

.card-header {
  font-weight: bold;
}
</style>