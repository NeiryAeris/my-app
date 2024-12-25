// React Native equivalent of the provided HTML + JS

import React, { useEffect, useState } from 'react';
import { View, Text, StyleSheet, ScrollView, Button } from 'react-native';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, set } from 'firebase/database';

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyD1Xm7iTsxbGVTCm2-rRl7Y_F5ztj2ZfZ0",
  authDomain: "btl-iot-2edd3.firebaseapp.com",
  databaseURL: "https://btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "btl-iot-2edd3",
  storageBucket: "btl-iot-2edd3.firebasestorage.app",
  messagingSenderId: "185251155252",
  appId: "1:185251155252:web:f053e03ecd34fd66946961"
};

const App = () => {
  const [data, setData] = useState({});

  useEffect(() => {
    // Initialize Firebase
    const app = initializeApp(firebaseConfig);
    const db = getDatabase(app);
    const dbRef = ref(db, '/');

    // Listen to database changes
    onValue(dbRef, (snapshot) => {
      const fetchedData = snapshot.val();
      setData(fetchedData);
    });
  }, []);

  const togglePumpStatus = () => {
    if (data.pump) {
      const newStatus = data.pump.status === "ON" ? "OFF" : "ON";
      const db = getDatabase();
      const pumpRef = ref(db, '/pump');
      set(pumpRef, { status: newStatus });
    }
  };

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <Text style={styles.header}>Real-time Data</Text>
      <View style={styles.card}>
        <Text style={styles.label}>Air Quality:</Text>
        <Text style={styles.value}>{data.airQuality || 'Loading...'}</Text>
      </View>
      <View style={styles.card}>
        <Text style={styles.label}>Pump Status:</Text>
        <Text style={styles.value}>{data.pump?.status || 'Loading...'}</Text>
        <Button
          title={data.pump?.status === "ON" ? "Turn Off" : "Turn On"}
          onPress={togglePumpStatus}
          disabled={!data.pump}
        />
      </View>
      <View style={styles.card}>
        <Text style={styles.label}>Ventilation Status:</Text>
        <Text style={styles.value}>{data.ventilation?.status || 'Loading...'}</Text>
      </View>
      <View style={styles.card}>
        <Text style={styles.label}>Water Level:</Text>
        <Text style={styles.value}>{data.waterLevel || 'Loading...'}</Text>
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#f5f5f5',
    padding: 20,
  },
  header: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
  },
  card: {
    width: '100%',
    backgroundColor: '#ffffff',
    padding: 15,
    marginBottom: 10,
    borderRadius: 8,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  label: {
    fontSize: 18,
    color: '#555',
  },
  value: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
  },
});

export default App;
