<template>
  <div class="modern-dashboard">
    <!-- Welcome Section -->
    <div class="welcome-section fade-in">
      <div class="welcome-content">
        <h1 class="welcome-title">{{ t('welcome_user', { name: userStore.user?.name || 'Admin' }) }}</h1>
        <p class="welcome-subtitle">{{ getCurrentGreeting() }}</p>
      </div>
      <div class="date-time">
        <div class="date">{{ currentDate }}</div>
        <div class="time">{{ currentTime }}</div>
      </div>
    </div>

    <!-- Stats Cards -->
    <div class="stats-grid">
      <div
        v-for="(stat, index) in statsCards"
        :key="stat.key"
        :class="['stat-card', 'glass-card', `slide-up`, `stagger-${index + 1}`]"
        :style="{ '--accent-color': stat.color }"
      >
        <div class="stat-icon" :style="{ background: stat.gradient }">
          <el-icon :size="24"><component :is="stat.icon" /></el-icon>
        </div>
        <div class="stat-content">
          <div class="stat-value">{{ stat.value }}</div>
          <div class="stat-label">{{ t(stat.label) }}</div>
          <div class="stat-change" :class="stat.trend">
            <el-icon :size="14"><component :is="stat.trendIcon" /></el-icon>
            {{ stat.change }}
          </div>
        </div>
      </div>
    </div>

    <!-- Main Content Grid -->
    <div class="content-grid">
      <!-- Recent Borrowings -->
      <div class="content-card glass-card slide-up stagger-5">
        <div class="card-header">
          <div class="header-left">
            <el-icon :size="20"><Reading /></el-icon>
            <h3>{{ t('recent_borrowings') }}</h3>
          </div>
          <el-button text @click="goToBorrowings" class="view-all-btn">
            {{ t('view_all') }}
            <el-icon><ArrowRight /></el-icon>
          </el-button>
        </div>
        <div class="card-body">
          <div v-if="loading" class="loading-state">
            <div class="spinner"></div>
            <p>{{ t('loading') }}...</p>
          </div>
          <div v-else-if="recentBorrowings.length === 0" class="empty-state">
            <el-icon :size="48" class="empty-icon"><Box /></el-icon>
            <p>{{ t('no_recent_borrowings') }}</p>
          </div>
          <div v-else class="borrowings-list">
            <div
              v-for="(record, index) in recentBorrowings"
              :key="record.recordId"
              class="borrowing-item"
              :style="{ animationDelay: `${index * 0.1}s` }"
            >
              <div class="borrowing-avatar">
                {{ record.studentName.charAt(0) }}
              </div>
              <div class="borrowing-info">
                <div class="borrowing-title">{{ record.bookTitle }}</div>
                <div class="borrowing-meta">
                  <el-icon :size="12"><User /></el-icon>
                  {{ record.studentName }}
                  <span class="separator">•</span>
                  <el-icon :size="12"><Calendar /></el-icon>
                  {{ record.borrowDate }}
                </div>
              </div>
              <div class="borrowing-status">
                <span :class="['status-badge', record.isOverdue ? 'overdue' : 'active']">
                  {{ record.isOverdue ? t('overdue') : t('active') }}
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Quick Actions -->
      <div class="content-card glass-card slide-up stagger-6">
        <div class="card-header">
          <div class="header-left">
            <el-icon :size="20"><Lightning /></el-icon>
            <h3>{{ t('quick_actions') }}</h3>
          </div>
        </div>
        <div class="card-body">
          <div class="actions-grid">
            <button
              v-for="action in quickActions"
              :key="action.key"
              @click="action.onClick"
              class="action-button"
              :style="{ '--action-color': action.color }"
            >
              <div class="action-icon">
                <el-icon :size="20"><component :is="action.icon" /></el-icon>
              </div>
              <div class="action-content">
                <div class="action-title">{{ t(action.title) }}</div>
                <div class="action-desc">{{ t(action.description) }}</div>
              </div>
              <el-icon :size="20" class="action-arrow"><ArrowRight /></el-icon>
            </button>
          </div>
        </div>
      </div>

      <!-- System Status -->
      <div class="content-card glass-card slide-up stagger-7">
        <div class="card-header">
          <div class="header-left">
            <el-icon :size="20"><Monitor /></el-icon>
            <h3>{{ t('system_status') }}</h3>
          </div>
        </div>
        <div class="card-body">
          <div class="status-list">
            <div class="status-item">
              <div class="status-indicator active"></div>
              <div class="status-info">
                <div class="status-label">{{ t('api_server') }}</div>
                <div class="status-value">{{ t('online') }}</div>
              </div>
            </div>
            <div class="status-item">
              <div class="status-indicator active"></div>
              <div class="status-info">
                <div class="status-label">{{ t('database') }}</div>
                <div class="status-value">{{ t('connected') }}</div>
              </div>
            </div>
            <div class="status-item">
              <div class="status-indicator active"></div>
              <div class="status-info">
                <div class="status-label">{{ t('last_backup') }}</div>
                <div class="status-value">{{ lastBackup }}</div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue';
