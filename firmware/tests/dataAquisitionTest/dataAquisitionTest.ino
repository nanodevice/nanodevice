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

  setupDAC();
  setupADC();

  resetDAC();
  resetADC();
  
  writeDAC('A', 1000000); // Set to 1V
  writeDAC('B', 1001000); // Set to 1001mV

  ADC_write_register(ADC_REG_DRATE, ADC_SR_1k);
  //ADC_write_register(ADC_REG_DRATE, ADC_SR_2_5);
  
  ADC_write_register(ADC_REG_MUX, ADC_MUX_VAL_CURRENT); // measure current
  // ADC_write_register(ADC_REG_MUX, ADC_MUX_VAL_TEMP); // measure temperature
  
  DAC_send_command(ADC_CMD_RDATAC); // Start continuous aquisition
}

byte daqBuf[3];
long full;
char charBuf[512];

void loop() {
  waitForDRDYPulse();  
  ADCReadData(daqBuf);
  full = daqBuf[0] | (daqBuf[1]<<8) | (daqBuf[2]<<16);

  //sprintf(charBuf, "0x%2x 0x%2x 0x%2x\n%8ld", a,b,c,full);
  sprintf(charBuf, " %8ld",full);
  Serial.println(charBuf);
}


