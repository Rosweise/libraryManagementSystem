<template>
  <div class="h-screen w-screen overflow-hidden bg-scene">
    <!-- Background gradient -->
    <div class="absolute inset-0 bg-gradient-to-br from-blue-50 via-indigo-50 to-purple-50"></div>

    <!-- Login Screen -->
    <div v-if="!isAuthenticated" class="relative h-full w-full flex items-center justify-center p-4">
      <div class="w-full max-w-md">
        <!-- Logo -->
        <div class="text-center mb-8 slide-up stagger-1">
          <div class="w-16 h-16 rounded-2xl btn-primary flex items-center justify-center mx-auto mb-4">
            <Library :size="32" class="text-white" />
          </div>
          <h1 class="text-3xl font-bold text-gray-800 mb-2">{{ t('library_system') }}</h1>
          <p class="text-gray-500 font-light">{{ t('welcome_message') }}</p>
        </div>

        <!-- Login Mode Toggle -->
        <div class="glass rounded-2xl p-1.5 flex gap-1.5 mb-6 slide-up stagger-2">
          <button
            @click="loginMode = 'admin'"
            :class="[
              'flex-1 py-2.5 rounded-xl text-sm font-medium transition-all',
              loginMode === 'admin' ? 'btn-primary text-white' : 'text-gray-600'
            ]"
          >
            <Shield :size="16" class="inline mr-1.5" />
            {{ t('admin') }}
          </button>
          <button
            @click="loginMode = 'student'"
            :class="[
              'flex-1 py-2.5 rounded-xl text-sm font-medium transition-all',
              loginMode === 'student' ? 'btn-primary text-white' : 'text-gray-600'
            ]"
          >
            <GraduationCap :size="16" class="inline mr-1.5" />
            {{ t('student') }}
          </button>
        </div>

        <!-- Login Form -->
        <div class="glass-strong rounded-3xl p-8 slide-up stagger-3">
          <div class="flex items-center gap-3 mb-6">
            <div :class="[
              'w-10 h-10 rounded-xl flex items-center justify-center',
              loginMode === 'admin' ? 'bg-indigo-100' : 'bg-sky-100'
            ]">
              <component :is="loginMode === 'admin' ? Shield : GraduationCap"
                :size="20"
                :class="loginMode === 'admin' ? 'text-indigo-600' : 'text-sky-600'"
              />
            </div>
            <h2 class="text-xl font-semibold text-gray-800">
              {{ loginMode === 'admin' ? t('admin') : t('student') }}
            </h2>
          </div>

          <form @submit.prevent="handleLogin">
            <label class="block text-sm font-medium text-gray-600 mb-1.5">
              {{ t('username') }}
            </label>
            <input
              v-model="credentials.username"
              type="text"
              :placeholder="loginMode === 'admin' ? 'admin' : 'student'"
              class="input-glass w-full rounded-xl px-4 py-3 mb-4 text-gray-700"
            />

            <label class="block text-sm font-medium text-gray-600 mb-1.5">
              {{ t('password') }}
            </label>
            <input
              v-model="credentials.password"
              type="password"
              placeholder="••••••••"
              class="input-glass w-full rounded-xl px-4 py-3 mb-6 text-gray-700"
            />

            <button
              type="submit"
              :class="[
                'w-full py-3 rounded-xl font-medium text-base flex items-center justify-center gap-2',
                loginMode === 'admin' ? 'btn-primary' : 'btn-secondary text-gray-700'
              ]"
              :disabled="loading"
            >
              <LogIn :size="18" />
              {{ loading ? t('logging_in') : t('login_button') }}
            </button>
          </form>
        </div>
      </div>
    </div>

    <!-- Dashboard -->
    <div v-else class="relative h-full w-full flex flex-col">
      <router-view />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue';
import { useRouter } from 'vue-router';
import { useI18n } from 'vue-i18n';
import { useUserStore } from '@/store/user';
import { Library, Shield, GraduationCap, LogIn } from 'lucide-vue-next';
import { ElMessage } from 'element-plus';
import apiService from '@/api';

const { t } = useI18n();
const router = useRouter();
const userStore = useUserStore();

const loginMode = ref<'admin' | 'student'>('admin');
const credentials = ref({
  username: '',
  password: ''
});
const loading = ref(false);

const isAuthenticated = computed(() => userStore.isAuthenticated);

const handleLogin = async () => {
  if (!credentials.value.username || !credentials.value.password) {
    ElMessage.warning(t('login_required'));
    return;
  }

  loading.value = true;
  try {
    const response = await apiService.login(credentials.value);

    if (response.success && response.data) {
      userStore.setUser(response.data.user);
      userStore.setToken(response.data.token);

      ElMessage.success(t('login_success'));

      // Redirect based on role
      if (response.data.user.role === 'ADMIN') {
        router.push('/admin');
      } else {
        router.push('/student');
      }
    } else {
      ElMessage.error(response.error || t('login_failed'));
    }
  } catch (error) {
    console.error('Login error:', error);
    ElMessage.error(t('login_error'));
  } finally {
    loading.value = false;
  }
};
</script>

<style scoped>
.bg-scene {
  position: relative;
  overflow: hidden;
}

.glass {
  background: rgba(255, 255, 255, 0.45);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.5);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.06);
}

.glass-strong {
  background: rgba(255, 255, 255, 0.65);
  backdrop-filter: blur(28px);
  -webkit-backdrop-filter: blur(28px);
  border: 1px solid rgba(255, 255, 255, 0.6);
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.08);
}

.input-glass {
  background: rgba(255, 255, 255, 0.6);
  backdrop-filter: blur(16px);
  -webkit-backdrop-filter: blur(16px);
  border: 1px solid rgba(200, 210, 230, 0.4);
  transition: all 0.3s ease;
}

.input-glass:focus {
  background: rgba(255, 255, 255, 0.75);
  border-color: rgba(129, 140, 248, 0.5);
  box-shadow: 0 0 0 3px rgba(129, 140, 248, 0.12), 0 4px 12px rgba(129, 140, 248, 0.08);
  outline: none;
}

.btn-primary {
  background: linear-gradient(135deg, #818cf8 0%, #6366f1 100%);
  color: white;
  transition: all 0.3s ease;
  box-shadow: 0 4px 15px rgba(99, 102, 241, 0.3);
}

.btn-primary:hover:not(:disabled) {
  box-shadow: 0 6px 25px rgba(99, 102, 241, 0.45);
  transform: translateY(-1px);
}

.btn-primary:disabled {
  opacity: 0.6;
  cursor: not-allowed;
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

.stagger-1 {
  animation-delay: 0.1s;
  opacity: 0;
}

.stagger-2 {
  animation-delay: 0.2s;
  opacity: 0;
}

.stagger-3 {
  animation-delay: 0.3s;
  opacity: 0;
}
</style>
