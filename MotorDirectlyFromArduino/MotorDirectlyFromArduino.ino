#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4
#define LEFT_BUTTON_PIN 6
#define RIGHT_BUTTON_PIN 7
#define SWING_TIME 10000
#define STEP_DELAY 500

bool movingLeft = true;
unsigned long startTime;

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(ENABLE_PIN, LOW);
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
  digitalWrite(DIR_PIN, LOW);
  stepMotor();
}

void moveRight() {
  digitalWrite(DIR_PIN, HIGH);
  stepMotor();
}

void stepMotor() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_DELAY);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_DELAY);
}

void stopMotor() {
  digitalWrite(ENABLE_PIN, HIGH);
}
