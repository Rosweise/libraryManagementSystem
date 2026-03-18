<template>
  <div class="account-management">
    <div class="account-header">
      <el-button @click="goBack" class="back-btn" plain>
        <el-icon><ArrowLeft /></el-icon>
        {{ t('back_to_home') }}
      </el-button>
      <h2>{{ t('account_management') }}</h2>
    </div>
    
    <el-tabs v-model="activeTab" class="account-tabs">
      <el-tab-pane :label="t('profile')" name="profile">
        <el-card class="profile-card">
          <template #header>
            <div class="card-header">
              <span>{{ t('personal_info') }}</span>
              <el-button @click="editProfile" type="primary" size="small">
                {{ isEditingProfile ? t('save') : t('edit') }}
              </el-button>
            </div>
          </template>
          
          <el-form 
            :model="profileForm" 
            :rules="profileRules" 
            ref="profileFormRef"
            label-width="120px"
            :disabled="!isEditingProfile"
          >
            <el-form-item :label="t('student_id')" prop="id">
              <el-input v-model="profileForm.id" disabled />
            </el-form-item>
            
            <el-form-item :label="t('username')" prop="username">
              <el-input v-model="profileForm.username" disabled />
            </el-form-item>
            
            <el-form-item :label="t('name')" prop="name">
              <el-input v-model="profileForm.name" :placeholder="t('name_placeholder')" />
            </el-form-item>
            
            <el-form-item :label="t('college')" prop="college">
              <el-input v-model="profileForm.college" :placeholder="t('college_placeholder')" />
            </el-form-item>
            
            <el-form-item :label="t('class')" prop="className">
              <el-input v-model="profileForm.className" :placeholder="t('class_placeholder')" />
            </el-form-item>
            
            <el-form-item :label="t('role')" prop="role">
              <el-tag>{{ profileForm.role === 'ADMIN' ? t('admin') : t('student') }}</el-tag>
            </el-form-item>
            
            <el-form-item :label="t('recovery_token_status')">
              <el-tag :type="profileForm.hasRecoveryToken ? 'success' : 'info'">
                {{ profileForm.hasRecoveryToken ? t('token_set') : t('token_not_set') }}
              </el-tag>
            </el-form-item>
          </el-form>
        </el-card>
      </el-tab-pane>
      
      <el-tab-pane :label="t('change_password')" name="password">
        <el-card class="password-card">
          <template #header>
            <div class="card-header">
              <span>{{ t('change_password') }}</span>
            </div>
          </template>
          
          <el-form 
            :model="passwordForm" 
            :rules="passwordRules" 
            ref="passwordFormRef"
            label-width="150px"
            style="max-width: 500px;"
          >
            <el-form-item :label="t('current_password')" prop="currentPassword" v-if="userStore.isStudent">
              <el-input 
                v-model="passwordForm.currentPassword" 
                type="password"
                show-password
                :placeholder="t('current_password_placeholder')"
              />
            </el-form-item>
            
            <el-form-item :label="t('new_password')" prop="newPassword">
              <el-input 
                v-model="passwordForm.newPassword" 
                type="password"
                show-password
                :placeholder="t('new_password_placeholder')"
              />
            </el-form-item>
            
            <el-form-item :label="t('confirm_new_password')" prop="confirmNewPassword">
              <el-input 
                v-model="passwordForm.confirmNewPassword" 
                type="password"
                show-password
                :placeholder="t('confirm_new_password_placeholder')"
              />
            </el-form-item>
            
            <el-form-item>
              <el-button type="primary" @click="changePassword" :loading="changingPassword">
                {{ t('change_password_button') }}
              </el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-tab-pane>
      
      <el-tab-pane :label="t('recovery_token')" name="token" v-if="userStore.isStudent">
        <el-card class="token-card">
          <template #header>
            <div class="card-header">
              <span>{{ t('recovery_token') }}</span>
            </div>
          </template>
          
          <el-form 
            :model="tokenForm" 
            :rules="tokenRules" 
            ref="tokenFormRef"
            label-width="150px"
            style="max-width: 500px;"
          >
            <el-form-item :label="t('current_token_status')">
              <el-tag :type="userStore.user?.hasRecoveryToken ? 'success' : 'info'">
                {{ userStore.user?.hasRecoveryToken ? t('token_set') : t('token_not_set') }}
              </el-tag>
            </el-form-item>
            
            <el-form-item :label="t('new_recovery_token')" prop="token">
              <el-input 
                v-model="tokenForm.token" 
                type="text"
                :placeholder="t('recovery_token_placeholder')"
              />
            </el-form-item>
            
            <el-form-item>
              <el-button type="primary" @click="setRecoveryToken" :loading="settingToken">
                {{ userStore.user?.hasRecoveryToken ? t('update_token') : t('set_token') }}
              </el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage } from 'element-plus';
