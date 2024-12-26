#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <NewPing.h>

// WiFi Credentials
#define WIFI_SSID "Herae"
#define WIFI_PASSWORD "khongcopass"

// Firebase Credentials
#define FIREBASE_HOST "btl-iot-2edd3-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "H8o2JMEHXyVhfUFtMu9oWXFkZJXT8Yu75uc1cnpm"

#define LED_WIFI 2
#define LED_PIN 13
#define PUMP_PIN 14
#define CONTACT_PIN 33
#define SERVO_DOOR_PIN 25
#define TRIG_PIN 26
#define ECHO_PIN 27

#define MAX_DISTANCE 20 // Maximum distance we want to ping for (in centimeters).

Servo doorServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, 20); // Max distance 20 cm

FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

bool ledManualControl = false;
bool pumpManualControl = false;
bool fanManualControl = false;
bool doorManualControl = false;

int doorStat = 0; // 0: closed, 1: open prevent unwanted actions when the door is already in the desired state
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
        Serial.println("Servo at 180 degrees");
        delay(175);
        doorServo.detach();
        doorStat = 0;
    }
}

void Door_Control()
{
    if (Firebase.getJSON(firebaseData, "/door"))
    {
        if (firebaseData.dataType() == "json")
        {
            FirebaseJson &json = firebaseData.jsonObject();
            FirebaseJsonData jsonData;

            json.get(jsonData, "/manualControl");
            if (jsonData.type == "boolean")
            {
                doorManualControl = jsonData.boolValue;
            }

            String doorStatus = "CLOSED";
            json.get(jsonData, "/status");
            if (jsonData.type == "string")
            {
                doorStatus = jsonData.stringValue;
            }

            if (doorManualControl)
            {
                if (doorStatus == "ON")
                {
                    OpenServo();
                }
                else
                {
                    CloseServo();
                }

                Serial.print("Manual Door Control - Status: ");
                Serial.println(doorStatus);
            }
            else
            {

                int contactState = digitalRead(CONTACT_PIN);
                if (contactState == HIGH)
                {
                    OpenServo();
                    Serial.println("Automatic Door Control - Door opened due to contact sensor");
                }
                else
                {
                    CloseServo();
                    Serial.println("Automatic Door Control - Door closed due to contact sensor");
                }
                delay(2000);
            }
        }
    }
    else
    {
        Serial.println("Failed to read door status from Firebase");
    }
}

void Pump_Control()
{
    if (Firebase.getJSON(firebaseData, "/pump"))
    {
        if (firebaseData.dataType() == "json")
        {
            FirebaseJson &json = firebaseData.jsonObject();
            FirebaseJsonData jsonData;

            json.get(jsonData, "/manualControl");
            if (jsonData.type == "boolean")
            {
                pumpManualControl = jsonData.boolValue;
            }

            String pumpStatus = "OFF";
            json.get(jsonData, "/status");
            if (jsonData.type == "string")
            {
                pumpStatus = jsonData.stringValue;
            }

            if (pumpManualControl)
            {
                if (pumpStatus == "ON")
                {
                    digitalWrite(PUMP_PIN, HIGH);
                    Serial.println("Manual Pump Control - Turned ON");
                }
                else
                {
                    digitalWrite(PUMP_PIN, LOW);
                    Serial.println("Manual Pump Control - Turned OFF");
                }
            }
            else
            {
                int waterLevel = sonar.ping_cm();
                Firebase.setInt(firebaseData, "/waterLevel", waterLevel);
                Serial.print("Water level: ");
                Serial.print(waterLevel);
                Serial.println();
                if (waterLevel > 3 && waterLevel < 20)
                {
                    digitalWrite(PUMP_PIN, HIGH);
                    Serial.println("Automatic Pump Control - Turned ON due to low water level");
                }
                else
                {
                    digitalWrite(PUMP_PIN, LOW);
                    Serial.println("Automatic Pump Control - Turned OFF due to sufficient water level");
                }
            }
        }
    }
    else
    {
        Serial.println("Failed to read pump status from Firebase");
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
}

void loop()
{
    Door_Control();
    Pump_Control();
    delay(1000);
}