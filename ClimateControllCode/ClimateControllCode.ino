#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Keypad.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define FAN_LED_PIN 13
#define HUMIDIFIER_LED_PIN 7
#define HEATER_PIN 12

LiquidCrystal_I2C lcd(0x27, 20, 4);

float tempThreshold = 30.0;
float humidityThreshold = 50.0;

bool inAdjustmentMode = false;
bool settingTemp = true;

// Keypad configuration
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputString = "";  // To capture keypad input

void setup() {
  lcd.begin(20, 4);
  lcd.backlight();

  Serial.begin(9600);
  dht.begin();

  pinMode(FAN_LED_PIN, OUTPUT);
  pinMode(HUMIDIFIER_LED_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(FAN_LED_PIN, LOW);
  digitalWrite(HUMIDIFIER_LED_PIN, LOW);

  setInitialThresholds();
}

void loop() {
  if (inAdjustmentMode) {
    adjustThresholds();
  } else {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      lcd.setCursor(0, 0);
      lcd.print("Error reading");
      Serial.println("Error reading from DHT sensor");
      return;
    }

    if (temperature > tempThreshold) {
      digitalWrite(FAN_LED_PIN, HIGH);
      Serial.println("Fan: ON");
    } else {
      digitalWrite(FAN_LED_PIN, LOW);
      Serial.println("Fan: OFF");
    }
    
    if (temperature > tempThreshold) {
      digitalWrite(HEATER_PIN, LOW);
      Serial.println("Heater OFF");
    } else {
      digitalWrite(HEATER_PIN, HIGH);
      Serial.println("Heater ON");
    }

    if (humidity < humidityThreshold) {
      digitalWrite(HUMIDIFIER_LED_PIN, HIGH);
      Serial.println("Humidifier: ON");
    } else {
      digitalWrite(HUMIDIFIER_LED_PIN, LOW);
      Serial.println("Humidifier: OFF");
    }

    updateLCD(temperature, humidity);
    delay(1000);
  }
}

void setInitialThresholds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Temp & Humid");

  inAdjustmentMode = true;
  adjustThresholds();
  inAdjustmentMode = false;
}

void adjustThresholds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Temp:");

  inputString = "";
  while (settingTemp) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        tempThreshold = inputString.toFloat();
        settingTemp = false;
        inputString = "";
      } else {
        inputString += key;
        lcd.setCursor(0, 1);
        lcd.print(inputString);
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Humidity:");

  settingTemp = true;
  while (settingTemp) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        humidityThreshold = inputString.toFloat();
        settingTemp = false;
        inputString = "";
      } else {
        inputString += key;
        lcd.setCursor(0, 1);
        lcd.print(inputString);
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  delay(1100);
  inAdjustmentMode = false;
}

void updateLCD(float temperature, float humidity) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Cur Temp: ");
  lcd.setCursor(1, 1);
  lcd.print(temperature, 1);
  lcd.print(" C");

  lcd.setCursor(10, 0);
  lcd.print("Set Temp: ");
  lcd.setCursor(10, 1);
  lcd.print(tempThreshold, 1);
  lcd.print(" C");

  lcd.setCursor(0, 2);
  lcd.print("Cur Humid: ");
  lcd.setCursor(0, 3);
  lcd.print(humidity, 1);
  lcd.print(" %");

  lcd.setCursor(10, 2);
  lcd.print("Set Humid: ");
  lcd.setCursor(10, 3);
  lcd.print(humidityThreshold, 1);
  lcd.print(" %");
}