import { useUserStore } from '@/store/user';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { User } from '@/types';
import { ArrowLeft } from '@element-plus/icons-vue';

const { t } = useI18n();
const userStore = useUserStore();
const router = useRouter();

const activeTab = ref('profile');
const isEditingProfile = ref(false);
const changingPassword = ref(false);
const settingToken = ref(false);

const profileFormRef = ref();
const passwordFormRef = ref();
const tokenFormRef = ref();

const profileForm = reactive<User>({
  id: '',
  username: '',
  name: '',
  college: '',
  className: '',
  role: 'STUDENT',
  hasRecoveryToken: false
});

const passwordForm = reactive({
  currentPassword: '',
  newPassword: '',
  confirmNewPassword: ''
});

const tokenForm = reactive({
  token: ''
});

const validatePassword = (rule: any, value: string, callback: Function) => {
  if (value !== passwordForm.newPassword) {
    callback(new Error(t('password_mismatch')));
  } else {
    callback();
  }
};

const profileRules = {
  name: [
    { required: true, message: t('name_required'), trigger: 'blur' }
  ],
  college: [
    { required: true, message: t('college_required'), trigger: 'blur' }
  ],
  className: [
    { required: true, message: t('class_required'), trigger: 'blur' }
  ]
};

const passwordRules = {
  currentPassword: [
    { required: true, message: t('current_password_required'), trigger: 'blur' }
  ],
  newPassword: [
    { required: true, message: t('new_password_required'), trigger: 'blur' },
    { min: 6, max: 20, message: t('password_length'), trigger: 'blur' }
  ],
  confirmNewPassword: [
    { required: true, message: t('confirm_new_password_required'), trigger: 'blur' },
    { validator: validatePassword, trigger: 'blur' }
  ]
};

const tokenRules = {
  token: [
    { required: true, message: t('token_required'), trigger: 'blur' },
    { min: 3, max: 50, message: t('token_length'), trigger: 'blur' }
  ]
};

const goBack = () => {
  if (window.history.length > 1) {
    router.back();
    return;
  }

  if (userStore.isAdmin) {
    router.push('/admin');
    return;
  }

  router.push('/student');
};

onMounted(() => {
  if (userStore.user) {
    // Initialize profile form with user data
    Object.assign(profileForm, userStore.user);
  }
});

const editProfile = async () => {
  if (isEditingProfile.value) {
    // Save profile changes
    try {
      const valid = await profileFormRef.value.validate();
      if (!valid) return;
      
      const response = await apiService.updateUser(profileForm);
      if (response.success) {
        ElMessage.success(t('profile_updated_success'));
        // Update user in store
        userStore.setUser(profileForm);
        isEditingProfile.value = false;
      } else {
        ElMessage.error(response.error || t('profile_update_error'));
      }
    } catch (error) {
      console.error('Error updating profile:', error);
      ElMessage.error(t('profile_update_error'));
    }
  } else {
    // Start editing
    isEditingProfile.value = true;
  }
};

const changePassword = async () => {
  try {
    const valid = await passwordFormRef.value.validate();
    if (!valid) return;
    
    changingPassword.value = true;
    
    if (userStore.isStudent) {
      // For students, need to provide current password
      const response = await apiService.changePassword(
        userStore.user!.id,
        passwordForm.currentPassword,
        passwordForm.newPassword
      );
      
      if (response.success) {
        ElMessage.success(t('password_changed_success'));
        // Reset form
        passwordForm.currentPassword = '';
        passwordForm.newPassword = '';
        passwordForm.confirmNewPassword = '';
      } else {
        ElMessage.error(response.error || t('password_change_error'));
      }
    } else {
      // For admin, we could implement a different mechanism
      ElMessage.warning(t('admin_password_change_note'));
    }
  } catch (error) {
    console.error('Error changing password:', error);
    ElMessage.error(t('password_change_error'));
  } finally {
    changingPassword.value = false;
  }
};

