// Example sketch for testing OLED display

// We need to include Wire.h for I2C communication
#include <Wire.h>
#include "OLED.h"

// Declare OLED display
// display(SDA, SCL);
// SDA and SCL are the GPIO pins of ESP8266 that are connected to respective pins of display.

// OLED::OLED(uint8_t sda, uint8_t scl, uint8_t address, uint8_t offset) //ojo usar GPIOS no Dx 

//OLED display(4, 5,0x3c,2);

OLED & init_display(void);

OLED & init_display(void) {

  // Initialize display
  display.begin();


}


void print_display(char *p)
{

    display.print(p);
}

void test(void)
{
  // Test message
  display.print("Hello World");
  delay(3*1000);

  // Test long message
  display.print("Prueba de display SSH1106 long line test 12345678 ABCDEFG.");
  delay(3*1000);

  // Test display clear
  display.clear();
  delay(3*1000);

  // Test message postioning
  display.print("TOP-LEFT");
  display.print("4th row", 4);
  display.print("RIGHT-BOTTOM", 7, 4);
  delay(3*1000);

  // Test display OFF
  display.off();
  display.print("3rd row", 3, 8);
  delay(3*1000);

  // Test display ON
  display.on();
  delay(3*1000);
}

int r = 0, c = 0;

void loop1() {
  r = r % 8;
  c = micros() % 6;

  if (r == 0)
    display.clear();

  display.print("Hello World", r++, c++);

  delay(500);
}

