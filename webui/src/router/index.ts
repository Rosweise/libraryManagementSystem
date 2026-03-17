import { createRouter, createWebHistory, RouteRecordRaw } from 'vue-router';
import { useUserStore } from '@/store/user';

// Define route components (lazy loaded)
const Login = () => import('@/views/EnhancedLogin.vue');
const Register = () => import('@/views/Register.vue');
const ForgotPassword = () => import('@/views/ForgotPassword.vue');
const AdminDashboard = () => import('@/views/admin/AdminDashboard.vue');
const StudentDashboard = () => import('@/views/student/StudentDashboard.vue');
const BookManagement = () => import('@/views/admin/BookManagement.vue');
const UserManagement = () => import('@/views/admin/UserManagement.vue');
const BorrowManagement = () => import('@/views/admin/BorrowManagement.vue');
const StudentBorrowings = () => import('@/views/student/StudentBorrowings.vue');
const AccountManagement = () => import('@/views/AccountManagement.vue');
const SearchBooks = () => import('@/views/SearchBooks.vue');
const NotFound = () => import('@/views/NotFound.vue');

const routes: Array<RouteRecordRaw> = [
  {
    path: '/',
    redirect: '/login'
  },
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: { requiresGuest: true }
  },
  {
    path: '/register',
    name: 'Register',
    component: Register,
    meta: { requiresGuest: true }
  },
  {
    path: '/forgot-password',
    name: 'ForgotPassword',
    component: ForgotPassword,
    meta: { requiresGuest: true }
  },
  {
    path: '/admin',
    name: 'AdminDashboard',
    component: AdminDashboard,
    meta: { requiresAuth: true, role: 'ADMIN' },
    children: [
      {
        path: 'books',
        name: 'BookManagement',
        component: BookManagement,
        meta: { requiresAuth: true, role: 'ADMIN', permission: 1 } // BOOK_MANAGER
      },
      {
        path: 'users',
        name: 'UserManagement',
        component: UserManagement,
        meta: { requiresAuth: true, role: 'ADMIN', permission: 2 } // USER_MANAGER
      },
      {
        path: 'borrowings',
        name: 'BorrowManagement',
        component: BorrowManagement,
        meta: { requiresAuth: true, role: 'ADMIN' }
      },
      {
        path: 'account',
        name: 'AdminAccountManagement',
        component: AccountManagement,
        meta: { requiresAuth: true, role: 'ADMIN' }
      }
    ]
  },
  {
    path: '/student',
    name: 'StudentDashboard',
    component: StudentDashboard,
    meta: { requiresAuth: true, role: 'STUDENT' },
    children: [
      {
        path: 'search',
        name: 'SearchBooks',
        component: SearchBooks,
        meta: { requiresAuth: true, role: 'STUDENT' }
      },
      {
        path: 'borrowings',
        name: 'StudentBorrowings',
        component: StudentBorrowings,
        meta: { requiresAuth: true, role: 'STUDENT' }
      },
      {
        path: 'account',
        name: 'StudentAccountManagement',
        component: AccountManagement,
        meta: { requiresAuth: true, role: 'STUDENT' }
      }
    ]
  },
  {
    path: '/:pathMatch(.*)*',
    name: 'NotFound',
    component: NotFound
  }
];

const router = createRouter({
  history: createWebHistory(),
  routes
});

// Navigation guard
router.beforeEach((to, from, next) => {
  const userStore = useUserStore();
  const token = localStorage.getItem('token');
  
  if (to.meta.requiresAuth) {
    if (!token || !userStore.isAuthenticated) {
      next('/login');
    } else if (to.meta.role && userStore.user?.role !== to.meta.role) {
      next('/login'); // Redirect unauthorized users
    } else if (typeof to.meta.permission === 'number' && userStore.user) {
      // Check permission level for admin functions
      if (userStore.user.role === 'ADMIN' && userStore.user.permissionLevel !== undefined) {
        if (userStore.user.permissionLevel > to.meta.permission) {
          next('/admin'); // Redirect to dashboard if insufficient permissions
        } else {
          next();
        }
      } else {
        next('/admin'); // Redirect if not admin
      }
    } else {
      next();
    }
  } else if (to.meta.requiresGuest && token) {
    // If guest route but user is authenticated, redirect to dashboard
    if (userStore.user?.role === 'ADMIN') {
      next('/admin');
    } else if (userStore.user?.role === 'STUDENT') {
      next('/student');
    } else {
      next();
    }
  } else {
    next();
  }
});

export default router;
