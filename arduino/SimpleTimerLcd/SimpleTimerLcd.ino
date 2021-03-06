/*
 * Simple timer with LCD, real-time clock and buzzer
 * 
 * Target: ATmega 8L at 8 MHz (w/o crystal)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include "custom_symbols.h"

//
// Prototypes
//
void setup();
void loop();
void readEncoder();
void updateByteUsingEncoderValue(int8_t &value, int8_t lower, int8_t upper);
void updateIntUsingEncoderValue(int &value, int lower, int upper);
void updateTimerTimeout();
void showTemplate();
void templateShowClock();
void templateShowTimer();
int nextMode();
void hwOn(uint8_t hwPin);
void hwOff(uint8_t hwPin);
bool hasUsb();
uint8_t battState();
void systemCheck();

//
// Types
//
enum ModeGlobal {
  ModeTesting = 0x0000,
  ModeClock   = 0x0080,
  ModeTimer   = 0x8000,
  ModeSetCurrentDay     = 0x0001,
  ModeSetCurrentMonth   = 0x0002,
  ModeSetCurrentYear    = 0x0004,
  ModeSetCurrentHours   = 0x0008,
  ModeSetCurrentMinutes = 0x0010,
  ModeSetCurrentSeconds = 0x0020,
  ModeSetTimerHours     = 0x0100,
  ModeSetTimerMinutes   = 0x0200,
  ModeSetTimerSeconds   = 0x0400,
  ModeTimerRun          = 0x0800,
};

//
// Hardware definitions
//
LiquidCrystal_I2C lcd(0x3F, 16, 2);
RtcDS3231<TwoWire> rtc(Wire);
const int encA   = 2;  // Encoder out A
const int encB   = 3;  // Encoder out B
const int encKey = 4;  // Encoder button
const int buzz = 12;   // Buzzer
const int ledR = 11;   // Red LED
const int ledY = 10;   // Yellow LED
const int ledG = 9;    // Green LED

//
// Software definitions
//
const char daysOfWeek[7][2] = {{'B','C'},{'\1','H'},{'B','T'},{'C','P'},{'\2','T'},{'\1','T'},{'C','\3'}};

//
// Global state
//
ModeGlobal currentMode = ModeTesting;
uint16_t batt_mV = 0;

//
// Button state (with debounce)
//
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
boolean enterSettings = false;

//
// Encoder states
//
volatile int encoder0Pos = 0;
volatile boolean PastA = 0;

//
// Current time (updates from RTC every 1 second)
//
int8_t seconds = 0;
int8_t minutes = 0;
int8_t hours   = 0;
int8_t dOfWeek = 0;
int8_t day     = 0;
int8_t month   = 0;
int year    = 0;
int temp    = 0;
unsigned long lastUpdated = 0;

//
// Current timer
//
int8_t init_timer_seconds = 0;
int8_t init_timer_minutes = 0;
int8_t init_timer_hours   = 0;
int8_t timer_seconds = 0;
int8_t timer_minutes = 0;
int8_t timer_hours   = 0;
int8_t lastRegisteredSecond = 0;
int8_t beepTime = 0;
int8_t timer_progress = 0;



void setup()
{
  // LCD setup
  lcd.begin();
  lcd.noBacklight();
  lcd.createChar(0, degree);
  lcd.createChar(1, bukva_P);
  lcd.createChar(2, bukva_CH);
  lcd.createChar(3, bukva_B);
  lcd.createChar(4, charge_empty);
  lcd.createChar(5, charge_usb);
  lcd.createChar(6, black_square);
  
  // RTC setup
  rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!rtc.IsDateTimeValid())
    rtc.SetDateTime(compiled);
  if (!rtc.GetIsRunning())
    rtc.SetIsRunning(true);
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  // GPIO setup
  //   not need

  // Setup Timer to read encoder
  cli();
  TCCR1A = 0;
  TCCR1B = (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TIMSK |= (1 << OCIE1A);
  OCR1A = 8191;
  sei();

  // System check
  //systemCheck();

  // Start working
  currentMode = ModeClock;
}

void loop()
{
  // Read encoder value
  readEncoder();

  // Detect button click
  int currentButtonState = digitalRead(encKey);
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = currentButtonState;
  }
  if ((millis() - lastDebounceTime) > 50 && (currentButtonState != buttonState)) {
    if (buttonState == LOW && currentButtonState == HIGH) {
      hwOn(buzz);
      delay(10);
      hwOff(buzz);
      currentMode = (ModeGlobal)nextMode();
    }
    buttonState = currentButtonState;
  }

  // Detect button hold
  if ((millis() - lastDebounceTime) > 1500 && (currentButtonState == LOW)) {
    if (!enterSettings) {
      hwOn(buzz);
      delay(50);
      hwOff(buzz);
    }
    enterSettings = true;
  }

  // Update state every 0.5 seconds
  if (millis() - lastUpdated > 500) {
    lastUpdated = millis();

    // Update current time and timer
    if (currentMode == ModeClock || currentMode == ModeTimer || currentMode == ModeTimerRun) {

      // Update time
      RtcDateTime now = rtc.GetDateTime();
      temp    = rtc.GetTemperature().AsWholeDegrees();
      dOfWeek = now.DayOfWeek();
      day     = now.Day();
      month   = now.Month();
      year    = now.Year();
      hours   = now.Hour();
      minutes = now.Minute();
      seconds = now.Second();

      // Update timer
      if (lastRegisteredSecond != seconds && currentMode == ModeTimerRun) {
        lastRegisteredSecond = seconds;
        updateTimerTimeout();

        // Timer pre-alert
        if (timer_hours == 0 && timer_minutes == 0 && (timer_seconds <= 30 && timer_seconds > 0)) {
          hwOn(ledY);
          delay(100);
          hwOff(ledY);
        }
      }

      // Timer beep
      if (beepTime > 0 && seconds % 2 == 0) {
        hwOn(ledY);
        hwOn(buzz); delay(100); hwOff(buzz); delay(50); hwOn(buzz); delay(100); hwOff(buzz);
        hwOff(ledY);

        if (--beepTime == 0)
          currentMode = ModeTimer;
      }
    }

    // Show template
    showTemplate();

    // Check health state
    if (hasUsb()) {
      if (currentMode == ModeTimerRun) hwOn(ledG);
      else hwOff(ledG);
    }
    else {
      hwOff(ledG);
      if (lcd.getBacklight()) lcd.noBacklight();
    }

    // Blink red LED if battery low
    if (battState() == 1) {
      hwOn(ledR); delay(50); hwOff(ledR); delay(10); hwOn(ledR); delay(50); hwOff(ledR);
    }

    // Change mode
    if (encoder0Pos > 3 && currentMode == ModeClock)
      currentMode = ModeTimer;
    else if (encoder0Pos < -3 && currentMode == ModeTimer)
      currentMode = ModeClock;
    encoder0Pos = 0;
  }
}



void readEncoder()
{
  if (encoder0Pos == 0) return;

  // Setup current time
  if (currentMode == ModeSetCurrentDay)          updateByteUsingEncoderValue(day, 1, 31);
  else if (currentMode == ModeSetCurrentMonth)   updateByteUsingEncoderValue(month, 1, 12);
  else if (currentMode == ModeSetCurrentYear)    updateIntUsingEncoderValue(year, 2000, 2100);
  else if (currentMode == ModeSetCurrentHours)   updateByteUsingEncoderValue(hours, 0, 24);
  else if (currentMode == ModeSetCurrentMinutes) updateByteUsingEncoderValue(minutes, 0, 59);
  else if (currentMode == ModeSetCurrentSeconds) updateByteUsingEncoderValue(seconds, 0, 59);

  // Setup timer
  else if (currentMode == ModeSetTimerHours)   updateByteUsingEncoderValue(timer_hours, 0, 99);
  else if (currentMode == ModeSetTimerMinutes) updateByteUsingEncoderValue(timer_minutes, 0, 59);
  else if (currentMode == ModeSetTimerSeconds) updateByteUsingEncoderValue(timer_seconds, 0, 59);
}

void updateByteUsingEncoderValue(int8_t &value, int8_t lower, int8_t upper)
{
  value += encoder0Pos; // actually abs(encoder0Pos) always less 127
  if (value > upper) value = upper;
  else if (value < lower) value = lower;
  encoder0Pos = 0;
}

void updateIntUsingEncoderValue(int &value, int lower, int upper)
{
  value += encoder0Pos;
  if (value > upper) value = upper;
  else if (value < lower) value = lower;
  encoder0Pos = 0;
}

void updateTimerTimeout()
{
  // Timer timeout
  if (!timer_hours && !timer_minutes && !timer_seconds && currentMode == ModeTimerRun && beepTime == 0) {
    beepTime = 31; // 30 seconds plus 1 signal
    return;
  }

  // Calculate timer progress
  uint32_t total_time_sec = init_timer_seconds + 60*init_timer_minutes + 60*60*init_timer_hours;
  uint32_t current_time_sec = timer_seconds + 60*timer_minutes + 60*60*timer_hours;
  timer_progress = (current_time_sec * 16) / total_time_sec;

  // Decrease timer counter
  if (timer_seconds > 0) {
    --timer_seconds;
    return;
  } else if (timer_minutes > 0) {
    --timer_minutes;
    timer_seconds = 59;
    return;
  } else if (timer_hours > 0) {
    --timer_hours;
    timer_minutes = 59;
    timer_seconds = 59;
    return;
  }
}

void showTemplate()
{
  if (currentMode & 0x00FF) templateShowClock();
  if (currentMode & 0xFF00) templateShowTimer();
}

void templateShowClock()
{
  // Line 1: [DD.MM.YYYY +##*C]
  lcd.home();
  
  if (enterSettings && currentMode != ModeSetCurrentDay) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + day/10);
    lcd.write('0' + day%10);
  }
  lcd.write('.');
  if (enterSettings && currentMode != ModeSetCurrentMonth) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + month/10);
    lcd.write('0' + month%10);
  }
  lcd.write('.');
  if (enterSettings && currentMode != ModeSetCurrentYear) {
    lcd.write('_');
    lcd.write('_');
    lcd.write('_');
    lcd.write('_'); }
  else {
    lcd.write('0' + year/1000%10);
    lcd.write('0' + year/100%10);
    lcd.write('0' + year/10%10);
    lcd.write('0' + year%10);
  }
  
  if (!enterSettings) {
    lcd.write(' ');
    lcd.write(temp > 0 ? '+' : '-');
    lcd.write('0' + temp/10);
    lcd.write('0' + temp%10);
    lcd.write(0); // degree
    lcd.write('C');
  }
  else {
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(' ');
  }

  // Line 2: [DD  HH:MM:SS  uc]
  lcd.setCursor(0, 1);

  lcd.write(enterSettings ? ' ' : daysOfWeek[dOfWeek][0]);
  lcd.write(enterSettings ? ' ' : daysOfWeek[dOfWeek][1]);
  lcd.write(' ');
  lcd.write(' ');

  if (enterSettings && currentMode != ModeSetCurrentHours) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + hours/10);
    lcd.write('0' + hours%10);
  }
  lcd.write(':');
  if (enterSettings && currentMode != ModeSetCurrentMinutes) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + minutes/10);
    lcd.write('0' + minutes%10);
  }
  lcd.write(':');
  if (enterSettings && currentMode != ModeSetCurrentSeconds) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + seconds/10);
    lcd.write('0' + seconds%10);
  }
  lcd.write(' ');
  lcd.write(' ');

  // Print battery status
  uint8_t batt = battState(); // 0 or 1..4
  if (batt > 0) lcd.write(4); // charge_empty
  else lcd.write(' ');        // no battery
  if (hasUsb()) lcd.write(5); // charge_usb
  else lcd.write(' ');        // no usb
}

void templateShowTimer()
{
  // Line 1: [################]
  lcd.home();
  for (uint8_t i = 16; i > 0; --i) {
    if (currentMode == ModeTimerRun) {
      if (timer_progress < i) lcd.write('#');
      else lcd.write('>');
    }
    else lcd.write(6); // black_square
  }

  // Line 2: [    HH:MM:SS    ]
  lcd.setCursor(0, 1);
  lcd.print("    ");

  if (enterSettings && currentMode != ModeSetTimerHours) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + timer_hours/10);
    lcd.write('0' + timer_hours%10);
  }
  lcd.write(':');
  if (enterSettings && currentMode != ModeSetTimerMinutes) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + timer_minutes/10);
    lcd.write('0' + timer_minutes%10);
  }
  lcd.write(':');
  if (enterSettings && currentMode != ModeSetTimerSeconds) {
    lcd.write('_');
    lcd.write('_');
  }
  else {
    lcd.write('0' + timer_seconds/10);
    lcd.write('0' + timer_seconds%10);
  }
  lcd.print("  ");

  // Print battery status
  uint8_t batt = battState(); // 0 or 1..4
  if (batt < 2) lcd.write(' '); // no battery or damaged
  else lcd.write(4); // empty/half/full
  lcd.write(hasUsb() ? 5 : ' '); // charge_usb
}

int nextMode()
{
  switch (currentMode) {
    case ModeClock: {
      if (enterSettings) return ModeSetCurrentDay;
      if (lcd.getBacklight()) {
        lcd.noBacklight();
      }
      else if (hasUsb()) {
        lcd.backlight();
      }
      return ModeClock;
    }
    case ModeTimer: {
      if (enterSettings) return ModeSetTimerHours;
      if (!timer_hours && !timer_minutes && !timer_seconds)
        return ModeTimer;
      else
        return ModeTimerRun;
    }
    case ModeTimerRun: {
      if (enterSettings) {
        timer_seconds = init_timer_seconds;
        timer_minutes = init_timer_minutes;
        timer_hours   = init_timer_hours;
        enterSettings = false;
      }
      beepTime = 0;
      return ModeTimer;
    }
    
    case ModeSetCurrentDay: return ModeSetCurrentMonth;
    case ModeSetCurrentMonth: return ModeSetCurrentYear;
    case ModeSetCurrentYear: return ModeSetCurrentHours;
    case ModeSetCurrentHours: return ModeSetCurrentMinutes;
    case ModeSetCurrentMinutes: return ModeSetCurrentSeconds;
    case ModeSetCurrentSeconds: {
      // Save RTC
      RtcDateTime newDateTime = RtcDateTime(year, month, day, hours, minutes, seconds);
      rtc.SetDateTime(newDateTime);
      enterSettings = false;
      return ModeClock;
    }

    case ModeSetTimerHours: return ModeSetTimerMinutes;
    case ModeSetTimerMinutes: return ModeSetTimerSeconds;
    case ModeSetTimerSeconds: {
      // Save Timer settings
      init_timer_seconds = timer_seconds;
      init_timer_minutes = timer_minutes;
      init_timer_hours   = timer_hours;
      enterSettings = false;
      return ModeTimer;
    }
    
    default: return ModeClock;
  }
}



void hwOn(uint8_t hwPin)
{
  digitalWrite(hwPin, LOW);
  pinMode(hwPin, OUTPUT);
}

void hwOff(uint8_t hwPin)
{
  pinMode(hwPin, INPUT);
  digitalWrite(hwPin, HIGH);
}

bool hasUsb()
{
  int usbVal = analogRead(A2);
  return (usbVal > 512);
}

uint8_t battState()
{
  // 1. Read reference (1.23V)
  
  // REFS1 --> select reference: 0 = AVCC; 1 = internal 2.56V reference
  // REFS0 --> connect reference to AREF: 0 = diconnect; 1 = connect
  // MUX   --> select analog input: 0111 = 1.23V (Vbg)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADSC); // start ADC
  while (ADCSRA & _BV(ADSC)); // wait for ADC finished
  uint16_t result_ADC = ADCL; // read ADCL (lock ADCH)
  result_ADC += ADCH << 8;    // read ADCH (unlock both)
  uint32_t vcc_mV = 1230ull * 1023ull / (uint32_t)result_ADC; 
  //float vcc_mV = (1.23 * 1000.0 * 1023.0) / result_ADC; // 1.23 * 1000mV * max_ADC / result_ADC
  
  // 2. Read battery voltage
  
  batt_mV = (vcc_mV * (uint32_t)analogRead(A3)) / 1023ull;
  if (batt_mV > 3500) // ~3500 mV
    return 4;
  else if (batt_mV > 3300) // ~3300 mV
    return 3;
  else if (batt_mV > 3000) // ~3000 mV
    return 2;
  else if (batt_mV > 1000) // ~1000 mV
    return 1;
  else // no battery
    return 0;
}



void systemCheck()
{
  // Load symbols
  lcd.createChar(0, bukva_P);
  lcd.createChar(1, bukva_I);
  lcd.createChar(2, bukva_IYI);
  lcd.createChar(3, bukva_Z);
  lcd.createChar(4, bukva_U);

  // Print a message "SYSTEM CHECK"
  lcd.home();
  lcd.write(0); // bukva_P
  lcd.print("POBEPKA C");
  lcd.write(1); // bukva_I
  lcd.print("CTEM");
  lcd.write(2); // bukva_IYI
  delay(100);
  
  // Check buzzer
  lcd.setCursor(0, 1);
  lcd.print("   ");
  lcd.write(3); // bukva_Z
  lcd.print("B");
  lcd.write(4); // bukva_U
  lcd.print("K ");
  delay(100);
  hwOn(buzz);
  delay(100);
  hwOff(buzz);

  // Check LEDs
  lcd.setCursor(0, 1);
  delay(200);
  
  // Red
  lcd.setCursor(0, 1);
  lcd.print(" CBET ...");
  hwOn(ledR);
  delay(350);
  hwOff(ledR);

  // Yellow
  lcd.print("...");
  hwOn(ledY);
  delay(350);
  hwOff(ledY);

  // Green
  lcd.print("...");
  hwOn(ledG);
  delay(350);
  hwOff(ledG);

  // Done
  lcd.setCursor(0, 1);
  lcd.print(" ===== OK ===== ");
  delay(500);
  lcd.clear();
}



//
// ===== ISR =====
//
ISR(TIMER1_COMPA_vect)
{
  /* Read encoder */
  boolean nowA = (boolean)digitalRead(encA);
  boolean nowB = (boolean)digitalRead(encB);
  if (!PastA && nowA) {
    if (!nowB)
      encoder0Pos--;
    else
      encoder0Pos++;
  }
  PastA = nowA;
}
