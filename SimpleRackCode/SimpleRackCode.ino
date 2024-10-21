const int stepPin = 9;
const int dirPin = 8;
const int enablePin = 7; 

const int leftLimitSwitch = 3;
const int rightLimitSwitch = 2;

bool movingLeft = true;
unsigned long stepDelay = 1000;  

void setup() {

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  pinMode(leftLimitSwitch, INPUT_PULLUP); 
  pinMode(rightLimitSwitch, INPUT_PULLUP); 

  digitalWrite(enablePin, LOW);  
  
  digitalWrite(dirPin, LOW);  
}

void loop() {
  
  if (digitalRead(leftLimitSwitch) == LOW && movingLeft) {
    movingLeft = false;
    digitalWrite(dirPin, HIGH); 
    delay(500);  
  } 

  else if (digitalRead(rightLimitSwitch) == LOW && !movingLeft) {
    
    movingLeft = true;
    digitalWrite(dirPin, LOW);  
    delay(500);  
  }

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(stepDelay);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(stepDelay);
}
