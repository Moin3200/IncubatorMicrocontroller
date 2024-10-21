const int stepPin = 8;    
const int dirPin = 9;     

const int limitSwitch1 = 2; 
const int limitSwitch2 = 3; 

const unsigned long delayAtLimitSwitch = 3000; 
const unsigned long stepDelay = 1000; 

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(limitSwitch1, INPUT_PULLUP);
  pinMode(limitSwitch2, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(limitSwitch1) == LOW) {
    delay(delayAtLimitSwitch);
    digitalWrite(dirPin, HIGH);
    moveMotorToLimit(limitSwitch2);
  }

  if (digitalRead(limitSwitch2) == LOW) {
    delay(delayAtLimitSwitch);
    digitalWrite(dirPin, LOW);
    moveMotorToLimit(limitSwitch1);
  }
}

void moveMotorToLimit(int limitSwitchPin) {
  while (digitalRead(limitSwitchPin) == HIGH) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}
