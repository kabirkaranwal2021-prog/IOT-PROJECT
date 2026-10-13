#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// =====================================================
// PIN DEFINITIONS
// =====================================================

// EXIT SENSOR
// TRIG -> D2
// ECHO -> D3
const byte EXIT_TRIG = 2;
const byte EXIT_ECHO = 3;

// SERVO
// Signal -> D4
const byte SERVO_PIN = 4;

// ENTRY SENSOR
// TRIG -> D5
// ECHO -> D6
const byte ENTRY_TRIG = 5;
const byte ENTRY_ECHO = 6;

// BUZZER
const byte BUZZER_PIN = 7;


// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// =====================================================
// SERVO
// =====================================================

Servo barrierServo;

const int CLOSED_ANGLE = 0;
const int OPEN_ANGLE = 90;


// =====================================================
// PARKING
// =====================================================

const int TOTAL_SLOTS = 4;

int availableSlots = TOTAL_SLOTS;


// =====================================================
// SENSOR DISTANCE SETTINGS
// =====================================================

// Object detected at 17 cm or less
const int DETECTION_DISTANCE = 17;

// Object must move to 25 cm or more
// before the barrier closes
const int CLEAR_DISTANCE = 25;


// =====================================================
// GET DISTANCE
// =====================================================

long getDistance(byte trigPin, byte echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long duration =
    pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) {
    return 999;
  }

  long distance = duration * 0.0343 / 2;

  return distance;
}


// =====================================================
// BUZZER
// =====================================================

void beep() {

  tone(BUZZER_PIN, 1000);

  delay(150);

  noTone(BUZZER_PIN);
}


// =====================================================
// OPEN BARRIER SLOWLY
// =====================================================

void openBarrier() {

  Serial.println("Opening barrier...");

  for (int angle = CLOSED_ANGLE;
       angle <= OPEN_ANGLE;
       angle++) {

    barrierServo.write(angle);

    delay(15);
  }

  Serial.println("Barrier OPEN");
}


// =====================================================
// CLOSE BARRIER SLOWLY
// =====================================================

void closeBarrier() {

  Serial.println("Closing barrier...");

  for (int angle = OPEN_ANGLE;
       angle >= CLOSED_ANGLE;
       angle--) {

    barrierServo.write(angle);

    delay(15);
  }

  Serial.println("Barrier CLOSED");
}


// =====================================================
// WAIT UNTIL VEHICLE MOVES AWAY
// =====================================================

void waitUntilVehicleLeaves(byte trigPin, byte echoPin) {

  Serial.println("Waiting for vehicle to move away...");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("BARRIER OPEN");

  lcd.setCursor(0, 1);
  lcd.print("MOVE VEHICLE");

  bool vehicleWasDetected = false;

  while (true) {

    long distance =
      getDistance(trigPin, echoPin);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");


    // -----------------------------------------------
    // VEHICLE STILL CLOSE
    // -----------------------------------------------

    if (distance <= DETECTION_DISTANCE) {

      vehicleWasDetected = true;

      Serial.println("Vehicle/object detected");
    }


    // -----------------------------------------------
    // VEHICLE MOVED AWAY
    // -----------------------------------------------

    if (vehicleWasDetected &&
        distance >= CLEAR_DISTANCE) {

      Serial.println("Vehicle moved away");

      // Wait before confirmation
      delay(700);

      long confirmDistance =
        getDistance(trigPin, echoPin);

      // Confirm that object is still away
      if (confirmDistance >= CLEAR_DISTANCE) {

        Serial.println("Path clear");

        break;
      }
    }

    delay(200);
  }
}


// =====================================================
// LCD NORMAL STATUS
// =====================================================

void showStatus() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SMART PARKING");

  lcd.setCursor(0, 1);
  lcd.print("SLOTS: ");

  lcd.print(availableSlots);

  lcd.print("/");

  lcd.print(TOTAL_SLOTS);
}


// =====================================================
// LCD ENTRY MESSAGE
// =====================================================

void showEntry() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CAR ENTERING");

  lcd.setCursor(0, 1);
  lcd.print("SLOTS LEFT: ");

  lcd.print(availableSlots);
}


// =====================================================
// LCD EXIT MESSAGE
// =====================================================

void showExit() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CAR EXITING");

  lcd.setCursor(0, 1);
  lcd.print("SLOTS NOW: ");

  lcd.print(availableSlots);
}


// =====================================================
// LCD PARKING FULL
// =====================================================

