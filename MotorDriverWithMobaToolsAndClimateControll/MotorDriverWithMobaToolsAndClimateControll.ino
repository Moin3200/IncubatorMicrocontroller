// Define pins for the motor driver
const int stepPin = 3;  // STEP pin
const int dirPin = 4;   // DIR pin
const int stepsPerRevolution = 200; // Adjust based on your motor's step count
int stepCounter = 0;

void setup() {
  // Initialize the pins as outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initialize Serial Monitor
  Serial.begin(9600);

  // Set motor direction (HIGH for one direction, LOW for the other)
  digitalWrite(dirPin, HIGH);
}

void loop() {
  // Take a step
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000); // Adjust delay for step speed
  digitalWrite(stepPin, LOW);
  delayMicroseconds(1000);

  // Increment step counter
  stepCounter++;

  // Print the current step count to the Serial Monitor
  Serial.print("Step: ");
  Serial.println(stepCounter);

  // Optional: Reset counter after one full revolution
  if (stepCounter >= stepsPerRevolution) {
    stepCounter = 0;  // Reset counter after one revolution
    delay(1000);      // Pause for 1 second after a revolution
  }
}
