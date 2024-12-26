#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

// WiFi Credentials
#define WIFI_SSID "Herae"
#define WIFI_PASSWORD "khongcopass"

// Firebase Credentials
#define FIREBASE_HOST "btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "H8o2JMEHXyVhfUFtMu9oWXFkZJXT8Yu75uc1cnpm"

// Sensor and Device Pins
#define LED_WIFI 2
#define DHTPIN 4
#define LED_PIN 13
#define FAN_PIN 12
#define LDR_PIN_ANALOG 34
#define MQ135_PIN 35

#define IN2 22
#define IN1 23
#define ENA 21
#define SPEED 50

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

bool ledManualControl = false;
bool fanManualControl = false;

void LED_Control()
{
    if (Firebase.getJSON(firebaseData, "/LED"))
    {
        if (firebaseData.dataType() == "json")
        {
            FirebaseJson &json = firebaseData.jsonObject();
            FirebaseJsonData jsonData;

            json.get(jsonData, "/manualControl");
            if (jsonData.type == "boolean")
            {
                ledManualControl = jsonData.boolValue;
            }

            String ledStatus = "OFF";
            json.get(jsonData, "/status");
            if (jsonData.type == "string")
            {
                ledStatus = jsonData.stringValue;
            }

            int lux = 4095 - analogRead(LDR_PIN_ANALOG);
            if (Firebase.setInt(firebaseData, "/ldrVal", lux))
            {
                Serial.print("LDR value sent to Firebase: ");
                Serial.println(lux);
            }
            else
            {
                Serial.println("Failed to send LDR value to Firebase");
            }

            if (ledManualControl)
            {
                if (ledStatus == "ON")
                {
                    digitalWrite(LED_PIN, HIGH);
                }
                else
                {
                    digitalWrite(LED_PIN, LOW);
                }

                Serial.print("Manual LED Control - Status: ");
                Serial.println(ledStatus);
            }
            else
            {

                if (lux < 800)
                {
                    digitalWrite(LED_PIN, HIGH);
                    Serial.println("Automatic LED Control - Turned ON due to low light");
                }
                else
                {
                    digitalWrite(LED_PIN, LOW);
                    Serial.println("Automatic LED Control - Turned OFF due to sufficient light");
                }
            }
        }
    }
    else
    {
        Serial.println("Failed to read LED status from Firebase");
    }
}


void setVentilationSpeed(String mode) {
    int speed = 0; // Default off

    if (mode == "ON") {
        speed = 78; // Normal speed
    } else if (mode == "ONHIGH") {
        speed = 178; // High speed
    } else if (mode == "OFF") {
        speed = 0; // Off
    } else {
        Serial.println("Invalid mode");
        return;
    }

    if (speed > 0) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, speed);
        Serial.print("Ventilation running at mode: ");
        Serial.println(mode);
    } else {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);
        Serial.println("Ventilation turned off");
    }
}

void Ventilation_Control() {
    if (Firebase.getJSON(firebaseData, "/ventilation")) {
        if (firebaseData.dataType() == "json") {
            FirebaseJson& json = firebaseData.jsonObject();
            FirebaseJsonData jsonData;

            json.get(jsonData, "/manualControl");
            if (jsonData.type == "boolean") {
                fanManualControl = jsonData.boolValue;
            }

            String fanMode = "OFF";
            json.get(jsonData, "/status");
            if (jsonData.type == "string") {
                fanMode = jsonData.stringValue;
            }

            if (fanManualControl) {
                setVentilationSpeed(fanMode);
                Serial.print("Manual Ventilation Control - Mode: ");
                Serial.println(fanMode);
            } else {
                int airQuality = analogRead(MQ135_PIN);
                if (airQuality > 200) {
                    setVentilationSpeed("ONHIGH");
                    Serial.println("Automatic Ventilation Control - High speed due to poor air quality");
                } else {
                    setVentilationSpeed("ON");
                    Serial.println("Automatic Ventilation Control - Normal speed");
                }
            }
        }
    } else {
        Serial.println("Failed to read ventilation status from Firebase");
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
    setup_wifi();

    firebaseConfig.host = FIREBASE_HOST;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&firebaseConfig, &firebaseAuth);

    pinMode(CONTACT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    dht.begin();
}

void loop()
{
    LED_Control();
    DHT_Control();
    Door_Control();
    Ventilation_Control();
    Pump_Control();
    delay(1000);
}