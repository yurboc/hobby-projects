/*
 * Based on "Tesla Time Machine"
 * https://bitbucket.org/yurboc/phobia/src/741c9fb6cfd5dcdb89090960fd74d2fd34c8dae3/tesla/arduino/TeslaNixieDisplay/TeslaNixieDisplay.ino
 * (without UART part)
 */

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

//#define DEBUG

/* ===== CONSTANTS ===== */

/* NIXIE anodes count: */
const int N_lamps = 5;

/* Constant for invalid lamp ID */
const int NO_LAMP = N_lamps;

/* NIXIE cathode address width: */
const int W_addr = 4;

/* ===== HARDWARE ===== */

/* Anodes: */
const int anodeEnablePin[N_lamps] = { 6, 5, 11, 12, 4 };

/* Cathodes: */
const int cathodeAddressPin[W_addr] = { 10, 8, 7, 9 };

/* Encoder */
const int encA = 3;
const int encB = 2;
const int encKey = A3;
const int ledPin = 13;

/* ===== VARIABLES ===== */

/* Map digits 0-9 to addreses { 0  1  2  3  4  5  6  7  8  9 } */
const int kathodesMap[10] =   { 5, 9, 6, 7, 0, 8, 1, 4, 2, 3 };

/* Display values (digits to show): */
int dispDigits[] = { 0, 0, 0, 0, 0 };

/* Enable lamps: */
int enableAnodes[] = { 1, 1, 1, 1, 1 };

/* Timer to read values from RTC */
unsigned long last_update_timestamp = 0;

/* Display modes */
enum display_mode_t {
  MODE_NONE,
  MODE_DIAG,
  MODE_SHOW_TIME,
  MODE_SHOW_SECONDS,
  MODE_SET_HOURS,
  MODE_SET_MINUTES,
  MODE_SET_WAIT,
  MODE_BRIGHTNESS,
  MODE_RECOVERY_CATHODES
};

/* Work stages */
enum work_stage_t {
  STAGE_DIAG,
  STAGE_WORK
};

/* Render options */
struct render_time_t {
  int lightTimeUs;
  int darkTimeUs;
};

/* Day periods */
enum day_period_t {
  PERIOD_NIGHT, // 21:00 -- 09:00
  PERIOD_DAY,   // 09:00 -- 21:00
  
  NR_PERIODS
};

/* All render options */
render_time_t render_times[NR_PERIODS];

/* Current render options */
int lightTimeUs = 500;
int darkTimeUs  = 500;

/* Setup brightness */
int brightnessCurrentMode = PERIOD_NIGHT;
int brightnessCurrentValue = 0;

/* Current display mode */
display_mode_t mode_current = MODE_NONE;
display_mode_t mode_prev = MODE_NONE;

/* Current work stage */
work_stage_t stage_current = STAGE_DIAG;

/* Current time */
int seconds = 0;
int minutes = 0;
int hours = 0;

/* Mode timer */
unsigned long mode_enter_timestamp = 0;

/* Recover kathode */
boolean kathodesRecovered = true;

/* Button states */
int buttonState = HIGH;
int lastButtonState = HIGH;
boolean enterSettings = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

/* Encoder states */
volatile int encoder0Pos = 0;
volatile boolean PastA = 0;
volatile boolean PastB = 0;

/* Encoder states */
long encoderValue = 0;


/* ===== FUNCTIONS ===== */

/* Select lamp */
void setAnode(int id)
{
  /* Switch-off all lamps */
  for (int i = 0; i < N_lamps; ++i)
    digitalWrite(anodeEnablePin[i], LOW);
  
  /* Check lamp ID */
  if (id < 0 || id >= N_lamps)
    return;
  
  /* Switch-on selected lamp */
  digitalWrite(anodeEnablePin[id], HIGH);
}

/* Select digit */
void setKathode(int digit)
{
  /* Check limits: */
  if (digit < 0 || digit > 9)
    return;
  
  /* Write address */
  for (int i = 0; i < W_addr; ++i) {
    int kathodeAddr = kathodesMap[digit];
    digitalWrite(cathodeAddressPin[i], (kathodeAddr & (1 << i)) ? HIGH : LOW);
  }
}

