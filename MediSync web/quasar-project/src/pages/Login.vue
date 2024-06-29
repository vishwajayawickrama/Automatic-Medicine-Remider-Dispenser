<template>
  <q-page class="q-pa-md">
    <div class="q-gutter-md">
      <q-input v-model="email" label="Email" />
      <q-input v-model="password" label="Password" type="password" />
      <q-btn @click="login" label="Login" color="primary" />
      <q-btn @click="signup" label="Sign Up" color="secondary" />
    </div>
    <q-spinner v-if="loading" color="primary" />
  </q-page>
</template>

<script>
import { ref } from 'vue';
import { useRouter } from 'vue-router';
import { auth } from 'src/firebase';
import { signInWithEmailAndPassword, createUserWithEmailAndPassword } from 'firebase/auth';
import { Notify } from 'quasar';

export default {
  setup() {
    const email = ref('');
    const password = ref('');
    const loading = ref(false);
    const router = useRouter();

    const login = async () => {
      loading.value = true;
      try {
        await signInWithEmailAndPassword(auth, email.value, password.value);
        Notify.create({
          type: 'positive',
          message: 'Login successful!'
        });
        router.push('/medicine-schedule'); // Redirect to medicine schedule page
      } catch (error) {
        console.error('Login error:', error.message);
        Notify.create({
          type: 'negative',
          message: `Login error: ${error.message}`
        });
      } finally {
        loading.value = false;
      }
    };

    const signup = async () => {
      loading.value = true;
      try {
        await createUserWithEmailAndPassword(auth, email.value, password.value);
        Notify.create({
          type: 'positive',
          message: 'Signup successful!'
        });
        router.push('/');
      } catch (error) {
        console.error('Signup error:', error.message);
        Notify.create({
          type: 'negative',
          message: `Signup error: ${error.message}`
        });
      } finally {
        loading.value = false;
      }
    };

    return { email, password, login, signup, loading };
  }
};
</script>

<style scoped>
.q-page {
  max-width: 300px;
  margin: auto;
}
</style>
