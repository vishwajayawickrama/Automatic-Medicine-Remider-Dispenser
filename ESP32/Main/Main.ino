#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <ArduinoJson.h>
#include <RTClib.h>

// Motor Specifications
const int dirPins[] =  {4, 18, 13, 27};
const int stepPins[] = {5, 19, 14, 26};
const int motorsCount = 4;
const int stepsPerRevolution = 200;

// Wifi Credential
const char* ssid = "Laka's S21+";
const char* password ="llllllll";

// call Me Bot API Information
String phoneNumber = "+94774730705";
String apiKey = "8375006";

// Firebase project credentials
// const char* firestoreProjectID = "medisync-b6adf";
// const char* firestoreAPIKey = "AIzaSyAsIe_Ih-Qf7iBXj3zd8qbqHjB3okDtDqA";
// String firestoreURL = "https://firestore.googleapis.com/v1/projects/" + String(firestoreProjectID) + "/databases/(default)/documents";

// Buzzer pin 
const int buzzerPin = 15;

// Initialize the RTC 
RTC_DS3231 rtc;

DateTime lastCheckDate;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3};
byte colPins[COLS] = {4, 5, 6, 7};

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, 0x20);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Buffer setup
const int bufferSize = 10; // Maximum number of keys to store
char buffer[bufferSize]; // Buffer to store pressed keys
int bufferIndex = 0; // Index to track position in the buffer

// Medicine Setup
struct Time {
  char time[5];
  bool dispensed;
};

// Define your Medicine struct here
struct Medicine {
  int id;
  char name[20]; // Adjust size based on your expected data
  int compartmentNumber;
  int dose;
  int times_per_day;
  struct Time times[5]; // Adjust size based on your expected number of times
  int quantity;
};

Medicine medicines[4];


void setup(){
  initializeMotors();
  setupBuzzer();
  setupI2C();
  setupKeypad();
  setupSerial();
  setupLCD();
  displayWelcomMessage();
  connectToWiFi();
  sendMessage("  Welcome to MediSync!  ");
  initializeRTC();


  // Getting Medicine Data and storing them in array and Displaying them
  fetchMedicine();
  displayMedicineNamesOnLCD();
}

void loop() {
  handleKeypadInput();
  //displayTime();
  checkMedicineTimes();
}

// Displaying Welcome Message untill it retireves medicine data from Server // Sachini
void displayWelcomMessage() {
  lcd.clear();
  lcd.setCursor(3, 0); 
  lcd.print(" Welcome  to ");
  lcd.setCursor(4, 2);
  lcd.print(" Medysinc! ");
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

// Send Message to whatsapp number using CallMe Bot // Sachini
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

// Function to set up the buzzer pin as an output // Lakshan
void setupBuzzer() {
  pinMode(buzzerPin, OUTPUT);
}

// Function to initialize the I2C bus
void setupI2C() {
  Wire.begin();
}

// Function to initialize the LCD // Sachini 
void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// Function to initialize serial communication
void setupSerial() {
  Serial.begin(115200);
}

// Function to initialize the keypad // Lakshan
void setupKeypad() {
  keypad.begin();
}

// Function to handle keypad input // Lakshan
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
      //dispenseMedicine(medicineIndex, quantity);
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

// Function to clear the input buffer // Lakshan
void clearBuffer() {
  bufferIndex = 0;
  memset(buffer, 0, bufferSize);
}

// Function to activate the buzzer for a specified number of times  // Lakshan
void buzz(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
    delay(500); // Buzz for 500 milliseconds (0.5 second)
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
    delay(500);
  }
}

// Print Json Documents
void printJsonDocument(const DynamicJsonDocument& doc) {
  // Serialize the JSON document to a string
  String jsonString;
  serializeJsonPretty(doc, jsonString);

  // Print the serialized JSON to Serial Monitor
  Serial.println("Parsed JSON Document:");
  Serial.println(jsonString);
}