void showFull() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PARKING FULL!");

  lcd.setCursor(0, 1);
  lcd.print("NO SLOT AVAILABLE");
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);


  // ---------------------------------------------------
  // EXIT SENSOR
  // ---------------------------------------------------

  pinMode(EXIT_TRIG, OUTPUT);
  pinMode(EXIT_ECHO, INPUT);


  // ---------------------------------------------------
  // ENTRY SENSOR
  // ---------------------------------------------------

  pinMode(ENTRY_TRIG, OUTPUT);
  pinMode(ENTRY_ECHO, INPUT);


  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------

  pinMode(BUZZER_PIN, OUTPUT);


  // ---------------------------------------------------
  // SERVO
  // ---------------------------------------------------

  barrierServo.attach(SERVO_PIN);

  barrierServo.write(CLOSED_ANGLE);


  // ---------------------------------------------------
  // LCD
  // ---------------------------------------------------

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SMART PARKING");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM READY");


  // ---------------------------------------------------
  // SERIAL INFORMATION
  // ---------------------------------------------------

  Serial.println();
  Serial.println("==============================");
  Serial.println("     SMART PARKING SYSTEM");
  Serial.println("==============================");

  Serial.println("D2/D3 = EXIT SENSOR");
  Serial.println("D5/D6 = ENTRY SENSOR");
  Serial.println("D4    = SERVO");
  Serial.println("D7    = BUZZER");
  Serial.println("LCD   = 0x27");

  Serial.print("TOTAL SLOTS = ");
  Serial.println(TOTAL_SLOTS);

  Serial.print("AVAILABLE = ");
  Serial.println(availableSlots);

  Serial.println("DETECTION = 17 cm");
  Serial.println("CLEAR = 25 cm");

  Serial.println("==============================");


  delay(1500);

  showStatus();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {


  // ===================================================
  // READ EXIT SENSOR
  // D2/D3
  // ===================================================

  long exitDistance =
    getDistance(EXIT_TRIG, EXIT_ECHO);

  delay(50);


  // ===================================================
  // READ ENTRY SENSOR
  // D5/D6
  // ===================================================

  long entryDistance =
    getDistance(ENTRY_TRIG, ENTRY_ECHO);


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.print("EXIT D2/D3 = ");

  Serial.print(exitDistance);

  Serial.print(" cm | ENTRY D5/D6 = ");

  Serial.print(entryDistance);

  Serial.print(" cm | SLOTS = ");

  Serial.println(availableSlots);


  // ===================================================
  // ENTRY
  // D5/D6
  //
  // CAR ENTERS
  // SLOTS DECREASE
  // ===================================================

  if (entryDistance <= DETECTION_DISTANCE) {

    Serial.println();
    Serial.println(">>> ENTRY SENSOR DETECTED");
    Serial.println(">>> CAR ENTERING");


    // -------------------------------------------------
    // CHECK AVAILABLE SLOTS
    // -------------------------------------------------

    if (availableSlots > 0) {

      // Car enters
      availableSlots--;

      Serial.println("ENTRY APPROVED");

      Serial.print("SLOTS LEFT = ");
      Serial.println(availableSlots);


      // LCD
      showEntry();


      // Buzzer
      beep();


      // Open barrier
      openBarrier();


      // Wait until car moves away
      waitUntilVehicleLeaves(
        ENTRY_TRIG,
        ENTRY_ECHO
      );


      // Close barrier
      closeBarrier();


      // Show current slots
      showStatus();


      delay(500);
    }


    else {

      // ------------------------------------------------
      // PARKING FULL
      // ------------------------------------------------

      Serial.println("ENTRY DENIED");

      Serial.println("PARKING FULL");


      showFull();


      // Two warning beeps
      beep();

      delay(250);

      beep();


      delay(1500);

      showFull();
    }


    return;
  }


  // ===================================================
  // EXIT
  // D2/D3
  //
  // CAR LEAVES
  // SLOTS INCREASE
  // ===================================================

  if (exitDistance <= DETECTION_DISTANCE) {

    Serial.println();
    Serial.println(">>> EXIT SENSOR DETECTED");
    Serial.println(">>> CAR EXITING");


    // -------------------------------------------------
    // CHECK THAT PARKING IS NOT EMPTY
    // -------------------------------------------------

    if (availableSlots < TOTAL_SLOTS) {

      // Car leaves
      availableSlots++;


      Serial.println("EXIT APPROVED");

      Serial.print("SLOTS NOW = ");

      Serial.println(availableSlots);


      // LCD
      showExit();


      // Buzzer
      beep();


      // Open barrier
      openBarrier();


      // Wait until car moves away
      waitUntilVehicleLeaves(
        EXIT_TRIG,
        EXIT_ECHO
      );


      // Close barrier
      closeBarrier();


      // Show current slots
      showStatus();


      delay(500);
    }


    else {

      // ------------------------------------------------
      // PARKING EMPTY
      // ------------------------------------------------

      Serial.println("PARKING ALREADY EMPTY");


      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("PARKING EMPTY");

      lcd.setCursor(0, 1);
      lcd.print("NO CAR INSIDE");


      delay(1000);


      showStatus();
    }


    return;
  }


  // ===================================================
  // NORMAL DISPLAY
  // ===================================================

  lcd.setCursor(0, 0);

  lcd.print("SMART PARKING ");


  lcd.setCursor(0, 1);

  lcd.print("SLOTS: ");

  lcd.print(availableSlots);

  lcd.print("/");

  lcd.print(TOTAL_SLOTS);

  lcd.print("        ");


  delay(100);
} 