import { useRouter } from 'vue-router';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { FullBorrowRecord } from '@/types';
import {
  Collection,
  User,
  Reading,
  Warning,
  Plus,
  UserFilled,
  Document,
  ArrowRight,
  Box,
  Calendar,
  Lightning,
  Monitor,
  CaretTop,
  CaretBottom
} from '@element-plus/icons-vue';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

const loading = ref(true);
const currentDate = ref('');
const currentTime = ref('');
const lastBackup = ref('2 hours ago');

const statsCards = ref([
  {
    key: 'books',
    label: 'total_books',
    value: 0,
    change: '+12%',
    trend: 'up',
    icon: Collection,
    trendIcon: CaretTop,
    color: '#6366f1',
    gradient: 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)'
  },
  {
    key: 'users',
    label: 'total_users',
    value: 0,
    change: '+8%',
    trend: 'up',
    icon: User,
    trendIcon: CaretTop,
    color: '#10b981',
    gradient: 'linear-gradient(135deg, #f093fb 0%, #f5576c 100%)'
  },
  {
    key: 'borrowings',
    label: 'total_borrowings',
    value: 0,
    change: '+23%',
    trend: 'up',
    icon: Reading,
    trendIcon: CaretTop,
    color: '#f59e0b',
    gradient: 'linear-gradient(135deg, #4facfe 0%, #00f2fe 100%)'
  },
  {
    key: 'overdue',
    label: 'overdue_books',
    value: 0,
    change: '-5%',
    trend: 'down',
    icon: Warning,
    trendIcon: CaretBottom,
    color: '#ef4444',
    gradient: 'linear-gradient(135deg, #fa709a 0%, #fee140 100%)'
  }
]);

const recentBorrowings = ref<FullBorrowRecord[]>([]);

const quickActions = computed(() => [
  {
    key: 'add_book',
    title: 'add_new_book',
    description: 'add_book_desc',
    icon: Plus,
    color: '#6366f1',
    onClick: () => router.push('/admin/books')
  },
  {
    key: 'manage_users',
    title: 'user_management',
    description: 'manage_users_desc',
    icon: UserFilled,
    color: '#10b981',
    onClick: () => router.push('/admin/users')
  },
  {
    key: 'view_logs',
    title: 'view_logs',
    description: 'view_logs_desc',
    icon: Document,
    color: '#f59e0b',
    onClick: () => router.push('/admin/logs')
  }
]);

const getCurrentGreeting = () => {
  const hour = new Date().getHours();
  if (hour < 12) return t('good_morning');
  if (hour < 18) return t('good_afternoon');
  return t('good_evening');
};

const updateDateTime = () => {
  const now = new Date();
  currentDate.value = now.toLocaleDateString('en-US', {
    weekday: 'long',
    year: 'numeric',
    month: 'long',
    day: 'numeric'
  });
  currentTime.value = now.toLocaleTimeString('en-US', {
    hour: '2-digit',
    minute: '2-digit'
  });
};