// Function to get data from Firestore and store in the medicine array // Lakshan
void getMedicineData(int medicineIndex) {
  HTTPClient http;

  // Specify the API endpoint for a specific medicine ID
  String url = "http://206.189.132.45:3000/medicine/" + String(medicineIndex + 1);
  Serial.println("Requesting URL: " + url);

  // Send GET request to the API endpoint
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response:");
    Serial.println(response);

    // Parsing JSON response
    DynamicJsonDocument doc(4096);  // Adjust size based on your JSON response size
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Print parsed JSON document
    printJsonDocument(doc);

    // Extracting and assigning data to the Medicine struct
    medicines[medicineIndex].id = medicineIndex;
    strlcpy(medicines[medicineIndex].name, doc["medicine_name"] | "", sizeof(medicines[medicineIndex].name));
    medicines[medicineIndex].compartmentNumber = doc["compartment_number"] | 0;
    medicines[medicineIndex].dose = doc["dose_per_time"] | 0;
    medicines[medicineIndex].times_per_day = doc["times_per_day"] | 0;
    medicines[medicineIndex].quantity = doc["quantity"] | 0;

    // Extracting times array
    JsonArray timesArray = doc["times"].as<JsonArray>();
    size_t numOfTimes = min(timesArray.size(), static_cast<size_t>(5)); // Limit to 5 times

    for (size_t i = 0; i < numOfTimes; i++) {
      strlcpy(medicines[medicineIndex].times[i].time, timesArray[i]["time"] | "", sizeof(medicines[medicineIndex].times[i].time));
    }

    // Print the extracted data for verification
    Serial.println("Parsed Medicine Data:");
    Serial.print("ID: ");
    Serial.println(medicines[medicineIndex].id);
    Serial.print("Name: ");
    Serial.println(medicines[medicineIndex].name);
    Serial.print("Compartment Number: ");
    Serial.println(medicines[medicineIndex].compartmentNumber);
    Serial.print("Dose: ");
    Serial.println(medicines[medicineIndex].dose);
    Serial.print("Times Per day: ");
    Serial.println(medicines[medicineIndex].times_per_day);
    Serial.print("Quantity: ");
    Serial.println(medicines[medicineIndex].quantity);

  } else {
    Serial.print("Error retrieving data from API: ");
    Serial.println(httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }

  http.end();  // Close connection
}

// Get documents from server and save in the medicines array // Lakshan
void fetchMedicine() {
  // Initialize and fetch data for each medicine in the array
  for (int i = 0; i < 4; ++i) { // Updated to 4 medicines
    getMedicineData(i); // Assuming medicine IDs start from 1
    delay(1000); // Delay between API requests (adjust as needed)
  }
}

// Update Medicine Data // Lakshan
void updateMedicineData(int medicineIndex) {
  HTTPClient http;

  // Specify the API endpoint for updating the medicine data
  String url = "http://206.189.132.45:3000/medicine/" + String(medicines[medicineIndex].id + 1); // Adjust endpoint as per your API
  Serial.println("Updating medicine data at URL: " + url);

  // Construct JSON payload for the PUT request
  DynamicJsonDocument jsonPayload(1024);
  jsonPayload["compartment_number"] = medicines[medicineIndex].compartmentNumber;
  jsonPayload["medicine_name"] = medicines[medicineIndex].name;
  jsonPayload["times_per_day"] = medicines[medicineIndex].times_per_day;
  JsonArray timesArray = jsonPayload.createNestedArray("times");
  for (int i = 0; i < medicines[medicineIndex].times_per_day; i++) {
    JsonObject timeObject = timesArray.createNestedObject();
    timeObject["time"] = medicines[medicineIndex].times[i].time;
  }
  jsonPayload["dose_per_time"] = medicines[medicineIndex].dose;
  jsonPayload["quantity"] = medicines[medicineIndex].quantity;

  // Serialize JSON payload to a string
  String jsonString;
  serializeJson(jsonPayload, jsonString);

  // Send PUT request to the API endpoint
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PUT(jsonString);
  Serial.println(jsonString);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Update Response:");
    Serial.println(response);
  } else {
    Serial.print("Error updating medicine data: ");
    Serial.println(httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }

  http.end();  // Close connection
  fetchMedicine();
}

// Function to print the medicine names data to the LCD Display //
void displayMedicineNamesOnLCD() {
  lcd.clear(); // Clear the display

  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i); // Set the cursor to column 0, row i
    lcd.print(i + 1);
    lcd.print(". ");
    lcd.print(medicines[i].name);
    lcd.setCursor(18, i);
    lcd.print(medicines[i].quantity);
  }
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
    Serial.print("Remaning Quantity: ");
    Serial.println(medicines[i].quantity);
    for (int j = 0; j < medicines[i].times_per_day; j++) {
      Serial.print("Time ");
      Serial.print(j + 1);
      Serial.print(": ");
      Serial.println(medicines[i].times[j].time);
    }
    Serial.println();
  }
}

// Function to initialize the RTC
void initializeRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // Set the time to a known value
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// To print time to serial montor
void displayTime() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  delay(1000);
}

// Function to check medicine times and alert if necessary
// void checkMedicineTimes() {
//   DateTime now = rtc.now();
//   char currentTime[5];
//   sprintf(currentTime, "%02d%02d", now.hour(), now.minute());

