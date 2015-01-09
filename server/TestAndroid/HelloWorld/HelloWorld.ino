/* Simple "Hello World" example.
 
 After uploading this to your board, use Serial Monitor
 to view the message.  When Serial is selected from the
 Tools > USB Type menu, the correct serial port must be
 selected from the Tools > Serial Port AFTER Teensy is
 running this code.  Teensy only becomes a serial device
 while this code is running!  For non-Serial types,
 the Serial port is emulated, so no port needs to be
 selected.
 
 This example code is in the public domain.
 */
int led = 13;


void setup()
{
  pinMode(led, OUTPUT);     
  Serial.begin(9600); // USB is always 12 Mbit/sec
}

void loop()
{
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  Serial.println("Hello World...");
  delay(1000);  // do not print too fast!
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("2 Hello World...");
  delay(1000);               // wait for a second
}