/* Update display */
void rendering()
{
  /* Scan digits */
  for (int i = 0; i < N_lamps-1; ++i) {
    setKathode(dispDigits[i]);
    if (enableAnodes[i]) setAnode(i);
    delayMicroseconds(lightTimeUs);
    setAnode(NO_LAMP);
    delayMicroseconds(darkTimeUs);
  }
  
  /* Scan points: 1 - bottom, 2 - top, 3 - both */
  int pointTop = !!(dispDigits[4] & 0x01);
  int pointBottom = !!(dispDigits[4] & 0x02);
  
  setKathode(pointTop ? 9 : 1);
  if (enableAnodes[4] && pointTop) setAnode(4);
  delayMicroseconds(lightTimeUs);
  setAnode(NO_LAMP);
  delayMicroseconds(darkTimeUs);
  
  setKathode(pointBottom ? 8 : 1);
  if (enableAnodes[4] && pointBottom) setAnode(4);
  delayMicroseconds(lightTimeUs);
  setAnode(NO_LAMP);
  delayMicroseconds(darkTimeUs);
}

/* Select stage */
void setStage(int stage)
{
  stage_current = (work_stage_t)stage;
}

/* Select mode */
void setMode(int mode)
{
  mode_enter_timestamp = millis();
  mode_current = (display_mode_t)mode;
}

/* Get time in mode */
unsigned long modeTimeMs()
{
  return (millis() - mode_enter_timestamp);
}

/* Update values */
void processing()
{
  unsigned long mode_current_time = modeTimeMs();
  
  switch (mode_current) {
    case MODE_NONE: {
      enableAnodes[0] = 0;
      enableAnodes[1] = 0;
      enableAnodes[2] = 0;
      enableAnodes[3] = 0;
      enableAnodes[4] = 0;
      break;
    }
    
    case MODE_DIAG: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      enableAnodes[4] = 1;
      
      if (mode_current_time < 10000) {
        int digit = (mode_current_time / 1000) % 10;
        dispDigits[0] = digit;
        dispDigits[1] = digit;
        dispDigits[2] = digit;
        dispDigits[3] = digit;
        dispDigits[4] = 3;
      }
      else {
        int digit = (mode_current_time / 1000) % 10;
        dispDigits[0] = (digit+4) % 10;
        dispDigits[1] = (digit+3) % 10;
        dispDigits[2] = (digit+2) % 10;
        dispDigits[3] = (digit+1) % 10;
        dispDigits[4] = digit % 4;
      }
      break;
    }
    
    case MODE_SHOW_TIME: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      if (!enterSettings)
        enableAnodes[4] = (seconds % 2 == 0) ? 1 : 0;
      else
        enableAnodes[4] = (mode_current_time / 50) % 2 ? 1 : 0;
      dispDigits[0] = minutes % 10;
      dispDigits[1] = minutes / 10;
      dispDigits[2] = hours % 10;
      dispDigits[3] = hours / 10;
      dispDigits[4] = 3;
      break;
    }
    
    case MODE_SHOW_SECONDS: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      if (!enterSettings)
        enableAnodes[4] = (mode_current_time % 1000 < 500) ? 1 : 0;
      else
        enableAnodes[4] = (mode_current_time / 50) % 2 ? 1 : 0;
      dispDigits[0] = seconds % 10;
      dispDigits[1] = seconds / 10;
      dispDigits[2] = minutes % 10;
      dispDigits[3] = minutes / 10;
      dispDigits[4] = 2;
      break;
    }
    
    case MODE_SET_HOURS: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = (mode_current_time % 500 < 300) ? 1 : 0;
      enableAnodes[3] = (mode_current_time % 500 < 300) ? 1 : 0;
      enableAnodes[4] = 1;
      dispDigits[0] = minutes % 10;
      dispDigits[1] = minutes / 10;
      dispDigits[2] = hours % 10;
      dispDigits[3] = hours / 10;
      dispDigits[4] = 1;
      break;
    }
    
    case MODE_SET_MINUTES: {
      enableAnodes[0] = (mode_current_time % 500 < 300) ? 1 : 0;
      enableAnodes[1] = (mode_current_time % 500 < 300) ? 1 : 0;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      enableAnodes[4] = 1;
      dispDigits[0] = minutes % 10;
      dispDigits[1] = minutes / 10;
      dispDigits[2] = hours % 10;
      dispDigits[3] = hours / 10;
      dispDigits[4] = 1;
      break;
    }
    
    case MODE_SET_WAIT: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      enableAnodes[4] = 1;
      dispDigits[0] = minutes % 10;
      dispDigits[1] = minutes / 10;
      dispDigits[2] = hours % 10;
      dispDigits[3] = hours / 10;
      dispDigits[4] = 3;
      break;
    }
    
    case MODE_BRIGHTNESS: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 0;
      enableAnodes[3] = (mode_current_time % 500 < 300) ? 1 : 0;
      enableAnodes[4] = 0;
      dispDigits[0] = brightnessCurrentValue % 10;
      dispDigits[1] = brightnessCurrentValue / 10;
      dispDigits[3] = brightnessCurrentMode % 10;
      break;
    }
    
    case MODE_RECOVERY_CATHODES: {
      enableAnodes[0] = 1;
      enableAnodes[1] = 1;
      enableAnodes[2] = 1;
      enableAnodes[3] = 1;
      enableAnodes[4] = 0;
      dispDigits[0] = dispDigits[1] = dispDigits[2] = dispDigits[3] = (mode_current_time / 10) % 10;
      break;
    }
  }
}

