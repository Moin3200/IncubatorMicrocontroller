#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Keypad.h>

#define DHTPIN1 2
#define DHTPIN2 3
#define DHTPIN3 4
#define DHTPIN4 5
#define DHTTYPE DHT22

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
DHT dht4(DHTPIN4, DHTTYPE);

#define FAN_LED_PIN 13
#define HUMIDIFIER_LED_PIN 7
#define HEATER_PIN 12

LiquidCrystal_I2C lcd(0x27, 20, 4);

float tempThreshold = 30.0;
float humidityThreshold = 50.0;

bool inAdjustmentMode = false;
bool settingTemp = true;
bool showingSensorData = false;

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
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();

  pinMode(FAN_LED_PIN, OUTPUT);
  pinMode(HUMIDIFIER_LED_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(FAN_LED_PIN, LOW);
  digitalWrite(HUMIDIFIER_LED_PIN, LOW);

  setInitialThresholds();
}

void loop() {
  char key = keypad.getKey();
  
  if (key == 'A') {
    inAdjustmentMode = true;
    adjustThresholds();
  } else if (key == 'B') {
    showingSensorData = true;
    showSensorData();
  }

  if (!inAdjustmentMode && !showingSensorData) {
    float avgTemperature, avgHumidity;
    calculateAverage(&avgTemperature, &avgHumidity);
    controlDevices(avgTemperature, avgHumidity);
    updateLCD(avgTemperature, avgHumidity);
    delay(1000);
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
            lcd.print("                "); // Clear the previous input
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
            lcd.print("                "); // Clear the previous input
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
  delay(1100);
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

  if (isnan(temp1) || isnan(temp2) || isnan(temp3) || isnan(temp4) ||
      isnan(humidity1) || isnan(humidity2) || isnan(humidity3) || isnan(humidity4)) {
    lcd.setCursor(0, 0);
    lcd.print("Error reading sensors");
    Serial.println("Error reading from DHT sensors");
    return;
  }

  *avgTemperature = (temp1 + temp2 + temp3 + temp4) / 4.0;
  *avgHumidity = (humidity1 + humidity2 + humidity3 + humidity4) / 4.0;
}

void controlDevices(float avgTemperature, float avgHumidity) {
  if (avgTemperature > tempThreshold) {
    digitalWrite(FAN_LED_PIN, HIGH);
    Serial.println("Fan: ON");
  } else {
    digitalWrite(FAN_LED_PIN, LOW);
    Serial.println("Fan: OFF");
  }
  
  if (avgTemperature > tempThreshold) {
    digitalWrite(HEATER_PIN, LOW);
    Serial.println("Heater OFF");
  } else {
    digitalWrite(HEATER_PIN, HIGH);
    Serial.println("Heater ON");
  }

  if (avgHumidity < humidityThreshold) {
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
  lcd.print("Sensor 1: T:");
  lcd.print(temp1, 1);
  lcd.print("C H:");
  lcd.print(humidity1, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Sensor 2: T:");
  lcd.print(temp2, 1);
  lcd.print("C H:");
  lcd.print(humidity2, 1);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Sensor 3: T:");
  lcd.print(temp3, 1);
  lcd.print("C H:");
  lcd.print(humidity3, 1);
  lcd.print("%");

  lcd.setCursor(0, 3);
  lcd.print("Sensor 4: T:");
  lcd.print(temp4, 1);
  lcd.print("C H:");
  lcd.print(humidity4, 1);
  lcd.print("%");

  delay(5000); // Show sensor data for 5 seconds
  showingSensorData = false;
}

void updateLCD(float avgTemperature, float avgHumidity) {
  lcd.clear
  lcd.setCursor(0, 0);
  lcd.print("Avg Temp: ");
  lcd.print(avgTemperature, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Avg Humidity: ");
  lcd.print(avgHumidity, 1);
  lcd.print(" %");

  lcd.setCursor(0, 2);
  lcd.print("Set Temp: ");
  lcd.print(tempThreshold, 1);
  lcd.print(" C");

  lcd.setCursor(0, 3);
  lcd.print("Set Humidity: ");
  lcd.print(humidityThreshold, 1);
  lcd.print(" %");
}
