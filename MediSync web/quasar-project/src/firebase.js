// src/firebase.js
import { initializeApp } from 'firebase/app';
import { getAuth } from 'firebase/auth';
import { getFirestore } from 'firebase/firestore';
import { getStorage } from 'firebase/storage';

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyAsIe_Ih-Qf7iBXj3zd8qbqHjB3okDtDqA",
  authDomain: "medisync-b6adf.firebaseapp.com",
  projectId: "medisync-b6adf",
  storageBucket: "medisync-b6adf.appspot.com",
  messagingSenderId: "463996390848",
  appId: "1:463996390848:web:b84b332291a412b5ca0c25",
  measurementId: "G-M8HQ8XL0ZC"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

// Initialize Firebase services
const auth = getAuth(app);
const firestore = getFirestore(app);
const storage = getStorage(app);

export { auth, firestore, storage };
