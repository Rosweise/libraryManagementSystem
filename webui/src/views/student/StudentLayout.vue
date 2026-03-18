<template>
  <div class="student-layout">
    <el-container>
      <!-- Sidebar -->
      <el-aside :width="sidebarWidth" class="sidebar">
        <div class="logo">
          <h3>{{ t('library_system') }}</h3>
        </div>
        <el-menu
          :default-active="$route.path"
          class="sidebar-menu"
          :collapse="isCollapse"
          router
          unique-opened
        >
          <el-menu-item index="/student">
            <el-icon><House /></el-icon>
            <span>{{ t('dashboard') }}</span>
          </el-menu-item>

          <el-menu-item index="/student/search">
            <el-icon><Search /></el-icon>
            <span>{{ t('search_books') }}</span>
          </el-menu-item>

          <el-menu-item index="/student/borrowings">
            <el-icon><Reading /></el-icon>
            <span>{{ t('my_borrowings') }}</span>
          </el-menu-item>

          <el-menu-item index="/student/account">
            <el-icon><Setting /></el-icon>
            <span>{{ t('account_settings') }}</span>
          </el-menu-item>
        </el-menu>
      </el-aside>

      <!-- Main content -->
      <el-container class="main-container">
        <el-header class="header">
          <div class="header-left">
            <el-tooltip :content="t('back_to_home')" placement="bottom">
              <el-button @click="goBack" class="back-btn">
                <el-icon><ArrowLeft /></el-icon>
              </el-button>
            </el-tooltip>
            <el-button @click="toggleCollapse" class="collapse-btn">
              <el-icon><Menu /></el-icon>
            </el-button>
          </div>

          <div class="header-right">
            <el-badge :value="overdueCount" class="item" type="danger" v-if="overdueCount > 0">
              <el-button size="small" type="warning" @click="goToBorrowings">
                {{ t('overdue_notice') }}
              </el-button>
            </el-badge>

            <el-dropdown @command="handleCommand">
              <span class="user-info">
                <el-avatar size="small" class="avatar">
                  {{ userStore.user?.name.charAt(0) || 'U' }}
                </el-avatar>
                <span class="username">{{ userStore.user?.name }}</span>
                <el-icon class="el-icon--right"><ArrowDown /></el-icon>
              </span>
              <template #dropdown>
                <el-dropdown-menu>
                  <el-dropdown-item command="profile">{{ t('profile') }}</el-dropdown-item>
                  <el-dropdown-item command="settings">{{ t('settings') }}</el-dropdown-item>
                  <el-dropdown-item command="logout" divided>{{ t('logout') }}</el-dropdown-item>
                </el-dropdown-menu>
              </template>
            </el-dropdown>
          </div>
        </el-header>

        <el-main class="main-content">
          <router-view />
        </el-main>
      </el-container>
    </el-container>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';

// Icons
import {
  House,
  Search,
  Reading,
  Setting,
  Menu,
  ArrowDown,
  ArrowLeft
} from '@element-plus/icons-vue';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

const isCollapse = ref(false);
const overdueCount = ref(0);

const sidebarWidth = computed(() => {
  return isCollapse.value ? '64px' : '220px';
});

onMounted(async () => {
  if (userStore.user?.id) {
    try {
      const response = await apiService.getOverdueBooks(userStore.user.id);
      if (response.success && response.data) {
        overdueCount.value = response.data.length;
      }
    } catch (error) {
      console.error('Error fetching overdue books:', error);
    }
  }
});

const toggleCollapse = () => {
  isCollapse.value = !isCollapse.value;
};

const goBack = () => {
  if (window.history.length > 1) {
    router.back();
    return;
  }
  router.push('/student');
};

const goToBorrowings = () => {
  router.push('/student/borrowings');
};

const handleCommand = async (command: string) => {
  if (command === 'logout') {
    try {
      await ElMessageBox.confirm(
        t('logout_confirm'),
        t('confirm'),
        {
          confirmButtonText: t('confirm'),
          cancelButtonText: t('cancel'),
          type: 'warning'
        }
      );

      userStore.logout();
      ElMessage.success(t('logout_success'));
      router.push('/login');
    } catch (error) {
      // User cancelled logout
    }
  } else if (command === 'profile' || command === 'settings') {
    router.push('/student/account');
  }
  // Add other command handlers as needed
};
</script>

