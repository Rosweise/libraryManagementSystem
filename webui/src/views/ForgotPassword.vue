<template>
  <div class="forgot-password-container">
    <div class="forgot-password-form">
      <div class="logo">
        <h2>{{ t('library_system') }}</h2>
      </div>
      
      <h3 class="form-title">{{ t('forgot_password_title') }}</h3>
      
      <el-steps :active="step" finish-status="success" align-center class="steps">
        <el-step :title="t('step_one')" />
        <el-step :title="t('step_two')" />
        <el-step :title="t('step_three')" />
      </el-steps>
      
      <el-form 
        v-if="step === 0"
        :model="stepOneForm" 
        :rules="stepOneRules" 
        ref="stepOneFormRef"
        label-width="120px"
        class="step-form"
      >
        <el-form-item :label="t('username')" prop="username">
          <el-input 
            v-model="stepOneForm.username" 
            :placeholder="t('username_placeholder')"
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="validateStepOne" 
            :loading="isLoading"
            class="form-button"
          >
            {{ t('next_step') }}
          </el-button>
        </el-form-item>
      </el-form>
      
      <el-form 
        v-if="step === 1"
        :model="stepTwoForm" 
        :rules="stepTwoRules" 
        ref="stepTwoFormRef"
        label-width="120px"
        class="step-form"
      >
        <el-form-item :label="t('recovery_token')" prop="token">
          <el-input 
            v-model="stepTwoForm.token" 
            :placeholder="t('recovery_token_placeholder')"
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="validateStepTwo" 
            :loading="isLoading"
            class="form-button"
          >
            {{ t('next_step') }}
          </el-button>
          <el-button @click="prevStep" class="form-button">
            {{ t('previous_step') }}
          </el-button>
        </el-form-item>
      </el-form>
      
      <el-form 
        v-if="step === 2"
        :model="stepThreeForm" 
        :rules="stepThreeRules" 
        ref="stepThreeFormRef"
        label-width="120px"
        class="step-form"
      >
        <el-form-item :label="t('new_password')" prop="newPassword">
          <el-input 
            v-model="stepThreeForm.newPassword" 
            type="password"
            :placeholder="t('new_password_placeholder')"
            show-password
          />
        </el-form-item>
        
        <el-form-item :label="t('confirm_password')" prop="confirmPassword">
          <el-input 
            v-model="stepThreeForm.confirmPassword" 
            type="password"
            :placeholder="t('confirm_password_placeholder')"
            show-password
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="resetPassword" 
            :loading="isLoading"
            class="form-button"
          >
            {{ t('reset_password_button') }}
          </el-button>
          <el-button @click="prevStep" class="form-button">
            {{ t('previous_step') }}
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
import apiService from '@/api';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();

const router = useRouter();
const isLoading = ref(false);
const step = ref(0); // 0: username, 1: token, 2: new password

// Form references
const stepOneFormRef = ref();
const stepTwoFormRef = ref();
const stepThreeFormRef = ref();

// Step 1 form
const stepOneForm = reactive({
  username: ''
});

const stepOneRules = {
  username: [
    { required: true, message: t('username_required'), trigger: 'blur' }
  ]
};

// Step 2 form
const stepTwoForm = reactive({
  token: ''
});

const stepTwoRules = {
  token: [
    { required: true, message: t('token_required'), trigger: 'blur' }
  ]
};

// Step 3 form
const stepThreeForm = reactive({
  newPassword: '',
  confirmPassword: ''
});

const validatePassword = (rule: any, value: string, callback: Function) => {
  if (value !== stepThreeForm.newPassword) {
    callback(new Error(t('password_mismatch')));
  } else {
    callback();
  }
};

const stepThreeRules = {
  newPassword: [
    { required: true, message: t('password_required'), trigger: 'blur' },
    { min: 6, max: 20, message: t('password_length'), trigger: 'blur' }
  ],
  confirmPassword: [
    { required: true, message: t('confirm_password_required'), trigger: 'blur' },
    { validator: validatePassword, trigger: 'blur' }
  ]
};

const validateStepOne = async () => {
  try {
    const valid = await stepOneFormRef.value.validate();
    if (!valid) return;
    step.value = 1;
  } catch (error) {
    console.error('Step 1 validation error:', error);
  }
};

const validateStepTwo = async () => {
  try {
    const valid = await stepTwoFormRef.value.validate();
    if (!valid) return;
    step.value = 2;
  } catch (error) {
    console.error('Step 2 validation error:', error);
  }
};

const prevStep = () => {
  if (step.value > 0) {
    step.value--;
  }
};

const resetPassword = async () => {
  try {
    const valid = await stepThreeFormRef.value.validate();
    if (!valid) return;
    
    isLoading.value = true;
    
    const response = await apiService.forgotPassword(
      stepOneForm.username, 
      stepTwoForm.token, 
      stepThreeForm.newPassword
    );
    
    if (response.success) {
      ElMessage.success(t('password_reset_success'));
      router.push('/login');
    } else {
      ElMessage.error(response.error || t('password_reset_failed'));
    }
  } catch (error: any) {
    console.error('Password reset error:', error);
    ElMessage.error(error.message || t('password_reset_error'));
  } finally {
    isLoading.value = false;
  }
};
</script>

<style scoped>
.forgot-password-container {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
  background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
  padding: 20px;
}

.forgot-password-form {
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

.steps {
  margin: 30px 0;
}

.step-form {
  margin-top: 30px;
}

.form-button {
  margin-right: 10px;
}

.form-footer {
  margin-top: 30px;
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