/*# ################################################## #
  #                                                    #
  # Basic MMA8452Q and LED matrix integration          #
  # =========================================          #
  # -> custom landscape detection instead of interupts #
  # -> (no custom I2C library needed)                  #
  # ################################################## #
*/

// Import Arduino I2C-lbrary

#include <Wire.h>

// Imports for Adafruit LED matrix
// -------------------------------

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Imports for MMA8452Q accel. IC
// -------------------------------


// Setup for Adafruit LED matrix
// -------------------------------

#define LEDMATRIX_ADDRESS 0x70
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

// Setup for MMA8452Q accel. IC
// -------------------------------

#define MMA8452_ADDRESS 0x1C


// Define arrow patterns for led matrix
// ------------------------------------

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


void setup()
{
    matrix.begin(LEDMATRIX_ADDRESS);  // pass in the address  
    
}


void loop()
{  
  matrix.clear();
  matrix.drawBitmap(0, 0, right_up, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(500);
  
 matrix.clear();
 matrix.drawBitmap(0, 0, right_down, 8, 8, LED_ON);
 matrix.writeDisplay();
 delay(500);

  
}
