#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <NewPing.h>
#include <DHT.h>

// WiFi Credentials
#define WIFI_SSID "Herae"
#define WIFI_PASSWORD "khongcopass"

// Firebase Credentials
#define FIREBASE_HOST "btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "H8o2JMEHXyVhfUFtMu9oWXFkZJXT8Yu75uc1cnpm"

// Sensor and Device Pins
#define DHTPIN 27
#define DHTTYPE DHT11
#define LDR_PIN_ANALOG 34
#define TRIG_PIN 26
#define ECHO_PIN 27
#define SERVO_DOOR_PIN 25
#define LED_PIN 13
#define LED_WIFI 2
#define FAN_PIN 12
#define PUMP_PIN 14

#define ENA 21 // Motor A speed control
#define IN1 23 // Motor A direction
#define IN2 22
#define SPEED 50

DHT dht(DHTPIN, DHTTYPE);
Servo doorServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, 400); // Max distance 400 cm

FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

bool manualControl = false; // Global control flag
int doorStat = 0;           // 0: closed, 1: open prevent unwanted actions when the door is already in the desired state
int isOpen = 0;             // 0: closed, 1: open curtains

void OpenServo()
{
  if (doorStat == 0)
  {
    doorServo.attach(SERVO_DOOR_PIN);
    doorServo.write(0);
    Serial.println("Servo at 0 degrees");
    delay(175);
    doorServo.detach();
    doorStat = 1;
  }
}

void CloseServo()
{
  if (doorStat == 1)
  {
    doorServo.attach(SERVO_DOOR_PIN);
    doorServo.write(180);
    Serial.println("Servo at 0 degrees");
    delay(175);
    doorServo.detach();
    doorStat = 0;
  }
}

void moveCurtainsOpen()
{
  // Start both motors
  // Motor A
  if (isOpen == 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 50); // 50% speed
    delay(1000);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }
  isOpen = 1;
}
void moveCurtainsClose()
{
  // Stop both motors
  if (isOpen == 1)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 50); // 50% speed
    delay(1000);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }
  isOpen = 0;
}

void detatching()
{
  doorServo.detach();
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_WIFI, HIGH);
    delay(100);
    digitalWrite(LED_WIFI, LOW);
    delay(100);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
}

void setup()
{
  Serial.begin(115200);

  // Initialize WiFi
  setup_wifi();

  // Firebase setup
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
}
// void setup()
// {
//   Serial.begin(115200);

//   // Initialize WiFi
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(500);
//   }
//   Serial.println("WiFi Connected");

//   // Firebase setup
//   firebaseConfig.host = FIREBASE_HOST;
//   firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
//   Firebase.begin(&firebaseConfig, &firebaseAuth);

//   // Initialize sensors and actuators
//   dht.begin();
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(FAN_PIN, OUTPUT);
//   pinMode(PUMP_PIN, OUTPUT);
//   doorServo.attach(SERVO_DOOR_PIN);
// }

// void loop()
// {
//   // Read control data from Firebase
//   if (Firebase.getJSON(firebaseData, "/"))
//   {
//     if (firebaseData.dataType() == "json")
//     {
//       FirebaseJson &json = firebaseData.jsonObject();
//       FirebaseJsonData jsonData;

//       // Handle LED control
//       bool ledManual = false;
//       String ledStatus = "OFF";
//       json.get(jsonData, "/LED/manualControl");
//       if (jsonData.type == "boolean")
//       {
//         ledManual = jsonData.boolValue;
//       }
//       json.get(jsonData, "/LED/status");
//       if (jsonData.type == "string")
//       {
//         ledStatus = jsonData.stringValue;
//       }
//       if (!ledManual)
//       {
//         digitalWrite(LED_PIN, ledStatus == "ON" ? HIGH : LOW);
//       }

//       // Handle Door control
//       bool doorManual = false;
//       String doorStatus = "OFF";
//       json.get(jsonData, "/door/manualControl");
//       if (jsonData.type == "boolean")
//       {
//         doorManual = jsonData.boolValue;
//       }
//       json.get(jsonData, "/door/status");
//       if (jsonData.type == "string")
//       {
//         doorStatus = jsonData.stringValue;
//       }
//       if (!doorManual)
//       {
//         if (doorStatus == "ON")
//         {
//           doorServo.write(0); // Open door
//         }
//         else
//         {
//           doorServo.write(180); // Close door
//         }
//       }

//       // Handle Ventilation control
//       bool fanManual = false;
//       String fanStatus = "OFF";
//       json.get(jsonData, "/ventilation/manualControl");
//       if (jsonData.type == "boolean")
//       {
//         fanManual = jsonData.boolValue;
//       }
//       json.get(jsonData, "/ventilation/status");
//       if (jsonData.type == "string")
//       {
//         fanStatus = jsonData.stringValue;
//       }
//       if (!fanManual)
//       {
//         digitalWrite(FAN_PIN, fanStatus == "ON" ? HIGH : LOW);
//       }

//       // Handle Pump control
//       bool pumpManual = false;
//       String pumpStatus = "OFF";
//       json.get(jsonData, "/pump/manualControl");
//       if (jsonData.type == "boolean")
//       {
//         pumpManual = jsonData.boolValue;
//       }
//       json.get(jsonData, "/pump/status");
//       if (jsonData.type == "string")
//       {
//         pumpStatus = jsonData.stringValue;
//       }
//       if (!pumpManual)
//       {
//         digitalWrite(PUMP_PIN, pumpStatus == "ON" ? HIGH : LOW);
//       }
//     }
//   }
//   else
//   {
//     Serial.println("Failed to read Firebase data");
//   }

//   // Update sensor data to Firebase
//   float temperature = dht.readTemperature();
//   float humidity = dht.readHumidity();
//   int waterLevel = sonar.ping_cm();

//   if (!isnan(temperature) && !isnan(humidity))
//   {
//     if (!Firebase.setFloat(firebaseData, "/dhtVal/Temp", temperature))
//     {
//       Serial.println("Failed to send temperature data to Firebase");
//     }
//     if (!Firebase.setFloat(firebaseData, "/dhtVal/Humid", humidity))
//     {
//       Serial.println("Failed to send humidity data to Firebase");
//     }
//   }
//   else
//   {
//     Serial.println("Failed to read DHT sensor data");
//   }

//   if (!Firebase.setInt(firebaseData, "/waterLevel", waterLevel))
//   {
//     Serial.println("Failed to send water level data to Firebase");
//   }

//   Serial.print("Temperature: ");
//   Serial.println(temperature);
//   Serial.print("Humidity: ");
//   Serial.println(humidity);
//   Serial.print("Water Level: ");
//   Serial.println(waterLevel);

//   delay(2000); // Update every 2 seconds
// }

void loop()
{
  if (Firebase.getJSON(firebaseData, "/LED"))
  {
    if (firebaseData.dataType() == "json")
    {
      FirebaseJson &json = firebaseData.jsonObject();
      FirebaseJsonData jsonData;

      // Check LED status from Firebase
      String ledStatus = "OFF";
      json.get(jsonData, "/status");
      if (jsonData.type == "string")
      {
        ledStatus = jsonData.stringValue;
      }

      // Control LED based on Firebase status
      if (ledStatus == "ON")
      {
        digitalWrite(LED_PIN, HIGH);
      }
      else
      {
        digitalWrite(LED_PIN, LOW);
      }

      Serial.print("LED status from Firebase: ");
      Serial.println(ledStatus);
    }
  }
  else
  {
    Serial.println("Failed to read LED status from Firebase");
  }

  delay(2000); // Check status every 2 seconds
}
