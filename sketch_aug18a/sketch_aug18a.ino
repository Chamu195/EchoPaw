#define LEFT_LED 23
#define RIGHT_LED 22
#define BUZZER 21
#define PIR_PIN 27

int motionCount = 0;
unsigned long firstMotionTime = 0;

const unsigned long aiTimeWindow = 20000; // 20 seconds

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  digitalWrite(LEFT_LED, LOW);
  digitalWrite(RIGHT_LED, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.println("EchoPaw AI Guardian Started");
  Serial.println("Waiting 30 seconds for PIR to settle...");
  delay(30000);

  Serial.println("EchoPaw is active now");
}

void loop() {
  int motion = digitalRead(PIR_PIN);
  unsigned long currentTime = millis();

  if (motion == HIGH) {
    if (motionCount == 0) {
      firstMotionTime = currentTime;
    }

    motionCount++;

    Serial.println("Motion detected!");
    Serial.print("AI Motion Count: ");
    Serial.println(motionCount);

    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, HIGH);

    if (motionCount == 1) {
      Serial.println("AI Status: NORMAL MOVEMENT");
      shortBeep();
    }
    else if (motionCount >= 2 && motionCount <= 3) {
      Serial.println("AI Status: MONITORING ACTIVITY");
      mediumBeep();
    }
    else {
      Serial.println("AI Status: SUSPICIOUS UNKNOWN MOVEMENT");
      dangerBeep();
    }

    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);

    delay(3000);
  }
  else {
    Serial.println("No motion. AI Status: SAFE");

    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    digitalWrite(BUZZER, LOW);

    if (motionCount > 0 && currentTime - firstMotionTime > aiTimeWindow) {
      motionCount = 0;
      firstMotionTime = 0;
      Serial.println("AI motion memory reset.");
    }

    delay(1000);
  }
}

void shortBeep() {
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void mediumBeep() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
}

void dangerBeep() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, HIGH);
    delay(200);

    digitalWrite(BUZZER, LOW);
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    delay(200);
  }
}