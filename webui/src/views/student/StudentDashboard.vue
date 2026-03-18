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
      <el-col :span="24">
        <el-card class="dashboard-stats-list">
          <ul class="stats-list">
            <li class="stats-item">
              <span class="stats-label">{{ t('current_borrowings') }}</span>
              <span class="stats-value">{{ stats.currentBorrowings }}</span>
            </li>
            <li class="stats-item">
              <span class="stats-label">{{ t('pending_returns') }}</span>
              <span class="stats-value">{{ stats.pendingReturns }}</span>
            </li>
            <li class="stats-item">
              <span class="stats-label">{{ t('borrow_history') }}</span>
              <span class="stats-value">{{ stats.history }}</span>
            </li>
          </ul>
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

const loadDashboardData = async () => {
  if (!userStore.user?.id) return;

  try {
    // Fetch all borrowings (current + history)
    const allBorrowResponse = await apiService.getAllBorrowings(userStore.user.id);
    if (allBorrowResponse.success && allBorrowResponse.data) {
      const allBorrowings = allBorrowResponse.data;
      recentBorrowings.value = allBorrowings.slice(0, 5); // Show only recent 5
      stats.value.currentBorrowings = allBorrowings.filter(b => !b.returnDate).length;
      stats.value.pendingReturns = allBorrowings.filter(b => !b.returnDate).length;
      stats.value.history = allBorrowings.filter(b => b.returnDate).length;
    }

    // Fetch overdue books
    const overdueResponse = await apiService.getOverdueBooks(userStore.user.id);
    if (overdueResponse.success && overdueResponse.data) {
      overdueBooks.value = overdueResponse.data;
    }
  } catch (error) {
    console.error('Error fetching dashboard data:', error);
    ElMessage.error(t('fetch_data_error'));
  }
};

onMounted(() => {
  loadDashboardData();
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
      await loadDashboardData();
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
      await loadDashboardData();
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
  min-height: 100%;
  padding: 0;
}

.student-dashboard h2 {
  margin: 0 0 1.5rem 0;
  font-size: 2rem;
  font-weight: 700;
  color: #1f2937;
  letter-spacing: -0.01em;
}

.overdue-alert {
  margin-bottom: 1.5rem;
  border-radius: 14px;
}

.dashboard-cards {
  margin-bottom: 1.5rem;
}

.dashboard-stats-list {
  border: 1px solid #e2e8f0;
  border-radius: 20px;
  background: #fff;
  box-shadow: 0 4px 20px rgba(15, 23, 42, 0.08);
}

:deep(.dashboard-stats-list .el-card__body) {
  padding: 0;
}

.stats-list {
  list-style: none;
  margin: 0;
  padding: 0;
}

.stats-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 20px;
  border-bottom: 1px solid #e2e8f0;
}

.stats-item:last-child {
  border-bottom: none;
}

.stats-label {
  font-size: 0.95rem;
  font-weight: 500;
  color: #334155;
  line-height: 1.35;
}

.stats-value {
  margin-left: 12px;
  font-size: 1.75rem;
  font-weight: 700;
  color: #1f2937;
  line-height: 1;
  flex-shrink: 0;
}

.dashboard-content {
  margin-top: 1.5rem;
}

.recent-borrowings {
  height: 100%;
  min-height: 400px;
  border: 1px solid #e2e8f0;
  border-radius: 20px;
  box-shadow: 0 4px 20px rgba(15, 23, 42, 0.08);
}

.quick-actions {
  height: 100%;
  min-height: 400px;
  border: 1px solid #e2e8f0;
  border-radius: 20px;
  box-shadow: 0 4px 20px rgba(15, 23, 42, 0.08);
}

:deep(.recent-borrowings .el-card__header),
:deep(.quick-actions .el-card__header) {
  background-color: transparent;
  border-bottom: 1px solid #e2e8f0;
  padding: 1.25rem 1.5rem;
}

:deep(.recent-borrowings .el-card__body),
:deep(.quick-actions .el-card__body) {
  padding: 1.25rem 1.5rem;
}

.actions-grid {
  display: flex;
  flex-direction: column;
  gap: 0.9rem;
}

.action-btn {
  width: 100%;
  justify-content: space-between;
  height: 48px;
  font-size: 15px;
  font-weight: 500;
  border-radius: 12px;
  border: 1px solid #e2e8f0;
  background: #fff;
  color: #334155;
  transition: all 0.3s ease;
}

.action-btn:hover {
  transform: translateX(4px);
  border-color: #6366f1;
  background: #f8faff;
}

.card-header {
  font-size: 1.1rem;
  font-weight: 600;
  color: #1f2937;
}

:deep(.recent-borrowings .el-table th) {
  background-color: #f8fafc;
}

@media (max-width: 992px) {
  .stats-value {
    font-size: 1.6rem;
  }
}

@media (max-width: 768px) {
  .student-dashboard h2 {
    font-size: 1.5rem;
  }

  .dashboard-cards {
    margin-bottom: 1rem;
  }

  .stats-item {
    padding: 14px 16px;
  }

  .stats-label {
    font-size: 0.88rem;
  }

  .stats-value {
    font-size: 1.4rem;
  }
}
</style>
