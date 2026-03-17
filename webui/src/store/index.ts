// stores/index.ts - Main store file
import { defineStore } from 'pinia';

export const useMainStore = defineStore('main', {
  state: () => ({
    isLoading: false,
    error: null as string | null,
    successMessage: null as string | null,
  }),
  
  actions: {
    setLoading(status: boolean) {
      this.isLoading = status;
    },
    
    setError(message: string | null) {
      this.error = message;
      if (message) {
        // Clear error after 5 seconds
        setTimeout(() => {
          this.error = null;
        }, 5000);
      }
    },
    
    setSuccessMessage(message: string | null) {
      this.successMessage = message;
      if (message) {
        // Clear success message after 3 seconds
        setTimeout(() => {
          this.successMessage = null;
        }, 3000);
      }
    }
  }
});