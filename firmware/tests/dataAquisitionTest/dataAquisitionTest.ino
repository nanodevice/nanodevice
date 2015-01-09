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
#include "nanoboard.h"

void setup() {
  Serial.begin(9600);
  Serial.println(F(""));
  
  SPI.begin();
  // Set some DAC voltages
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE0);
  resetDAC();
  writeDAC('A', 1000000);
  writeDAC('B', 1000500);
  

  // Set SPI for ADC
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE1);

  pinMode(DAC_INV_CS, OUTPUT);
  digitalWrite(DAC_INV_CS, 1);
  
  // ADC Clock
  analogWriteFrequency(ADC_CLOCK_OUT, 375000*20); // 8MHz
  analogWrite(ADC_CLOCK_OUT, 125); 
  
  // Set up the ADC pins
  pinMode(ADC_INV_DRDY, INPUT);
  pinMode(ADC_INV_SYNC, OUTPUT);
  pinMode(ADC_INV_CS, OUTPUT);
  pinMode(ADC_INV_RESET, OUTPUT);

  reset();
  DAC_write_register(ADC_REG_DRATE, ADC_SR_1k);
  //DAC_write_register(ADC_REG_DRATE, ADC_SR_2_5);

  
  DAC_write_register(ADC_REG_MUX, ADC_MUX_VAL_CURRENT); // measure current
  //DAC_write_register(ADC_REG_MUX, ADC_MUX_VAL_TEMP); // measure current
  
  SPI.beginTransaction(spi_adc_settings); 
  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(ADC_CMD_RDATAC);
  digitalWrite(ADC_INV_CS, 1); 
  SPI.endTransaction();
}

byte a,b,c;
long full;
char charBuf[512];

void loop() {
  waitForDRDYPulse();  
  SPI.beginTransaction(spi_adc_settings); 
  digitalWrite(ADC_INV_CS, 0); 
  a = SPI.transfer(0);
  b = SPI.transfer(0);
  c = SPI.transfer(0);
  full = c | (b<<8) | (a<<16);
  digitalWrite(ADC_INV_CS, 1); 
  SPI.endTransaction();
  //sprintf(charBuf, "0x%2x 0x%2x 0x%2x\n%8ld", a,b,c,full);
  sprintf(charBuf, " %8ld",full);
  Serial.println(charBuf);
}