const setRecoveryToken = async () => {
  try {
    const valid = await tokenFormRef.value.validate();
    if (!valid) return;
    
    settingToken.value = true;
    
    const response = await apiService.setRecoveryToken(userStore.user!.id, tokenForm.token);
    if (response.success) {
      ElMessage.success(t('token_set_success'));
      // Update user in store
      userStore.user!.hasRecoveryToken = true;
      // Reset form
      tokenForm.token = '';
    } else {
      ElMessage.error(response.error || t('token_set_error'));
    }
  } catch (error) {
    console.error('Error setting recovery token:', error);
    ElMessage.error(t('token_set_error'));
  } finally {
    settingToken.value = false;
  }
};
</script>

<style scoped>
.account-management {
  padding: 0;
  width: 100%;
  box-sizing: border-box;
}

.account-header {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 24px;
}

.account-header h2 {
  margin: 0;
  font-size: 26px;
  font-weight: 600;
  color: #1f2937;
  letter-spacing: -0.025em;
}

.back-btn {
  padding: 8px 12px;
  border-radius: 8px;
  font-weight: 500;
  transition: all 0.2s ease;
}

.back-btn:hover {
  transform: translateX(-2px);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.account-tabs {
  margin-top: 20px;
}

.account-tabs :deep(.el-tabs__header) {
  background-color: #fff;
  padding: 0 20px;
  border-radius: 12px 12px 0 0;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1), 0 1px 2px rgba(0, 0, 0, 0.06);
}

.account-tabs :deep(.el-tabs__item) {
  font-size: 15px;
  font-weight: 500;
  color: #6b7280;
  padding: 0 24px;
  height: 48px;
  line-height: 48px;
  transition: all 0.2s ease;
}

.account-tabs :deep(.el-tabs__item:hover) {
  color: #10b981;
}

.account-tabs :deep(.el-tabs__item.is-active) {
  color: #10b981;
  font-weight: 600;
}

.account-tabs :deep(.el-tabs__active-bar) {
  background: linear-gradient(90deg, #10b981 0%, #059669 100%);
  height: 3px;
  border-radius: 3px 3px 0 0;
}

.account-tabs :deep(.el-tabs__content) {
  padding: 24px;
  background-color: #f9fafb;
  border-radius: 0 0 12px 12px;
}

.profile-card, .password-card, .token-card {
  max-width: 800px;
  background-color: #fff;
  border-radius: 12px;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1), 0 1px 2px rgba(0, 0, 0, 0.06);
  border: none;
}

.profile-card :deep(.el-card__header),
.password-card :deep(.el-card__header),
.token-card :deep(.el-card__header) {
  background: linear-gradient(135deg, #f9fafb 0%, #f3f4f6 100%);
  border-bottom: 1px solid #e5e7eb;
  padding: 16px 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-header span {
  font-weight: 600;
  font-size: 17px;
  color: #1f2937;
  letter-spacing: -0.01em;
}

.profile-card :deep(.el-form-item__label) {
  font-weight: 500;
  color: #374151;
  font-size: 14px;
}

.profile-card :deep(.el-input__wrapper) {
  border-radius: 8px;
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);
}

.profile-card :deep(.el-tag) {
  padding: 4px 12px;
  border-radius: 6px;
  font-weight: 500;
  font-size: 12px;
  border: none;
}

/* Responsive design */
@media (max-width: 768px) {
  .account-management {
    padding: 10px;
  }
  
  .account-header h2 {
    font-size: 22px;
  }
  
  .profile-card, .password-card, .token-card {
    max-width: 100%;
  }
  
  .account-tabs :deep(.el-tabs__item) {
    padding: 0 16px;
    font-size: 14px;
  }
}
</style>
