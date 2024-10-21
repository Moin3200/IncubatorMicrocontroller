#define STEP_PIN_1 2
#define STEP_PIN_2 3
#define STEP_PIN_3 4
#define STEP_PIN_4 5

#define LEFT_BUTTON_PIN 6
#define RIGHT_BUTTON_PIN 7

#define SWING_TIME 10000

#define STEP_DELAY 5

bool movingLeft = true;
unsigned long startTime;

void setup() {
 
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
  pinMode(STEP_PIN_3, OUTPUT);
  pinMode(STEP_PIN_4, OUTPUT);

  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);  
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP); 

  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis();


  if (currentTime - startTime >= SWING_TIME) {
  
    stopMotor();
    return;
  }

  if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
    movingLeft = false;
  }
  if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
    movingLeft = true;
  }

  if (movingLeft) {
    moveLeft();
  } else {
    moveRight();
  }
}

void moveLeft() {
  stepMotor(0, 0, 0, 1);
  delay(STEP_DELAY);
  stepMotor(0, 0, 1, 1);
  delay(STEP_DELAY);
  stepMotor(0, 0, 1, 0);
  delay(STEP_DELAY);
  stepMotor(0, 1, 1, 0);
  delay(STEP_DELAY);
  stepMotor(0, 1, 0, 0);
  delay(STEP_DELAY);
  stepMotor(1, 1, 0, 0);
  delay(STEP_DELAY);
  stepMotor(1, 0, 0, 0);
  delay(STEP_DELAY);
  stepMotor(1, 0, 0, 1);
  delay(STEP_DELAY);
}

void moveRight() {
  stepMotor(1, 0, 0, 1);
  delay(STEP_DELAY);
  stepMotor(1, 0, 0, 0);
  delay(STEP_DELAY);
  stepMotor(1, 1, 0, 0);
  delay(STEP_DELAY);
  stepMotor(0, 1, 0, 0);
  delay(STEP_DELAY);
  stepMotor(0, 1, 1, 0);
  delay(STEP_DELAY);
  stepMotor(0, 0, 1, 0);
  delay(STEP_DELAY);
  stepMotor(0, 0, 1, 1);
  delay(STEP_DELAY);
  stepMotor(0, 0, 0, 1);
  delay(STEP_DELAY);
}

void stepMotor(int a, int b, int c, int d) {
  digitalWrite(STEP_PIN_1, a);
  digitalWrite(STEP_PIN_2, b);
  digitalWrite(STEP_PIN_3, c);
  digitalWrite(STEP_PIN_4, d);
}

void stopMotor() {
  digitalWrite(STEP_PIN_1, LOW);
  digitalWrite(STEP_PIN_2, LOW);
  digitalWrite(STEP_PIN_3, LOW);
  digitalWrite(STEP_PIN_4, LOW);
}
