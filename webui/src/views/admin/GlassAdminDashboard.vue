<template>
  <div class="h-full w-full flex flex-col">
    <!-- Glass Navigation -->
    <nav class="glass-nav sticky top-0 z-50 px-6 py-3">
      <div class="max-w-7xl mx-auto flex items-center justify-between">
        <div class="flex items-center gap-3">
          <div class="w-9 h-9 rounded-xl btn-primary flex items-center justify-center">
            <Library :size="18" class="text-white" />
          </div>
          <span class="font-semibold text-gray-800 text-lg">{{ t('library_system') }}</span>
        </div>

        <div class="hidden sm:flex items-center gap-6">
          <button
            v-for="tab in tabs"
            :key="tab.key"
            @click="currentTab = tab.key"
            :class="[
              'nav-link text-sm font-medium py-1 transition-all',
              currentTab === tab.key ? 'active text-indigo-600' : 'text-gray-600'
            ]"
          >
            <span class="flex items-center gap-1.5">
              <component :is="tab.icon" :size="16" />
              {{ t(tab.label) }}
            </span>
          </button>
        </div>

        <div class="flex items-center gap-3">
          <span class="text-xs font-medium px-3 py-1.5 rounded-full bg-indigo-100 text-indigo-700">
            {{ userStore.user?.name || 'Admin' }}
          </span>
          <button
            @click="handleLogout"
            class="btn-secondary rounded-xl px-3 py-2 text-gray-600 text-sm flex items-center gap-1.5"
          >
            <LogOut :size="15" />
            <span class="hidden sm:inline">{{ t('logout') }}</span>
          </button>
        </div>
      </div>
    </nav>

    <!-- Main Content -->
    <main class="flex-1 overflow-y-auto">
      <div class="max-w-7xl mx-auto p-4 sm:p-6">
        <!-- Welcome Section -->
        <div class="mb-6 slide-up stagger-1">
          <h2 class="text-2xl font-bold text-gray-800">
            {{ t('welcome_user', { name: userStore.user?.name || 'Admin' }) }}
          </h2>
          <p class="text-gray-500 font-light mt-1">
            {{ t('admin_dashboard_subtitle') }}
          </p>
        </div>

        <!-- Stats Cards -->
        <div class="grid grid-cols-2 md:grid-cols-4 gap-4 mb-8">
          <div
            v-for="(stat, index) in stats"
            :key="stat.key"
            :class="['glass rounded-2xl p-5 slide-up', `stagger-${index + 2}`]"
          >
            <div class="flex items-center gap-2 mb-2">
              <div :class="['w-8 h-8 rounded-lg flex items-center justify-center', stat.bgClass]">
                <component :is="stat.icon" :size="16" :class="stat.iconClass" />
              </div>
              <span class="text-xs font-medium text-gray-500 uppercase tracking-wide">
                {{ t(stat.label) }}
              </span>
            </div>
            <span class="text-2xl font-bold text-gray-800">{{ stat.value }}</span>
          </div>
        </div>

        <!-- Tab Content -->
        <div class="fade-in">
          <component :is="currentTabComponent" />
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { useI18n } from 'vue-i18n';
import { useUserStore } from '@/store/user';
import {
  Library,
  BookOpen,
  Users,
  BookMarked,
  FileText,
  LogOut,
  Book,
  Bookmark,
  AlertCircle,
  CheckCircle
} from 'lucide-vue-next';
import { ElMessage } from 'element-plus';
import BookManagement from './BookManagement.vue';
import UserManagement from './UserManagement.vue';
import BorrowManagement from './BorrowManagement.vue';
import LogsManagement from './LogsManagement.vue';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

const currentTab = ref('books');

const tabs = [
  { key: 'books', label: 'book_management', icon: BookOpen },
  { key: 'users', label: 'user_management', icon: Users },
  { key: 'borrowings', label: 'borrow_management', icon: BookMarked },
  { key: 'logs', label: 'log_management', icon: FileText }
];

const stats = ref([
  {
    key: 'total',
    label: 'total_books',
    value: 0,
    icon: Book,
    bgClass: 'bg-indigo-100',
    iconClass: 'text-indigo-600'
  },
  {
    key: 'borrowed',
    label: 'total_borrowings',
    value: 0,
    icon: Bookmark,
    bgClass: 'bg-amber-100',
    iconClass: 'text-amber-600'
  },
  {
    key: 'overdue',
    label: 'overdue_books',
    value: 0,
    icon: AlertCircle,
    bgClass: 'bg-red-100',
    iconClass: 'text-red-600'
  },
  {
    key: 'available',
    label: 'available_books',
    value: 0,
    icon: CheckCircle,
    bgClass: 'bg-green-100',
    iconClass: 'text-green-600'
  }
]);

const currentTabComponent = computed(() => {
  switch (currentTab.value) {
    case 'books':
      return BookManagement;
    case 'users':
      return UserManagement;
    case 'borrowings':
      return BorrowManagement;
    case 'logs':
      return LogsManagement;
    default:
      return BookManagement;
  }
});

const handleLogout = () => {
  userStore.logout();
  ElMessage.success(t('logout_success'));
  router.push('/login');
};

onMounted(() => {
  // Load stats - you can fetch from API
  // For now using placeholder values
  stats.value[0].value = 150;
  stats.value[1].value = 45;
  stats.value[2].value = 3;
  stats.value[3].value = 105;
});
</script>

<style scoped>
.glass-nav {
  background: rgba(255, 255, 255, 0.5);
  backdrop-filter: blur(24px);
  -webkit-backdrop-filter: blur(24px);
  border-bottom: 1px solid rgba(255, 255, 255, 0.4);
}

.glass {
  background: rgba(255, 255, 255, 0.45);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.5);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.06);
}

.btn-primary {
  background: linear-gradient(135deg, #818cf8 0%, #6366f1 100%);
}

.btn-secondary {
  background: rgba(255, 255, 255, 0.6);
  border: 1px solid rgba(200, 210, 230, 0.5);
  transition: all 0.3s ease;
}

.btn-secondary:hover {
  background: rgba(255, 255, 255, 0.85);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.06);
}

.nav-link {
  position: relative;
  transition: all 0.3s ease;
}

.nav-link.active::after {
  content: '';
  position: absolute;
  bottom: -8px;
  left: 0;
  right: 0;
  height: 2px;
  background: linear-gradient(90deg, #818cf8, #6366f1);
  border-radius: 2px;
}

.slide-up {
  animation: slideUp 0.6s ease forwards;
}

@keyframes slideUp {
  from {
    opacity: 0;
    transform: translateY(24px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.stagger-1 { animation-delay: 0.1s; opacity: 0; }
.stagger-2 { animation-delay: 0.2s; opacity: 0; }
.stagger-3 { animation-delay: 0.3s; opacity: 0; }
.stagger-4 { animation-delay: 0.4s; opacity: 0; }
.stagger-5 { animation-delay: 0.5s; opacity: 0; }

.fade-in {
  animation: fadeIn 0.5s ease forwards;
}

@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}
</style>
