# ESP32 Medicine Dispenser

## Overview

This directory contains the consolidated ESP32 firmware for the Automatic Medicine Reminder and Dispenser system.

## File Structure

- **MedicineDispenser.ino** - Main firmware file containing all the code for the ESP32
- **.vscode/** - Visual Studio Code configuration files for development

## Features

The consolidated firmware includes:

- **WiFi Connectivity** - Connects to WiFi for remote data access
- **Firebase Integration** - Fetches medicine data from Firebase Realtime Database
- **RTC Module** - Real-time clock for scheduled medicine dispensing
- **LCD Display** - 20x4 LCD display for user interface
- **Keypad Input** - 4x4 keypad for user interaction
- **Stepper Motors** - Controls 4 stepper motors for medicine dispensing
- **Ultrasonic Sensor** - Detects tray presence for safety
- **Buzzer Alerts** - Audio notifications for medicine reminders
- **WhatsApp Notifications** - Sends notifications via CallMeBot API

## Hardware Requirements

- ESP32 Development Board
- DS3231 RTC Module
- 20x4 I2C LCD Display (Address: 0x27)
- 4x4 I2C Keypad (Address: 0x20)
- 4x Stepper Motors with drivers
- HC-SR04 Ultrasonic Sensor
- Buzzer
- WiFi Network

## Pin Configuration

### Stepper Motors
- Motor 1: DIR=4, STEP=5
- Motor 2: DIR=18, STEP=19
- Motor 3: DIR=13, STEP=14
- Motor 4: DIR=27, STEP=26

### Ultrasonic Sensor
- TRIGGER_PIN: 32
- ECHO_PIN: 33

### Buzzer
- BUZZER_PIN: 15

### I2C Devices
- LCD: 0x27
- Keypad: 0x20

## Configuration

Before uploading, configure the following in MedicineDispenser.ino:

```cpp
// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// CallMeBot API
String phoneNumber = "+YOUR_PHONE_NUMBER";
String apiKey = "YOUR_API_KEY";

// Firebase
const char* firebaseHost = "YOUR_FIREBASE_URL";
```

## Required Libraries

Install these libraries via Arduino Library Manager:

1. Wire (built-in)
2. Keypad_I2C
3. Keypad
4. LiquidCrystal_I2C
5. WiFi (ESP32 core)
6. HTTPClient (ESP32 core)
7. UrlEncode
8. ArduinoJson
9. RTClib
10. AccelStepper
11. NewPing

## Usage

### Uploading the Code

1. Open MedicineDispenser.ino in Arduino IDE
2. Select Board: "ESP32 Dev Module"
3. Configure WiFi and API credentials
4. Upload to ESP32

### User Interface

- **Press 1-4**: Select medicine compartment
- **Press 0-9**: Enter quantity
- **Press #**: Confirm selection
- **Press A**: Update medicine data from server

### Automatic Operation

The system automatically:
- Checks medicine schedule every minute
- Dispenses medicine at scheduled times
- Sends WhatsApp notifications
- Monitors medicine quantities
- Alerts when tray is not present

## Troubleshooting

### Common Issues

1. **LCD not displaying**: Check I2C address (0x27 or 0x3F)
2. **Keypad not responding**: Verify I2C address (0x20)
3. **RTC not found**: Check RTC module connections
4. **WiFi connection failed**: Verify SSID and password
5. **Firebase errors**: Check Firebase URL and network

## Development

### Building

This is an Arduino sketch (.ino file) that can be compiled using:
- Arduino IDE 2.0+
- PlatformIO
- Arduino CLI

### Testing

Due to hardware dependencies, this code requires physical ESP32 hardware and connected peripherals for testing. Syntax validation can be performed using Arduino IDE's verify function.

## Change History

- **v3.0** - Consolidated all code into single file
  - Merged Final, Main, and Lastupdate versions
  - Added ultrasonic sensor support
  - Improved error handling
  - Enhanced quantity management

## Contributors

- Vishwa Jayawickrama
- Lakshan Abenayake
- Pasindula Bandara
- Sachini Kalansooriya

## License

Part of the Automatic Medicine Reminder and Dispenser project
University of Moratuwa - Faculty of Information Technology
