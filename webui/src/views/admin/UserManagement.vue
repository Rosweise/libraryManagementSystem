<template>
  <div class="user-management">
    <div class="header-section">
      <div class="header-left">
        <el-button @click="goBack" class="back-btn" plain>
          <el-icon><ArrowLeft /></el-icon>
          {{ t('back_to_home') }}
        </el-button>
        <h2>{{ t('user_management') }}</h2>
      </div>
      <el-button type="primary" @click="addNewUser" class="add-user-btn">
        <el-icon><Plus /></el-icon>
        {{ t('add_new_user') }}
      </el-button>
    </div>
    
    <el-card class="search-card">
      <el-form :model="searchForm" inline class="search-form">
        <el-form-item :label="t('search_keyword')">
          <el-input 
            v-model="searchForm.keyword" 
            :placeholder="t('search_placeholder')"
            @keyup.enter="searchUsers"
          />
        </el-form-item>
        <el-form-item :label="t('user_role')">
          <el-select v-model="searchForm.role" class="role-select">
            <el-option :label="t('all_roles')" value="" />
            <el-option :label="t('student')" value="STUDENT" />
            <el-option :label="t('admin')" value="ADMIN" />
          </el-select>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="searchUsers">{{ t('search') }}</el-button>
          <el-button @click="resetSearch">{{ t('reset') }}</el-button>
        </el-form-item>
      </el-form>
    </el-card>
    
    <el-table 
      :data="users" 
      v-loading="loading"
      class="users-table"
      row-key="id"
    >
      <el-table-column prop="id" :label="t('student_id')" width="150" />
      <el-table-column prop="username" :label="t('username')" width="150" />
      <el-table-column prop="name" :label="t('name')" width="150" />
      <el-table-column prop="college" :label="t('college')" width="200" show-overflow-tooltip />
      <el-table-column prop="className" :label="t('class')" width="150" />
      <el-table-column prop="role" :label="t('role')" width="100">
        <template #default="scope">
          <el-tag :type="scope.row.role === 'ADMIN' ? 'warning' : 'info'">
            {{ scope.row.role === 'ADMIN' ? t('admin') : t('student') }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column :label="t('actions')" width="250" fixed="right">
        <template #default="scope">
          <el-button size="small" @click="editUser(scope.row)" type="primary">
            {{ t('edit') }}
          </el-button>
          <el-button size="small" @click="viewUserDetails(scope.row)" type="info">
            {{ t('details') }}
          </el-button>
          <el-button size="small" @click="changePassword(scope.row)" type="warning">
            {{ t('change_password') }}
          </el-button>
          <el-popconfirm
            :title="t('delete_confirm', { name: scope.row.name })"
            @confirm="deleteUser(scope.row.id)"
          >
            <template #reference>
              <el-button size="small" type="danger">
                {{ t('delete') }}
              </el-button>
            </template>
          </el-popconfirm>
        </template>
      </el-table-column>
    </el-table>
    
    <div class="pagination">
      <el-pagination
        @size-change="handleSizeChange"
        @current-change="handleCurrentChange"
        :current-page="pagination.currentPage"
        :page-sizes="[10, 20, 50, 100]"
        :page-size="pagination.pageSize"
        layout="total, sizes, prev, pager, next, jumper"
        :total="pagination.total"
      />
    </div>
    
    <!-- User Dialog -->
    <el-dialog 
      :title="dialogTitle" 
      v-model="dialogVisible" 
      width="50%" 
      :before-close="handleClose"
    >
      <el-form 
        :model="currentUser" 
        :rules="userRules" 
        ref="userFormRef"
        label-width="120px"
      >
        <el-form-item :label="t('student_id')" prop="id">
          <el-input 
            v-model="currentUser.id" 
            :disabled="isEdit"
            :placeholder="t('student_id_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('username')" prop="username">
          <el-input 
            v-model="currentUser.username" 
            :disabled="isEdit"
            :placeholder="t('username_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('name')" prop="name">
          <el-input 
            v-model="currentUser.name" 
            :placeholder="t('name_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('college')" prop="college">
          <el-input 
            v-model="currentUser.college" 
            :placeholder="t('college_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('class')" prop="className">
          <el-input 
            v-model="currentUser.className" 
            :placeholder="t('class_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('role')" prop="role">
          <el-radio-group v-model="currentUser.role">
            <el-radio label="STUDENT">{{ t('student') }}</el-radio>
            <el-radio label="ADMIN">{{ t('admin') }}</el-radio>
          </el-radio-group>
        </el-form-item>
        
        <el-form-item 
          :label="t('permission_level')" 
          prop="permissionLevel"
          v-if="currentUser.role === 'ADMIN'"
        >
          <el-select v-model="currentUser.permissionLevel" style="width: 100%">
            <el-option :label="t('super_admin')" :value="0" />
            <el-option :label="t('book_manager')" :value="1" />
            <el-option :label="t('user_manager')" :value="2" />
            <el-option :label="t('log_manager')" :value="3" />
            <el-option :label="t('basic_admin')" :value="4" />
          </el-select>
        </el-form-item>
        
        <el-form-item :label="t('password')" v-if="!isEdit" prop="password">
          <el-input 
            v-model="currentUser.password" 
            type="password"
            show-password
            :placeholder="t('password_placeholder')"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="handleClose">{{ t('cancel') }}</el-button>
          <el-button type="primary" @click="saveUser" :loading="saving">
            {{ isEdit ? t('update') : t('create') }}
          </el-button>
        </span>
      </template>
    </el-dialog>
    
    <!-- Change Password Dialog -->
    <el-dialog 
      :title="t('change_password')" 
      v-model="passwordDialogVisible" 
      width="400px" 
      :before-close="closePasswordDialog"
    >
      <el-form 
        :model="passwordForm" 
        :rules="passwordRules" 
        ref="passwordFormRef"
        label-width="120px"
      >
        <el-form-item :label="t('new_password')" prop="newPassword">
          <el-input 
            v-model="passwordForm.newPassword" 
            type="password"
            show-password
            :placeholder="t('new_password_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('confirm_password')" prop="confirmPassword">
          <el-input 
            v-model="passwordForm.confirmPassword" 
            type="password"
            show-password
            :placeholder="t('confirm_password_placeholder')"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="closePasswordDialog">{{ t('cancel') }}</el-button>
          <el-button type="primary" @click="confirmChangePassword" :loading="changingPassword">
            {{ t('confirm') }}
          </el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage, ElMessageBox } from 'element-plus';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { User, PermissionLevel } from '@/types';

