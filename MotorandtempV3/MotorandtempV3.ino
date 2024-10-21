#include <MobaTools.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include "DHT.h"

#define DHTPIN1 2
#define DHTPIN2 11
#define DHTPIN3 12
#define DHTPIN4 13
#define DHTTYPE1 DHT11
#define DHTTYPE2 DHT22

DHT dht1(DHTPIN1, DHTTYPE2);
DHT dht2(DHTPIN2, DHTTYPE1);
DHT dht3(DHTPIN3, DHTTYPE1);
DHT dht4(DHTPIN4, DHTTYPE1);

#define FAN_LED_PIN 1
#define HUMIDIFIER_LED_PIN 1
#define HEATER_PIN 1

#define motorLeft 4
#define motorRight 5
LiquidCrystal_I2C lcd(0x27, 20, 4);

float tempThreshold = 30.0;
float humidityThreshold = 50.0;

bool inAdjustmentMode = false;
bool settingTemp = true;
bool showingSensorData = false;
bool fanState = false;

const int buttonLeftPin = 6;
const int buttonRightPin = 7;

int direction = 1;
unsigned long startTime;
bool isRunning = false;
unsigned long timeToRun = 60000;
unsigned long waitTime = 10000;
unsigned long afterSwingWaitTime = 5000;
unsigned long waitTimeStart = 0;
unsigned long afterSwingWaitStart = 0;
unsigned long swingStart = 0;
long currentPosition = 0;

float lastAvgTemp = -100.0;
float lastAvgHumidity = -100.0;
int lastDirection = 0;
long lastRemainingTime = 0;

const int stepsPerRev = 200;
MoToStepper myStepper(stepsPerRev, STEPDIR);
int stepIncrement = 200;

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

String inputString = "";

bool limitSwitchPressed = false;  // Flag to disable input when limit switch is pressed

void loop() {
  char key = keypad.getKey();

  // Prevent any input if limit switch is pressed and wait time hasn't passed
  if (!limitSwitchPressed) {
    if (key == 'A') {
      getUserInput();
    }
    if (key == 'B') {
      inAdjustmentMode = true;
      adjustThresholds();
    } else if (key == 'C') {
      showingSensorData = true;
      showSensorData();
    } else if (key == 'D') {
      toggleFan();
    }
  }

  unsigned long currentMillis = millis();

  if (isRunning) {
    if (currentMillis - swingStart >= timeToRun) {
      isRunning = false;
      lcd.setCursor(0, 0);
      lcd.print("Rack Swing Stopping");
      returnToZero();
      lcd.setCursor(0, 2);
      lcd.print("Returned to Zero  ");

      afterSwingWaitStart = millis();
    }

    if (!isRunning && currentMillis - afterSwingWaitStart >= afterSwingWaitTime && afterSwingWaitTime != 0) {
      afterSwingWaitStart = 0;
      waitForNextRun();
    }

    if (!limitSwitchPressed) {
      if (digitalRead(buttonLeftPin) == LOW && direction == -1) {
        limitSwitchPressed = true;
        direction = 1;
        digitalWrite(motorRight, HIGH);
        digitalWrite(motorLeft, LOW);
        waitTimeStart = millis();
      } else if (digitalRead(buttonRightPin) == LOW && direction == 1) {
        limitSwitchPressed = true;
        direction = -1;
        digitalWrite(motorLeft, HIGH);
        digitalWrite(motorRight, LOW);
        waitTimeStart = millis();
      }
    }

    if (currentMillis - waitTimeStart >= waitTime && limitSwitchPressed) {
      limitSwitchPressed = false;
      moveStepper(direction);
      currentPosition += direction * stepIncrement;
    }

    if (lastDirection != direction) {
      lcd.setCursor(0, 1);
      lcd.print("Direction: ");
      lcd.print(direction == 1 ? "LEFT " : "RIGHT  ");
      lastDirection = direction;
    }

    long remainingTime = (timeToRun - (millis() - swingStart)) / 1000;
    if (lastRemainingTime != remainingTime) {
      lcd.setCursor(0, 2);
      lcd.print("Time left: ");
      lcd.print(remainingTime);
      lcd.print(" sec   ");
      lastRemainingTime = remainingTime;
    }
  }
}

void waitForNextRun() {
  unsigned long currentMillis = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for next run");

  if (currentMillis - swingStart >= waitTime) {
    limitSwitchPressed = false;
    startRackSwing();
  }
}

void startRackSwing() {
  swingStart = millis();
  isRunning = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rack Swing Running");
}

void returnToZero() {
  myStepper.moveTo(0);
  currentPosition = 0;
}

