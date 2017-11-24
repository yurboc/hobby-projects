/* Tesla NIXIE controller

Device schematics copied from http://cxem.net/mc/mc206.php

There are 8 lamps. Each lamp can display digits 0..9
To control displayed values send packed data to UART.
Format: [ 7 6 5 4  | 3 2 1 0 ]
        [ LAMP_ID  |  DIGIT  ]
Each received byte describes one digit only.
Incorrect bytes silently drops.

NIXIE controller pin mapping

                     +--------------------------------+
                     |                                |
                 N/C | TXO   +----------------+   RAW | N/C
                     |       |                |       |
          FTDI ====> | RXI   |                |   GND | GND
                     |       |                |       |
                 N/C | RST   |                |   RST | N/C
                     |       |                |       |
                 GND | GND   |                |   VCC | VCC
                     |       |                |       |
  CATHODE_ADDR_BIT_2 | 2     |                |    A3 |
                     |       |                | A5    |
  CATHODE_ADDR_BIT_1 | 3     |                |    A2 |
                     |       |                | A4    |
  CATHODE_ADDR_BIT_0 | 4     |    Pro Mini    |    A1 |
                     |       |                |       |
             ANODE_0 | 5     |                |    A0 |
                     |       |                |       |
             ANODE_1 | 6     |                |    13 | CATHODE_ADDR_BIT_3
                     |       |                |       |
             ANODE_2 | 7     |                |    12 | ANODE_7
                     |       |                |       |
             ANODE_3 | 8     |                |    11 | ANODE_6
                     |       +----------------+       |
             ANODE_4 | 9                           10 | ANODE_5
                     |         GND    A6   A7         |
                     +--------------------------------+
*/


/* NIXIE anodes count: */
const int N_lamps = 8;

/* Constant for invalid lamp ID */
const int NO_LAMP = N_lamps;

/* NIXIE cathode address width: */
const int W_addr = 4;

/* Anodes: */
const int anodeEnablePin[N_lamps] = {5, 6, 7, 8, 9, 10, 11, 12};

/* Cathodes: */
const int cathodeAddressPin[W_addr] = {4, 3, 2, 13};

/* Map of digits 0-9 to addreses:  0  1  2  3  4  5  6  7  8  9 */
const int digitToAddressMap[10] = {5, 9, 8, 7, 1, 0, 2, 6, 4, 3};

/* Display values (digits to show): */
int dispDigits[] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  
  Serial.begin(9600);
  
  /* Prepare MPSA42 (anode) controller: */
  for (int i = 0; i < N_lamps; ++i)
    pinMode(anodeEnablePin[i], OUTPUT);
    
  /* Prepare K155ID1 (cathode) controller: */  
  for (int i = 0; i < W_addr; ++i)
    pinMode(cathodeAddressPin[i], OUTPUT);
}



/* Select lamp */
void selectAnode(int id) {
  
  /* Switch-off all lamps */
  for (int i = 0; i < N_lamps; ++i)
    digitalWrite(anodeEnablePin[i], LOW);
  
  /* Check lamp ID */
  if (0 > id || id > N_lamps)
    return;
    
  /* Switch-on selected lamp */
  digitalWrite(anodeEnablePin[id], HIGH);
}



/* Set address on K155ID1 */
void selectCathode(int addr) {
  
  /* Check limits: */
  if (0 > addr || addr > 9)
    return;
    
  /* Write address */
  for (int i = 0; i < W_addr; ++i)
    digitalWrite(cathodeAddressPin[i], (addr & (1 << i)) ? HIGH : LOW);
}



/* Set digit on NIXIE */
void selectDigit(int digit) {
  
  /* Check limits: */
  if (0 > digit || digit > 9)
    return;
    
  /* Set address using digit-to-address map */
  selectCathode(digitToAddressMap[digit]);
}



void loop() {
  
  /* Scan */
  for (int i = 0; i < N_lamps; ++i) {
    selectDigit(dispDigits[i]);
    selectAnode(i);
    delay(1);
    selectAnode(NO_LAMP);
    delay(1);
  }
}



/* New digit(s) from UART */
void serialEvent() {
  
  /* Read all bytes from serial port */
  while (Serial.available()) {
    
    /* Unpack data */
    char inChar = (char)Serial.read();
    char lamp = (inChar & 0xF0) >> 4;
    char digit = (inChar & 0x0F);
  
    /* Verify */
    if (lamp > N_lamps || digit > 9)
      continue;
    
    /* Store */
    dispDigits[lamp] = digit;
  }
}
