<template>
  <q-layout view="hHh lpR fFf">
    <!-- Header -->
    <q-header elevated class="bg-primary">
      <q-toolbar class="text-white">
        <q-toolbar-title class="q-pa-sm">
          <span style="font-size: 1.5rem; font-weight: bold;">MediSync</span>
        </q-toolbar-title>

        <!-- Navigation Buttons -->
        <q-space />

        <q-btn flat to="/medicine-schedule" class="text-white" active-class="bg-secondary" exact>
          Medicine Schedule
        </q-btn>

        <q-btn flat to="/refill" class="text-white" active-class="bg-secondary" exact>
          Refill
        </q-btn>

        <q-btn flat to="/login" class="text-white" active-class="bg-secondary" exact>
          Login
        </q-btn>
      </q-toolbar>
    </q-header>

    <!-- Page Container -->
    <q-page-container class="q-pa-md">
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<script>
export default {
  name: 'MainLayout'
};
</script>

<style scoped>
/* Custom Styling for Header */
.bg-primary {
  background-color: #1976D2; /* Primary color */
}

.text-white {
  color: #ffffff; /* White text color */
}

/* Adjust Toolbar Title */
.q-toolbar-title {
  font-size: 1.5rem;
}

/* Space between buttons */
.q-space {
  flex: 1; /* Pushes buttons to the right */
}

/* Active class for buttons */
.bg-secondary {
  background-color: #2196F3; /* Lighter shade of primary color */
}
</style>
