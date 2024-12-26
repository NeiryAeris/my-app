// React Native equivalent of the provided HTML + JS

import React, { useEffect, useState } from 'react';
import { View, Text, StyleSheet, ScrollView, Switch } from 'react-native';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, update } from 'firebase/database';
import CircularProgress from 'react-native-circular-progress-indicator';

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

const interpretAirQualityColor = (sensorValue) => {
  if (sensorValue < 50) {
    return "#00FF00"; // Bright Green (Excellent)
  } else if (sensorValue < 100) {
    return "#32CD32"; // Green (Good)
  } else if (sensorValue < 150) {
    return "#FFFF00"; // Yellow (Moderate)
  } else if (sensorValue < 200) {
    return "#FFA500"; // Orange (Poor)
  } else {
    return "#FF0000"; // Red (Dangerous)
  }
};

const interpretAirQualityText = (sensorValue) => {
  if (sensorValue < 50) {
    return " - Xuất sắc";
  } else if (sensorValue < 100) {
    return " - Tốt";
  } else if (sensorValue < 150) {
    return " - Trung bình";
  } else if (sensorValue < 200) {
    return " - Kém";
  } else {
    return " - Nguy hiểm";
  }
};

const interpretWaterLevelText = (waterLevel) => {
  if (waterLevel < 20) {
    return " - Rất thấp";
  } else if (waterLevel < 50) {
    return " - Thấp";
  } else {
    return " - Bình thường";
  }
};

const interpretLightLevelColor = (lightValue) => {
  if (lightValue < 40) {
    return "#000000"; // Black (Dark)
  } else if (lightValue < 800) {
    return "#808080"; // Grey (Dim)
  } else if (lightValue < 2000) {
    return "#f2f25e"; // Light Yellow (Light)
  } else if (lightValue < 3200) {
    return "#FFFF00"; // Yellow (Bright)
  } else {
    return "#FFA500"; // Sunny Orange (Very Bright)
  }
};

const interpretLightLevelText = (lightValue) => {
  if (lightValue < 40) {
    return " - Tối";
  } else if (lightValue < 800) {
    return " - Mờ";
  } else if (lightValue < 2000) {
    return " - Sáng";
  } else if (lightValue < 3200) {
    return " - Rất sáng";
  } else {
    return " - Quá sáng";
  }
};

const App = () => {
  const [data, setData] = useState({});

  useEffect(() => {
    // Initialize Firebase
    const app = initializeApp(firebaseConfig);
    const db = getDatabase(app);
    const dbRef = ref(db, '/');

    // Listen for changes in the database
    onValue(dbRef, (snapshot) => {
      const fetchedData = snapshot.val() || {};
      setData(fetchedData);
    });
  }, []);

  const handleControlToggle = (device, field, value) => {
    const db = getDatabase();
    const path = `/${device}`;
    update(ref(db, path), { [field]: value });
  };

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <Text style={styles.header}>Điều khiển thiết bị</Text>

      <View style={styles.gaugeGrid}>
        <View style={styles.gaugeBlock}>
          <Text style={[styles.gaugeLabel, { color: interpretAirQualityColor(safeNumber(data.airQuality)) }]}>Chất lượng không khí{interpretAirQualityText(safeNumber(data.airQuality))}</Text>
          <CircularProgress
            value={safeNumber(data.airQuality)}
            maxValue={100}
            radius={50}
            textColor="#000"
            activeStrokeColor={interpretAirQualityColor(safeNumber(data.airQuality))}
            inActiveStrokeColor="#E0E0E0"
          />
        </View>
        <View style={styles.gaugeBlock}>
          <Text style={[styles.gaugeLabel, { color: "#2196F3" }]}>Mực nước (%) {interpretWaterLevelText(safeNumber(data.waterLevel))}</Text>
          <CircularProgress
            value={(safeNumber(data.waterLevel) / 25) * 100}
            maxValue={100}
            radius={50}
            textColor="#000"
            activeStrokeColor="#2196F3"
            inActiveStrokeColor="#E0E0E0"
          />
        </View>
        <View style={styles.gaugeBlock}>
          <Text style={[styles.gaugeLabel, { color: interpretLightLevelColor(safeNumber(data.lightLevel)) }]}>
            Mức sáng (%){interpretLightLevelText(safeNumber(data.lightLevel))}
          </Text>
          <CircularProgress
            value={(Math.min(safeNumber(data.lightLevel), 3200)/3200)*100} // Cap at 3200
            maxValue={100}
            radius={50}
            textColor="#000"
            activeStrokeColor={interpretLightLevelColor(safeNumber(data.lightLevel))}
            inActiveStrokeColor="#E0E0E0"
          />
        </View>

        <View style={styles.gaugeBlock}>
          <Text style={[styles.gaugeLabel, { color: "#FF5722" }]}>Nhiệt độ (°C)</Text>
          <CircularProgress
            value={parseFloat(safeNumber(data?.dhtVal?.Temp).toFixed(1))}
            maxValue={50}
            radius={50}
            textColor="#000"
            activeStrokeColor="#FF5722"
            inActiveStrokeColor="#E0E0E0"
          />
        </View>
        <View style={styles.gaugeBlock}>
          <Text style={[styles.gaugeLabel, { color: "#03A9F4" }]}>Độ ẩm (%)</Text>
          <CircularProgress
            value={safeNumber(data?.dhtVal?.Humid)}
            maxValue={100}
            radius={50}
            textColor="#000"
            activeStrokeColor="#03A9F4"
            inActiveStrokeColor="#E0E0E0"
          />
        </View>
      </View>

      {Object.keys(data).map((device) => (
        data[device]?.manualControl !== undefined && data[device]?.status !== undefined && (
          <View key={device} style={styles.controlCard}>
            <Text style={styles.title}>{device.toUpperCase()}</Text>
            <View style={styles.row}>
              <Text style={styles.label}>Điều khiển tay:</Text>
              <Switch
                value={data[device].manualControl}
                onValueChange={(value) => handleControlToggle(device, 'manualControl', value)}
              />
            </View>
            <View style={styles.row}>
              <Text style={styles.label}>Trạng thái hoạt động:</Text>
              {data[device].manualControl ? (
                <Switch
                  value={data[device].status === 'ON'}
                  onValueChange={(value) => handleControlToggle(device, 'status', value ? 'ON' : 'OFF')}
                />
              ) : (
                <Text style={styles.status}>{data[device].status}</Text>
              )}
            </View>
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
    alignItems: 'center',
  },
  header: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
    textAlign: 'center',
  },
  gaugeGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'center',
    marginBottom: 20,
  },
  gaugeBlock: {
    width: 400,
    marginBottom: 20,
    alignItems: 'center',
  },
  gaugeLabel: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 10,
    textAlign: 'center',
  },
  controlCard: {
    backgroundColor: '#fff',
    padding: 15,
    marginBottom: 15,
    borderRadius: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 2,
    width: 400,
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
  title: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 10,
    textAlign: 'center',
  },
  status: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#000',
  },
});

export default App;
