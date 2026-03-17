<template>
  <div class="login-container">
    <div class="login-form">
      <div class="logo">
        <h2>{{ t('library_system') }}</h2>
      </div>
      
      <el-form 
        :model="loginForm" 
        :rules="rules" 
        ref="loginFormRef"
        @keyup.enter="handleLogin"
      >
        <el-form-item prop="username">
          <el-input 
            v-model="loginForm.username" 
            :placeholder="t('username_placeholder')"
            prefix-icon="User"
          />
        </el-form-item>
        
        <el-form-item prop="password">
          <el-input 
            v-model="loginForm.password" 
            type="password"
            :placeholder="t('password_placeholder')"
            prefix-icon="Lock"
            show-password
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="handleLogin" 
            :loading="isLoading"
            class="login-button"
          >
            {{ t('login_button') }}
          </el-button>
        </el-form-item>
      </el-form>
      
      <div class="form-footer">
        <p>
          <router-link to="/register">{{ t('register_link') }}</router-link>
        </p>
        <p>
          <router-link to="/forgot-password">{{ t('forgot_password_link') }}</router-link>
        </p>
        <div class="language-selector">
          <el-select v-model="selectedLanguage" @change="changeLanguage">
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
import { ElMessage } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useMainStore } from '@/store/index';
import { LoginCredentials } from '@/types';
import apiService from '@/api';
import { useI18n } from 'vue-i18n';

const { t, locale } = useI18n();

const router = useRouter();
const userStore = useUserStore();
const mainStore = useMainStore();

const loginFormRef = ref();
const isLoading = ref(false);
const selectedLanguage = ref(locale.value);

const loginForm = reactive<LoginCredentials>({
  username: '',
  password: ''
});

const rules = {
  username: [
    { required: true, message: t('username_required'), trigger: 'blur' }
  ],
  password: [
    { required: true, message: t('password_required'), trigger: 'blur' },
    { min: 3, max: 20, message: t('password_length'), trigger: 'blur' }
  ]
};

const handleLogin = async () => {
  try {
    const valid = await loginFormRef.value.validate();
    if (!valid) return;
    
    isLoading.value = true;
    mainStore.setLoading(true);
    
    const response = await apiService.login(loginForm);
    
    if (response.success && response.data) {
      const { user, token } = response.data;
      userStore.setUser(user);
      userStore.setToken(token);
      
      ElMessage.success(t('login_success'));
      
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
    ElMessage.error(error.message || t('login_error'));
  } finally {
    isLoading.value = false;
    mainStore.setLoading(false);
  }
};

const changeLanguage = (lang: string) => {
  locale.value = lang;
  localStorage.setItem('locale', lang);
};
</script>

<style scoped>
.login-container {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
  background: white;
  padding: 20px;
}

.login-form {
  background: white;
  border-radius: 10px;
  box-shadow: 0 15px 35px rgba(50, 50, 93, 0.1), 0 5px 15px rgba(0, 0, 0, 0.07);
  padding: 40px;
  width: 100%;
  max-width: 400px;
}

.logo h2 {
  text-align: center;
  margin-bottom: 30px;
  color: #303133;
}

.el-form {
  margin-top: 20px;
}

.login-button {
  width: 100%;
  height: 45px;
  font-size: 16px;
}

.form-footer {
  margin-top: 20px;
  text-align: center;
}

.form-footer p {
  margin: 10px 0;
}

.form-footer a {
  color: #409eff;
  text-decoration: none;
}

.form-footer a:hover {
  text-decoration: underline;
}

.language-selector {
  margin-top: 15px;
}
</style>