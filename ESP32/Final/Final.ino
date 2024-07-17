#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <AccelStepper.h>
#include <NewPing.h>
// Motor Specifications
const int dirPins[] = {4, 18, 13, 27};
const int stepPins[] = {5, 19, 14, 26};
const int motorsCount = 4;
const int stepsPerRevolution = 200;

// Create an array of AccelStepper objects
AccelStepper steppers[motorsCount] = {
    AccelStepper(AccelStepper::DRIVER, stepPins[0], dirPins[0]),
    AccelStepper(AccelStepper::DRIVER, stepPins[1], dirPins[1]),
    AccelStepper(AccelStepper::DRIVER, stepPins[2], dirPins[2]),
    AccelStepper(AccelStepper::DRIVER, stepPins[3], dirPins[3])
};

// WiFi Credential
const char* ssid = "Laka's S21+";
const char* password = "llllllll";

// CallMeBot API Information
String phoneNumber = "+94773403218";
String apiKey = "3460097";

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
byte rowPins[ROWS] = {0, 1, 2, 3}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; // Connect to the column pinouts of the keypad

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
// Define pins for ultrasonic sensor
#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define MAX_DISTANCE 200 // Maximum distance we want to sense (in centimeters)

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance



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
    printMedicine();
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
    lcd.print(" MediSync! ");
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
    lcd.begin();
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
        Serial.print("Key pressed: "); Serial.println(key); // Debugging statement

        if (bufferIndex == 1 && isdigit(buffer[0])) {
            int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
            Serial.print("Medicine Index: "); Serial.println(medicineIndex); // Debugging statement

            if (medicineIndex >= 0 && medicineIndex < motorsCount) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Enter ");
                lcd.print(medicines[medicineIndex].name);
                lcd.print(" Qty:");
                Serial.print("Enter ");
                Serial.print(medicines[medicineIndex].name);
                Serial.println(" Quantity");
            } else {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Invalid Medicine");
                Serial.println("Invalid Medicine");
                clearBuffer();
                delay(2000);
                displayMedicineNamesOnLCD();
            }
        } else if (bufferIndex == 1 && buffer[0] == 'A') {
            connectToWiFi();
            fetchMedicine();
            printMedicine();
            Serial.println("Medicine Data has been updated");
            clearBuffer();
            displayMedicineNamesOnLCD();
        } else if (bufferIndex == 3 && key == '#') {
            int medicineIndex = buffer[0] - '0' - 1; // If need to access through array just add -1
            int quantity = buffer[1] - '0';

            //print on lsc display
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Quantity: ");
            lcd.print(quantity);
            Serial.print("Medicine Index: "); Serial.println(medicineIndex); // Debugging statement
            Serial.print("Quantity: "); Serial.println(quantity); // Debugging statement
            delay(1000);

            if (medicineIndex >= 0 && medicineIndex < motorsCount && quantity > 0) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Dispensing ");
                lcd.print(quantity);
                lcd.print(" ");
                lcd.print(medicines[medicineIndex].name);
                lcd.print("s.");
                Serial.print("Dispensing ");
                Serial.print(quantity);
                Serial.print(" ");
                Serial.print(medicines[medicineIndex].name);
                Serial.println("s.");

                dispenseMedicine(medicineIndex, quantity);
                

                String message = "Dispensing " + String(quantity) + " " + String(medicines[medicineIndex].name) + "s.";
                sendMessage(message);

                clearBuffer();
                delay(2000);
                displayMedicineNamesOnLCD();
            } else {
                lcd.setCursor(0, 0);
                lcd.print("Invalid Input");
                Serial.println("Invalid Input");
                clearBuffer();
                delay(2000);
                displayMedicineNamesOnLCD();
            }
        } else if (key == '#') {
            lcd.setCursor(0, 0);
            lcd.print("Enter Quantity");
            Serial.println("Enter Quantity");
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
    const char* firebaseHost = "https://medisync-60405-default-rtdb.asia-southeast1.firebasedatabase.app";

    String url = String(firebaseHost) + "/medicines/" + String(medicineIndex) + ".json"; // Adjust path and index as needed
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
        medicines[medicineIndex].id = doc["id"] | 0; // Assuming 'id' is an integer
        strlcpy(medicines[medicineIndex].name, doc["medicine_name"] | "", sizeof(medicines[medicineIndex].name));
        medicines[medicineIndex].compartmentNumber = doc["compartment_number"] | 0;
        medicines[medicineIndex].dose = doc["dose_per_time"] | 0;
        medicines[medicineIndex].times_per_day = doc["times_per_day"] | 0;

        // Handle times array
        JsonArray timesArray = doc["times"].as<JsonArray>();
        for (int i = 0; i < timesArray.size() && i < 5; i++) { // Adjust size based on your expected number of times
            if (!timesArray[i].isNull()) {
                strlcpy(medicines[medicineIndex].times[i].time, timesArray[i] | "", sizeof(medicines[medicineIndex].times[i].time));
                medicines[medicineIndex].times[i].dispensed = false; // Initialize to false
            }
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
        steppers[i].setMaxSpeed(1000); // Set max speed (steps per second)
        steppers[i].setAcceleration(500); // Set acceleration (steps per second^2)
    }
}