let timeInterval: number;

onMounted(async () => {
  updateDateTime();
  timeInterval = window.setInterval(updateDateTime, 1000);

  try {
    // Fetch real data
    const [booksRes, usersRes, borrowingsRes] = await Promise.all([
      apiService.getBooks(),
      apiService.getUsers(),
      apiService.getFullBorrowRecords('borrowDate')
    ]);

    if (booksRes.success && booksRes.data) {
      statsCards.value[0].value = booksRes.data.totalCount || 0;
    }

    if (usersRes.success && usersRes.data) {
      statsCards.value[1].value = usersRes.data.totalCount || 0;
    }

    if (borrowingsRes.success && borrowingsRes.data) {
      const borrowings = borrowingsRes.data;
      statsCards.value[2].value = borrowings.length;
      statsCards.value[3].value = borrowings.filter(b => b.isOverdue).length;
      recentBorrowings.value = borrowings.slice(0, 5);
    }
  } catch (error) {
    console.error('Error fetching dashboard data:', error);
  } finally {
    loading.value = false;
  }
});

onUnmounted(() => {
  if (timeInterval) {
    clearInterval(timeInterval);
  }
});

const goToBorrowings = () => {
  router.push('/admin/borrowings');
};
</script>

<style scoped>
.modern-dashboard {
  min-height: 100vh;
  padding: 2rem;
  background: white;
  background-attachment: fixed;
}

/* Welcome Section */
.welcome-section {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 2rem;
  padding: 2rem;
  background: white;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
  border-radius: 20px;
  border: 1px solid #eee;
}

.welcome-title {
  font-size: 2rem;
  font-weight: 700;
  color: #333;
  margin: 0 0 0.5rem 0;
}

.welcome-subtitle {
  font-size: 1.1rem;
  color: #666;
  margin: 0;
}

.date-time {
  text-align: right;
  color: #666;
}

.date {
  font-size: 0.9rem;
  opacity: 0.9;
  margin-bottom: 0.25rem;
}

.time {
  font-size: 1.5rem;
  font-weight: 600;
}

/* Stats Grid */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 1.5rem;
  margin-bottom: 2rem;
}

.stat-card {
  display: flex;
  align-items: center;
  gap: 1.5rem;
  padding: 1.5rem;
  cursor: pointer;
  transition: all 0.3s ease;
}

.stat-card:hover {
  transform: translateY(-5px);
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);
}

.stat-icon {
  width: 60px;
  height: 60px;
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  flex-shrink: 0;
}

.stat-content {
  flex: 1;
}

.stat-value {
  font-size: 2rem;
  font-weight: 700;
  color: #333;
  line-height: 1;
  margin-bottom: 0.5rem;
}

.stat-label {
  font-size: 0.9rem;
  color: #666;
  margin-bottom: 0.5rem;
}

.stat-change {
  display: inline-flex;
  align-items: center;
  gap: 0.25rem;
  font-size: 0.85rem;
  font-weight: 600;
  padding: 0.25rem 0.5rem;
  border-radius: 6px;
}

.stat-change.up {
  color: #10b981;
  background: rgba(16, 185, 129, 0.1);
}

.stat-change.down {
  color: #ef4444;
  background: rgba(239, 68, 68, 0.1);
}

/* Content Grid */
.content-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
  gap: 1.5rem;
}

.content-card {
  padding: 0;
  overflow: hidden;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.5rem;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.header-left {
  display: flex;
  align-items: center;
  gap: 0.75rem;
}

.header-left h3 {
  margin: 0;
  font-size: 1.1rem;
  font-weight: 600;
  color: #333;
}

.header-icon {
  color: #666;
}

.view-all-btn {
  color: #667eea !important;
}

.card-body {
  padding: 1.5rem;
}

/* Glass Card Effect */
.glass-card {
  background: white;
  border-radius: 20px;
  border: 1px solid #eee;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
}

/* Borrowings List */
.borrowings-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.borrowing-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
  transition: all 0.3s ease;
  animation: slideIn 0.5s ease forwards;
  opacity: 0;
}

