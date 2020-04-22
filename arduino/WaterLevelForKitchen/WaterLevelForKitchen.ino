#define PIN_TRIG 12 // HC-SR04 pin "Trig"
#define PIN_ECHO 11 // HC-SR04 pin "Echo"
#define PIN_PIEZO 6 // Buzzer control

#define LED_COUNT 10 // LED bar
int ledGroup[LED_COUNT] = {2,3,4,5,A5,A4,A3,A2,A1,A0}; // Pins for LED bar

#define LEVEL_EMPTY 390 // no water
#define LEVEL_FULL  160 // water about to overflow

uint32_t duration, mm; // measure intermediate variables
uint32_t iter = 0; // measure iteration number
uint32_t mm_sum = 0; // summ for average calculation
uint32_t mm_prev = 0; // previous height
uint32_t skip_decrease = 0; // do not update level immediately when decreased
uint32_t currentLedMask = 0; // bitmask for LED bar: 1 - on, 0 - off
uint32_t beepWarning = 0; // 1 - one beeps (warning); 2 - two beeps (ready to drain); 3 - three beeps (overflowing)

void setup() {
  Serial.begin (9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  for (int i = 0; i < LED_COUNT; ++i) {
    pinMode(ledGroup[i], OUTPUT);
  }
}

void ledOut(uint32_t enabledLedMask) {
  for (int i = 0; i < LED_COUNT; ++i) {
    digitalWrite(ledGroup[i], (enabledLedMask & (1 << i)) ? HIGH : LOW);
  }
}

void updateLevelMark(uint8_t levelMark) {
  if (levelMark > LED_COUNT)
    return;
  currentLedMask = 0;
  for (int i = 0; i < levelMark; ++i) {
    currentLedMask |= 1 << i;
  }

  // Beep warning
  if (levelMark == 0) {
    beepWarning = 0;
  }
  else if (levelMark == 8) {
    if (beepWarning < 1) {
        tone(PIN_PIEZO, 1500); delay(100); noTone(PIN_PIEZO); delay(300);
        beepWarning = 1;
    }
  }
  else if (levelMark == 9) {
    if (beepWarning < 2) {
        tone(PIN_PIEZO, 2000); delay(100); noTone(PIN_PIEZO); delay(300);
        tone(PIN_PIEZO, 2000); delay(100); noTone(PIN_PIEZO); delay(300);
        beepWarning = 2;
    }
  }
  else if (levelMark == 10) {
    if (beepWarning < 3) {
        tone(PIN_PIEZO, 2500); delay(100); noTone(PIN_PIEZO); delay(300);
        tone(PIN_PIEZO, 2500); delay(100); noTone(PIN_PIEZO); delay(300);
        tone(PIN_PIEZO, 2500); delay(100); noTone(PIN_PIEZO); delay(300);
        beepWarning = 3;
    }
  }
}

void processLevel(uint32_t level) {
  if (level > LEVEL_EMPTY) {
    updateLevelMark(0);
    return;
  }
  uint32_t actualLevel = LEVEL_EMPTY - level;
  uint32_t levelOneStep = (LEVEL_EMPTY - LEVEL_FULL) / 10;
  uint32_t currentLevelStep = actualLevel / levelOneStep;
  Serial.print("actualLevel: ");
  Serial.print(actualLevel);
  Serial.print("; levelOneStep: ");
  Serial.print(levelOneStep);
  Serial.print("; currentLevelStep: ");
  Serial.println(currentLevelStep);
  if (currentLevelStep > LED_COUNT) {
    updateLevelMark(LED_COUNT);
    return;
  }
  updateLevelMark(currentLevelStep);
}

void loop() {
  // LED output
  ledOut(currentLedMask);
  
  // Run sonar
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  mm = duration * 10 / 58;
  mm_sum += mm;

  // Average (for 10 measurements)
  if (++iter == 10) {
    mm = mm_sum / 10;
    mm_sum = 0;
    iter = 0;
    if (mm > mm_prev && skip_decrease < 5) { // water level decrease (measured leve increase)
      skip_decrease++;
    }
    else {
      mm_prev = mm;
      skip_decrease = 0;
      processLevel(mm);
    }
  }
  
  delay(100);
}
