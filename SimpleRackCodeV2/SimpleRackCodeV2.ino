#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4
#define SWITCH_LEFT_PIN 5
#define SWITCH_RIGHT_PIN 6

LiquidCrystal_I2C lcd(0x27, 20, 4);

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

unsigned long startTime;
unsigned long runDuration = 30000; 

enum State { RUNNING, RETURNING, STOPPED };
State motorState = RUNNING;

enum Direction { LEFT, RIGHT };
Direction currentDirection = LEFT;

void setup() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); 

  pinMode(SWITCH_LEFT_PIN, INPUT_PULLUP);
  pinMode(SWITCH_RIGHT_PIN, INPUT_PULLUP);

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Swinging Rack");

  startTime = millis();
}

void loop() {
  unsigned long elapsedTime = millis() - startTime;

  switch (motorState) {
    case RUNNING:
      if (elapsedTime >= runDuration) {
        motorState = RETURNING;
        lcd.setCursor(0, 3);
        lcd.print("Returning to 0   ");
        stepper.moveTo(0); 
      } else {
        if (digitalRead(SWITCH_LEFT_PIN) == LOW && currentDirection == LEFT) {
          currentDirection = RIGHT;
          stepper.setSpeed(500);
          lcd.setCursor(0, 1);
          lcd.print("Direction: RIGHT");
        } else if (digitalRead(SWITCH_RIGHT_PIN) == LOW && currentDirection == RIGHT) {
          currentDirection = LEFT;
          stepper.setSpeed(-500);
          lcd.setCursor(0, 1);
          lcd.print("Direction: LEFT ");
        }

        stepper.runSpeed();
        lcd.setCursor(0, 2);
        lcd.print("Position: ");
        lcd.print(stepper.currentPosition());
      }
      break;

    case RETURNING:
      if (stepper.distanceToGo() == 0) {
        motorState = STOPPED;
        lcd.setCursor(0, 3);
        lcd.print("Stopped at 0 pos ");
      } else {
        stepper.run(); 
      }
      break;

    case STOPPED:
      digitalWrite(ENABLE_PIN, HIGH); 
      break;
  }
}
