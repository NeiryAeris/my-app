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
#define LED_WIFI 2
#define LED_PIN 13
#define PUMP_PIN 14
#define FAN_PIN 12
#define SERVO_DOOR_PIN 25
#define TRIG_PIN 26
#define ECHO_PIN 27
#define DHTPIN 27
#define LDR_PIN_ANALOG 34
#define MQ135_PIN 35

#define IN2 22
#define IN1 23
#define ENA 21
#define SPEED 50

#define MAX_DISTANCE 20 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo doorServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, 20); // Max distance 400 cm

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
void LED_Control()
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
}

void DHT_Control()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity))
  {
    // Send DHT data to Firebase
    if (Firebase.setFloat(firebaseData, "/dhtVal/Temp", temperature))
    {
      Serial.print("Temperature sent to Firebase: ");
      Serial.println(temperature);
    }
    else
    {
      Serial.println("Failed to send temperature to Firebase");
    }

    if (Firebase.setFloat(firebaseData, "/dhtVal/Humid", humidity))
    {
      Serial.print("Humidity sent to Firebase: ");
      Serial.println(humidity);
    }
    else
    {
      Serial.println("Failed to send humidity to Firebase");
    }
  }
  else
  {
    Serial.println("Failed to read DHT sensor data");
  }
}

void Light_Control()
{
  int ldrValue = analogRead(LDR_PIN_ANALOG);
  if (Firebase.setInt(firebaseData, "/ldrVal", ldrValue))
  {
    Serial.print("LDR value sent to Firebase: ");
    Serial.println(ldrValue);
  }
  else
  {
    Serial.println("Failed to send LDR value to Firebase");
  }
}

void AIR_Control()
{
  int airQuality = analogRead(MQ135_PIN);
  if (Firebase.setInt(firebaseData, "/airQuality", airQuality))
  {
    Serial.print("Air quality sent to Firebase: ");
    Serial.println(airQuality);
  }
  else
  {
    Serial.println("Failed to send air quality to Firebase");
  }
  float normalizedAirQuality = (airQuality / 4095.0) * 100.0;

  Serial.print("Normalized Air Quality: ");
  Serial.print(normalizedAirQuality);
  Serial.println(" %");

  // Send normalized value to Firebase
  if (Firebase.setFloat(firebaseData, "/airQualityPercentage", normalizedAirQuality))
  {
    Serial.print("Normalized Air Quality sent to Firebase: ");
    Serial.println(normalizedAirQuality);
  }
  else
  {
    Serial.println("Failed to send normalized air quality to Firebase");
  }
}

void Water_Control()
{
  int measuredDistance = sonar.ping_cm(); // Distance in cm

  if (measuredDistance == 0)
  {
    Serial.println("Out of range or no echo received");
  }
  else
  {
    int waterLevel = MAX_DISTANCE - measuredDistance; // Calculate water level
    waterLevel = max(waterLevel, 0);                  // Ensure it doesn’t go below 0
    Serial.print("Water Level: ");
    Serial.print(waterLevel);
    Serial.println(" cm");

    // Example of sending data to Firebase
    if (Firebase.setInt(firebaseData, "/waterLevel", waterLevel))
    {
      Serial.print("Water Level sent to Firebase: ");
      Serial.println(waterLevel);
    }
    else
    {
      Serial.println("Failed to send water level to Firebase");
    }
  }
}

void loop()
{

  LED_Control();
  DHT_Control();
  Light_Control();
  AIR_Control();
  Water_Control();

  delay(2000); // Check status every 2 seconds
}