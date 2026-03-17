<template>
  <div class="not-found">
    <div class="container">
      <h1>404</h1>
      <h2>{{ t('page_not_found') }}</h2>
      <p>{{ t('page_does_not_exist') }}</p>
      <el-button type="primary" @click="goHome" class="home-button">
        {{ t('back_to_home') }}
      </el-button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useRouter } from 'vue-router';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();
const router = useRouter();

const goHome = () => {
  router.push('/');
};
</script>

<style scoped>
.not-found {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  background-color: #f5f5f5;
}

.container {
  text-align: center;
  padding: 40px;
  background: white;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
}

h1 {
  font-size: 72px;
  color: #409eff;
  margin: 0;
}

h2 {
  font-size: 24px;
  color: #303133;
  margin: 20px 0 10px;
}

p {
  color: #909399;
  margin-bottom: 30px;
}

.home-button {
  margin-top: 20px;
}
</style>