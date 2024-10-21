#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define STEPPER_PIN_1 8
#define STEPPER_PIN_2 9
#define STEPPER_PIN_3 10
#define STEPPER_PIN_4 11
#define motorLeft 4
#define motorRight 5
LiquidCrystal_I2C lcd(0x27, 20, 4);

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

const int buttonLeftPin = 6;
const int buttonRightPin = 7;

int direction = 1;
unsigned long startTime;
bool isRunning = false;
long totalSteps = 0;
int stepIncrement = 100;

unsigned long timeToRun = 60000;
unsigned long waitTime = 10000;
unsigned long afterSwingWaitTime = 5000;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 3, 2};
byte colPins[COLS] = {A0, A1, A2, A3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  stepper.setMaxSpeed(250);
  stepper.setAcceleration(1023);

  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(motorLeft, OUTPUT);
  pinMode(motorRight, OUTPUT);
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.print("Rack Swing Ready");
  Serial.begin(9600);
  startRackSwing();
}

void loop() {
  if (keypad.getKey() == 'A') {
    getUserInput();
  }

  if (isRunning) {
    if (millis() - startTime >= timeToRun) {
      isRunning = false;
      stepper.stop();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing");
      lcd.setCursor(0, 1);
      lcd.print("Stopping...");
      returnToZero();
      lcd.setCursor(0, 2);
      lcd.print("Returned to Zero");
      delay(afterSwingWaitTime);
      waitForNextRun();
    }

    if (digitalRead(buttonLeftPin) == LOW && direction == -1) {
      direction = 1;
      lcd.setCursor(0, 1);
      lcd.print("Switching to Right");
      digitalWrite(motorRight, HIGH);
      digitalWrite(motorLeft, LOW);
      delay(waitTime);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Running");
    } else if (digitalRead(buttonRightPin) == LOW && direction == 1) {
      direction = -1;
      lcd.setCursor(0, 1);
      lcd.print("Switching to Left");
      digitalWrite(motorLeft, HIGH);
      digitalWrite(motorRight, LOW);
      delay(waitTime);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Running");
    }

    stepper.move(direction * stepIncrement);
    stepper.run();

    totalSteps += direction * stepIncrement;

    lcd.setCursor(0, 1);
    lcd.print("Direction: ");
    lcd.print(direction == 1 ? "RIGHT" : "LEFT");

    lcd.setCursor(0, 2);
    lcd.print("Time left: ");
    lcd.print((timeToRun - (millis() - startTime)) / 1000);
    lcd.print(" sec");
  }
}

void returnToZero() {
  stepper.runToNewPosition(0);
  totalSteps = 0;
}

void startRackSwing() {
  startTime = millis();
  isRunning = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rack Swing Running");
}

void getUserInput() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Run Time (sec): ");
  timeToRun = getNumberInput() * 1000;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wait Time (sec): ");
  waitTime = getNumberInput() * 1000;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("After Swing Wait (sec): ");
  afterSwingWaitTime = getNumberInput() * 1000;

  startRackSwing();
}

void waitForNextRun() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting ");
  lcd.print(waitTime / 1000);
  lcd.print(" sec...");
  delay(1);
  startRackSwing();
}

long getNumberInput() {
  String input = "";
  char key;

  while (true) {
    key = keypad.getKey();

    if (key) {
      if (key == '#') {
        if (input.length() > 0) {
          return input.toInt();
        }
      } else if (key == '*') {
        input = "";
        lcd.setCursor(0, 1);
        lcd.print("                ");
      } else {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(input);
      }
    }
  }
}