/* Select next mode */
void nextMode()
{
  switch (mode_current) {
    case MODE_NONE: {
      break;
    }
    case MODE_DIAG: {
      setStage(STAGE_WORK);
      setMode(MODE_SHOW_TIME);
      break;
    }
    case MODE_SHOW_TIME: {
      if (enterSettings) {
        mode_prev = mode_current;
        setMode(MODE_SET_HOURS);
      }
      else setMode(MODE_SHOW_SECONDS);
      break;
    }
    case MODE_SHOW_SECONDS: {
      if (enterSettings) {
        mode_prev = mode_current;
        setMode(MODE_SET_HOURS);
      }
      else setMode(MODE_SHOW_TIME);
      break;
    }
    case MODE_SET_HOURS: {
      setMode(MODE_SET_MINUTES);
      break;
    }
    case MODE_SET_MINUTES: {
      setMode(MODE_SET_WAIT);
      break;
    }
    case MODE_SET_WAIT: {
      /* Set new date and time */
      RtcDateTime newDateTime = RtcDateTime(2017, 11, 24, hours, minutes, 0);
      /* Apply settings */
      Rtc.SetDateTime(newDateTime);

      enterSettings = false;
      setMode(mode_prev);
      break;
    }
    case MODE_BRIGHTNESS: {
      brightnessCurrentMode = (brightnessCurrentMode == PERIOD_NIGHT) ? PERIOD_DAY : PERIOD_NIGHT;
      brightnessCurrentValue = render_times[brightnessCurrentMode].lightTimeUs / 10;
      break;
    }
  }
}


/* ========== SETUP ========== */

