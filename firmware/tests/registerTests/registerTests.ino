/* This test reads back the default values of the register, 
   makes sure the setup is done correctly. 
   It reads the registers back, and sends them to the serial viewer. 
   
   Expected output:
   STAT 0x30 MUX  0x 1 CON  0x20 RATE 0xf0 IO   0xe1
   STAT 0x31 MUX  0x 1 CON  0x20 RATE 0xf0 IO   0xe1

   (the change in the Status buffer indicates that the !DRDY pin has gone high, 
    which happens occasionaly.)
   
   To watch on a logic analyzer, hook up the logic analyzer as follows, and trigger off a falling 
   edge of ADC_INV_CS to see the communication.
                         +-----USB-------+
                     GND |GND         VIN|
                         | 0         AGND|
            ADC_INV_SYNC | 1   top   3.3V|
                         | 2   view    23|
            ADC_INV_DRDY*| 3           22|
           ADC_INV_RESET | 4    T      21|
             DAC_INV_CLR | 5    E      20|
    (ADC_RAW_CLK - 8MHz) | 6    E      19|
            DAC_INV_LDAC | 7    N      18|
                         | 8    S      17|
              ADC_INV_CS*| 9    Y      16|
              DAC_INV_CS |10           15|
                    MOSI*|11           14|
                    MISO*|12           13|*SPI_CLK
                         +---------------+
 */

#include <SPI.h>

// Board Pinouts
static const int DAC_INV_CS     = 10; // DAC Pin 6
static const int DAC_INV_LDAC   =  7; // DAC Pin 4
static const int DAC_INV_CLR    =  5; // DAC PIN 5

static const int ADC_INV_SYNC  = 1; // ADC Pin 20
static const int ADC_INV_DRDY  = 3; // ADC Pin 14
static const int ADC_INV_RESET = 4;
static const int ADC_CLOCK_OUT = 6; // ADC Pin 21
static const int ADC_INV_CS    = 9; // ADC Pin 15


static const byte ADC_REG_STATUS = 0x00; // Status
static const byte ADC_REG_MUX    = 0x01; // Multiplexer
static const byte ADC_REG_ADCON  = 0x02; // Control Register
static const byte ADC_REG_DRATE  = 0x03; // Datarate
static const byte ADC_REG_GPIO   = 0x04; // GPIO config
static const byte ADC_REG_OFC0   = 0x05; // Calibration: Offset
static const byte ADC_REG_OFC1   = 0x06;
static const byte ADC_REG_OFC2   = 0x07;
static const byte ADC_REG_FSC0   = 0x08; // Calibration: Scaling
static const byte ADC_REG_FSC1   = 0x09;
static const byte ADC_REG_FSC2   = 0x0A;


static const byte ADC_MUX_VAL_TEMP    = 0x23; // AIN2, AIN3: Differential Temperature
static const byte ADC_MUX_VAL_CURRENT = 0x01; // AIN0, AIN1: Differential Current

// All delays in ns.
static const int CLKIN_PERIOD = 3; // 8MHz;

// DAQ Chip Select

void setup() {
  Serial.begin(9600);
  Serial.println(F(""));
  
  SPI.setClockDivider(SPI_CLOCK_DIV8); // 8, 16
  int mode = SPI_MODE1;
  SPI.setDataMode(mode);
  SPI.begin();
  SPI.setDataMode(mode);

  pinMode(DAC_INV_CS, OUTPUT);
  digitalWrite(DAC_INV_CS, 1);

  SPI.setClockDivider(SPI_CLOCK_DIV8);

  // ADC Clock
  analogWriteFrequency(ADC_CLOCK_OUT, 375000*20); // 8MHz
  analogWrite(ADC_CLOCK_OUT, 125); 
  
  // Set up the ADC pins
  pinMode(ADC_INV_DRDY, INPUT);
  pinMode(ADC_INV_SYNC, OUTPUT);
  pinMode(ADC_INV_CS, OUTPUT);
  pinMode(ADC_INV_RESET, OUTPUT);

  reset();
  
}

void reset () {
  digitalWrite(ADC_INV_SYNC, 1);
  digitalWrite(ADC_INV_CS, 1);
  digitalWrite(ADC_INV_RESET, 1);
  delayMicroseconds(1000);
  digitalWrite(ADC_INV_RESET, 0);
  delayMicroseconds(1000);
  digitalWrite(ADC_INV_RESET, 1);
  delayMicroseconds(8000);  // Self Calibration takes 3.8ms. This is twice as much.

  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(0x0F);  // SDATAC - Stop Data Continuous
  digitalWrite(ADC_INV_CS, 1); 

  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(0x00);  // Complete sync and exit standby mode
  digitalWrite(ADC_INV_CS, 1); 
  delayMicroseconds(100000);   
}

byte s, m, a, d, i;
char charBuf[512];

void loop() {
  waitForDRDYPulse();
  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(0b00010000); // Read Register 0
  SPI.transfer(5-1); // ... and the following 4
  delayMicroseconds(30); // Wait for t6 - 50x tclkin = 125ns
  // Wait fo drdy as well
  waitForDRDYPulse();
  s = SPI.transfer(0b00000000); // Read it
  m = SPI.transfer(0b00000000); // Read it
  a = SPI.transfer(0b00000000); // Read it
  d = SPI.transfer(0b00000000); // Read it
  i = SPI.transfer(0b00000000);
  digitalWrite(ADC_INV_CS, 1); 
          
  sprintf(charBuf, "STAT 0x%2x MUX  0x%2x CON  0x%2x RATE 0x%2x IO   0x%2x", s, m, a, d, i );
  Serial.println(charBuf);
  reset();
  delayMicroseconds(10000); 
}

void waitForDRDYLow() {
  // wait up to 1 second for DRDY to go low
  unsigned int count = 0;
  while (digitalRead(ADC_INV_DRDY) && count < 1000000 ) {
    delayMicroseconds(1);
    count ++;
  }
}
void waitForDRDYHigh() {
  // wait up to 1 second for DRDY to go low
  unsigned int count = 0;
  while (!(digitalRead(ADC_INV_DRDY)) && count < 1000000 ) {
    delayMicroseconds(1);
    count ++;
  }
}
void waitForDRDYPulse() {
  waitForDRDYHigh();
  waitForDRDYLow();
}
