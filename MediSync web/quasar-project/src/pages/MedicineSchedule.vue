<template>
  <q-page class="q-pa-md">
    <div class="q-gutter-md">
      <q-card flat bordered class="q-mb-md">
        <q-card-section class="bg-primary text-white">
          <div class="text-h5">Medicine Dosage Schedule</div>
        </q-card-section>
        <q-form @submit.prevent="submitAllRows">
          <q-table :rows="rows" :columns="columns" flat square class="medicine-table">
            <template v-slot:body-cell-compartment_number="props">
              <q-td>
                <span>{{ props.row.compartment_number }}</span>
              </q-td>
            </template>
            <template v-slot:body-cell-medicine_name="props">
              <q-td>
                <q-input
                  v-model="props.row.medicine_name"
                  :error="submittedRows[props.row.compartment_number] && !props.row.medicine_name"
                  dense
                  clearable
                  outlined
                  label="Medicine Name"
                  class="q-pa-sm"
                />
              </q-td>
            </template>
            <template v-slot:body-cell-times_per_day="props">
              <q-td>
                <q-select
                  v-model="props.row.times_per_day"
                  :options="[1, 2, 3]"
                  @update:model-value="updateTimesFields(props.row)"
                  dense
                  outlined
                  class="q-pa-sm"
                />
              </q-td>
            </template>
            <template v-slot:body-cell-times="props">
              <q-td>
                <div v-for="(time, index) in props.row.times" :key="index" class="q-mb-sm">
                  <q-input
                    v-model="props.row.times[index]"
                    :placeholder="'Time ' + (index + 1)"
                    :error="submittedRows[props.row.compartment_number] && !props.row.times[index]"
                    dense
                    outlined
                    clearable
                    :label="'Time ' + (index + 1)"
                    class="q-pa-sm"
                  />
                </div>
              </q-td>
            </template>
            <template v-slot:body-cell-dose_per_time="props">
              <q-td>
                <q-select
                  v-model="props.row.dose_per_time"
                  :options="[1, 2, 3]"
                  dense
                  outlined
                  class="q-pa-sm"
                />
              </q-td>
            </template>
            <template v-slot:body-cell-action="props">
              <q-td>
                <q-btn @click="submitRow(props.row)" label="Submit" color="primary" dense class="full-width q-pa-sm" />
              </q-td>
            </template>
          </q-table>
        </q-form>
      </q-card>

      <q-card flat bordered>
        <q-card-section class="bg-info text-white">
          <div class="text-h5">Medicine List</div>
        </q-card-section>
        <q-list bordered>
          <q-item v-for="medicine in medicines" :key="medicine.id" class="q-pa-md q-item-type">
            <q-item-section>
              <div><strong>Compartment:</strong> {{ medicine.compartment_number }}</div>
              <div><strong>Medicine:</strong> {{ medicine.medicine_name }}</div>
              <div><strong>Times per day:</strong> {{ medicine.times_per_day }}</div>
              <div><strong>Times:</strong> {{ medicine.times.join(', ') }}</div>
              <div><strong>Dose per Time:</strong> {{ medicine.dose_per_time }}</div>
            </q-item-section>
            <q-item-section side>
              <q-btn @click="deleteMedicine(medicine.id)" icon="delete" color="negative" dense flat />
            </q-item-section>
          </q-item>
        </q-list>
      </q-card>
    </div>
  </q-page>
</template>

<script>
import { ref, onMounted } from 'vue';
import { collection, getDocs, query, where, doc, setDoc, deleteDoc } from 'firebase/firestore';
import { firestore } from 'src/firebase';
import { Notify } from 'quasar';

export default {
  setup() {
    const rows = ref([
      { compartment_number: 1, medicine_name: '', times_per_day: 1, times: [''], dose_per_time: 1 },
      { compartment_number: 2, medicine_name: '', times_per_day: 1, times: [''], dose_per_time: 1 },
      { compartment_number: 3, medicine_name: '', times_per_day: 1, times: [''], dose_per_time: 1 },
      { compartment_number: 4, medicine_name: '', times_per_day: 1, times: [''], dose_per_time: 1 }
    ]);
    const columns = ref([
      { name: 'compartment_number', label: 'Compartment Number', field: 'compartment_number' },
      { name: 'medicine_name', label: 'Medicine Name', field: 'medicine_name' },
      { name: 'times_per_day', label: 'Times per day', field: 'times_per_day' },
      { name: 'times', label: 'Times', field: 'times' },
      { name: 'dose_per_time', label: 'Dose per Time', field: 'dose_per_time' },
      { name: 'action', label: 'Action', field: 'action' }
    ]);
    const medicines = ref([]);
    const submittedRows = ref({});

    const updateTimesFields = (row) => {
      row.times = Array(row.times_per_day).fill('');
    };

    const validateRow = (row) => {
      return row.medicine_name && row.times.every(time => time);
    };

    const submitRow = async (row) => {
      submittedRows.value[row.compartment_number] = true;
      if (!validateRow(row)) {
        Notify.create({ type: 'warning', message: 'Please fill in all fields.' });
        return;
      }
      try {
        // Check if there's an existing record for this compartment number
        const q = query(collection(firestore, 'medicines'), where('compartment_number', '==', row.compartment_number));
        const querySnapshot = await getDocs(q);

        if (!querySnapshot.empty) {
          // Update existing record
          const docId = querySnapshot.docs[0].id;
          await setDoc(doc(firestore, 'medicines', docId), row);
        } else {
          // Add new record
          await setDoc(doc(firestore, 'medicines', `${row.compartment_number}`), row);
        }

        Notify.create({ type: 'positive', message: 'Medicine added/updated successfully!' });
        fetchMedicines();
        submittedRows.value[row.compartment_number] = false; // Clear validation state after successful submission
      } catch (error) {
        Notify.create({ type: 'negative', message: `Error: ${error.message}` });
      }
    };

    const submitAllRows = () => {
      rows.value.forEach(row => submitRow(row));
    };

    const fetchMedicines = async () => {
      const querySnapshot = await getDocs(collection(firestore, 'medicines'));
      medicines.value = querySnapshot.docs.map(doc => ({ id: doc.id, ...doc.data() }));
    };

    const deleteMedicine = async (id) => {
      try {
        await deleteDoc(doc(firestore, 'medicines', id));
        Notify.create({ type: 'positive', message: 'Medicine deleted successfully!' });
        fetchMedicines();
      } catch (error) {
        Notify.create({ type: 'negative', message: `Error: ${error.message}` });
      }
    };

    onMounted(() => {
      fetchMedicines();
    });

    return { rows, columns, submitRow, submitAllRows, medicines, updateTimesFields, deleteMedicine, submittedRows };
  }
};
</script>

<style scoped>
.table-container {
  margin-top: 50px;
  border: 1px solid #ddd;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 0 20px rgba(0,0,0,0.1);
  animation: fadeIn 0.5s ease;
}

.q-table th, .q-table td {

  text-align: center;
  vertical-align: middle;
}

.q-table th {
  background-color: #f1f1f1;
}

.q-input, .q-select {
  width: 100%;
}

.q-btn {
  width: 100%;
}

.medicine-table .q-td {
  padding: 8px;
}

.medicine-table .q-input,
.medicine-table .q-select,
.medicine-table .q-btn {
  margin-bottom: 8px;
}

.q-item-type {
  border-bottom: 1px solid #eee;
}

.q-item-type:last-child {
  border-bottom: none;
}

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