void moveStepper(int dir) {
  if (dir == 1) {
    myStepper.move(stepIncrement);
  } else {
    myStepper.move(-stepIncrement);
  }
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
  unsigned long currentMillis = millis();
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Waiting ");
  // lcd.print(waitTime / 1000);
  // lcd.print(" sec...");

  if (currentMillis - swingStart >= waitTime) {
    startRackSwing();
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
void toggleFan() {
  fanState = !fanState;
  digitalWrite(FAN_LED_PIN, fanState ? HIGH : LOW);

  if (fanState) {
    Serial.println("Fan: ON");
  } else {
    Serial.println("Fan: OFF");
  }
}

void setInitialThresholds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Avg Temp & Humid");

  inAdjustmentMode = true;
  adjustThresholds();
  inAdjustmentMode = false;
}

void adjustThresholds() {
  lcd.clear();
  if (settingTemp) {
    lcd.setCursor(0, 0);
    lcd.print("Enter Avg Temp:");

    inputString = "";
    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (key == '#') {
          tempThreshold = inputString.toFloat();
          settingTemp = false;
          break;
        } else if (key == '*') {
          if (inputString.length() > 0) {
            inputString.remove(inputString.length() - 1);
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print(inputString);
          }
        } else {
          inputString += key;
          lcd.setCursor(0, 1);
          lcd.print(inputString);
        }
      }
    }
  }

  if (!settingTemp) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Avg Humidity:");

    inputString = "";
    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (key == '#') {
          humidityThreshold = inputString.toFloat();
          settingTemp = true;
          break;
        } else if (key == '*') {
          if (inputString.length() > 0) {
            inputString.remove(inputString.length() - 1);
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print(inputString);
          }
        } else {
          inputString += key;
          lcd.setCursor(0, 1);
          lcd.print(inputString);
        }
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  delay(10);
  inAdjustmentMode = false;
}

void calculateAverage(float *avgTemperature, float *avgHumidity) {
  float temp1 = dht1.readTemperature();
  float temp2 = dht2.readTemperature();
  float temp3 = dht3.readTemperature();
  float temp4 = dht4.readTemperature();

  float humidity1 = dht1.readHumidity();
  float humidity2 = dht2.readHumidity();
  float humidity3 = dht3.readHumidity();
  float humidity4 = dht4.readHumidity();

  if (isnan(temp2) || isnan(temp3) || isnan(temp4) || isnan(humidity2) || isnan(humidity3) || isnan(humidity4)) {
    lcd.setCursor(0, 0);
    lcd.print("Error reading sensors");
    Serial.println("Error reading from DHT sensors");
    return;
  }

  *avgTemperature = (temp1 + temp2 + temp3 + temp4) / 4.0;
  *avgHumidity = (humidity1 + humidity2 + humidity3 + humidity4) / 4.0;
}

void controlDevices(float avgTemperature, float avgHumidity) {
  if (avgTemperature < tempThreshold) {
    digitalWrite(FAN_LED_PIN, HIGH);
    Serial.println("Fan: ON");
  } else {
    digitalWrite(FAN_LED_PIN, LOW);
    Serial.println("Fan: OFF");
  }

  if (avgTemperature < tempThreshold) {
    digitalWrite(HEATER_PIN, LOW);
    Serial.println("Heater OFF");
  } else {
    digitalWrite(HEATER_PIN, HIGH);
    Serial.println("Heater ON");
  }

  if (avgHumidity > humidityThreshold) {
    digitalWrite(HUMIDIFIER_LED_PIN, HIGH);
    Serial.println("Humidifier: ON");
  } else {
    digitalWrite(HUMIDIFIER_LED_PIN, LOW);
    Serial.println("Humidifier: OFF");
  }
}

void showSensorData() {
  float temp1 = dht1.readTemperature();
  float temp2 = dht2.readTemperature();
  float temp3 = dht3.readTemperature();
  float temp4 = dht4.readTemperature();

  float humidity1 = dht1.readHumidity();
  float humidity2 = dht2.readHumidity();
  float humidity3 = dht3.readHumidity();
  float humidity4 = dht4.readHumidity();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sen 1: T:");
  lcd.print(temp1, 1);
  lcd.print(" H:");
  lcd.print(humidity1, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Sen 2: T:");
  lcd.print(temp2, 1);
  lcd.print(" H:");
  lcd.print(humidity2, 1);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Sen 3: T:");
  lcd.print(temp3, 1);
  lcd.print(" H:");
  lcd.print(humidity3, 1);
  lcd.print("%");

  lcd.setCursor(0, 3);
  lcd.print("Sen 4: T:");
  lcd.print(temp4, 1);
  lcd.print(" H:");
  lcd.print(humidity4, 1);
  lcd.print("%");

  delay(100);
  showingSensorData = false;
}

void updateLCD(float avgTemperature, float avgHumidity) {

  lcd.setCursor(0, 1);
  lcd.print("Cabin T&H: ");
  lcd.print(avgTemperature, 1);
  lcd.println(avgHumidity, 1);

  // lcd.setCursor(0, 1);
  // lcd.print("Cabin Humidity: ");
  // lcd.print(avgHumidity, 1);
  // lcd.print("%   ");

  lcd.setCursor(0, 3);
  lcd.print("Set T&H: ");
  lcd.print(tempThreshold, 1);
  lcd.println(humidityThreshold, 1);

  // lcd.setCursor(0, 3);
  // lcd.print("Set Humidity: ");
  // lcd.print(humidityThreshold, 1);
  // lcd.print(" %  ");
}