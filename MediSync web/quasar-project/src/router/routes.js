const routes = [
  {
    path: '/',
    component: () => import('layouts/MainLayout.vue'),
    children: [
      { path: '', component: () => import('pages/IndexPage.vue') },
      { path: 'login', component: () => import('pages/Login.vue') },
      { path: 'medicine-schedule', component: () => import('pages/MedicineSchedule.vue') },
      { path: 'home', component: () => import('pages/IntroPage.vue') },
      { path: 'refill', component: () => import('pages/RefillPage.vue') }
    ]
  },

  // Always leave this as last one,
  // but you can also remove it
  {
    path: '/:catchAll(.*)*',
    component: () => import('pages/ErrorNotFound.vue')
  }
]

export default routes
