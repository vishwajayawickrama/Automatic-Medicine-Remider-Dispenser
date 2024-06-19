#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <ArduinoJson.h>


// Wifi Credentials
const char* ssid = "Vishwa's Galaxy";
const char* password = "vishwacat";

// call Me Bot API Information
String phoneNumber = "+94774730705";
String apiKey = "8375006";

// Firebase project credentials
const char* firestoreProjectID = "medisync-b6adf";
const char* firestoreAPIKey = "AIzaSyAsIe_Ih-Qf7iBXj3zd8qbqHjB3okDtDqA";
String firestoreURL = "https://firestore.googleapis.com/v1/projects/" + String(firestoreProjectID) + "/databases/(default)/documents";

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
  char time[5]; // Changed to 24-hour format -- Eg: "0830"
};

struct Medicine {
  int compartmentNumber;
  char name[20];
  int dose;
  int times_per_day;
  Time times[10]; // Fixed size for simplicity, adjust as needed
};

Medicine medicines[4];

void setup() {
  setupBuzzer();
  setupI2C();
  setupKeypad();
  setupSerial();
  setupLCD();
  connectToWiFi();
  sendMessage("Hello from ESP32!"); 

  fetchMedicine();
  printMedicine();
  displayMedicineNamesOnLCD();
}

void loop() {
  handleKeypadInput();
}

// Function to set up the buzzer pin as an output
void setupBuzzer() {
  pinMode(buzzerPin, OUTPUT);
}

// Function to activate the buzzer for a specified number of times
void buzz(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
    delay(500); // Buzz for 500 milliseconds (0.5 second)
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
    delay(500);
  }
}

// Function to initialize the I2C bus
void setupI2C() {
  Wire.begin();
}

// Function to initialize the keypad
void setupKeypad() {
  keypad.begin();
}

// Function to handle keypad input
void handleKeypadInput() {
  char key = keypad.getKey();

  if (key) {
    buffer[bufferIndex] = key;
    bufferIndex++;
    Serial.println(key); // for debugging purposes

    if (bufferIndex == 1 && isdigit(buffer[0])) {
      int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Medicine ");
      lcd.print(medicines[medicineIndex].name);
      lcd.print(" Quantity");
    }else if (bufferIndex == 1 && buffer[0] == 'A') {
      fetchMedicine();
      printMedicine();
      Serial.println("Medicine Data has been updated");
      clearBuffer();
      displayMedicineNamesOnLCD();
    }

    if (bufferIndex == 3 && key == '#') {
      int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
      int quantity = buffer[1] - '0';

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing ");
      lcd.print(quantity);
      lcd.print(" ");
      lcd.print(medicines[medicineIndex].name);
      lcd.print("s.");
      // rotateMotor(medicineIndex, quantity);
      buzz(quantity); // TODO: Needs to add this inside of the rotate Motor Function so when it will buzzer after each Pill dispense.

      String message = "Dispensing " + String(quantity) + " " + String(medicines[medicineIndex].name) + "s.";
      sendMessage(message);

      clearBuffer();
      delay(2000);
      displayMedicineNamesOnLCD();

    } else if (key == '#') {
      lcd.setCursor(0, 0);
      lcd.print("Need to Enter Quantity");
      delay(2000);
      clearBuffer();
      displayMedicineNamesOnLCD();
    }
  }
}

// Function to clear the input buffer
void clearBuffer() {
  bufferIndex = 0;
  memset(buffer, 0, bufferSize);
}

// Function to initialize serial communication
void setupSerial() {
  Serial.begin(115200);
}

// Function to initialize the LCD
void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// Function to connect to the WiFi network
void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function to send a message via the Call Me Bot API
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


// Function to get data from Firestore and store in the medicine array
void getDocument(const String &collection, const String &document, Medicine &medicine) {
  HTTPClient http;
  String url = firestoreURL + "/" + collection + "/" + document + "?key=" + firestoreAPIKey;
  Serial.println("Requesting URL: " + url);

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Document retrieved successfully");
    Serial.println(response);

    // Parsing JSON response
    DynamicJsonDocument doc(4096);  // Adjust size based on your document
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.f_str());
      return;
    }

    // Print the entire JSON document
    Serial.println("Parsed JSON document:");
    serializeJson(doc, Serial);
    Serial.println();

    // Extracting and assigning data to the Medicine struct
    strlcpy(medicine.name, doc["fields"]["medicine_name"]["stringValue"] | "", sizeof(medicine.name));
    medicine.compartmentNumber = doc["fields"]["compartment_number"]["integerValue"].as<int>() | 0;
    medicine.times_per_day = doc["fields"]["times_per_day"]["integerValue"].as<int>() | 0;
    medicine.dose = doc["fields"]["dose_per_time"]["integerValue"].as<int>() | 0;
    
    JsonArray timesArray = doc["fields"]["times"]["arrayValue"]["values"].as<JsonArray>();
    for (size_t i = 0; i < timesArray.size() && i < medicine.times_per_day; i++) {
      strlcpy(medicine.times[i].time, timesArray[i]["stringValue"] | "", sizeof(medicine.times[i].time));
    }

  } else {
    Serial.print("Error retrieving document: ");
    Serial.println(httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}

// Get documents from Firestore and save in the medicines array
void fetchMedicine() {
  getDocument("medicines", "1", medicines[0]);
  getDocument("medicines", "2", medicines[1]);
  getDocument("medicines", "3", medicines[2]);
  getDocument("medicines", "4", medicines[3]);
}

// Function to print the medicine data to the Serial Monitor
void printMedicine() {
  for (int i = 0; i < 4; i++) {
    Serial.print("Medicine ");
    Serial.println(i + 1);
    Serial.print("Name: ");
    Serial.println(medicines[i].name);
    Serial.print("Compartment Number: ");
    Serial.println(medicines[i].compartmentNumber);
    Serial.print("Dose: ");
    Serial.println(medicines[i].dose);
    Serial.print("Times Per day: ");
    Serial.println(medicines[i].times_per_day);
    for (int j = 0; j < medicines[i].times_per_day; j++) {
      Serial.print("Time ");
      Serial.print(j + 1);
      Serial.print(": ");
      Serial.println(medicines[i].times[j].time);
    }
    Serial.println();
  }
}

// Function to print the medicine names data to the LCD Display
void displayMedicineNamesOnLCD() {
  lcd.clear(); // Clear the display

  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i); // Set the cursor to column 0, row i
    lcd.print(i + 1);
    lcd.print(". ");
    lcd.print(medicines[i].name);
  }
}

