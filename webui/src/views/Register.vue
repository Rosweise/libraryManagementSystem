<template>
  <div class="register-container">
    <div class="register-form">
      <div class="logo">
        <h2>{{ t('library_system') }}</h2>
      </div>
      
      <h3 class="form-title">{{ t('register_title') }}</h3>
      
      <el-form 
        :model="registerForm" 
        :rules="rules" 
        ref="registerFormRef"
        label-width="120px"
      >
        <el-form-item :label="t('student_id')" prop="id">
          <el-input 
            v-model="registerForm.id" 
            :placeholder="t('student_id_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('username')" prop="username">
          <el-input 
            v-model="registerForm.username" 
            :placeholder="t('username_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('name')" prop="name">
          <el-input 
            v-model="registerForm.name" 
            :placeholder="t('name_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('college')" prop="college">
          <el-input 
            v-model="registerForm.college" 
            :placeholder="t('college_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('class')" prop="className">
          <el-input 
            v-model="registerForm.className" 
            :placeholder="t('class_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('password')" prop="password">
          <el-input 
            v-model="registerForm.password" 
            type="password"
            :placeholder="t('password_placeholder')"
            show-password
          />
        </el-form-item>
        
        <el-form-item :label="t('confirm_password')" prop="confirmPassword">
          <el-input 
            v-model="registerForm.confirmPassword" 
            type="password"
            :placeholder="t('confirm_password_placeholder')"
            show-password
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="handleRegister" 
            :loading="isLoading"
            class="register-button"
          >
            {{ t('register_button') }}
          </el-button>
        </el-form-item>
      </el-form>
      
      <div class="form-footer">
        <p>
          <router-link to="/login">{{ t('back_to_login') }}</router-link>
        </p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage } from 'element-plus';
import { RegisterData } from '@/types';
import apiService from '@/api';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();

const router = useRouter();
const isLoading = ref(false);
const registerFormRef = ref();

const registerForm = reactive<RegisterData & { confirmPassword: string }>({
  id: '',
  username: '',
  name: '',
  college: '',
  className: '',
  password: '',
  confirmPassword: ''
});

const validatePassword = (rule: any, value: string, callback: Function) => {
  if (value !== registerForm.password) {
    callback(new Error(t('password_mismatch')));
  } else {
    callback();
  }
};

const rules = {
  id: [
    { required: true, message: t('student_id_required'), trigger: 'blur' },
    { min: 3, max: 20, message: t('student_id_length'), trigger: 'blur' }
  ],
  username: [
    { required: true, message: t('username_required'), trigger: 'blur' },
    { min: 3, max: 20, message: t('username_length'), trigger: 'blur' }
  ],
  name: [
    { required: true, message: t('name_required'), trigger: 'blur' }
  ],
  college: [
    { required: true, message: t('college_required'), trigger: 'blur' }
  ],
  className: [
    { required: true, message: t('class_required'), trigger: 'blur' }
  ],
  password: [
    { required: true, message: t('password_required'), trigger: 'blur' },
    { min: 6, max: 20, message: t('password_length'), trigger: 'blur' }
  ],
  confirmPassword: [
    { required: true, message: t('confirm_password_required'), trigger: 'blur' },
    { validator: validatePassword, trigger: 'blur' }
  ]
};

const handleRegister = async () => {
  try {
    const valid = await registerFormRef.value.validate();
    if (!valid) return;
    
    isLoading.value = true;
    
    const { confirmPassword, ...userData } = registerForm;
    const response = await apiService.register(userData);
    
    if (response.success) {
      ElMessage.success(t('register_success'));
      router.push('/login');
    } else {
      ElMessage.error(response.error || t('register_failed'));
    }
  } catch (error: any) {
    console.error('Registration error:', error);
    ElMessage.error(error.message || t('register_error'));
  } finally {
    isLoading.value = false;
  }
};
</script>

<style scoped>
.register-container {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
  background: white;
  padding: 20px;
}

.register-form {
  background: white;
  border-radius: 10px;
  box-shadow: 0 15px 35px rgba(50, 50, 93, 0.1), 0 5px 15px rgba(0, 0, 0, 0.07);
  padding: 40px;
  width: 100%;
  max-width: 500px;
}

.logo h2 {
  text-align: center;
  margin-bottom: 20px;
  color: #303133;
}

.form-title {
  text-align: center;
  margin-bottom: 30px;
  color: #606266;
}

.el-form {
  margin-top: 20px;
}

.register-button {
  width: 100%;
  height: 45px;
  font-size: 16px;
}

.form-footer {
  margin-top: 20px;
  text-align: center;
}

.form-footer a {
  color: #409eff;
  text-decoration: none;
}

.form-footer a:hover {
  text-decoration: underline;
}
</style>