.borrowing-item:hover {
  background: #f8f9ff;
  transform: translateX(5px);
}

.borrowing-avatar {
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background: #667eea;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: 600;
  flex-shrink: 0;
}

.borrowing-info {
  flex: 1;
  min-width: 0;
}

.borrowing-title {
  font-weight: 600;
  color: #333;
  margin-bottom: 0.25rem;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.borrowing-meta {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  font-size: 0.85rem;
  color: #666;
}

.separator {
  color: #ccc;
}

.status-badge {
  padding: 0.25rem 0.75rem;
  border-radius: 12px;
  font-size: 0.75rem;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.status-badge.active {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.status-badge.overdue {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

/* Quick Actions */
.actions-grid {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.action-button {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: white;
  border: 1px solid #eee;
  border-radius: 12px;
  cursor: pointer;
  transition: all 0.3s ease;
  color: #333;
  text-align: left;
}

.action-button:hover {
  background: #f8f9ff;
  transform: translateX(5px);
  border-color: #667eea;
}

.action-icon {
  width: 40px;
  height: 40px;
  border-radius: 10px;
  background: var(--action-color);
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.action-content {
  flex: 1;
}

.action-title {
  font-weight: 600;
  margin-bottom: 0.25rem;
}

.action-desc {
  font-size: 0.85rem;
  color: #666;
}

.action-arrow {
  color: #999;
  transition: transform 0.3s ease;
}

.action-button:hover .action-arrow {
  transform: translateX(5px);
}

/* System Status */
.status-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: white;
  border: 1px solid #eee;
  border-radius: 12px;
}

.status-indicator {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  flex-shrink: 0;
}

.status-indicator.active {
  background: #10b981;
  box-shadow: 0 0 10px rgba(16, 185, 129, 0.5);
  animation: pulse 2s ease-in-out infinite;
}

.status-info {
  flex: 1;
}

.status-label {
  font-size: 0.9rem;
  color: #666;
  margin-bottom: 0.25rem;
}

.status-value {
  font-weight: 600;
  color: #333;
}

/* Loading & Empty States */
.loading-state,
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 3rem 1rem;
  color: #666;
}

.spinner {
  width: 40px;
  height: 40px;
  border: 3px solid #eee;
  border-top-color: #667eea;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 1rem;
}

.empty-icon {
  color: rgba(255, 255, 255, 0.3);
  margin-bottom: 1rem;
}

/* Animations */
@keyframes fadeIn {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

@keyframes slideUp {
  from {
    opacity: 0;
    transform: translateY(30px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateX(-20px);
  }
  to {
    opacity: 1;
    transform: translateX(0);
  }
}

@keyframes spin {
  to {
    transform: rotate(360deg);
  }
}

@keyframes pulse {
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
}

.fade-in {
  animation: fadeIn 0.6s ease forwards;
}

.slide-up {
  animation: slideUp 0.6s ease forwards;
  opacity: 0;
}

.stagger-1 { animation-delay: 0.1s; }
.stagger-2 { animation-delay: 0.2s; }
.stagger-3 { animation-delay: 0.3s; }
.stagger-4 { animation-delay: 0.4s; }
.stagger-5 { animation-delay: 0.5s; }
.stagger-6 { animation-delay: 0.6s; }
.stagger-7 { animation-delay: 0.7s; }

/* Responsive */
@media (max-width: 768px) {
  .modern-dashboard {
    padding: 1rem;
  }

  .welcome-section {
    flex-direction: column;
    align-items: flex-start;
    gap: 1rem;
  }

  .date-time {
    text-align: left;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .content-grid {
    grid-template-columns: 1fr;
  }
}
</style>
