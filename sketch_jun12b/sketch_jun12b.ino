#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <string.h>
#include <Keypad.h>
#include <Keypad_I2C.h>

#define STEP_PIN 2
#define DIR_PIN 5
#define BUZZER_PIN 12 // Change this to the appropriate pin number

// Keypad
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {0, 1, 2, 3}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; // Connect to the column pinouts of the keypad

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, 0x20);

const int bufferSize = 10; // Maximum number of keys to store
char buffer[bufferSize]; // Buffer to store pressed keys
int bufferIndex = 0; // Index to track position in the buffer

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;

RTC_DS3231 rtc;
// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

struct Time {
  int hours;
  int minutes;
  int quantity;
};

struct Medicine {
  int number;
  char name[20];
  struct Time times[3];
};

Medicine medicines[1]; // Example: array of 1 medicine

void setup() {
  // Step and Dir pins in motor 1
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // TODO: Needs to get relevant medicine information from Firebase 
  medicines[0].number = 1;
  strcpy(medicines[0].name, "Aspirin");
  medicines[0].times[0].hours = 8;
  medicines[0].times[0].minutes = 0;
  medicines[0].times[0].quantity = 2;
  medicines[0].times[1].hours = 12;
  medicines[0].times[1].minutes = 0;
  medicines[0].times[1].quantity = 2;
  medicines[0].times[2].hours = 16;
  medicines[0].times[2].minutes = 0;
  medicines[0].times[2].quantity = 2;

  Serial.begin(115200);
  // Initialize I2C and RTC
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.init();
  // Turn on LCD backlight
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(medicines[0].name);
}

void loop() {
  DateTime now = rtc.now();
 
  // Checking for Times and Dispensing Sensing Notification and alerting via Buzzer
  for (int i = 0; i < 3; i++) {
    if (now.hour() == medicines[0].times[i].hours && now.minute() == medicines[0].times[i].minutes) {
      rotateMotor(1, medicines[0].times[i].quantity);
      // Needs to send Notification to the phone
      // TODO: Need to make function with relevant library with API 
      // Call the buzz function
      buzz();
    }
  }

  // Dispensing via Keypad
  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
    buffer[bufferIndex] = key;
    bufferIndex++;

    if (bufferIndex == 1 || key == '#') {
      int medicineIndex = buffer[0] - '0' - 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter ");
      lcd.print(medicines[medicineIndex].name);
      lcd.print(" quantity: ");
    }

    if (bufferIndex == 3 || key == '#') {
      int medicineIndex = buffer[0] - '0' - 1;
      int quantity = buffer[2] - '0';

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing ");
      lcd.print(quantity);
      lcd.print(" ");
      lcd.print(medicines[medicineIndex].name);
      rotateMotor(medicineIndex, quantity);
      buzz();

      // Clear the buffer
      bufferIndex = 0;
      memset(buffer, 0, bufferSize);
    }
  }
}

void printDigits(int digits) {
  // Add leading 0 if the number is less than 10
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

// Motor Rotating Function also Calls in relevant time
void rotateMotor(int motorNumber, int times) {
  // Calculate the number of steps required for a 15-degree rotation
  int stepsPer15Degrees = 200 * 15 / 360; // Assuming 200 steps per revolution

  // Set the direction based on motor number
  int dir = (motorNumber == 1) ? HIGH : LOW; // Change direction if necessary

  // Rotate the motor the specified number of times
  for (int i = 0; i < times; i++) {
    // Set the direction
    digitalWrite(DIR_PIN, dir);

    // Generate pulses to step the motor
    for (int j = 0; j < stepsPer15Degrees; j++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(500); // Adjust the delay as needed
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(500); // Adjust the delay as needed
    }
  }
}

// Buzz Function calls in relevant time
void buzz() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
  delay(1000); // Buzz for 1000 milliseconds (1 second)
  digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
}