// Icons
import { Plus, ArrowLeft } from '@element-plus/icons-vue';

const { t } = useI18n();
const router = useRouter();

interface SearchForm {
  keyword: string;
  role: string;
}

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

interface PasswordForm {
  newPassword: string;
  confirmPassword: string;
}

type EditableUser = User & { password?: string };

const users = ref<User[]>([]);
const loading = ref(false);
const saving = ref(false);
const changingPassword = ref(false);
const dialogVisible = ref(false);
const passwordDialogVisible = ref(false);
const isEdit = ref(false);

const searchForm = ref<SearchForm>({
  keyword: '',
  role: ''
});

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

const currentUser = ref<EditableUser>({
  id: '',
  username: '',
  name: '',
  college: '',
  className: '',
  role: 'STUDENT',
  hasRecoveryToken: false,
  permissionLevel: PermissionLevel.BASIC_ADMIN
});

const passwordForm = ref<PasswordForm>({
  newPassword: '',
  confirmPassword: ''
});

const userFormRef = ref();
const passwordFormRef = ref();

const dialogTitle = computed(() => {
  return isEdit.value ? t('edit_user') : t('add_new_user');
});

const validatePassword = (rule: any, value: string, callback: Function) => {
  if (value !== passwordForm.value.newPassword) {
    callback(new Error(t('password_mismatch')));
  } else {
    callback();
  }
};

const userRules = {
  id: [
    { required: true, message: t('student_id_required'), trigger: 'blur' }
  ],
  username: [
    { required: true, message: t('username_required'), trigger: 'blur' }
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
  ]
};

const passwordRules = {
  newPassword: [
    { required: true, message: t('new_password_required'), trigger: 'blur' },
    { min: 6, max: 20, message: t('password_length'), trigger: 'blur' }
  ],
  confirmPassword: [
    { required: true, message: t('confirm_password_required'), trigger: 'blur' },
    { validator: validatePassword, trigger: 'blur' }
  ]
};

onMounted(() => {
  fetchUsers();
});

const goBack = () => {
  if (window.history.length > 1) {
    router.back();
    return;
  }
  router.push('/admin');
};

const fetchUsers = async () => {
  loading.value = true;
  try {
    const response = await apiService.getUsers(
      searchForm.value.role,
      pagination.value.currentPage - 1,
      pagination.value.pageSize
    );
    
    if (response.success && response.data) {
      users.value = response.data.users;
      pagination.value.total = response.data.totalCount;
    } else {
      ElMessage.error(response.error || t('fetch_users_error'));
    }
  } catch (error) {
    console.error('Error fetching users:', error);
    ElMessage.error(t('fetch_users_error'));
  } finally {
    loading.value = false;
  }
};

const searchUsers = () => {
  pagination.value.currentPage = 1;
  fetchUsers();
};

const resetSearch = () => {
  searchForm.value.keyword = '';
  searchForm.value.role = '';
  pagination.value.currentPage = 1;
  fetchUsers();
};

const handleSizeChange = (size: number) => {
  pagination.value.pageSize = size;
  pagination.value.currentPage = 1;
  fetchUsers();
};

const handleCurrentChange = (page: number) => {
  pagination.value.currentPage = page;
  fetchUsers();
};

