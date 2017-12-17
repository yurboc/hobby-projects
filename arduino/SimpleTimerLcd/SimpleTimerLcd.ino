/*
 * Simple timer with LCD, real-time clock and buzzer
 * 
 * Target: ATmega 8L at 8 MHz (w/o crystal)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include "custom_symbols.h"

enum ModeGlobal {
  ModeTesting,
  ModeClock,
  ModeTimer,
  ModeSetup
};

// Hardware definitions
LiquidCrystal_I2C lcd(0x3F, 16, 2);
RtcDS3231<TwoWire> rtc(Wire);
const int encA   = 2;  // Encoder out A
const int encB   = 3;  // Encoder out B
const int encKey = 4;  // Encoder button
const int buzzer = 12;     // Buzzer
const int ledRed = 11;     // Red LED
const int ledYellow = 10;  // Yellow LED
const int ledGreen = 9;    // Green LED

// Global state
ModeGlobal currentMode = ModeTesting;
unsigned long lastUpdated = 0;

void setup()
{
  // LCD setup
  lcd.begin();
  lcd.noBacklight();
  
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
  digitalWrite(buzzer,    LOW);
  digitalWrite(ledRed,    LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen,  LOW);
  pinMode(buzzer,    INPUT);
  pinMode(ledRed,    INPUT);
  pinMode(ledYellow, INPUT);
  pinMode(ledGreen,  INPUT);

  // System check
  systemCheck();

  // Start working
  currentMode = ModeClock;
}

void loop()
{
  // Show clock
  if (currentMode == ModeClock) {
    if (millis() - lastUpdated < 500)
      return;
    
    templateShowClock();
    lastUpdated = millis();
  }

  // Show timer
  if (currentMode == ModeTimer) {
    if (millis() - lastUpdated < 500)
      return;
    
    templateShowTimer();
    lastUpdated = millis();
  }

  // Read encoder
}

void systemCheck()
{
  // Load symbols
  lcd.createChar(0, bukva_P);
  lcd.createChar(1, bukva_I);
  lcd.createChar(2, bukva_IYI);
  lcd.createChar(3, bukva_Z);
  lcd.createChar(4, bukva_U);
  lcd.createChar(5, check);
  lcd.createChar(6, check);
  lcd.createChar(7, check);

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
  beepOn();
  lcd.print("....... ");
  delay(100);
  beepOff();

  // Check LEDs
  lcd.setCursor(0, 1);
  lcd.print("                ");
  delay(200);
  
  // Red
  lcd.setCursor(0, 1);
  lcd.print(" CBET ...");
  redOn();
  delay(350);
  redOff();

  // Yellow
  lcd.print("...");
  yellowOn();
  delay(350);
  yellowOff();

  // Green
  lcd.print("...");
  greenOn();
  delay(350);
  greenOff();

  // Done
  lcd.setCursor(0, 1);
  lcd.print(" ===== OK ===== ");
  delay(500);
  lcd.clear();
}

void templateShowClock()
{
  // Load symbols
  lcd.createChar(0, degree);
  lcd.createChar(1, charge_damage);
  lcd.createChar(2, charge_empty);
  lcd.createChar(3, charge_half);
  lcd.createChar(4, charge_full);
  lcd.createChar(5, charge_usb);
  lcd.createChar(6, check);
  lcd.createChar(7, check);

  // Read values
  RtcDateTime now = rtc.GetDateTime();
  int temp    = rtc.GetTemperature().AsWholeDegrees();
  int day     = now.Day();
  int month   = now.Month();
  int year    = now.Year();
  int hours   = now.Hour();
  int minutes = now.Minute();
  int seconds = now.Second();
  
  // Line 1: [DD.MM.YYYY +##*C]
  lcd.home();
  lcd.write('0' + day/10);
  lcd.write('0' + day%10);
  lcd.write('.');
  lcd.write('0' + month/10);
  lcd.write('0' + month%10);
  lcd.write('.');
  lcd.write('0' + year/1000%10);
  lcd.write('0' + year/100%10);
  lcd.write('0' + year/10%10);
  lcd.write('0' + year%10);
  lcd.write(' ');
  lcd.write(temp > 0 ? '+' : '-');
  lcd.write('0' + temp/10);
  lcd.write('0' + temp%10);
  lcd.write(0); // degree
  lcd.write('C');

  // Line 2: [    HH:MM:SS  uc]
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.write('0' + hours/10);
  lcd.write('0' + hours%10);
  lcd.write(':');
  lcd.write('0' + minutes/10);
  lcd.write('0' + minutes%10);
  lcd.write(':');
  lcd.write('0' + seconds/10);
  lcd.write('0' + seconds%10);
  lcd.print("  ");

  // Print battery status
  lcd.write(hasUsb() ? 5 : ' '); // charge_usb
  int batt = battState(); // 0 or 1..4
  if (batt == 0) lcd.write(' '); // none
  else lcd.write(batt); // damage/empty/half/full
}

void templateShowTimer()
{
  // Load symbols
  lcd.createChar(0, degree);
  lcd.createChar(1, charge_damage);
  lcd.createChar(2, charge_empty);
  lcd.createChar(3, charge_half);
  lcd.createChar(4, charge_full);
  lcd.createChar(5, charge_usb);
  lcd.createChar(6, check);
  lcd.createChar(7, check);

  // Read values
  int hours   = 0;
  int minutes = 0;
  int seconds = 0;

  // Line 1: [################]
  lcd.home();
  lcd.print("################");

  // Line 2: [    HH:MM:SS  uc]
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.write('0' + hours/10);
  lcd.write('0' + hours%10);
  lcd.write(':');
  lcd.write('0' + minutes/10);
  lcd.write('0' + minutes%10);
  lcd.write(':');
  lcd.write('0' + seconds/10);
  lcd.write('0' + seconds%10);
  lcd.print("  ");

  // Print battery status
  lcd.write(hasUsb() ? 5 : ' '); // charge_usb
  int batt = battState(); // 0 or 1..4
  if (batt == 0) lcd.write(' '); // none
  else lcd.write(batt); // damage/empty/half/full
}



void beepOn()
{
  digitalWrite(buzzer, LOW);
  pinMode(buzzer, OUTPUT);
}

void beepOff()
{
  pinMode(buzzer, INPUT);
  digitalWrite(buzzer, HIGH);
}

void redOn()
{
  digitalWrite(ledRed, LOW);
  pinMode(ledRed, OUTPUT);
}

void redOff()
{
  pinMode(ledRed, INPUT);
  digitalWrite(ledRed, HIGH);
}

void greenOn()
{
  digitalWrite(ledGreen, LOW);
  pinMode(ledGreen, OUTPUT);
}

void greenOff()
{
  pinMode(ledGreen, INPUT);
  digitalWrite(ledGreen, HIGH);
}

void yellowOn()
{
  digitalWrite(ledYellow, LOW);
  pinMode(ledYellow, OUTPUT);
}

void yellowOff()
{
  pinMode(ledYellow, INPUT);
  digitalWrite(ledYellow, HIGH);
}

bool hasUsb()
{
  int usbVal = analogRead(A2);
  return (usbVal > 512);
}

int battState()
{
  int battVal = analogRead(A3);
  
  if (battVal > 833) // ~3.5V
    return 4;
  else if (battVal > 785) // ~3.3V
    return 3;
  else if (battVal > 714) // ~3.0V
    return 2;
  else if (battVal > 238) // ~1.0V
    return 1;
  else // no battery
    return 0;
}

