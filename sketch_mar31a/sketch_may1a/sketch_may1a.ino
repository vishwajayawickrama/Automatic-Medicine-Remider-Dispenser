#define STEP_PIN 4
#define DIR_PIN 0

// Define the number of steps per revolution for your stepper motor
const int stepsPerRevolution = 200;  // Change this according to your motor

void setup() {
  // Set the step and direction pins as outputs
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

void loop() {
  rotateMotor(1, 3);
  delay(500);


  // Rotate the motor one full revolution clockwise
  // digitalWrite(dirPin, LOW); // Set direction to clockwise
  // for (int i = 0; i < stepsPerRevolution; i++) {
  //   digitalWrite(stepPin, HIGH); // Trigger a step
  //   delayMicroseconds(500); // Adjust this value for the desired speed
  //   digitalWrite(stepPin, LOW);
  //   delayMicroseconds(500);
  // }
  // delay(50);
  // Rotate the motor one full revolution counterclockwise
  // digitalWrite(dirPin, LOW); // Set direction to counterclockwise
  // for (int i = 0; i < stepsPerRevolution; i++) {
  //   digitalWrite(stepPin, HIGH); // Trigger a step
  //   delayMicroseconds(500); // Adjust this value for the desired speed
  //   digitalWrite(stepPin, LOW);
  //   delayMicroseconds(500);
  //}
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


