<template>
  <div class="login-page">
    <!-- Animated background elements -->
    <div class="background-elements">
      <div class="circle circle-1"></div>
      <div class="circle circle-2"></div>
      <div class="circle circle-3"></div>
      <div class="circle circle-4"></div>
    </div>

    <!-- Main login container -->
    <div class="login-container">
      <div class="login-card">
        <!-- Header with logo and title -->
        <div class="login-header">
          <div class="logo-container">
            <div class="logo-icon">
              <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M2 19a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V7.5L14.5 2H6a2 2 0 0 0-2 2v3"></path>
                <polyline points="17 13 12 8 5 15"></polyline>
                <line x1="12" y1="8" x2="12" y2="15"></line>
              </svg>
            </div>
            <h1 class="logo-text">{{ t('library_system') }}</h1>
          </div>
          <p class="subtitle">{{ t('welcome_message') }}</p>
        </div>

        <!-- Role selection tabs -->
        <div class="role-tabs">
          <button 
            @click="setRole('admin')" 
            :class="['tab-button', { active: selectedRole === 'admin' }]"
          >
            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <polygon points="12 2 2 7 12 12 22 7 12 2"></polygon>
              <polyline points="2 17 12 22 22 17"></polyline>
              <polyline points="2 12 12 17 22 12"></polyline>
            </svg>
            {{ t('admin') }}
          </button>
          <button 
            @click="setRole('student')" 
            :class="['tab-button', { active: selectedRole === 'student' }]"
          >
            <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <path d="M14 19a6 6 0 0 0-12 0"></path>
              <path d="M22 19a6 6 0 0 0-12 0"></path>
              <path d="M12 19a6 6 0 0 0-6-6"></path>
              <path d="M12 13c4 0 6-4 6-6"></path>
              <path d="M12 2a6 6 0 0 0-6 6c0 4 2 6 6 6"></path>
            </svg>
            {{ t('student') }}
          </button>
        </div>

        <!-- Login form -->
        <form @submit.prevent="handleLogin" class="login-form">
          <div class="input-group">
            <label for="username">{{ t('username') }}</label>
            <div class="input-wrapper">
              <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"></path>
                <circle cx="12" cy="7" r="4"></circle>
              </svg>
              <input
                id="username"
                v-model="credentials.username"
                type="text"
                :placeholder="selectedRole === 'admin' ? 'admin' : 'student123'"
                class="input-field"
              />
            </div>
          </div>

          <div class="input-group">
            <label for="password">{{ t('password') }}</label>
            <div class="input-wrapper">
              <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <rect x="3" y="11" width="18" height="11" rx="2" ry="2"></rect>
                <path d="M7 11V7a5 5 0 0 1 10 0v4"></path>
              </svg>
              <input
                id="password"
                v-model="credentials.password"
                type="password"
                :placeholder="t('password_placeholder')"
                class="input-field"
              />
              <button 
                type="button" 
                @click="togglePasswordVisibility"
                class="password-toggle"
              >
                <svg v-if="showPassword" xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                  <path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path>
                  <line x1="1" y1="1" x2="23" y2="23"></line>
                </svg>
                <svg v-else xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                  <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                  <circle cx="12" cy="12" r="3"></circle>
                </svg>
              </button>
            </div>
          </div>

          <button 
            type="submit" 
            :disabled="loading"
            class="login-button"
          >
            <span v-if="!loading" class="button-content">
              <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M15 3h4a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-4"></path>
                <polyline points="10 17 15 12 10 7"></polyline>
                <line x1="15" y1="12" x2="3" y2="12"></line>
              </svg>
              {{ t('login_button') }}
            </span>
            <span v-else class="button-content">
              <svg class="animate-spin" xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M21 12a9 9 0 1 1-6.219-8.56"></path>
              </svg>
              {{ t('logging_in') }}
            </span>
          </button>
        </form>

        <!-- Additional links -->
        <div class="login-footer">
          <router-link to="/register" class="footer-link">
            {{ t('register_link') }}
          </router-link>
          <router-link to="/forgot-password" class="footer-link">
            {{ t('forgot_password_link') }}
          </router-link>
        </div>

        <!-- Language selector -->
        <div class="language-selector">
          <el-select 
            v-model="selectedLanguage" 
            @change="changeLanguage"
            size="small"
          >
            <el-option :label="t('english')" value="en"></el-option>
            <el-option :label="t('chinese')" value="zh"></el-option>
          </el-select>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage, ElNotification } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useMainStore } from '@/store/index';
