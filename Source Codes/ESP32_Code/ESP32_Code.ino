// ESP32
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>
#include "thingProperties.h"

// --- Pin Mapping ---
#define DHTPIN 18
#define DHTTYPE DHT22
#define AQ_PIN 34
#define MIST_PIN 19
#define FAN_PIN 23
#define PELTIER_PIN 25
#define PELTIER_FAN_PIN 26
#define INLET_SERVO_PIN 13
#define OUTLET_SERVO_PIN 12
#define BUZZER_PIN 27

// --- Servo positions ---
const int inletClosed = 160;
const int outletClosed = 75;
const int inletOpen = 120;
const int outletOpen = 125;

// --- Peltier cycle variables ---
bool peltierCycling = false;
bool peltierState = false;
unsigned long peltierTimer = 0;
const unsigned long peltierInterval = 5UL * 60UL * 1000UL; // 5 minutes

// --- States ---
bool ventsOpen = false;

// --- Components ---
DHT dht(DHTPIN, DHTTYPE);
Servo inletServo;
Servo outletServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD I2C Address

void setup() {
  Serial.begin(115200);
  delay(1500);

  // Initialize Arduino Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Pin setup
  pinMode(MIST_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PELTIER_PIN, OUTPUT);
  pinMode(PELTIER_FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Attach servos
  inletServo.attach(INLET_SERVO_PIN);
  outletServo.attach(OUTLET_SERVO_PIN);
  inletServo.write(inletClosed);
  outletServo.write(outletClosed);

  // LCD
  lcd.begin(16, 2);
  lcd.backlight();

  // DHT
  dht.begin();

  // Initial OFF states
  digitalWrite(MIST_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(PELTIER_PIN, HIGH);
  digitalWrite(PELTIER_FAN_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
}

void loop() {
  ArduinoCloud.update();

  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  airQuality = analogRead(AQ_PIN);

  // Update LCD
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print(" H:");
  lcd.print(humidity);

  lcd.setCursor(0, 1);
  lcd.print("AQ:");
  lcd.print(airQuality);
  lcd.print("       ");

  // --- Mist Logic ---
  if (humidity < 80) {
    digitalWrite(MIST_PIN, LOW);
    mistOn = true;
  } else if (humidity >= 90) {
    digitalWrite(MIST_PIN, HIGH);
    mistOn = false;
  }

  // --- Vent, Fan, Buzzer Logic ---
  if (airQuality > 1000) {
    if (!ventsOpen) {
      inletServo.write(inletOpen);
      outletServo.write(outletOpen);
      delay(500);
      ventsOpen = true;
    }
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    fanRunning = true;
    alarmOn = true;
  } else {
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    fanRunning = false;
    alarmOn = false;
    if (ventsOpen) {
      inletServo.write(inletClosed);
      outletServo.write(outletClosed);
      ventsOpen = false;
    }
  }

  // --- Peltier 5-min Cycle Logic ---
  bool peltierCondition = (temperature > 15.0 || humidity > 90.0);
  if (peltierCondition) {
    if (!peltierCycling) {
      peltierCycling = true;
      peltierState = true;
      peltierTimer = millis();
      digitalWrite(PELTIER_PIN, LOW);
      digitalWrite(PELTIER_FAN_PIN, LOW);
      peltierRunning = true;
    } else {
      if (millis() - peltierTimer >= peltierInterval) {
        peltierState = !peltierState;
        peltierTimer = millis();
        if (peltierState) {
          digitalWrite(PELTIER_PIN, LOW);
          digitalWrite(PELTIER_FAN_PIN, LOW);
          peltierRunning = true;
        } else {
          digitalWrite(PELTIER_PIN, HIGH);
          digitalWrite(PELTIER_FAN_PIN, HIGH);
          peltierRunning = false;
        }
      }
    }
  } else {
    peltierCycling = false;
    peltierState = false;
    digitalWrite(PELTIER_PIN, HIGH);
    digitalWrite(PELTIER_FAN_PIN, HIGH);
    peltierRunning = false;
  }

  delay(2000); // Slow down loop
}
