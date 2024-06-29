<template>
  <q-page class="q-pa-md">
    <div class="q-gutter-md">
      <table class="table">
        <thead>
          <tr>
            <th>Compartment Number</th>
            <th>Medicine Name</th>
            <th>Current Quantity</th>
            <th>Refill Quantity</th>
            <th>Action</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="medicine in medicines" :key="medicine.id">
            <td>{{ medicine.compartment_number }}</td>
            <td>{{ medicine.medicine_name }}</td>
            <td>{{ medicine.quantity }}</td>
            <td>
              <q-input v-model="medicine.refillQuantity" type="number" />
            </td>
            <td>
              <q-btn @click="submitRefill(medicine)" label="Submit Refill" color="primary" />
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <q-spinner v-if="loading" color="primary" />
  </q-page>
</template>

<script>
import { ref } from 'vue';
import { firestore } from 'src/firebase';
import { doc, getDoc, setDoc } from 'firebase/firestore';
import { Notify } from 'quasar';

export default {
  setup() {
    const medicineNames = ref({});
    const medicines = ref([]);
    const loading = ref(false);

    // Fetch medicine names from Firestore
    async function fetchMedicineNames() {
      try {
        for (let i = 1; i <= 4; i++) { // Assuming you have 4 compartments
          const docRef = doc(firestore, 'medicines', i.toString());
          const docSnap = await getDoc(docRef);
          if (docSnap.exists()) {
            const medicineData = docSnap.data();
            const medicineName = medicineData.medicine_name || '';
            medicineNames.value[i] = medicineName; // Store medicine name in an object with index
          } else {
            console.warn(`Medicine name for compartment ${i} not found.`);
          }
        }
      } catch (error) {
        console.error('Error fetching medicine names:', error);
        Notify.create({
          type: 'negative',
          message: `Error: ${error.message}`
        });
      }
    }

    // Function to fetch compartment data from Firestore
    async function fetchData() {
      const compartmentsData = [];
      for (let i = 1; i <= 4; i++) { // Assuming you have 4 compartments
        try {
          const docRef = doc(firestore, 'medicines', i.toString());
          const docSnap = await getDoc(docRef);
          if (docSnap.exists()) {
            const data = docSnap.data();
            const quantity = parseInt(data.quantity) || 0; // Ensure quantity is an integer
            compartmentsData.push({
              id: i,
              compartment_number: i,
              medicine_name: medicineNames.value[i] || '', // Use medicine name from medicineNames object
              quantity: quantity,
              refillQuantity: 0 // Initialize refill quantity to zero as integer
            });
          } else {
            console.warn(`Compartment ${i} data not found in database.`);
          }
        } catch (error) {
          console.error(`Error fetching compartment ${i} data:`, error);
        }
      }
      medicines.value = compartmentsData;
    }

    // Submit refill for a specific medicine

const submitRefill = async (medicine) => {
  if (medicine.refillQuantity <= 0) {
    Notify.create({
      type: 'negative',
      message: 'Refill quantity must be greater than zero.'
    });
    return;
  }

  loading.value = true;
  try {
    const docRef = doc(firestore, 'medicines', medicine.id.toString());
    await setDoc(docRef, {
      quantity: medicine.quantity + parseInt(medicine.refillQuantity) // Update quantity in Firestore
    }, { merge: true });
    Notify.create({
      type: 'positive',
      message: 'Refill submitted successfully!'
    });
    medicine.refillQuantity = 0; // Reset refill quantity after submission
  } catch (error) {
    console.error('Error submitting refill:', error);
    Notify.create({
      type: 'negative',
      message: `Error: ${error.message}`
    });
  } finally {
    loading.value = false;
  }
};

    fetchMedicineNames(); // Fetch medicine names when component is mounted
    fetchData(); // Fetch compartment data when component is mounted

    return { medicines, submitRefill, loading };
  }
};
</script>

<style scoped>
.q-page {
  max-width: 600px;
  margin: auto;
}
</style>



<style scoped>
.q-page {
  max-width: 800px; /* Adjust as needed */
  margin: auto;
}

.table-container {
  margin-top: 20px;
  border: 1px solid #ddd;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 0 20px rgba(0,0,0,0.1);
  animation: fadeIn 0.5s ease;
}

.table {
  width: 100%;
  border-collapse: collapse;
}

.table th,
.table td {
  padding: 10px;
  text-align: center;
}

.thead-dark {
  background-color: #343a40; /* Dark header background */
  color: white;
}

.text-primary {
  color: #1976d2; /* Primary color */
}

/* Fade in animation */
@keyframes fadeIn {
  0% {
    opacity: 0;
    transform: translateY(20px);
  }
  100% {
    opacity: 1;
    transform: translateY(0);
  }
}
</style>
