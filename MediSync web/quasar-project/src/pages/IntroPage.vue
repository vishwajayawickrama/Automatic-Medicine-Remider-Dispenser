<template>
  <q-page class="q-pa-md">
    <q-card>
      <q-card-section class="q-pt-lg q-pb-lg">
        <h1 class="text-h2 text-primary q-mb-sm q-mb-xs-md text-center">Welcome to MediSync</h1>
        <p class="text-body-1 q-mb-md text-center">Your Personalized Medicine Management System</p>

        <!-- Features List -->
        <div class="q-mb-lg text-center ">
          <h2 class="text-h4 q-mb-sm text-center">Features</h2>
          <ul class="q-list">
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Automatic Medicine Dispensing System</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Schedule Management with notifications</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> User-friendly Interface for prescription management</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Alarm and Notifications for medication reminders</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Override schedule for custom medications</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Dose Verification through the interface</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Refill Indicator for medication supply alerts</li>
            <li class="q-mb-xs"><q-icon name="done" class="text-primary" /> Backup Battery power for uninterrupted operation</li>
          </ul>
        </div>

        <!-- Call to Action Buttons -->
        <div class="text-center"> <!-- Center aligning container -->
          <q-btn to="/medicine-schedule" class="q-mr-md" color="primary" label="Manage Medicine Schedule" />
          <q-btn to="/refill" class="q-mr-md" color="primary" label="Refill" />
          <q-btn to="/login" class="q-mr-md" color="secondary" label="Login" />
        </div>
      </q-card-section>
    </q-card>
  </q-page>
</template>

<script>
export default {
  name: 'IntroPage'
};
</script>

<style scoped>
.text-primary {
  color: #1976D2; /* Primary color */
  font-weight: bold; /* Bold text */
}

.q-pt-lg {
  padding-top: 24px; /* Large top padding */
}

.q-pb-lg {
  padding-bottom: 24px; /* Large bottom padding */
}

.q-mb-md {
  margin-bottom: 16px; /* Medium bottom margin */
}

.q-mb-lg {
  margin-bottom: 24px; /* Large bottom margin */
}

.q-mb-xs-md {
  margin-bottom: 12px; /* Medium bottom margin for extra small screens */
}

.text-center {
  text-align: center; /* Center align text */
}

.q-list {
  list-style-type: none; /* Remove default list styles */
  padding-left: 0; /* Remove default padding */
}

.q-btn {
  margin-top: 10px;
  min-width: 150px; /* Ensure buttons have consistent width */
}
</style>