import { LoginCredentials } from '@/types';
import apiService from '@/api';
import { useI18n } from 'vue-i18n';

const { t, locale } = useI18n();
const router = useRouter();
const userStore = useUserStore();
const mainStore = useMainStore();

const selectedRole = ref<'admin' | 'student'>('admin');
const showPassword = ref(false);
const loading = ref(false);
const selectedLanguage = ref(locale.value);

const credentials = reactive<LoginCredentials>({
  username: '',
  password: ''
});

const setRole = (role: 'admin' | 'student') => {
  selectedRole.value = role;
  // Reset credentials when switching roles
  credentials.username = role === 'admin' ? 'admin' : 'student123';
  credentials.password = role === 'admin' ? 'admin' : 'pass';
};

const togglePasswordVisibility = () => {
  showPassword.value = !showPassword.value;
};

// Form validation rules
const validateForm = (): boolean => {
  if (!credentials.username.trim()) {
    ElMessage.warning(t('username_required'));
    return false;
  }

  if (!credentials.password) {
    ElMessage.warning(t('password_required'));
    return false;
  }

  if (credentials.password.length < 3) {
    ElMessage.warning(t('password_too_short'));
    return false;
  }

  if (credentials.password.length > 50) {
    ElMessage.warning(t('password_too_long'));
    return false;
  }

  return true;
};

const handleLogin = async () => {
  if (!validateForm()) {
    return;
  }

  loading.value = true;
  try {
    mainStore.setLoading(true);
    
    const response = await apiService.login(credentials);

    if (response.success && response.data) {
      const { user, token } = response.data;
      userStore.setUser(user);
      userStore.setToken(token);

      // Show success notification
      ElNotification({
        title: t('login_success'),
        message: t('login_welcome_message'),
        type: 'success',
        duration: 3000
      });

      // Redirect based on user role
      if (user.role === 'ADMIN') {
        router.push('/admin');
      } else {
        router.push('/student');
      }
    } else {
      ElMessage.error(response.error || t('login_failed'));
    }
  } catch (error: any) {
    console.error('Login error:', error);
    // More specific error handling
    if (error.response?.status === 401) {
      ElMessage.error(t('invalid_credentials'));
    } else if (error.code === 'NETWORK_ERROR') {
      ElMessage.error(t('network_error'));
    } else {
      ElMessage.error(error.message || t('login_error'));
    }
  } finally {
    loading.value = false;
    mainStore.setLoading(false);
  }
};

const changeLanguage = (lang: string) => {
  locale.value = lang;
  localStorage.setItem('locale', lang);
};
</script>

<style scoped>
.login-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
  overflow: hidden;
  background: white;
  font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
}

.background-elements {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
  pointer-events: none;
}

.circle {
  position: absolute;
  border-radius: 50%;
  background: rgba(102, 126, 234, 0.1); /* Purple-transparent circles to be visible on white */
  backdrop-filter: blur(10px);
  animation: float 6s ease-in-out infinite;
}

.circle-1 {
  width: 200px;
  height: 200px;
  top: 10%;
  left: 10%;
  animation-delay: 0s;
}

.circle-2 {
  width: 150px;
  height: 150px;
  top: 60%;
  left: 80%;
  animation-delay: 2s;
}

.circle-3 {
  width: 100px;
  height: 100px;
  top: 30%;
  left: 70%;
  animation-delay: 4s;
}

.circle-4 {
  width: 120px;
  height: 120px;
  top: 70%;
  left: 20%;
  animation-delay: 1s;
}

@keyframes float {
  0%, 100% {
    transform: translateY(0px) rotate(0deg);
  }
  50% {
    transform: translateY(-20px) rotate(180deg);
  }
}

.login-container {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  padding: 20px;
  z-index: 10;
}

.login-card {
  background: white;
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  border-radius: 20px;
  border: 1px solid rgba(200, 200, 200, 0.3);
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
  padding: 40px;
  width: 100%;
  max-width: 420px;
  position: relative;
  overflow: hidden;
  transition: all 0.3s ease;
}