void setup()
{
  Serial.begin(57600);

  Serial.print("NixieForTanya compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  
  /* RTC SETUP */
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

  /* Prepare MPSA42 (anode) controller: */
  for (int i = 0; i < N_lamps; ++i)
    pinMode(anodeEnablePin[i], OUTPUT);
    
  /* Prepare K155ID1 (cathode) controller: */  
  for (int i = 0; i < W_addr; ++i)
    pinMode(cathodeAddressPin[i], OUTPUT);
    
  /* Prepare GPIO */
  pinMode(encKey, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  
  /* Prepare Encoder */
  PastA = (boolean)digitalRead(encA);
  PastB = (boolean)digitalRead(encB);
  
  /* Setup Timer */
  cli(); //stop interrupts
  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR2B |= (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();//allow interrupts
    
  /* Set default brightness */
  render_times[PERIOD_NIGHT].lightTimeUs = 500;
  render_times[PERIOD_NIGHT].darkTimeUs = 500;
  render_times[PERIOD_DAY].lightTimeUs = 500;
  render_times[PERIOD_DAY].darkTimeUs = 500;
  
  /* Set diagnostics mode (Release) or show time (Debug) */
  
  #ifndef DEBUG
    setStage(STAGE_DIAG);
    setMode(MODE_DIAG);
  #else
    setStage(STAGE_WORK);
    setMode(MODE_SHOW_TIME);
  #endif
}

/* ========== LOOP ========== */

void loop()
{
  /* Set digits */
  processing();
  
  /* Display digits */
  rendering();
  
  /* Switch mode: DIAG --> WORK */
  if (stage_current == STAGE_DIAG && mode_current == MODE_DIAG && modeTimeMs() > 20000) {
    setMode(MODE_NONE);
  }
  else if (stage_current == STAGE_DIAG && mode_current == MODE_NONE && modeTimeMs() > 700) {
    setStage(STAGE_WORK);
    setMode(MODE_SHOW_TIME);
    encoder0Pos = 0;
  }
  else if (stage_current == STAGE_DIAG && digitalRead(encKey) == LOW) {
    setStage(STAGE_WORK);
    setMode(MODE_SHOW_TIME);
    encoder0Pos = 0; 
  }
  
  /* Exit diag mode stage */
  if (stage_current != STAGE_WORK) return;
  
  /* Switch mode: BRIGHTNESS --> TIME */
  if ((mode_current == MODE_BRIGHTNESS) && (modeTimeMs() > 5000)) {
    setMode(mode_prev);
  }
  
  /* Update brightness for setup */
  if (mode_current == MODE_BRIGHTNESS) {
    lightTimeUs = brightnessCurrentValue * 10;
    darkTimeUs = (100 - brightnessCurrentValue) * 10;
  }
  
  /* Update HH:MM */
  if ((mode_current == MODE_SHOW_TIME || mode_current == MODE_SHOW_SECONDS) && modeTimeMs() > 1000) {
    setMode(mode_current); // reset timer

    char datestring[20];
    RtcDateTime now = Rtc.GetDateTime();
    snprintf_P(datestring,
            sizeof(datestring) / sizeof(datestring[0]),
            PSTR("%02u.%02u.%04u %02u:%02u:%02u"),
            now.Day(),
            now.Month(),
            now.Year(),
            now.Hour(),
            now.Minute(),
            now.Second() );
    Serial.print(datestring);
    Serial.println();

    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(temp.AsFloat());
    Serial.println("C");

    seconds = now.Second();
    minutes = now.Minute();
    hours = now.Hour();
    
    /* Set brightness */
    if (hours >= 9 && hours < 21) {
      lightTimeUs = render_times[PERIOD_DAY].lightTimeUs;
      darkTimeUs = render_times[PERIOD_DAY].darkTimeUs;
    }
    else {
      lightTimeUs = render_times[PERIOD_NIGHT].lightTimeUs;
      darkTimeUs = render_times[PERIOD_NIGHT].darkTimeUs;
    }
    
    /* Start kathode recovering every 10 minutes */
    if (minutes % 10 == 0) {
      if (!kathodesRecovered) {
        mode_prev = mode_current;
        setMode(MODE_RECOVERY_CATHODES);
        kathodesRecovered = true;
      }
    }
    else {
      kathodesRecovered = false;
    }
  }
  
  /* Stop kathodes recovering */
  if ((mode_current == MODE_RECOVERY_CATHODES) && (modeTimeMs() > 300)) {
    setMode(mode_prev);
  }
  
  /* Read button */
  int reading = digitalRead(encKey);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = reading;
  }
  if ((millis() - lastDebounceTime) > debounceDelay && (reading != buttonState)) {
    if (buttonState == LOW && reading == HIGH) {
      nextMode();
    }
    buttonState = reading;
    //digitalWrite(ledPin, buttonState);
  }
  if ((millis() - lastDebounceTime) > 1500 && (reading == LOW)) {
    enterSettings = true;
  }
  
  /* Process encoder value */
  if (encoder0Pos == 0) return;
  
  if (mode_current == MODE_SET_HOURS) {
    encoder0Pos %= 24;
    hours += encoder0Pos;
    if (hours >= 24) hours -= 24;
    else if (hours < 0) hours = 24 + hours;
    encoder0Pos = 0;
  }
  else if (mode_current == MODE_SET_MINUTES) {
    encoder0Pos %= 60;
    minutes += encoder0Pos;
    if (minutes >= 60) minutes -= 60;
    else if (minutes < 0) minutes = 60 + minutes;
    encoder0Pos = 0;
  }
  else if (mode_current == MODE_SHOW_TIME || mode_current == MODE_SHOW_SECONDS) {
    mode_prev = mode_current;
    setMode(MODE_BRIGHTNESS);
    
    brightnessCurrentMode = (hours >= 9 && hours < 21) ? PERIOD_DAY : PERIOD_NIGHT;
    brightnessCurrentValue = render_times[brightnessCurrentMode].lightTimeUs / 10;
  }
  else if (mode_current == MODE_BRIGHTNESS) {
    setMode(MODE_BRIGHTNESS); // reset mode timer

    brightnessCurrentValue += encoder0Pos;
    if (brightnessCurrentValue > 95) brightnessCurrentValue = 95;
    else if (brightnessCurrentValue < 5) brightnessCurrentValue = 5;
    encoder0Pos = 0;

    render_times[brightnessCurrentMode].lightTimeUs = brightnessCurrentValue * 10;
    render_times[brightnessCurrentMode].darkTimeUs = (100 - brightnessCurrentValue) * 10;
  }
}

ISR(TIMER2_COMPA_vect) // timer2 interrupt
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
  PastB = nowB;
}
