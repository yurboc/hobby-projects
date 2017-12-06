/*
 * Simple timer with LCD, real-time clock and buzzer
 */

// Target: Arduino Pro Mini

#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include "custom_symbols.h"

// Hardware definitions
const int encA   = 2;  // Encoder out A
const int encB   = 3;  // Encoder out B
const int encKey = 4;  // Encoder key
const int ledPin = 13; // Led on board

// LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// RTC
RtcDS3231<TwoWire> Rtc(Wire);

void templateShowClock(int day, int month, int year, int hours, int minutes, int seconds, int temperature)
{
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
  lcd.write(temperature > 0 ? '+' : '-');
  lcd.write('0' + temperature/10);
  lcd.write('0' + temperature%10);
  lcd.write(3); // degree
  lcd.write('C');

  // Line 2: [    HH:MM:SS    ]
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
  lcd.print("    ");
}

void setup()
{
  // Serial port setup
  Serial.begin(57600);
  Serial.print("SimpleTimerLcd compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  
  // LCD setup
  lcd.begin(16, 2);
  
  // RTC setup
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  
  // GPIO setup
  pinMode(ledPin, OUTPUT);

  // Turn on the backlight
  lcd.backlight();
  
  // Load symbols
  lcd.createChar(0, bukva_Ya);
  lcd.createChar(1, bukva_IY);
  lcd.createChar(2, bukva_P);
  lcd.createChar(3, degree);
  lcd.createChar(4, check);
  lcd.createChar(5, check);
  lcd.createChar(6, check);
  lcd.createChar(7, check);
  
  // Print a message "TIME | TIMER"
  lcd.home();
  lcd.print("29.11.2017 00:53");
  lcd.setCursor(0, 1);
  lcd.print(" BPEM");
  lcd.write(0); // bukva_Ya
  lcd.print(" | TA");
  lcd.write(1); // bukva_IY
  lcd.print("MEP");
  delay(3000);
  
  // Print a message "START | STOP"
  lcd.clear();
  lcd.print("29.11.2017 00:53");
  lcd.setCursor(0, 1);
  lcd.print(" CTAPT  |  CTO");
  lcd.write(2); // bukva_P
  delay(3000);
  
  // Print something else.....
  
  // Turn off the blacklight
  lcd.noBacklight();
}

void loop()
{
  // TEST LED...
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  
  // TEST LCD...
  RtcDateTime now = Rtc.GetDateTime();
  templateShowClock(now.Day(), now.Month(), now.Year(), now.Hour(), now.Minute(), now.Second(), Rtc.GetTemperature().AsWholeDegrees());
}
