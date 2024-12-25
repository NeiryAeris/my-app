#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NewPing.h>

// WiFi Kết nối wifi
//GalaxyA05s asdfghjkl
#define WIFI_SSID "BlueCandle"
#define WIFI_PASSWORD "blazingeyes"

// Thông tin Firebase 
#define FIREBASE_HOST "btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app" // 
#define FIREBASE_AUTH "H8o2JMEHXyVhfUFtMu9oWXFkZJXT8Yu75uc1cnpm"

FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

//Chân sensor MQ135
#define MQ135_PIN 34

// Chân cảm biến siêu âm
#define TRIG_PIN 26
#define ECHO_PIN 27
NewPing sonar(TRIG_PIN, ECHO_PIN, 20); // Maximum distance 20 cm

// Chân quatkj thông gió 
#define MOTOR_PIN 25

// Ngưỡng giá trị
const int AIR_QUALITY_THRESHOLD = 300; // Ngưỡng chất lượng không khí
const int WATER_LEVEL_THRESHOLD = 20; // Ngưỡng dưới bình chứa

void setup() {
  Serial.begin(115200);

  // Kết nối Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Kết nối vào firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, &firebaseAuth);

  
  pinMode(MQ135_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW); 
}

void loop() {
  // Đọc thông tin chất lượng không khí
  int airQuality = analogRead(MQ135_PIN);
  Serial.print("Air Quality: ");
  Serial.println(airQuality);

  // Đọc mực nước trong bể 
  int waterLevel = sonar.ping_cm();
  if (waterLevel == 0) {
    waterLevel = 20; // Quá tầm đọc của sensor đặt bằng min
  }
  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.println(" cm");

  // Kiểm soát quạt thông gió 
  if (airQuality > AIR_QUALITY_THRESHOLD) {
    digitalWrite(MOTOR_PIN, HIGH); // Bật quạt
    Firebase.setString(firebaseData, "/ventilation/status", "ON");
  } else {
    digitalWrite(MOTOR_PIN, LOW); // Tắt quạt
    Firebase.setString(firebaseData, "/ventilation/status", "OFF");
  }

  // Cập nhật giá trị 
  Firebase.setInt(firebaseData, "/airQuality", airQuality);
  Firebase.setInt(firebaseData, "/waterLevel", waterLevel);
  // Firebase.setInt(firebaseData, "/airQuality", 666);
  // Firebase.setInt(firebaseData, "/waterLevel", 131);
  // Update pump status based on water level
  if (waterLevel < WATER_LEVEL_THRESHOLD) {
    Firebase.setString(firebaseData, "/pump/status", "LOW");
  } else {
    Firebase.setString(firebaseData, "/pump/status", "NORMAL");
  }

  delay(2000); // Wait before the next loop
}
