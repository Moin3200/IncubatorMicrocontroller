#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define FAN_LED_PIN 13
#define HUMIDIFIER_LED_PIN 7
#define HEATER_PIN 12

#define UP_BUTTON_PIN 3
#define DOWN_BUTTON_PIN 4
#define CONFIRM_BUTTON_PIN 8

LiquidCrystal_I2C lcd(0x27, 20, 4);

float tempThreshold = 30.0;
float humidityThreshold = 50.0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;

bool inAdjustmentMode = false;
bool settingTemp = true;

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

  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONFIRM_BUTTON_PIN, INPUT_PULLUP);

  setInitialThresholds();
}

void loop() {
  if (digitalRead(CONFIRM_BUTTON_PIN) == LOW && millis() - lastDebounceTime > debounceDelay) {
    inAdjustmentMode = true;
    lastDebounceTime = millis();
    adjustThresholds();
  }

  if (!inAdjustmentMode) {
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
    if(temperature > tempThreshold){
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
  while (inAdjustmentMode) {
    if (millis() - lastDebounceTime > debounceDelay) {
      if (settingTemp) {
        if (digitalRead(UP_BUTTON_PIN) == LOW) {
          tempThreshold += 0.5;
          lastDebounceTime = millis();
        }
        if (digitalRead(DOWN_BUTTON_PIN) == LOW) {
          tempThreshold -= 0.5;
          lastDebounceTime = millis();
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Temp: ");
        lcd.print(tempThreshold, 1);
        lcd.print(" C");

        if (digitalRead(CONFIRM_BUTTON_PIN) == LOW && millis() - lastDebounceTime > debounceDelay) {
          settingTemp = false;
          lastDebounceTime = millis();
        }
        delay(250);
      } else {
        if (digitalRead(UP_BUTTON_PIN) == LOW) {
          humidityThreshold += 1.0;
          lastDebounceTime = millis();
        }
        if (digitalRead(DOWN_BUTTON_PIN) == LOW) {
          humidityThreshold -= 1.0;
          lastDebounceTime = millis();
        }

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Set Humid: ");
        lcd.print(humidityThreshold, 1);
        lcd.print(" %");

        if (digitalRead(CONFIRM_BUTTON_PIN) == LOW && millis() - lastDebounceTime > debounceDelay) {
          inAdjustmentMode = false;
          settingTemp = true;
          lastDebounceTime = millis();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Starting...");
          delay(1100);
        }
        delay(250);
      }
    }
  }
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