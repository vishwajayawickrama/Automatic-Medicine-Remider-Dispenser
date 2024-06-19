#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>

// Wifi Credentials
const char* ssid = "Vishwa's Galaxy";
const char* password = "vishwacat";

// call Me Bot API Information
String phoneNumber = "+94774730705";
String apiKey = "8375006";


// Buzzer pin 
const int buzzerPin = 15;

// Keypad setup
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; // Connect to the column pinouts of the keypad

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, 0x20); // Replace 0x20 with your keypad's I2C address

// LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // Replace 0x27 with your LCD's I2C address

// Buffer setup
const int bufferSize = 10; // Maximum number of keys to store
char buffer[bufferSize]; // Buffer to store pressed keys
int bufferIndex = 0; // Index to track position in the buffer

// Medicine setup 
struct Time {
  char time[4]; // Changes this to 24 hour format --  Eg: "0830"; -- changed on 2024/06/16  
  int quantity;
};

struct Medicine {
  int compartmentNumber;
  char name[20];
  struct Time times[3];
};

Medicine medicines[4] = {
  {1, "Aspirin", {{8, 30, 1}, {14, 0, 1}, {20, 30, 1}}},
  {2, "Paracetamol", {{9, 0, 2}, {15, 0, 2}, {21, 0, 2}}},
  {3, "Ibuprofen", {{10, 30, 1}, {16, 30, 1}, {22, 30, 1}}},
  {4, "Amoxicillin", {{8, 0, 1}, {14, 0, 1}, {20, 0, 1}}}
};

void setup() {
  setupBuzzer();
  setupI2C();
  setupKeypad();
  setupSerial();
  setupLCD();
  connectToWiFi();
  sendMessage("Hello from ESP32!"); 
}

void loop() {
  handleKeypadInput();
}

void setupBuzzer() {
  pinMode(buzzerPin, OUTPUT);
}

void setupI2C() {
  Wire.begin();
}

void setupKeypad() {
  keypad.begin();
}

void setupSerial() {
  Serial.begin(115200);
}

void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void handleKeypadInput() {
  char key = keypad.getKey();

  if (key) {
    buffer[bufferIndex] = key;
    bufferIndex++;
    Serial.println(key); // for debugging purposes

    if (bufferIndex == 1) {
      int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Medicine ");
      lcd.print(medicines[medicineIndex].name);
      lcd.print(" Quantity");
    }

    if (bufferIndex == 3 && key == '#') {
      int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
      int quantity = buffer[1] - '0';

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing ");
      lcd.print(quantity);
      lcd.print(" of ");
      lcd.print(medicines[medicineIndex].name);
      // rotateMotor(medicineIndex, quantity);
      buzz(quantity); // TODO: Needs to add this inside of the rotate Motor Function so when it will buzzer after each Pill dispense.

      String message = "Dispensing " + String(quantity) + " of " + String(medicines[medicineIndex].name);
      sendMessage(message);

      clearBuffer();
    } else if (key == '#') {
      lcd.setCursor(0, 0);
      lcd.print("Need to Enter Quantity");
      delay(2000);
      clearBuffer();
    }
  }
}

void clearBuffer() {
  bufferIndex = 0;
  memset(buffer, 0, bufferSize);
}

void buzz(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
    delay(500); // Buzz for 500 milliseconds (0.5 second)
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
    delay(500);
  }
}

