#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>

#define motorLeft 4
#define motorRight 5
#define stepPin 9
#define dirPin 8

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int buttonLeftPin = 6;
const int buttonRightPin = 7;
int direction = 1;
unsigned long startTime;
bool isRunning = false;
unsigned long timeToRun = 600000;
unsigned long waitTime = 10000;
unsigned long afterSwingWaitTime = 5000;
long currentPosition = 0;
const int stepsPerRev = 200;
int stepIncrement = 2000;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { A5, A4, 10, 3 };
byte colPins[COLS] = { A0, A1, A2, A3 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long lastStepTime = 0;
unsigned long stepInterval = 1000;
bool start;
void setup() {
  start = false;
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(motorLeft, OUTPUT);
  pinMode(motorRight, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  lcd.begin(20, 4);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Rack Swing Ready");
  Serial.begin(9600);
}

void loop() {
  if (keypad.getKey() == 'B') {
    run();
  }

  if (keypad.getKey() == 'A') {
    getUserInput();
  }

  if (isRunning) {
    if (millis() - startTime >= timeToRun) {
      isRunning = false;
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
      waitForMillis(waitTime);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Running");
    } else if (digitalRead(buttonRightPin) == LOW && direction == 1) {
      direction = -1;
      lcd.setCursor(0, 1);
      lcd.print("Switching to Left");
      digitalWrite(motorLeft, HIGH);
      digitalWrite(motorRight, LOW);
      waitForMillis(waitTime);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Running");
    }

    moveStepper(direction);
    currentPosition += direction * stepIncrement;
    lcd.setCursor(0, 1);
    lcd.print("Direction: ");
    lcd.print(direction == 1 ? "RIGHT" : "LEFT");
    lcd.setCursor(0, 2);
    lcd.print("Time left: ");
    lcd.print((timeToRun - (millis() - startTime)) / 1000);
    lcd.print(" sec");
  }
}


void run() {
  startRackSwing();
}

void moveStepper(int dir) {
  if (millis() - lastStepTime >= stepInterval / 1000) {
    digitalWrite(dirPin, dir == 1 ? HIGH : LOW);
    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin, LOW);

    lastStepTime = millis();
  }
}

void returnToZero() {
  while (currentPosition != 0) {
    int dir = currentPosition > 0 ? -1 : 1;
    moveStepper(dir);
    currentPosition += dir * stepIncrement;
  }
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
  waitForMillis(waitTime);
  startRackSwing();
}

void waitForMillis(unsigned long duration) {
  unsigned long waitStart = millis();
  while (millis() - waitStart < duration) {
  }
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
        } else {
          lcd.setCursor(0, 2);
          lcd.print("Invalid Input");
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
