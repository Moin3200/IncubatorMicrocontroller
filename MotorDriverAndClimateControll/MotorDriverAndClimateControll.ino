#include <MobaTools.h> 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
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

#define STEP_PIN 8
#define DIR_PIN 9

const int buttonLeftPin = 6;
const int buttonRightPin = 7;

MoToStepper stepper(200, FULLSTEP);  
int direction = 1;
unsigned long startTime;
bool isRunning = false;
unsigned long timeToRun = 60000;
unsigned long waitTime = 10000;
unsigned long afterSwingWaitTime = 5000;
int stepSpeed = 250; 

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
  
  stepper.attach(STEP_PIN, DIR_PIN);  
  stepper.setSpeed(stepSpeed);       
  stepper.setRampLen(50);            
  
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);

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
      stepper.setSpeed(stepSpeed);
      delay(waitTime);
    } else if (digitalRead(buttonRightPin) == LOW && direction == 1) {
      direction = -1;
      stepper.setSpeed(-stepSpeed);
      delay(waitTime);
    }

    stepper.move(200 * direction);
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
  // Logic for returning the motor to a known position
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