//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < medicines[i].times_per_day; j++) {
//       if (strcmp(currentTime, medicines[i].times[j].time) == 0) {
//         String message = "Take Your Medicine. " + String(medicines[i].dose) + " " + String(medicines[i].name) + "s.";
//         sendMessage(message);
//         buzz(medicines[i].dose); // Needs to change
//         dispenseMedicine(i, medicines[i].dose);
//         displayMedicineAlert(medicines[i].name, medicines[i].dose);
//         displayMedicineNamesOnLCD();

        
//       }
//     }
//   }
// }

void resetDispensedFlags() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < medicines[i].times_per_day; j++) {
      medicines[i].times[j].dispensed = false;
    }
  }
}

void checkMedicineTimes() {
  DateTime now = rtc.now();
  int currentTimeMinutes = now.hour() * 60 + now.minute();

  // Check if the date has changed and reset dispensed flags if it has
  if (now.day() != lastCheckDate.day() || now.month() != lastCheckDate.month() || now.year() != lastCheckDate.year()) {
    resetDispensedFlags();
    lastCheckDate = now;
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < medicines[i].times_per_day; j++) {
      int medicineTimeHour = atoi(medicines[i].times[j].time) / 100;
      int medicineTimeMinute = atoi(medicines[i].times[j].time) % 100;
      int medicineTimeMinutes = medicineTimeHour * 60 + medicineTimeMinute;

      // Check if the time is within ±5 minutes and if it hasn't been dispensed yet
      if (abs(currentTimeMinutes - medicineTimeMinutes) <= 5 && !medicines[i].times[j].dispensed) {
        String message = "Take Your Medicine. " + String(medicines[i].dose) + " " + String(medicines[i].name) + "s.";
        sendMessage(message);
        buzz(10); // Needs to change
        displayMedicineAlert(medicines[i].name, medicines[i].dose);
        dispenseMedicine(i, medicines[i].dose);

        // Mark this time slot as dispensed
        medicines[i].times[j].dispensed = true;
        displayMedicineNamesOnLCD();
      }
    }
  }
}


// Function to display the medicine alert on the LCD and Serial
void displayMedicineAlert(const char* name, int dose) {
  // Clear the LCD
  lcd.clear();

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Take Medicine:");
  lcd.setCursor(0, 1);
  lcd.print(name);
  lcd.setCursor(0, 2);
  lcd.print("Dose: ");
  lcd.print(dose);

  // Display on Serial Monitor
  Serial.println("Take Medicine:");
  Serial.println(name);
  Serial.print("Dose: ");
  Serial.println(dose);

  // Delay to keep the message displayed on LCD for 5 seconds
  delay(5000); // 5 seconds
}


// Medicine Dispencing System


// Function to initialize moto
void initializeMotors() {
  for (int i = 0; i < motorsCount; i++) {
    pinMode(stepPins[i], OUTPUT);
    pinMode(dirPins[i], OUTPUT);
    digitalWrite(dirPins[i], HIGH);  // Set initial direction to clockwise
  }
  //pinMode(enable, OUTPUT);
  //digitalWrite(enable, LOW); // Enable the motors (assuming active low)
}

// Function to dispense medicine
void dispenseMedicine(int medicineIndex, int quantity) {
  Medicine &medicine = medicines[medicineIndex];

  if (quantity > medicines[medicineIndex].quantity) {
    Serial.print("Insufficient quantity of ");
    Serial.print(medicines[medicineIndex].name);
    Serial.println(". Cannot dispense.");
    displayInsufficientQuantityMessage(medicineIndex);
    return;
  }

  int twoTimes = quantity * 5; // 10 steps per unit dose

  for (int z = 0; z < twoTimes; z++) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(stepPins[medicineIndex], HIGH);
      delayMicroseconds(1500); // Control speed with this delay
      digitalWrite(stepPins[medicineIndex], LOW);
      delayMicroseconds(1500); // Control speed with this delay
    }
    delay(1000);
  }


  medicines[medicineIndex].quantity -= quantity; // Update remaining quantity

  Serial.print("Dispensed ");
  Serial.print(quantity);
  Serial.print(" of ");
  Serial.print(medicines[medicineIndex].name);
  Serial.println(".");
  Serial.print("Remaining quantity: ");
  Serial.println(medicines[medicineIndex].quantity);
  // Updating FireBase Database
  //updateMedicineInFirestore(medicineIndex);
  updateMedicineData(medicineIndex);
}

void displayInsufficientQuantityMessage(int medicineIndex) {
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor to the first column, first row
  lcd.print("Insufficient");
  lcd.setCursor(0, 1); // Set cursor to the first column, second row
  lcd.print("quantity of ");
  lcd.setCursor(0, 2); // Set cursor to the first column, third row
  lcd.print(medicines[medicineIndex].name);
  lcd.setCursor(0, 3); // Set cursor to the first column, fourth row
  lcd.print("Cannot dispense.");
  delay(5000);

}