// Function to dispense the specified quantity of a medicine
void dispenseMedicine(int medicineIndex, int quantity) {
    if (medicines[medicineIndex].quantity < quantity) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Not Enough Quantity");
      delay(5000);
      return;
    }
    if (medicineIndex >= 0 && medicineIndex < motorsCount) {
        for (int q = 0; q < quantity; q++) {
            for (int i = 0; i < 5; i++) { // Rotate 5 times for each quantity
                rotateMotor(medicineIndex);
                delay(1000); // Adjust delay between rotations if needed
            }
            // Decrement the quantity of the dispensed medicine
            medicines[medicineIndex].quantity--;   
        } 
        // Update quantity in Firestore
        updateFirestoreQuantity(medicineIndex);

        // Get updated medicine data after updating quantity
        fetchMedicine();
        buzz(quantity);
       
    }
}

// Function to rotate the motor of a specific compartment
void rotateMotor(int compartmentNumber) {
    int motorIndex = compartmentNumber;

    if (motorIndex >= 0 && motorIndex < motorsCount) {
        steppers[motorIndex].move(2); // Move 2 steps
        steppers[motorIndex].runToPosition(); // Block until the motor reaches the position
        // Reset the current position to 0 after each move to avoid backward movement
        
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

// Function to display the current time on the LCD
void displayTime() {
    DateTime now = rtc.now();

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(buffer);
}

// Function to initialize the RTC
void initializeRTC() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        // Following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize the last check date
    lastCheckDate = rtc.now();
    lastCheckedQuantityDate = rtc.now();
}
// Function to check if it's time to dispense medicine and dispense it if it is

void checkMedicineTimes() {
    bool medicineDispensed = false;
    DateTime now = rtc.now();
    char currentTime[6];
    sprintf(currentTime, "%02d%02d", now.hour(), now.minute());
    
   

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
                //buzz(medicines[i].dose);
                medicines[i].times[j].dispensed = true;

                String message = String(medicines[i].dose) + " " + medicines[i].name + " dispensed at " + currentTime;
                sendMessage(message);

                Serial.print(medicines[i].dose);
                Serial.print(" ");
                Serial.print(medicines[i].name);
                Serial.print(" dispensed at ");
                Serial.println(currentTime);

                 medicineDispensed = true;
            }
        }
    }  
    if (medicineDispensed) {
      

        // While the tray is close (distance <= 5), activate the buzzer
        while (checkDistance() <= 5) {
            digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
            delay(1000);
            digitalWrite(buzzerPin, LOW);
            delay(1000);
            digitalWrite(buzzerPin, HIGH);
        }
        
        // Turn off the buzzer after the tray is moved away
        digitalWrite(buzzerPin, LOW);
        
        // Reset the flag after the buzzer is turned off
        medicineDispensed = false;
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

// Function to update the quantity of a medicine in Firestore
void updateFirestoreQuantity(int medicineIndex) {
    HTTPClient http;

    // Specify the API endpoint for updating quantity
    const char* firebaseHost = "https://medisync-60405-default-rtdb.asia-southeast1.firebasedatabase.app";

    String url = String(firebaseHost) + "/medicines/" + String(medicineIndex) + ".json"; // Adjust path and index as needed

    // Construct the JSON payload
    String payload = "{\"quantity\": " + String(medicines[medicineIndex].quantity) + "}";

    // Send PATCH request to update quantity
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PATCH(payload);

    if (httpResponseCode > 0) {
        Serial.println("Quantity updated successfully");
    } else {
        Serial.print("Error updating quantity: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    
}

// Function to check the distance using the ultrasonic sensor
unsigned int checkDistance() {
    unsigned int distance = sonar.ping_cm();
    if (distance == 0) {
        Serial.println("Error: No response");
    } else {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
    }
    return distance;
}



