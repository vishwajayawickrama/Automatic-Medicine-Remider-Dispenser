#include <Stepper.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

const int stepsPerRevolution = 200;
Stepper steppermotor(stepsPerRevolution, 8, 9, 10, 11);

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {14, 15, 16, 17}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {23, 25, 27, 29}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int bufferSize = 10; // Maximum number of keys to store
char buffer[bufferSize]; // Buffer to store pressed keys
int bufferIndex = 0; // Index to track position in the buffer


#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


char medicines[5][20] = {"Aspirin", "Ibuprofen", "Acetaminophen", "Amoxicillin", "Metformin"};

void setup() {
  // put your setup code here, to run once:

  lcd.init();
  lcd.backlight();

  // Printing Medicines
  lcd.setCursor(0, 0);
  lcd.print(medicines[0]);
  lcd.setCursor(0, 1);
  lcd.print(medicines[1]);
  lcd.setCursor(0, 2);
  lcd.print(medicines[2]);
  lcd.setCursor(0, 3);
  lcd.print(medicines[3]);
  
  // Setting Stepper Motor Speed
  steppermotor.setSpeed(60);
  
}

void loop() {
  
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
        steppermotor.step(quantity);
        
        // Clear the buffer
        bufferIndex = 0;
        memset(buffer, 0, bufferSize);
      }
    }
  }
}