const addNewUser = () => {
  isEdit.value = false;
  currentUser.value = {
    id: '',
    username: '',
    name: '',
    college: '',
    className: '',
    role: 'STUDENT',
    hasRecoveryToken: false,
    permissionLevel: PermissionLevel.BASIC_ADMIN
  };
  dialogVisible.value = true;
};

const editUser = (user: User) => {
  isEdit.value = true;
  currentUser.value = { ...user };
  dialogVisible.value = true;
};

const viewUserDetails = (user: User) => {
  ElMessageBox.alert(`
    <div class="user-details">
      <p><strong>${t('student_id')}:</strong> ${user.id}</p>
      <p><strong>${t('username')}:</strong> ${user.username}</p>
      <p><strong>${t('name')}:</strong> ${user.name}</p>
      <p><strong>${t('college')}:</strong> ${user.college}</p>
      <p><strong>${t('class')}:</strong> ${user.className}</p>
      <p><strong>${t('role')}:</strong> ${user.role === 'ADMIN' ? t('admin') : t('student')}</p>
      <p><strong>${t('recovery_token_status')}:</strong> ${user.hasRecoveryToken ? t('token_set') : t('token_not_set')}</p>
      ${user.role === 'ADMIN' ? `<p><strong>${t('permission_level')}:</strong> ${getPermissionLevelText(user.permissionLevel)}</p>` : ''}
    </div>
  `, t('user_details'), {
    dangerouslyUseHTMLString: true,
    confirmButtonText: t('ok'),
  });
};

const getPermissionLevelText = (level: PermissionLevel | undefined): string => {
  switch (level) {
    case PermissionLevel.SUPER_ADMIN: return t('super_admin');
    case PermissionLevel.BOOK_MANAGER: return t('book_manager');
    case PermissionLevel.USER_MANAGER: return t('user_manager');
    case PermissionLevel.LOG_MANAGER: return t('log_manager');
    case PermissionLevel.BASIC_ADMIN: return t('basic_admin');
    default: return t('unknown');
  }
};

const saveUser = async () => {
  try {
    const valid = await userFormRef.value.validate();
    if (!valid) return;
    
    saving.value = true;
    
    if (isEdit.value) {
      // Update existing user
      const { password: _password, ...userPayload } = currentUser.value;
      const response = await apiService.updateUser(userPayload);
      if (response.success) {
        ElMessage.success(t('user_updated_success'));
        dialogVisible.value = false;
        fetchUsers();
      } else {
        ElMessage.error(response.error || t('user_update_error'));
      }
    } else {
      // Add new user
      const { password, ...userData } = currentUser.value;
      const response = await apiService.register({
        ...userData,
        password: password || ''
      });
      if (response.success) {
        ElMessage.success(t('user_added_success'));
        dialogVisible.value = false;
        fetchUsers();
      } else {
        ElMessage.error(response.error || t('user_add_error'));
      }
    }
  } catch (error) {
    console.error('Error saving user:', error);
    ElMessage.error(t('save_error'));
  } finally {
    saving.value = false;
  }
};

const deleteUser = async (userId: string) => {
  try {
    const response = await apiService.updateUser({ ...currentUser.value, id: userId }); // Placeholder for actual delete API
    // Note: The actual API doesn't seem to have a delete user endpoint, so we'll simulate it
    ElMessage.warning(t('delete_user_not_implemented'));
  } catch (error) {
    console.error('Error deleting user:', error);
    ElMessage.error(t('delete_error'));
  }
};

const changePassword = (user: User) => {
  currentUser.value = { ...user };
  passwordForm.value.newPassword = '';
  passwordForm.value.confirmPassword = '';
  passwordDialogVisible.value = true;
};

const confirmChangePassword = async () => {
  try {
    const valid = await passwordFormRef.value.validate();
    if (!valid) return;
    
    changingPassword.value = true;
    
    // For now, we'll simulate changing a user's password
    // In a real implementation, there would be an API endpoint for this
    ElMessage.warning(t('change_user_password_not_implemented'));
    closePasswordDialog();
  } catch (error) {
    console.error('Error changing password:', error);
    ElMessage.error(t('password_change_error'));
  } finally {
    changingPassword.value = false;
  }
};

const handleClose = () => {
  dialogVisible.value = false;
};

const closePasswordDialog = () => {
  passwordDialogVisible.value = false;
};
</script>

<style scoped>
.user-management {
  padding: 20px;
}

.header-section {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 12px;
}

.back-btn {
  padding: 8px 12px;
}

.add-user-btn {
  margin-left: 20px;
}

.search-card {
  margin-bottom: 20px;
}

.search-form {
  display: flex;
  flex-wrap: wrap;
}

.search-form .el-form-item {
  margin-right: 20px;
  margin-bottom: 0;
}

.role-select {
  width: 150px;
}

.users-table {
  margin-bottom: 20px;
}

.pagination {
  text-align: right;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}

.user-details p {
  margin: 5px 0;
}
</style>
