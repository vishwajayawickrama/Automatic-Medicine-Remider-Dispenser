import { route } from 'quasar/wrappers';
import { createRouter, createMemoryHistory, createWebHistory, createWebHashHistory } from 'vue-router';
import routes from './routes'; // Assuming you have defined routes elsewhere

// Import IntroPage.vue
import IntroPage from '../pages/IntroPage.vue'; // Adjust the path as per your project structure

export default route(function (/* { store, ssrContext } */) {
  const createHistory = process.env.SERVER
    ? createMemoryHistory
    : (process.env.VUE_ROUTER_MODE === 'history' ? createWebHistory : createWebHashHistory);

  // Add IntroPage route to existing routes
  const routesWithIntro = [
    {
      path: '/',
      redirect: '/home' // Redirect root to IntroPage
    },
    {
      path: '/intro',
      name: 'IntroPage',
      component: IntroPage
    },
    ...routes // Spread existing routes
  ];

  const Router = createRouter({
    scrollBehavior: () => ({ left: 0, top: 0 }),
    routes: routesWithIntro,

    // Leave this as is and make changes in quasar.conf.js instead!
    // quasar.conf.js -> build -> vueRouterMode
    // quasar.conf.js -> build -> publicPath
    history: createHistory(process.env.VUE_ROUTER_BASE)
  });

  return Router;
});
