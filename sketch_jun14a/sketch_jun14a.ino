#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>

// Wifi Credentials
const char* ssid = "P M Jayawickrama";
const char* password = "20020706";

// call Me Bot API Information
// +international_country_code + phone number
// Portugal +351, example: +351912345678
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

  // Set the buzzer pin as an output
  pinMode(buzzerPin, OUTPUT);

  // Join the I2C bus as master
  Wire.begin(); 

  // Initialize the keypad       
  keypad.begin();      // Initialize the keypad

  // Initialize serial communication
  Serial.begin(115200); 
  // Initialize the LCD
  lcd.init();        
  lcd.backlight();     
  lcd.clear();     

  // Initializing Wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  sendMessage("Hello from ESP32!"); 


}

void loop() {
  char key = keypad.getKey();

  if (key) {

    buffer[bufferIndex] = key;
    bufferIndex++;

    Serial.println(key); // for debugging purposes

    // Dispensing Via keypad 

    // Selecting which medicine number via 1st index of the buffer
    if (bufferIndex == 1) {
      int medicineIndex = buffer[0] - '0'; // If need to acces through array just addd -1
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Med ");
      lcd.print(medicines[medicineIndex].name);
      lcd.print(" Quantity");
    }

    // Selecting Quantity and Dispencing them
    if (bufferIndex == 3 && key == '#') {
      int medicineIndex = buffer[0] - '0'; // If need to acces through array just addd -1
      int quantity = buffer[1] - '0';

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing ");
      lcd.print(quantity);
      lcd.print(" of ");
      lcd.print(medicines[medicineIndex].name);
      //rotateMotor(medicineIndex, quantity);
      buzz(quantity);

      // Sending Whatsapp Notofication
      String message = "Dispensing " + String(quantity) + " of " + String(medicines[medicineIndex].name);
      sendMessage(message);

      // Clear the buffer
      bufferIndex = 0;
      memset(buffer, 0, bufferSize);
    } else if (key == '#') {
      lcd.setCursor(0, 0);
      lcd.print("Need to Enter Quantity");
      delay(2000);


      bufferIndex = 0;
      memset(buffer, 0, bufferSize);
    }
  }
}

void buzz(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
    delay(500); // Buzz for 1000 milliseconds (1 second)
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
    delay(500);
  }
  
}

// Call Me Bot Whats Message Sending FUnction
void sendMessage(String message){
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    


  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}
