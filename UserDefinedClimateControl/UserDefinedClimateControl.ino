#define DHT11_PIN 2 
#define DHT22_PIN 3 

int readDHT(int pin, float &humidity, float &temperature, bool isDHT22);

void setup() {
  Serial.begin(9600);
}

void loop() {
  float temperature = 0;
  float humidity = 0;

  if (readDHT(DHT11_PIN, humidity, temperature, false)) {
    Serial.print("DHT11 - Temperature: ");
    Serial.print(temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("DHT11 Read Failed!");
  }

  if (readDHT(DHT22_PIN, humidity, temperature, true)) {
    Serial.print("DHT22 - Temperature: ");
    Serial.print(temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("DHT22 Read Failed!");
  }

  delay(2000);
}

int readDHT(int pin, float &humidity, float &temperature, bool isDHT22) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  uint8_t bitTimes[40];

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);
  digitalWrite(pin, HIGH);
  delayMicroseconds(40);

  pinMode(pin, INPUT);

  if (pulseIn(pin, LOW, 1000) == 0) return 0; 
  if (pulseIn(pin, HIGH, 1000) == 0) return 0;

  for (int i = 0; i < 40; i++) {
    bitTimes[i] = pulseIn(pin, HIGH, 1000);
  }

  for (int i = 0; i < 40; i++) {
    data[i / 8] <<= 1;
    if (bitTimes[i] > 40) {
      data[i / 8] |= 1;
    }
  }

  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return 0;  // Checksum error
  }

  if (isDHT22) {
    humidity = ((data[0] << 8) | data[1]) * 0.1;
    temperature = (((data[2] & 0x7F) << 8) | data[3]) * 0.1;
    if (data[2] & 0x80) {
      temperature *= -1;
    }
  } else {
    humidity = data[0];
    temperature = data[2];
  }

  return 1;
}
