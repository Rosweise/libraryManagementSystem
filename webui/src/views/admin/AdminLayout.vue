<template>
  <div class="admin-layout">
    <el-container>
      <!-- Sidebar -->
      <el-aside width="200px" class="sidebar">
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
          <el-menu-item index="/admin">
            <el-icon><House /></el-icon>
            <span>{{ t('dashboard') }}</span>
          </el-menu-item>
          
          <el-sub-menu index="book-management">
            <template #title>
              <el-icon><Collection /></el-icon>
              <span>{{ t('book_management') }}</span>
            </template>
            <el-menu-item index="/admin/books">{{ t('book_list') }}</el-menu-item>
            <el-menu-item index="/admin/books/add">{{ t('add_book') }}</el-menu-item>
          </el-sub-menu>
          
          <el-sub-menu index="user-management" v-if="userStore.hasPermission(2)">
            <template #title>
              <el-icon><User /></el-icon>
              <span>{{ t('user_management') }}</span>
            </template>
            <el-menu-item index="/admin/users">{{ t('user_list') }}</el-menu-item>
            <el-menu-item index="/admin/users/add">{{ t('add_user') }}</el-menu-item>
          </el-sub-menu>
          
          <el-sub-menu index="borrow-management" v-if="userStore.hasPermission(0)">
            <template #title>
              <el-icon><Reading /></el-icon>
              <span>{{ t('borrow_management') }}</span>
            </template>
            <el-menu-item index="/admin/borrowings">{{ t('borrow_records') }}</el-menu-item>
            <el-menu-item index="/admin/borrowings/query">{{ t('query_student') }}</el-menu-item>
          </el-sub-menu>
          
          <el-sub-menu index="log-management" v-if="userStore.hasPermission(3)">
            <template #title>
              <el-icon><Document /></el-icon>
              <span>{{ t('log_management') }}</span>
            </template>
            <el-menu-item index="/admin/logs">{{ t('view_logs') }}</el-menu-item>
          </el-sub-menu>
          
          <el-menu-item index="/admin/account">
            <el-icon><Setting /></el-icon>
            <span>{{ t('account_settings') }}</span>
          </el-menu-item>
        </el-menu>
      </el-aside>
      
      <!-- Main content -->
      <el-container>
        <el-header class="header">
          <div class="header-left">
            <el-button @click="toggleCollapse" class="collapse-btn">
              <el-icon><Menu /></el-icon>
            </el-button>
          </div>
          
          <div class="header-right">
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
import { ref } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';

// Icons
import { 
  House, 
  Collection, 
  User, 
  Reading, 
  Document, 
  Setting, 
  Menu, 
  ArrowDown,
  CaretLeft,
  CaretRight
} from '@element-plus/icons-vue';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

const isCollapse = ref(false);

const toggleCollapse = () => {
  isCollapse.value = !isCollapse.value;
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
  } else if (command === 'profile') {
    router.push('/admin/account');
  }
  // Add other command handlers as needed
};
</script>

<style scoped>
.admin-layout {
  height: 100vh;
}

.sidebar {
  background-color: #545c64;
  color: white;
  height: 100vh;
  position: fixed;
  top: 0;
  left: 0;
  z-index: 100;
}

.logo {
  height: 60px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-bottom: 1px solid #434a50;
}

.logo h3 {
  margin: 0;
  color: white;
}

.sidebar-menu:not(.el-menu--collapse) {
  width: 200px;
}

.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  background-color: white;
  box-shadow: 0 1px 4px rgba(0,21,41,.08);
  padding: 0 20px;
  height: 60px !important;
}

.header-left {
  display: flex;
  align-items: center;
}

.collapse-btn {
  margin-right: 20px;
}

.header-right {
  display: flex;
  align-items: center;
}

.user-info {
  display: flex;
  align-items: center;
  cursor: pointer;
}

.avatar {
  margin-right: 8px;
}

.main-content {
  margin-top: 20px;
  background-color: #f0f2f5;
  min-height: calc(100vh - 80px);
}
</style>