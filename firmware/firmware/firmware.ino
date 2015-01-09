/* Adapted from Basic Raw HID Example */

/* This firmware mostly deals with the protocol, the nanoboard file 
   addresses the SPI parts. 
   
   This sends pseudo-json through the HID interface for speed optimization. 

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
// RawHID packets are always 64 bytes
byte sendBuffer[64];
byte readBuffer[64];
byte daqBuffer[64];
byte daqReadBuffer[3];


char charBuf[256];
byte byteBuf[3];

#define RAWHID_TX_SIZE          64      // transmit packet size (can be increased for speed)
#define RAWHID_TX_INTERVAL      1       // max # of ms between transmit packets

unsigned int packetCount = 0;

boolean acquisition = false;
int daqCounter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F(""));
  
  SPI.begin();

  setupDAC();
  setupADC();

  resetDAC();
  resetADC();
 
  writeDAC('A', 1000000);
  writeDAC('B', 1000500);
  
  pinMode(DAC_INV_CS, OUTPUT);
  digitalWrite(DAC_INV_CS, 1);
  

  ADC_write_register(ADC_REG_DRATE, ADC_SR_1k);
  ADC_write_register(ADC_REG_MUX, ADC_MUX_VAL_CURRENT); // measure current
  
}




int bufToInt(byte* buffer, int starting) {
  int value = 0;
  int digitValue = 0;
  int count = starting;
  while (buffer[count] >= '0' && buffer[count] <= '9') {
    digitValue = buffer[count]-'0';
    value = value *10 + digitValue;
    count ++;
  }
  return value;
}

void setDataRate(char* message, byte rate) {
  acquisition = false;
  daqCounter = 0;
  waitForDRDYPulse();
  stopADCContinuousAcquisition();
  ADC_write_register(ADC_REG_DRATE, rate);
  ADC_write_register(ADC_REG_MUX, ADC_MUX_VAL_CURRENT); // measure current
 
  sprintf(charBuf, "Rate set to %.", message);
  transmitString('c', charBuf);
  acquisition = true;
  startADCContinuousAcquisition();
  waitForDRDYPulse();

}

void processHostCommand() { 
  int n;
  n = RawHID.recv(readBuffer, 0); // 0 timeout = do not wait
  if (n > 0) {
    int count = 0;
    char whatToSet = '\0';
    char whatToRead = '\0';
    int value = 0;
    switch(readBuffer[0]) {
      case 'e': // echo
          sendBuffer[0] ='e'; // error - for messaging
          count = stringAppend("you said", sendBuffer, 1, 60);
          count = bytesAppend(readBuffer, sendBuffer, count, 60);
          transmitBuffer(sendBuffer, count);
        break;
      case 's': // set
          whatToSet = readBuffer[1];
          value =  bufToInt(readBuffer,2);
          switch(whatToSet) {
            case 'r': //red
              break;
            case 'b': //green
              break;
            case 'g': //blue
              break;
            case 't': // trans; outside wells, voltage in uV
              // Pin 2 = VOut2
              writeDAC('B', value);

              break;
            case 'c': // cis, center well, voltage in uV
               writeDAC('A', value);

              break;
            case 'T': //temperature
              break;
            default:
              // TODO: Error message 
              break;
          }
        break;
      case 'r': // read
        whatToRead = readBuffer[1];
        switch(whatToRead) {
          case 't': //temperature
            break;
          default: 
            // TODO: Error message
            break;
        }
        break;
     
        case 'H': // HALT data acquisition
          stopADCContinuousAcquisition();
        break;
        
        case 'G': // GO, continue data acquisition
          startADCContinuousAcquisition();
        break;
        
      case 'd': // Configure data acquisition
        switch(readBuffer[1]) {
          case 'r': // Reset
              acquisition = false;
              resetADC();
            break;
          case '0': // disable daq
              stopADCContinuousAcquisition();
            break;
            
          case '2': // 1 record per second
              setDataRate("5Hz", ADC_SR_5);
            break;
          case '3': 
              setDataRate("10Hz", ADC_SR_10);
            break;
          case '4': 
              setDataRate("15Hz", ADC_SR_15);
            break;
          case '5': 
              setDataRate("30Hz", ADC_SR_30);
            break;
          case '6': 
              setDataRate("50Hz", ADC_SR_50);
            break;
          case '7': 
              setDataRate("60Hz", ADC_SR_60);
            break;
          case '8': 
              setDataRate("100Hz", ADC_SR_100);
            break;
          case '9': 
              setDataRate("500Hz", ADC_SR_500);
            break;
          case 'a': 
              setDataRate("1kHz", ADC_SR_1k);
            break;
          case 'b': 
              setDataRate("2kHz", ADC_SR_2k);
            break;
          case 'c': 
              setDataRate("3.7kHz", ADC_SR_3k7);
            break;
          case 'd': 
              setDataRate("7.5kHz", ADC_SR_7k5);
            break;
          case 'e': 
              setDataRate("15kHz", ADC_SR_15k);
            break;                        
          default: 
            // TODO: Debug
          break;
        }
        break;
      default: // TODO: Error message
        break;
    }
  }
}

void transmitBuffer(byte* buffer, int fillLevel) {
  // fill the rest with zeros
  for (int i=fillLevel; i<62; i++) {
    buffer[i] = 0;
  }
  // and put a count of packets sent at the end
  buffer[62] = packetCount >> 8 & 0xFF;
  buffer[63] = packetCount & 0xFF;
  // actually send the packet
  int n = RawHID.send(buffer, 100);
  if (n > 0) {
    packetCount++;
  } else {
    Serial.println(F("Unable to transmit packet"));
  }
}

// returns the new end of target.
int stringAppend(char* source, byte* target, int targetIndex, int maxChars) {
  int i = 0;
  while (target[i] != 0 && i < 62) {
    sendBuffer[i+targetIndex] = source[i];
    i++;
  }
  target[i] = '\0'; // Terminate, just in case
  return i+targetIndex-1;
}

// returns the new end of target.
int bytesAppend(byte* source, byte* target, int targetIndex, int maxChars) {
  int i = 0;
  while (target[i] != 0 && i < 62) {
    sendBuffer[i+targetIndex] = source[i];
    i++;
  }
  target[i] = '\0'; // Terminate, just in case
  return i+targetIndex-1;
}


void transmitString(char responseId, char* text) {
  sendBuffer[0] = 'e';
  int size = stringAppend(text, sendBuffer, 1, 60);
  transmitBuffer(sendBuffer, size);
  daqBuffer[0]='d';
}

void loop() {
  // Jitter reduction. Either send DAQ package _or_ deal with user requests.
  boolean canSendPackage = 1;
  if (acquisition) {
    int baseAddress = 1+daqCounter*3;
    daqCounter++;
    waitForDRDYPulse();
    ADCReadData(daqReadBuffer);
    daqBuffer[baseAddress+0] = daqReadBuffer[2];
    daqBuffer[baseAddress+1] = daqReadBuffer[1];
    daqBuffer[baseAddress+2] = daqReadBuffer[0];
    if (daqCounter > 20) {
      transmitBuffer(daqBuffer, baseAddress+2);
      daqCounter = 0;
      canSendPackage = false;
    }
  }
  if (canSendPackage) {
    processHostCommand();
  }
}

// Command set:
// h\n        - shows this list
// rv\n       - read version information:
// rs\n       - read current conversion factors from eeprom, in json.
// sT24000\n  - sets temperature to 24000 (see etc)
// rt\n       - reads current temperature
// slr200\n   - sets red to 20% brightness
// slb100\n   - sets blue led to 10% brightness
// slg050\n   - sets green led to 5% brightness
// sc1200\n   - set cis well left to 1200uV
// st0800\n   - set trans well center to 800uV
// rwl\n      - read voltage of well left
// rwr\n      - read voltage of well right
// rwc\n      - read voltage of well center
// rpc\n      - read instant pore current
// etc\n      - enable temperature control
// dtc\n      - disable temperature control
// rfc\n      - read overcurrent flag...

// daca1000\n - acquire data starting on channel a now at 1kHz; reset buffer
// dacb1000\n - acquire data starting on channel b now at 1kHz; reset buffer

// rda\n      - read data acquisiton buffer in ascii. Returns a,[0],123,124,122,125...\n [0] indicates 0 lost datapoints.
// rdb\n      - read binary data acquisition. 
//              Returns a, followed by the number of lost values, 
//              followed by the number of values to follow, followed by 3
//              bytes of binary data per dataset, followed by \n.
