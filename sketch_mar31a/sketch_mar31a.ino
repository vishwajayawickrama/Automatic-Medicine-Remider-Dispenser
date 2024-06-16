#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <string.h>
#include <Keypad.h>

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
  char hours[2];
  char minutes[2];
  int quantity;
};

struct Medicine {
  int number;
  char name[20];
  struct Time times[3];
};


Medicine medicines[1]; // Example: array of 3 medicines

void setup(){

  // Step and Dir pins in motor 1
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  medicines[0].number = 1;
  strcpy(medicines[0].name, "Aspirin");
  strcpy(medicines[0].times[0].hours, "08");
  strcpy(medicines[0].times[0].minutes, "00");
  medicines[0].times[0].quantity = 2;
  strcpy(medicines[0].times[1].hours, "12");
  strcpy(medicines[0].times[1].minutes, "00");
  medicines[0].times[1].quantity = 2;
  strcpy(medicines[0].times[2].hours, "16");
  strcpy(medicines[0].times[2].minutes, "00");
  medicines[0].times[2].quantity = 2;
  
  Serial.begin(115200);
  // initialize LCD
  Wire.begin();
  rtc.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
}

void loop(){

  DateTime now = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  printDigits(now.hour());
  lcd.print(':');
  printDigits(now.minute());
  lcd.print(':');
  printDigits(now.second());
  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(now.year(), DEC);
  lcd.print('/');
  printDigits(now.month());
  lcd.print('/');
  printDigits(now.day());

  lcd.setCursor(0,2);
  lcd.print(medicines[0].name);
  lcd.setCursor(0, 3);
  lcd.print(now.hour());
  delay(1000); // Delay for 1 second


  // Checking for Times and Dispensing Sensing Notification adn alerting via Buzzer
  if ((now.hour() == medicines[0].times[0].hour && now.minutes() == medicines[0].times[0].minutes)) {
    
    rotateMotor(1, medicine[0].times[0].quantity);
    // Needs to send Notofication to the phoneS
    // Call the buzz function
    buzz();
  } else if ((now.hour() == medicines[0].times[1].hour && now.minutes() == medicines[0].times[1].minutes)) {
    rotateMotor(1, medicine[0].times[1].quantity);
    // Needs to send Notofication to the phoneS
    // Call the buzz function
    buzz();
  } else if ((now.hour() == medicines[0].times[2].hour && now.minutes() == medicines[0].times[2].minutes)) {
    rotateMotor(1, medicine[0].times[2].quantity);
    // Needs to send Notofication to the phoneS
    // Call the buzz function
    buzz();
  }

  // Dispensing via Keypad
  char key = keypad.getKey();

  if (key != NO_KEY) {
    // Store the pressed key in the buffer
    buffer[bufferIndex] = key;
    bufferIndex++;
    // Check if the buffer is full or if 'Enter' key is pressed
    if (bufferIndex == 1 || key == '#') {
      int medicineno = buffer[0] - '0';
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter ");
      lcd.print(medicines[medicineno]);
      lcd.print(" quantity: ");
    
      buffer[bufferIndex] = key;
      bufferIndex++;

      if (bufferIndex == 3 || key == '#') {
        int medicineIndex = buffer[0] - '0';
        int quantity = buffer[2] - '0';

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dispensing ");
        lcd.print(quantity);
        lcd.print(" ");
        lcd.print(medicines[medicineIndex]);
        lcd.print("s");
        rotateMotor(medicineIndex, quantity);
        buzz();
        
        // Clear the buffer
        bufferIndex = 0;
        memset(buffer, 0, bufferSize);
      }
    }
  }
}

void printDigits(int digits) {
  // Add leading 0 if the number is less than 10
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

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


void buzz() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
  delay(1000); // Buzz for 100 milliseconds (adjust as needed)
  digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
}



