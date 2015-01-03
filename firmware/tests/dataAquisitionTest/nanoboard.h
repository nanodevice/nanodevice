/*                        +-----USB-------+
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

// Board Pinouts: DAC
static const int DAC_INV_CLR    =  5; // DAC PIN 5
static const int DAC_INV_LDAC   =  7; // DAC Pin 4
static const int DAC_INV_CS     = 10; // DAC Pin 6

// Board Pinouts: ADC
static const int ADC_INV_SYNC  = 1; // ADC Pin 20
static const int ADC_INV_DRDY  = 3; // ADC Pin 14
static const int ADC_INV_RESET = 4; // ADC Pin
static const int ADC_CLOCK_OUT = 6; // ADC Pin 21
static const int ADC_INV_CS    = 9; // ADC Pin 15

// Board specific ADC configuration
static const byte ADC_MUX_VAL_TEMP    = 0x23; // AIN2, AIN3: Differential Temperature
static const byte ADC_MUX_VAL_CURRENT = 0x01; // AIN0, AIN1: Differential Current

// ADC Registers
static const byte ADC_REG_STATUS = 0x00; // Status
static const byte ADC_REG_MUX    = 0x01; // Multiplexer
static const byte ADC_REG_ADCON  = 0x02; // Control Register
static const byte ADC_REG_DRATE  = 0x03; // Datarate
static const byte ADC_REG_GPIO   = 0x04; // GPIO config
static const byte ADC_REG_OFC0   = 0x05; // Calibration: Offset
static const byte ADC_REG_OFC1   = 0x06; // Calibration: Offset
static const byte ADC_REG_OFC2   = 0x07; // Calibration: Offset
static const byte ADC_REG_FSC0   = 0x08; // Calibration: Scaling
static const byte ADC_REG_FSC1   = 0x09; // Calibration: Scaling
static const byte ADC_REG_FSC2   = 0x0A; // Calibration: Scaling

// ADC Commands
static const byte ACD_CMD_WAKEUP   = 0x00; // Complete SYNC and exit standby mode
static const byte ADC_CMD_RDATA    = 0x01; // Read data
static const byte ADC_CMD_RDATAC   = 0x03; // Read data continuously
static const byte ADC_CMD_SDATAC   = 0x0F; // Stop read data continuous
static const byte ADC_CMD_RREG     = 0x10; // Read from register. BITWISE OR with register  
static const byte ADC_CMD_WREG     = 0x50; // Write to register.  BITWISE OR with register
static const byte ADC_CMD_SELFCAL  = 0xF0; // Offset and Gain Self-Calibration
static const byte ADC_CMD_SELFOCAL = 0xF1; // Offset Self-Calibration
static const byte ADC_CMD_SELFGCAL = 0xF2; // Gain Self-Calibration
static const byte ADC_CMD_SYSOCAL  = 0xF3; // System Offset Calibration
static const byte ADC_CMD_SYSGCAL  = 0xF4; // System Gain Calibration
static const byte ADC_CMD_SYNC     = 0xFC; // Synchronize the A/D Conversion
static const byte ADC_CMD_STANDBY  = 0xFD; // Begin Standby Mode
static const byte ADC_CMD_RESET    = 0xFE; // Resets to Power-Up Values
static const byte ADC_CMD_WAKEUP   = 0xFF; // Completes SYNC and Exits Standby Mode

// Sample Rates
static const byte ADC_SR_30k  = 0xF0;
static const byte ADC_SR_15k  = 0xE0;
static const byte ADC_SR_7k5  = 0xD0;
static const byte ADC_SR_3k7  = 0xC0;
static const byte ADC_SR_2k   = 0xB0;
static const byte ADC_SR_1k   = 0xA0;
static const byte ADC_SR_500  = 0x92;
static const byte ADC_SR_100  = 0x82;
static const byte ADC_SR_60   = 0x72;
static const byte ADC_SR_50   = 0x63;
static const byte ADC_SR_30   = 0x53;
static const byte ADC_SR_25   = 0x43;
static const byte ADC_SR_15   = 0x33;
static const byte ADC_SR_10   = 0x23;
static const byte ADC_SR_5    = 0x13;
static const byte ADC_SR_2_5  = 0x03;

// All delays in ns.
static const int CLKIN_PERIOD = 3; // 8MHz;

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

byte DAC_read_register(byte reg) {
  waitForDRDYPulse(); // Wait fo drdy - this appears to increase stability
  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(ADC_CMD_RREG | reg); // Read Register 0
  SPI.transfer(1-1); // exactly 1 register
  delayMicroseconds(1); // Wait for t6 - 50x tclkin = 125ns
  waitForDRDYPulse();   // Wait fo drdy as well - this appears to increase stability
  byte retval = SPI.transfer(0x00); // Read it
  digitalWrite(ADC_INV_CS, 1); 
  return retval;
}

void DAC_write_register(byte reg, byte value) {
  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(ADC_CMD_WREG | reg); // Read Register 0
  SPI.transfer(1-1); // exactly 1 register
  SPI.transfer(value); // Read it
  digitalWrite(ADC_INV_CS, 1); 
}

void reset() {
  digitalWrite(ADC_INV_SYNC, 1);
  digitalWrite(ADC_INV_CS, 1);
  digitalWrite(ADC_INV_RESET, 0);
  delayMicroseconds(10);
  digitalWrite(ADC_INV_RESET, 1);
  delayMicroseconds(8000);  // Self Calibration takes 3.8ms. This is twice as much.

  digitalWrite(ADC_INV_CS, 0); 
  SPI.transfer(ADC_CMD_SDATAC);  // SDATAC - Stop Data Continuous

  SPI.transfer(ADC_CMD_WAKEUP);  // Complete sync and exit standby mode
  
  digitalWrite(ADC_INV_CS, 1); 

  delayMicroseconds(8000);   
  
  DAC_write_register(ADC_REG_DRATE, ADC_SR_1k);

}

void writeToDAC(char a, char b, char c) {
  digitalWrite(DAC_INV_CS, 0);
  delayMicroseconds(1);
  // Disable internal ref
  SPI.transfer(a); // DB23..16 = X,X, C2..0, A2...0
  SPI.transfer(b);       // DB15..08 = 
  SPI.transfer(c);       // DB07..00 =
  delayMicroseconds(2);
  digitalWrite(DAC_INV_CS, 1); 
}

void resetDAC() {
    // Set up the DAC pins
  pinMode(DAC_INV_CS, OUTPUT);
  digitalWrite(DAC_INV_CS, 1);
  pinMode(DAC_INV_LDAC, OUTPUT);
  digitalWrite(DAC_INV_LDAC, 0); // In syncronous mode, LDAC needs to be tied to GND
  pinMode(DAC_INV_CLR,  OUTPUT);
  
  digitalWrite(DAC_INV_CLR, 1);
  digitalWrite(DAC_INV_CLR, 0);
  delayMicroseconds(10);
  digitalWrite(DAC_INV_CLR, 1); // Let's not clear it later on - in case we want to continue an experiment
  writeToDAC(0b00111000, 0b0, 0b0); // Disable internal reference
};

// Set DAC in uV
void writeDAC(const char channel, const int microVolts) {
  // Voltage for reference MAX6126 is 2.048V
  // Bits for DAC5862 are 16.
  // step = 2.048V / 2^16
  // Therefore, step size is 31.25uV
  int setBits = 1.0*microVolts/31.25;
  char low  = 0xFF & setBits;
  char high = 0xFF & (setBits >> 8);
  if (channel == 'A') {
    writeToDAC(0b00011000, high, low);
  } else if (channel == 'B') {
    writeToDAC(0b00011001, high, low);
  } 
}
