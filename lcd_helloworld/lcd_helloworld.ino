/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
* 1 lcd gnd
* 2 lcd vdd 5v
* 3 V0 contrast potentiometer gnd
* 4 LCD RS pin to D10
* 5 lcd RW pin to gnd
* 6 LCD E (Enable) pin D9
* 11 LCD D4 pin to D8
* 12 LCD D5 pin to D7
* 13 LCD D6 pin to D6
* 14 LCD D7 pin to D5
* 15 lcd LED+ to vcc with 220ohm resistor
* 16 lcd LED- to gnd


 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 https://docs.arduino.cc/learn/electronics/lcd-displays

*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = D10, en = D9, d4 = D8, d5 = D7, d6 = D6, d7 = D5;
//const int rs = 10, en = 9, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
	Serial.begin(9600);
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	//lcd.display();
	// Print a message to the LCD.
	lcd.print("hello, world!");
}

void loop() {
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd.setCursor(0, 1);
	// print the number of seconds since reset:
	lcd.print(millis() / 1000);
	delay(1000);
	Serial.println("diocan");
}