//Arduino
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ==========================
// CONFIGURATION SECTION
// ==========================

// --- Pin Assignments ---
const int dhtPin = 50;
const int airQualityPin = A15;
const int mistPin = 8;
const int fanPin = 9;
const int peltierPin = 11;
const int peltierFanPin = 12;
const int inletServoPin = 27;
const int outletServoPin = 26;
const int buzzerPin = 31;

// --- Threshold Values ---
const int airQualityThreshold = 1000;
const float tempThreshold = 15.0;
const float humidityThresholdHigh = 90.0;
const float humidityMistOn = 80.0;
const float humidityMistOff = 90.0;

// --- Servo Positions ---
const int inletClosedPos = 160;
const int outletClosedPos = 75;
const int inletOpenPos = 120;
const int outletOpenPos = 125;

// ==========================
// COMPONENT INITIALIZATION
// ==========================

#define DHTTYPE DHT22
DHT dht(dhtPin, DHTTYPE);
Servo inletServo;
Servo outletServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change to 0x3F if needed

bool ventsOpen = false;  // vent state

// --- Peltier Timer Variables ---
bool peltierCycling = false;
bool peltierState = false; // false = OFF, true = ON
unsigned long peltierTimer = 0;
const unsigned long peltierInterval = 5UL * 60UL * 1000UL; // 5 minutes in milliseconds

// ==========================
// SETUP
// ==========================

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); // UART for ESP8266 communication

  pinMode(mistPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(peltierPin, OUTPUT);
  pinMode(peltierFanPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  inletServo.attach(inletServoPin);
  outletServo.attach(outletServoPin);

  // Initial States
  digitalWrite(mistPin, HIGH);         // Mist initially ON (HIGH = OFF)
  digitalWrite(fanPin, HIGH);          // Fan OFF
  digitalWrite(peltierPin, HIGH);      // Peltier OFF
  digitalWrite(peltierFanPin, HIGH);   // Peltier fan OFF
  digitalWrite(buzzerPin, HIGH);       // Buzzer OFF
  inletServo.write(inletClosedPos);
  outletServo.write(outletClosedPos);
}

// ==========================
// MAIN LOOP
// ==========================

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int airQuality = analogRead(airQualityPin);

  // --- LCD Display ---
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print(" H:");
  lcd.print(humidity);

  lcd.setCursor(0, 1);
  lcd.print("AQ:");
  lcd.print(airQuality);
  lcd.print("       "); // Clear residual chars

  // --- Mist Logic ---
  if (humidity < humidityMistOn) {
    digitalWrite(mistPin, LOW);  // Turn ON
  } else if (humidity >= humidityMistOff) {
    digitalWrite(mistPin, HIGH); // Turn OFF
  }

  // --- Vent + Fan + Buzzer Logic ---
  if (airQuality > airQualityThreshold) {
    if (!ventsOpen) {
      inletServo.write(inletOpenPos);
      outletServo.write(outletOpenPos);
      delay(500);
      ventsOpen = true;
    }
    digitalWrite(fanPin, LOW);       // Fan ON
    digitalWrite(buzzerPin, LOW);    // Buzzer ON
  } else {
    digitalWrite(fanPin, HIGH);      // Fan OFF
    digitalWrite(buzzerPin, HIGH);   // Buzzer OFF
    if (ventsOpen) {
      inletServo.write(inletClosedPos);
      outletServo.write(outletClosedPos);
      ventsOpen = false;
    }
  }

  // --- Peltier Logic with 5-min ON/OFF cycle ---
  bool peltierCondition = (temperature > tempThreshold || humidity > humidityThresholdHigh);

  if (peltierCondition) {
    if (!peltierCycling) {
      // Start cycle
      peltierCycling = true;
      peltierState = true;
      peltierTimer = millis();
      digitalWrite(peltierPin, LOW);
      digitalWrite(peltierFanPin, LOW);
    } else {
      // Continue cycling
      if (millis() - peltierTimer >= peltierInterval) {
        peltierState = !peltierState; // Toggle state
        peltierTimer = millis();      // Reset timer
        if (peltierState) {
          digitalWrite(peltierPin, LOW);      // ON
          digitalWrite(peltierFanPin, LOW);   // ON
        } else {
          digitalWrite(peltierPin, HIGH);     // OFF
          digitalWrite(peltierFanPin, HIGH);  // OFF
        }
      }
    }
  } else {
    // Stop cycle
    peltierCycling = false;
    peltierState = false;
    digitalWrite(peltierPin, HIGH);
    digitalWrite(peltierFanPin, HIGH);
  }

  // ==========================
  // ESP8266 Data Transmission
  // ==========================
  bool fanState = (digitalRead(fanPin) == LOW);           // Fan ON = LOW
  bool peltierActive = (digitalRead(peltierPin) == LOW);  // Peltier ON = LOW

  Serial1.print("DATA,");
  Serial1.print(temperature);
  Serial1.print(",");
  Serial1.print(humidity);
  Serial1.print(",");
  Serial1.print(airQuality);
  Serial1.print(",");
  Serial1.print(peltierActive);
  Serial1.print(",");
  Serial1.println(fanState);

  // ==========================
  // ESP8266 Command Reception
  // ==========================
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    if (cmd.startsWith("CMD")) {
      int peltierCmd = cmd.charAt(4) - '0';  // CMD,P,F
      int fanCmd = cmd.charAt(6) - '0';

      digitalWrite(peltierPin, peltierCmd ? LOW : HIGH);
      digitalWrite(peltierFanPin, peltierCmd ? LOW : HIGH);
      digitalWrite(fanPin, fanCmd ? LOW : HIGH);
    }
  }

  delay(2000); // 2 seconds loop delay
}
