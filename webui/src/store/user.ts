// stores/user.ts - User authentication store
import { defineStore } from 'pinia';
import { User } from '@/types';

export const useUserStore = defineStore('user', {
  state: () => ({
    user: null as User | null,
    token: localStorage.getItem('token') || null as string | null,
    isAuthenticated: false,
  }),
  
  getters: {
    isAdmin(): boolean {
      return this.user?.role === 'ADMIN';
    },
    
    isStudent(): boolean {
      return this.user?.role === 'STUDENT';
    },
    
    hasPermission(): (level: number) => boolean {
      return (requiredLevel: number) => {
        if (!this.user || this.user.role !== 'ADMIN') return false;

        // Backward compatibility: old backend responses may omit permissionLevel.
        if (this.user.permissionLevel === undefined) return true;

        return this.user.permissionLevel <= requiredLevel;
      };
    }
  },
  
  actions: {
    setToken(token: string) {
      this.token = token;
      localStorage.setItem('token', token);
      this.isAuthenticated = true;
    },
    
    setUser(user: User) {
      this.user = {
        ...user,
        hasRecoveryToken: user.hasRecoveryToken ?? false,
        permissionLevel: user.role === 'ADMIN' ? (user.permissionLevel ?? 0) : undefined
      };
      this.isAuthenticated = true;
    },
    
    logout() {
      this.user = null;
      this.token = null;
      this.isAuthenticated = false;
      localStorage.removeItem('token');
    },
    
    clearAuth() {
      this.logout();
    }
  }
});
