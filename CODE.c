#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Pin configuration
const byte ENTRY_TRIG = 2;
const byte ENTRY_ECHO = 3;
const byte SERVO_PIN = 4;
const byte EXIT_TRIG = 5;
const byte EXIT_ECHO = 6;
const byte BUZZER_PIN = 7;

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gate;

// Parking configuration
const int TOTAL_SLOTS = 4;
int occupiedCars = 0;
int slotsLeft = TOTAL_SLOTS;

// Distance thresholds
const float DETECTION_DISTANCE = 15.8;
const float OBJECT_AWAY_DISTANCE = 18.0;

// Gate clearance times
const unsigned long ENTRY_CLEAR_TIME = 4000;
const unsigned long EXIT_CLEAR_TIME = 2000;

// Get ultrasonic distance in centimetres
float getDistance(byte trigPin, byte echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration =
      pulseIn(echoPin, HIGH, 25000UL);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}

// Display a message on the LCD
void showMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// Update available parking slots
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slots Left: ");
  lcd.print(slotsLeft);

  lcd.setCursor(0, 1);

  if (slotsLeft == 0) {
    lcd.print("PARKING FULL");
  } else {
    lcd.print("System Ready");
  }
}

// Entry buzzer
void entryBeep() {
  tone(BUZZER_PIN, 700, 100);
  delay(120);
  tone(BUZZER_PIN, 1100, 120);
  delay(130);
  noTone(BUZZER_PIN);
}

// Exit buzzer
void exitBeep() {
  tone(BUZZER_PIN, 1500, 100);
  delay(120);
  tone(BUZZER_PIN, 700, 120);
  delay(130);
  noTone(BUZZER_PIN);
}

// Warning buzzer
void warningBeep() {
  tone(BUZZER_PIN, 500, 250);
  delay(250);
  noTone(BUZZER_PIN);
}

// Open and close gate
void openGate() {
  gate.write(90);
  Serial.println("GATE OPEN");
}

void closeGate() {
  gate.write(0);
  Serial.println("GATE CLOSED");
}

// Wait until the sensor reports a clear path
// continuously for the required duration.
void waitUntilObjectAway(
    byte trigPin,
    byte echoPin,
    unsigned long requiredTime) {

  unsigned long awayStart = 0;

  while (true) {
    float distance = getDistance(trigPin, echoPin);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance >= OBJECT_AWAY_DISTANCE) {
      if (awayStart == 0) {
        awayStart = millis();
      }

      if (millis() - awayStart >= requiredTime) {
        return;
      }
    } else {
      awayStart = 0;
    }

    delay(50);
  }
}

// Handle vehicle entry
void handleEntry() {
  if (slotsLeft <= 0) {
    showMessage("PARKING FULL", "Entry denied");
    warningBeep();
    delay(500);
    updateLCD();
    return;
  }

  entryBeep();
  showMessage("ENTRY DETECTED", "Gate opening");
  openGate();

  waitUntilObjectAway(
      ENTRY_TRIG, ENTRY_ECHO, ENTRY_CLEAR_TIME);

  closeGate();

  occupiedCars++;
  slotsLeft = TOTAL_SLOTS - occupiedCars;

  showMessage("ENTRY COMPLETE", "Vehicle counted");
  delay(500);
  updateLCD();
}

// Handle vehicle exit
void handleExit() {
  if (occupiedCars <= 0) {
    showMessage("PARKING EMPTY", "No car to exit");
    warningBeep();
    delay(500);
    updateLCD();
    return;
  }

  exitBeep();
  showMessage("EXIT DETECTED", "Gate opening");
  openGate();

  waitUntilObjectAway(
      EXIT_TRIG, EXIT_ECHO, EXIT_CLEAR_TIME);

  closeGate();

  occupiedCars--;
  slotsLeft = TOTAL_SLOTS - occupiedCars;

  showMessage("EXIT COMPLETE", "Vehicle counted");
  delay(500);
  updateLCD();
}

// Check for a vehicle
bool vehicleDetected(byte trigPin, byte echoPin) {
  float distance = getDistance(trigPin, echoPin);

  Serial.print("Detection: ");
  Serial.print(distance);
  Serial.println(" cm");

  return distance > 0 &&
         distance <= DETECTION_DISTANCE;
}

// Arduino initialization
void setup() {
  Serial.begin(9600);

  pinMode(ENTRY_TRIG, OUTPUT);
  pinMode(ENTRY_ECHO, INPUT);

  pinMode(EXIT_TRIG, OUTPUT);
  pinMode(EXIT_ECHO, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(ENTRY_TRIG, LOW);
  digitalWrite(EXIT_TRIG, LOW);

  gate.attach(SERVO_PIN);
  gate.write(0);

  lcd.init();
  lcd.backlight();

  showMessage("SMART PARKING", "System Ready");
  delay(1000);
  updateLCD();

  Serial.println("Smart Parking System Ready");
}

// Main Arduino loop
void loop() {
  if (vehicleDetected(ENTRY_TRIG, ENTRY_ECHO)) {
    handleEntry();
    return;
  }

  if (vehicleDetected(EXIT_TRIG, EXIT_ECHO)) {
    handleExit();
    return;
  }

  delay(50);
}
