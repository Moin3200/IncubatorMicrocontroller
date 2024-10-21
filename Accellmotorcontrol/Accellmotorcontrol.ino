#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <AccelStepper.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define FPIN 13
#define HEATPIN 12
#define HUMIDPIN 7

float tempThreshold = 30.0;
float humidityThreshold = 50.0;
bool inAdjustmentMode = false;

#define STEPPER_PIN_1 8
#define STEPPER_PIN_2 9
#define STEPPER_PIN_3 10
#define STEPPER_PIN_4 11
#define motorLeft 4
#define motorRight 5
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

LiquidCrystal_I2C lcd(0x27, 20, 4);
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
  lcd.begin(20, 4);
  lcd.backlight();
  dht.begin();
  
  pinMode(FPIN, OUTPUT);
  pinMode(HUMIDPIN, OUTPUT);
  pinMode(HEATPIN, OUTPUT);
  digitalWrite(FPIN, LOW);
  digitalWrite(HUMIDPIN, LOW);
  digitalWrite(HEATPIN, LOW);
  
  stepper.setMaxSpeed(250);
  stepper.setAcceleration(1023);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(motorLeft, OUTPUT);
  pinMode(motorRight, OUTPUT);

  Serial.begin(9600);
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void loop() {
  char key = keypad.getKey();
  
  if (key == 'A') {
    adjustThresholds();
  }
  
  handleRackSwing();

  handleClimateControl();
}

void adjustThresholds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Adjusting Temp & Humid");
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Temp:");
  tempThreshold = getNumberInput() * 1.0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Humid:");
  humidityThreshold = getNumberInput() * 1.0;
  
  lcd.clear();
  lcd.print("Thresholds Updated");
  delay(1000);
}

void handleRackSwing() {
  if (isRunning) {
    if (millis() - startTime >= timeToRun) {
      isRunning = false;
      stepper.stop();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Stopping...");
      returnToZero();
      delay(afterSwingWaitTime);
    }
    
    if (digitalRead(buttonLeftPin) == LOW && direction == -1) {
      direction = 1;
      digitalWrite(motorRight, HIGH);
      digitalWrite(motorLeft, LOW);
      delay(waitTime);
    } else if (digitalRead(buttonRightPin) == LOW && direction == 1) {
      direction = -1;
      digitalWrite(motorLeft, HIGH);
      digitalWrite(motorRight, LOW);
      delay(waitTime);
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

void handleClimateControl() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    lcd.setCursor(0, 0);
    lcd.print("Error reading sensor");
    return;
  }

  if (temperature > tempThreshold) {
    digitalWrite(FPIN, HIGH);
  } else {
    digitalWrite(FPIN, LOW);
  }

  if (humidity < humidityThreshold) {
    digitalWrite(HUMIDPIN, HIGH);
  } else {
    digitalWrite(HUMIDPIN, LOW);
  }

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(humidity);
  lcd.print(" %");
}

void returnToZero() {
  stepper.runToNewPosition(0);
  totalSteps = 0;
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