.login-card::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
  z-index: -1;
  opacity: 0.3;
}

.login-header {
  text-align: center;
  margin-bottom: 32px;
}

.logo-container {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
  margin-bottom: 8px;
}

.logo-icon {
  width: 48px;
  height: 48px;
  border-radius: 16px;
  background: white;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #667eea; /* Keeping the purple color for the icon */
  box-shadow: 0 8px 20px rgba(102, 126, 234, 0.1);
}

.logo-text {
  font-size: 24px;
  font-weight: 700;
  color: #333;
  margin: 0;
}

.subtitle {
  color: #666;
  font-size: 14px;
  margin: 0;
  font-weight: 400;
}

.role-tabs {
  display: flex;
  background: #f0f0f0;
  border-radius: 12px;
  padding: 4px;
  margin-bottom: 32px;
}

.tab-button {
  flex: 1;
  padding: 12px 16px;
  border: none;
  border-radius: 8px;
  background: transparent;
  color: #666;
  font-weight: 500;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  transition: all 0.3s ease;
}

.tab-button:hover {
  background: #e0e0e0;
}

.tab-button.active {
  background: white;
  color: #667eea;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.login-form {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.input-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.input-group label {
  color: #333;
  font-size: 14px;
  font-weight: 500;
  margin: 0;
}

.input-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.input-field {
  width: 100%;
  padding: 14px 14px 14px 44px;
  border: 1px solid #ddd;
  border-radius: 12px;
  background: white;
  color: #333;
  font-size: 16px;
  transition: all 0.3s ease;
}

.input-field::placeholder {
  color: #aaa;
}

.input-field:focus {
  outline: none;
  border-color: #667eea;
  box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.2);
}

.input-wrapper svg:first-child {
  position: absolute;
  left: 14px;
  color: #777;
}

.password-toggle {
  position: absolute;
  right: 14px;
  background: transparent;
  border: none;
  color: #777;
  cursor: pointer;
  padding: 4px;
  border-radius: 4px;
  transition: all 0.3s ease;
}

.password-toggle:hover {
  background: #f0f0f0;
  color: #333;
}

.login-button {
  width: 100%;
  padding: 16px;
  border: none;
  border-radius: 12px;
  background: #667eea; /* Keeping purple for the button to maintain visibility */
  color: white;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  transition: all 0.3s ease;
  box-shadow: 0 8px 20px rgba(102, 126, 234, 0.2);
}

.login-button:hover:not(:disabled) {
  transform: translateY(-2px);
  box-shadow: 0 12px 25px rgba(102, 126, 234, 0.4);
}

.login-button:disabled {
  opacity: 0.7;
  cursor: not-allowed;
  transform: none;
}

.button-content {
  display: flex;
  align-items: center;
  gap: 8px;
}

.animate-spin {
  animation: spin 1s linear infinite;
}

@keyframes spin {
  from {
    transform: rotate(0deg);
  }
  to {
    transform: rotate(360deg);
  }
}

.login-footer {
  display: flex;
  justify-content: space-between;
  margin-top: 24px;
  padding-top: 24px;
  border-top: 1px solid #eee;
}

.footer-link {
  color: #667eea;
  text-decoration: none;
  font-size: 14px;
  transition: all 0.3s ease;
}

.footer-link:hover {
  color: #5a6fd8;
  text-decoration: underline;
}

.language-selector {
  margin-top: 20px;
  text-align: center;
}

.language-selector :deep(.el-select) {
  width: 120px;
}

/* Responsive design */
@media (max-width: 480px) {
  .login-card {
    padding: 30px 20px;
    margin: 0 16px;
  }
  
  .logo-text {
    font-size: 20px;
  }
  
  .login-footer {
    flex-direction: column;
    gap: 12px;
    align-items: center;
  }
  
  .language-selector {
    margin-top: 16px;
  }
  
  .language-selector :deep(.el-select) {
    width: 100%;
  }
}

@media (max-width: 360px) {
  .login-card {
    padding: 24px 16px;
  }
  
  .logo-text {
    font-size: 18px;
  }
  
  .logo-icon {
    width: 40px;
    height: 40px;
  }
}
</style>