<style scoped>
.student-layout {
  height: 100vh;
  width: 100vw;
  overflow: hidden;
}

.el-container {
  height: 100%;
}

.sidebar {
  background: linear-gradient(180deg, #1f2937 0%, #111827 100%);
  color: white;
  height: 100vh;
  overflow-x: hidden;
  transition: width 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.logo {
  height: 60px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  background: linear-gradient(135deg, rgba(255, 255, 255, 0.05) 0%, transparent 100%);
}

.logo h3 {
  margin: 0;
  color: white;
  font-size: 16px;
  font-weight: 600;
  letter-spacing: -0.01em;
  white-space: nowrap;
  overflow: hidden;
}

.sidebar-menu {
  border-right: none;
  background: transparent;
}

.sidebar-menu:not(.el-menu--collapse) {
  width: 220px;
}

.sidebar-menu.el-menu--collapse {
  width: 64px;
}

:deep(.sidebar-menu .el-menu-item) {
  color: #cbd5e1;
  border-radius: 8px;
  margin: 6px 8px;
  height: 44px;
  line-height: 44px;
  transition: all 0.2s ease;
}

:deep(.sidebar-menu .el-menu-item:hover) {
  background-color: rgba(255, 255, 255, 0.1);
  color: #fff;
}

:deep(.sidebar-menu .el-menu-item.is-active) {
  background: linear-gradient(90deg, rgba(99, 102, 241, 0.3) 0%, rgba(99, 102, 241, 0.1) 100%);
  color: #fff;
  box-shadow: 0 2px 8px rgba(99, 102, 241, 0.2);
}

:deep(.sidebar-menu .el-icon) {
  font-size: 18px;
}

.main-container {
  height: 100%;
  overflow: hidden;
}

.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  background: linear-gradient(135deg, #ffffff 0%, #f8fafc 100%);
  box-shadow: 0 1px 3px rgba(15, 23, 42, 0.08), 0 1px 2px rgba(15, 23, 42, 0.06);
  padding: 0 20px;
  height: 60px !important;
  flex-shrink: 0;
}

.header-left {
  display: flex;
  align-items: center;
}

.collapse-btn {
  margin-right: 20px;
  border-radius: 8px;
  border: none;
  background-color: #f3f4f6;
  transition: all 0.2s ease;
}

.collapse-btn:hover {
  background-color: #e5e7eb;
  transform: scale(1.05);
}

.back-btn {
  margin-right: 10px;
  border-radius: 8px;
  border: none;
  background-color: #f3f4f6;
  transition: all 0.2s ease;
}

.back-btn:hover {
  background-color: #e5e7eb;
  transform: translateX(-2px);
}

.header-right {
  display: flex;
  align-items: center;
  gap: 15px;
}

.user-info {
  display: flex;
  align-items: center;
  cursor: pointer;
  padding: 6px 12px;
  border-radius: 8px;
  transition: all 0.2s ease;
}

.user-info:hover {
  background-color: #f3f4f6;
}

.avatar {
  margin-right: 8px;
  background: linear-gradient(135deg, #6366f1 0%, #4f46e5 100%);
}

.username {
  margin-right: 5px;
  font-weight: 500;
  color: #334155;
}

.header-right :deep(.el-button--warning) {
  background: linear-gradient(135deg, #f97316 0%, #ea580c 100%);
  border: none;
  border-radius: 8px;
  font-weight: 500;
  transition: all 0.2s ease;
}

.header-right :deep(.el-button--warning:hover) {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(234, 88, 12, 0.3);
}

.main-content {
  background-color: #f8fafc;
  padding: 24px;
  overflow-y: auto;
  height: calc(100vh - 60px);
}

/* Scrollbar styling */
.main-content::-webkit-scrollbar {
  width: 6px;
}

.main-content::-webkit-scrollbar-track {
  background: #f1f5f9;
  border-radius: 3px;
}

.main-content::-webkit-scrollbar-thumb {
  background: #cbd5e1;
  border-radius: 3px;
}

.main-content::-webkit-scrollbar-thumb:hover {
  background: #94a3b8;
}
</style>
