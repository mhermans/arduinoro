#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup()
{
    matrix.begin(0x70);  // pass in the address  
    
}

static const uint8_t PROGMEM
  right_up[] =
  { B00100000,
    B00010000,
    B11111000,
    B00010000,
    B00100100,
    B00001110,
    B00010101,
    B00000100 },
  right_down[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 };

void loop()
{  
  matrix.clear();
  matrix.drawBitmap(0, 0, right_up, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(500);
  
 // matrix.clear();
 // matrix.drawBitmap(0, 0, right_down, 8, 8, LED_ON);
 // matrix.writeDisplay();
 // delay(500);

  
}
