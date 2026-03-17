<template>
  <div class="logs-management">
    <h2>{{ t('log_management') }}</h2>
    
    <el-card class="filters-card">
      <el-form :model="filterForm" inline class="filter-form">
        <el-form-item :label="t('start_time')">
          <el-date-picker
            v-model="filterForm.startTime"
            type="datetime"
            :placeholder="t('start_time_placeholder')"
            format="YYYY-MM-DD HH:mm:ss"
            value-format="YYYY-MM-DD HH:mm:ss"
          />
        </el-form-item>
        
        <el-form-item :label="t('end_time')">
          <el-date-picker
            v-model="filterForm.endTime"
            type="datetime"
            :placeholder="t('end_time_placeholder')"
            format="YYYY-MM-DD HH:mm:ss"
            value-format="YYYY-MM-DD HH:mm:ss"
          />
        </el-form-item>
        
        <el-form-item :label="t('user')">
          <el-input 
            v-model="filterForm.user" 
            :placeholder="t('user_placeholder')"
          />
        </el-form-item>
        
        <el-form-item :label="t('level')">
          <el-select v-model="filterForm.level" class="level-select">
            <el-option :label="t('all_levels')" value="" />
            <el-option :label="t('debug')" value="DEBUG" />
            <el-option :label="t('info')" value="INFO" />
            <el-option :label="t('warn')" value="WARN" />
            <el-option :label="t('error')" value="ERROR" />
          </el-select>
        </el-form-item>
        
        <el-form-item>
          <el-button type="primary" @click="fetchLogs">{{ t('search') }}</el-button>
          <el-button @click="resetFilters">{{ t('reset') }}</el-button>
        </el-form-item>
      </el-form>
    </el-card>
    
    <el-table 
      :data="logs" 
      v-loading="loading"
      class="logs-table"
      row-key="id"
      height="500"
    >
      <el-table-column prop="id" :label="t('id')" width="80" />
      <el-table-column prop="timestamp" :label="t('time')" width="160" />
      <el-table-column prop="level" :label="t('level')" width="100">
        <template #default="scope">
          <el-tag :type="getLogLevelType(scope.row.level)">
            {{ scope.row.level }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="user" :label="t('user')" width="120" />
      <el-table-column prop="action" :label="t('action')" width="150" show-overflow-tooltip />
      <el-table-column prop="message" :label="t('message')" show-overflow-tooltip />
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
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { ElMessage } from 'element-plus';
import { useI18n } from 'vue-i18n';
import apiService from '@/api';
import { LogEntry } from '@/types';

const { t } = useI18n();

interface FilterForm {
  startTime: string | null;
  endTime: string | null;
  user: string;
  level: string;
}

interface Pagination {
  currentPage: number;
  pageSize: number;
  total: number;
}

const logs = ref<LogEntry[]>([]);
const loading = ref(false);

const filterForm = ref<FilterForm>({
  startTime: null,
  endTime: null,
  user: '',
  level: ''
});

const pagination = ref<Pagination>({
  currentPage: 1,
  pageSize: 10,
  total: 0
});

onMounted(() => {
  fetchLogs();
});

const fetchLogs = async () => {
  loading.value = true;
  try {
    const response = await apiService.getLogs(
      filterForm.value.startTime || undefined,
      filterForm.value.endTime || undefined,
      filterForm.value.user || undefined,
      filterForm.value.level || undefined
    );
    
    if (response.success && response.data) {
      logs.value = response.data;
      pagination.value.total = response.data.length; // Simplified for demo
    } else {
      ElMessage.error(response.error || t('fetch_logs_error'));
    }
  } catch (error) {
    console.error('Error fetching logs:', error);
    ElMessage.error(t('fetch_logs_error'));
  } finally {
    loading.value = false;
  }
};

const resetFilters = () => {
  filterForm.value = {
    startTime: null,
    endTime: null,
    user: '',
    level: ''
  };
  pagination.value.currentPage = 1;
  fetchLogs();
};

const handleSizeChange = (size: number) => {
  pagination.value.pageSize = size;
  pagination.value.currentPage = 1;
  fetchLogs();
};

const handleCurrentChange = (page: number) => {
  pagination.value.currentPage = page;
  fetchLogs();
};

const getLogLevelType = (level: string): string => {
  switch (level) {
    case 'DEBUG': return 'info';
    case 'INFO': return 'success';
    case 'WARN': return 'warning';
    case 'ERROR': return 'danger';
    default: return 'info';
  }
};
</script>

<style scoped>
.logs-management {
  padding: 20px;
}

.filters-card {
  margin-bottom: 20px;
}

.filter-form {
  display: flex;
  flex-wrap: wrap;
}

.filter-form .el-form-item {
  margin-right: 20px;
  margin-bottom: 0;
}

.level-select {
  width: 120px;
}

.logs-table {
  margin-bottom: 20px;
}

.pagination {
  text-align: right;
}
</style>
