<template>
  <div class="student-layout">
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
      <el-container>
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
import { ref, onMounted } from 'vue';
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
  } else if (command === 'profile') {
    router.push('/student/account');
  }
  // Add other command handlers as needed
};
</script>

<style scoped>
.student-layout {
  height: 100vh;
}

.sidebar {
  background-color: #545c64;
  color: white; /* Keeping white text for dark sidebar */
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
  color: white; /* Keeping white text for dark sidebar */
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

.back-btn {
  margin-right: 10px;
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
