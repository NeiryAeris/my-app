// React Native equivalent of the provided HTML + JS

import React, { useEffect, useState } from 'react';
import { View, Text, StyleSheet, ScrollView, Switch } from 'react-native';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, update } from 'firebase/database';
import { ProgressBar } from 'react-native-paper';
import { LineChart } from 'react-native-chart-kit';
import { Dimensions } from 'react-native';

// Firebase Configuration
const firebaseConfig = {
  apiKey: "AIzaSyD1Xm7iTsxbGVTCm2-rRl7Y_F5ztj2ZfZ0",
  authDomain: "btl-iot-2edd3.firebaseapp.com",
  databaseURL: "https://btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "btl-iot-2edd3",
  storageBucket: "btl-iot-2edd3.firebasestorage.app",
  messagingSenderId: "185251155252",
  appId: "1:185251155252:web:f053e03ecd34fd66946961",
};

const safeNumber = (value, defaultValue = 0) => (typeof value === 'number' ? value : defaultValue);

const App = () => {
  const [data, setData] = useState({});
  const [dhtValHistory, setDhtValHistory] = useState([]);

  useEffect(() => {
    // Initialize Firebase
    const app = initializeApp(firebaseConfig);
    const db = getDatabase(app);
    const dbRef = ref(db, '/');

    // Listen for changes in the database
    onValue(dbRef, (snapshot) => {
      const fetchedData = snapshot.val() || {};
      setData(fetchedData);

      // Update DHT history for the graph
      if (fetchedData?.dhtVal) {
        setDhtValHistory((prev) => [
          ...prev.slice(-19),
          { humid: safeNumber(fetchedData.dhtVal.Humid), temp: safeNumber(fetchedData.dhtVal.Temp) },
        ]);
      }
    });
  }, []);

  const handleControlToggle = (device, field, value) => {
    const db = getDatabase();
    const path = `/${device}`;
    update(ref(db, path), { [field]: value });
  };

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <Text style={styles.header}>Device Control</Text>

      {/* Air Quality Display */}
      <View style={styles.card}>
        <Text style={styles.title}>Air Quality</Text>
        <Text style={styles.value}>{safeNumber(data.airQuality)} / 100</Text>
        <ProgressBar progress={safeNumber(data.airQuality) / 100} color="#4CAF50" style={styles.progressBar} />
      </View>

      {/* Water Level Display */}
      <View style={styles.card}>
        <Text style={styles.title}>Water Level</Text>
        <Text style={styles.value}>{safeNumber(data.waterLevel)} / 100</Text>
        <ProgressBar progress={safeNumber(data.waterLevel) / 100} color="#2196F3" style={styles.progressBar} />
      </View>

      {/* Light Level Display */}
      <View style={styles.card}>
        <Text style={styles.title}>Light Level</Text>
        <Text style={styles.value}>{safeNumber(data.lightLevel)} / 100</Text>
        <ProgressBar progress={safeNumber(data.lightLevel) / 100} color="#FFC107" style={styles.progressBar} />
      </View>

      {/* DHT Values Trend Graph */}
      <View style={styles.card}>
        <Text style={styles.title}>Temperature & Humidity Trends</Text>
        <LineChart
          data={{
            labels: Array.from({ length: dhtValHistory.length }, (_, i) => `${i + 1}`),
            datasets: [
              {
                data: dhtValHistory.map((entry) => safeNumber(entry.temp)),
                color: () => '#FF5722', // Temperature line color
                label: 'Temp (°C)',
              },
              {
                data: dhtValHistory.map((entry) => safeNumber(entry.humid)),
                color: () => '#03A9F4', // Humidity line color
                label: 'Humid (%)',
              },
            ],
          }}
          width={Dimensions.get('window').width - 40}
          height={220}
          yAxisSuffix="%"
          chartConfig={{
            backgroundColor: '#f5f5f5',
            backgroundGradientFrom: '#ffffff',
            backgroundGradientTo: '#eeeeee',
            decimalPlaces: 1,
            color: (opacity = 1) => `rgba(0, 0, 0, ${opacity})`,
            labelColor: (opacity = 1) => `rgba(0, 0, 0, ${opacity})`,
          }}
          bezier
          style={{ marginVertical: 10 }}
        />
      </View>

      {/* Device Control Section */}
      {Object.keys(data).map((device) => (
        data[device]?.manualControl !== undefined && data[device]?.status !== undefined && (
          <View key={device} style={styles.card}>
            <Text style={styles.title}>{device.toUpperCase()}</Text>
            <View style={styles.row}>
              <Text style={styles.label}>Manual Control:</Text>
              <Switch
                value={data[device].manualControl}
                onValueChange={(value) => handleControlToggle(device, 'manualControl', value)}
              />
            </View>
            {data[device].manualControl && (
              <View style={styles.row}>
                <Text style={styles.label}>Device Status:</Text>
                <Switch
                  value={data[device].status === 'ON'}
                  onValueChange={(value) => handleControlToggle(device, 'status', value ? 'ON' : 'OFF')}
                />
              </View>
            )}
          </View>
        )
      ))}
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    padding: 20,
    backgroundColor: '#f5f5f5',
  },
  header: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
    textAlign: 'center',
  },
  card: {
    backgroundColor: '#fff',
    padding: 15,
    marginBottom: 15,
    borderRadius: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 2,
  },
  title: {
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 10,
  },
  row: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 10,
  },
  label: {
    fontSize: 16,
    color: '#555',
  },
  value: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 10,
  },
  progressBar: {
    height: 10,
    borderRadius: 5,
  },
});

export default App;
