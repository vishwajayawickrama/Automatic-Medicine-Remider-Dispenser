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
const int dirPins[] = {4, 18, 13, 27};
const int stepPins[] = {5, 19, 14, 26};
const int motorsCount = 4;
const int stepsPerRevolution = 200;

// WiFi Credential
const char* ssid = "Dialog 4G";
const char* password = "E781A2F1";

// CallMeBot API Information
String phoneNumber = "+94774730705";
String apiKey = "8375006";

// Buzzer pin
const int buzzerPin = 15;

// Initialize the RTC
RTC_DS3231 rtc;

DateTime lastCheckDate;
DateTime lastCheckedQuantityDate;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {4, 5, 6, 7};
byte colPins[COLS] = {0, 1, 2, 3};

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, 0x20);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Buffer setup
const int bufferSize = 10; // Maximum number of keys to store
char buffer[bufferSize];   // Buffer to store pressed keys
int bufferIndex = 0;       // Index to track position in the buffer

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

void setup() {
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
    // displayTime();
    checkMedicineTimes();
    checkMedicineQuantities();
}

// Displaying Welcome Message until it retrieves medicine data from Server // Sachini
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

// Send Message to WhatsApp number using CallMe Bot // Sachini
void sendMessage(String message) {
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }

    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(url);
    if (httpResponseCode == 200) {
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
            Serial.print("Enter Medicine");
            lcd.print(medicines[medicineIndex].name);
            Serial.print(medicines[medicineIndex].name);
            lcd.print(" Quantity");
        } else if (bufferIndex == 1 && buffer[0] == 'A') {
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
            dispenseMedicine(medicineIndex, quantity);
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
        delay(500);                    // Buzz for 500 milliseconds (0.5 second)
        digitalWrite(buzzerPin, LOW);  // Turn off the buzzer
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
        DynamicJsonDocument doc(4096); // Adjust size based on your JSON response size
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
        medicines[medicineIndex].compartmentNumber = doc["compartment_no"] | 0;
        medicines[medicineIndex].dose = doc["dosage"] | 0;
        medicines[medicineIndex].times_per_day = doc["time_per_day"] | 0;

        JsonArray timesArray = doc["times"].as<JsonArray>();
        for (int i = 0; i < timesArray.size() && i < 5; i++) { // Adjust size based on your expected number of times
            strlcpy(medicines[medicineIndex].times[i].time, timesArray[i] | "", sizeof(medicines[medicineIndex].times[i].time));
            medicines[medicineIndex].times[i].dispensed = false;
        }

        medicines[medicineIndex].quantity = doc["quantity"] | 0;
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

// Function to fetch all medicines data
void fetchMedicine() {
    for (int i = 0; i < 4; i++) { // Adjust size based on your number of medicines
        getMedicineData(i);
    }
}

// Initialize Motor Pins
void initializeMotors() {
    for (int i = 0; i < motorsCount; i++) {
        pinMode(dirPins[i], OUTPUT);
        pinMode(stepPins[i], OUTPUT);
    }
}

// Function to dispense the specified quantity of a medicine
void dispenseMedicine(int medicineIndex, int quantity) {
    if (medicineIndex >= 0 && medicineIndex < motorsCount) {
        for (int i = 0; i < quantity; i++) {
            rotateMotor(medicineIndex);
            // Decrement the quantity of the dispensed medicine
            medicines[medicineIndex].quantity--;
        }
    }
}

// Function to rotate the motor of a specific compartment
void rotateMotor(int compartmentNumber) {
    int motorIndex = compartmentNumber - 1;

    if (motorIndex >= 0 && motorIndex < motorsCount) {
        digitalWrite(dirPins[motorIndex], HIGH);
        for (int i = 0; i < stepsPerRevolution; i++) {
            digitalWrite(stepPins[motorIndex], HIGH);
            delayMicroseconds(500);
            digitalWrite(stepPins[motorIndex], LOW);
            delayMicroseconds(500);
        }
    }
}

// Print Medicine Data on Serial Monitor
void printMedicine() {
    for (int i = 0; i < 4; i++) {
        Serial.println("----------------------");
        Serial.print("Medicine ");
        Serial.print(i + 1);
        Serial.println(":");
        Serial.print("ID: ");
        Serial.println(medicines[i].id);
        Serial.print("Name: ");
        Serial.println(medicines[i].name);
        Serial.print("Compartment Number: ");
        Serial.println(medicines[i].compartmentNumber);
        Serial.print("Dose: ");
        Serial.println(medicines[i].dose);
        Serial.print("Times per Day: ");
        Serial.println(medicines[i].times_per_day);
        Serial.println("Times:");
        for (int j = 0; j < medicines[i].times_per_day; j++) {
            Serial.print("  Time ");
            Serial.print(j + 1);
            Serial.print(": ");
            Serial.print(medicines[i].times[j].time);
            Serial.print(" (Dispensed: ");
            Serial.print(medicines[i].times[j].dispensed ? "Yes" : "No");
            Serial.println(")");
        }
        Serial.print("Quantity: ");
        Serial.println(medicines[i].quantity);
        Serial.println("----------------------");
        Serial.println();
    }
}

// Function to display the names of the medicines on the LCD // Sachini
void displayMedicineNamesOnLCD() {
    lcd.clear();
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(0, i);
        lcd.print(i + 1);
        lcd.print(": ");
        lcd.print(medicines[i].name);
    }
}

// Function to display the current time on the LCD
void displayTime() {
    DateTime now = rtc.now();

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(buffer);
}

// Function to initialize the RTC module and set the time if not already set
void initializeRTC() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        lcd.print("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

// Check Medicine Time Function
void checkMedicineTimes() {
    DateTime now = rtc.now();
    char currentTime[6]; // Buffer to store the current time in "HH:MM" format
    snprintf(currentTime, sizeof(currentTime), "%02d:%02d", now.hour(), now.minute());

    if (now.day() != lastCheckDate.day() || now.month() != lastCheckDate.month() || now.year() != lastCheckDate.year()) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < medicines[i].times_per_day; j++) {
                medicines[i].times[j].dispensed = false;
            }
        }
        lastCheckDate = now;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < medicines[i].times_per_day; j++) {
            if (strcmp(currentTime, medicines[i].times[j].time) == 0 && !medicines[i].times[j].dispensed) {
                dispenseMedicine(i, medicines[i].dose);
                buzz(medicines[i].dose);
                medicines[i].times[j].dispensed = true;

                String message = "Dispensed " + String(medicines[i].dose) + " " + String(medicines[i].name) + "s at " + String(currentTime) + ".";
                sendMessage(message);
            }
        }
    }
}

// Function to check medicine quantities and send a message if any quantity is less than 5
void checkMedicineQuantities() {
  DateTime now = rtc.now();

  // Check if a day has passed since the last check
  if (now.day() != lastCheckedQuantityDate.day() || now.month() != lastCheckedQuantityDate.month() || now.year() != lastCheckedQuantityDate.year()) {
    for (int i = 0; i < 4; i++) {
      if (medicines[i].quantity < 5) {
        String message = "Warning: Low quantity of " + String(medicines[i].name) + ". Only " + String(medicines[i].quantity) + " left.";
        sendMessage(message);
      }
    }
    // Update the last check date
    lastCheckedQuantityDate = now;
  }
}




