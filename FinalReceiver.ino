/*
  LiquidCrystal Library - Hello World

  Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
  library works with all LCD displays that are compatible with the
  Hitachi HD44780 driver. There are many of them out there, and you
  can usually tell them by the 16-pin interface.

  This sketch prints "Hello World!" to the LCD
  and shows the time.

  The circuit:
   LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)

  Library originally added 18 Apr 2008
  by David A. Mellis
  library modified 5 Jul 2009
  by Limor Fried (http://www.ladyada.net)
  example added 9 Jul 2009
  by Tom Igoe
  modified 22 Nov 2010
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/LiquidCrystal
*/

// include the library code:
#include <LiquidCrystal.h>
#include <VirtualWire.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
int numPeople = 0;
void setup() {
  // set up the LCD's number of columns and rows:
  
  // Print a message to the LCD.
  
 // lcd.setCursor(0, 1);
 // lcd.print(numPeople);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);      // Bits per sec
  vw_rx_start();
  vw_set_rx_pin(2);
  lcd.begin(16, 2);
  lcd.print("People in Room:");
  digitalWrite(3, HIGH);
  Serial.begin(9600);
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)){ // Non-blocking
    numPeople = buf[0];
  if (buf[0] != 0) {
    digitalWrite(4, HIGH);
    digitalWrite(3, LOW);
  }
  else {
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
  }
  
}
lcd.setCursor(7, 1);
lcd.print(numPeople);
  Serial.println(numPeople);
}

