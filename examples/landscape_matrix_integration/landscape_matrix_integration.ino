
// Includes for Adafruit LED matrix
// --------------------------------

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Includes for accelerometer
// --------------------------

#include "i2c.h"  // not the wire library, can't use pull-ups

// Setup for Adafruit LED Matrix
// -----------------------------

//Adafruit_8x8matrix matrix = Adafruit_8x8matrix();


// Setup for MMA8452Q accelerometer
// --------------------------------

#define SA0 0
#if SA0
  #define MMA8452_ADDRESS 0x1D  // SA0 is high, 0x1C if low
#else
  #define MMA8452_ADDRESS 0x1C
#endif

/* Set the scale below either 2, 4 or 8*/
const byte SCALE = 8;  // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values. //previously set to 2 and now the scale is being set to 8
/* Set the output data rate below. Value should be between 0 and 7*/
const byte dataRate = 2;  // 0=800Hz, 1=400, 2=200, 3=100, 4=50, 5=12.5, 6=6.25, 7=1.56 // previously 0

/* Pin definitions */
int int1Pin = 2;  // These can be changed, 2 and 3 are the Arduinos ext int pins //interchanged by me
int int2Pin = 3;
int x,y,z;
int accelCount[3];  // Stores the 12-bit signed value
float accelG[3];  // Stores the real accel value in g's


void setup()
{
  //  matrix.begin(0x70);  // pass in the address  
    
   
  byte c;
  
  Serial.begin(115200);//previously set as 115200
  
  /* Set up the interrupt pins, they're set as active high, push-pull */
  pinMode(int1Pin, INPUT);
  digitalWrite(int1Pin, LOW);
  pinMode(int2Pin, INPUT);
  digitalWrite(int2Pin, LOW);
  
  /* Read the WHO_AM_I register, this is a good test of communication */
  c = readRegister(0x0D);  // Read WHO_AM_I register
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {  
    initMMA8452(SCALE, dataRate);  // init the accelerometer if communication is OK
    Serial.println("MMA8452Q is online...");
  }
  else
  {
    Serial.print("Could not connect to MMA8452Q: 0x");
    Serial.println(c, HEX);
    while (1)  // Loop forever if communication doesn't happen
      ;
  }
    
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
    B00000100 };

void loop()
{ 
  /* 
  matrix.clear();
  matrix.drawBitmap(0, 0, right_up, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(500);
  */

  static byte source;
  
  /* If int1 goes high, all data registers have new data */
  if (digitalRead(int1Pin)==1)  // Interrupt pin, should probably attach to interrupt function
  {
    readAccelData(accelCount);  // Read the x/y/z adc values
    /* Now we'll calculate the accleration value into actual g's */
    for (int i=0; i<3; i++)
      accelG[i] = (int) (accelCount[i]/10)/((1<<12)/(SCALE));  // get actual g value, this depends on scale being set, scale being multiplied by 2
    /* print out values */
    for (int i=0; i<3; i++)
    {
 //     Serial.print(accelG[i],0);  // Print g values //previously precision was of 4 which is changed to 0 for the purpose of calculation
   //   Serial.print("\t\t");  // tabs in between axes
    }
    Serial.println();
  }
  
  /* If int2 goes high, either p/l has changed or there's been a single/double tap */
  if (digitalRead(int2Pin)==1)
  {
    source = readRegister(0x0C);  // Read the interrupt source reg.
    if ((source & 0x10)==0x10)  // If the p/l bit is set, go check those registers
      portraitLandscapeHandler();
    else if ((source & 0x08)==0x08)  // Otherwise, if tap register is set go check that
      tapHandler();
  }
  delay(100);  // Delay here for visibility
  
}



void readAccelData(int * destination)
{
  byte rawData[6];  // x/y/z accel register data stored here

  readRegisters(0x01, 6, &rawData[0]);  // Read the six raw data registers into data array
  
  /* loop to calculate 12-bit ADC and g value for each axis */
  for (int i=0; i<6; i+=2)
  {
    destination[i/2] = ((rawData[i] << 8) | rawData[i+1]) >> 4;  // Turn the MSB and LSB into a 12-bit value
    if (rawData[i] > 0x7F)
    {  // If the number is negative, we have to make it so manually (no 12-bit data type)
      destination[i/2] = ~destination[i/2] + 1;
      destination[i/2] *= -1;  // Transform into negative 2's complement #
    }
//    x=destination[i/2];
    
//    Serial.print("The value of x as recorded is given by:");
//    Serial.println(x);
  }
  for(int i=0;i<3;i+=1)
  {
    //Serial.print("Value of ");
//    Serial.print(a[i]);
//    Serial.print(" ");
   // Serial.print("=");
    Serial.print(-destination[i]);
    Serial.print(" ");
  }
    Serial.print(";");
}

/* This function will read the status of the tap source register.
   And print if there's been a single or double tap, and on what
   axis. */
void tapHandler()
{
/*  byte source = readRegister(0x22);  // Reads the PULSE_SRC register
  
  if ((source & 0x10)==0x10)  // If AxX bit is set
  {
    if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
      Serial.print("Double Tap on X");  // tabbing here for visibility
    else
      Serial.print("Single tap on X");
      
    if ((source & 0x01)==0x01)  // If PoIX is set
      Serial.println(" +");
    else
      Serial.println(" -");
  }
  if ((source & 0x20)==0x20)  // If AxY bit is set
  {
    if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
      Serial.print("Double Tap on Y");
    else
      Serial.print("Single tap on Y");
      
    if ((source & 0x02)==0x02)  // If PoIY is set
      Serial.println(" +");
    else
      Serial.println(" -");
  }
  if ((source & 0x40)==0x40)  // If AxZ bit is set
  {
    if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
      Serial.print("Double Tap on Z");
    else
      Serial.print("Single tap on Z");
    if ((source & 0x04)==0x04)  // If PoIZ is set
      Serial.println(" +");
    else
      Serial.println(" -");
  }
*/
}

/* This function will read the p/l source register and
   print what direction the sensor is now facing */
void portraitLandscapeHandler()
{
  byte pl = readRegister(0x10);  // Reads the PL_STATUS register
  switch((pl&0x06)>>1)  // Check on the LAPO[1:0] bits
  {
    case 0:
      Serial.print("Portrait up, ");
      break;
    case 1:
      Serial.print("Portrait Down, ");
      break;
    case 2:
      Serial.print("Landscape Right, ");
      break;
    case 3:
      Serial.print("Landscape Left, ");
      break;
  }
  if (pl&0x01)  // Check the BAFRO bit
    Serial.print("Back");
  else
    Serial.print("Front");
  if (pl&0x40)  // Check the LO bit
    Serial.print(", Z-tilt!");
  Serial.println();
}

/* Initialize the MMA8452 registers 
   See the many application notes for more info on setting 
   all of these registers:
   http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
   
   Feel free to modify any values, these are settings that work well for me.
*/
void initMMA8452(byte fsr, byte dataRate)
{
  MMA8452Standby();  // Must be in standby to change registers
  
  /* Set up the full scale range to 2, 4, or 8g. */
  if ((fsr==2)||(fsr==4)||(fsr==8))
    writeRegister(0x0E, fsr >> 2);  
  else
    writeRegister(0x0E, 0);
    
  /* Setup the 3 data rate bits, from 0 to 7 */
  writeRegister(0x2A, readRegister(0x2A) & ~(0x38));
  if (dataRate <= 7)
    writeRegister(0x2A, readRegister(0x2A) | (dataRate << 3));  
    
  /* Set up portrait/landscap registers - 4 steps:
     1. Enable P/L
     2. Set the back/front angle trigger points (z-lock)
     3. Set the threshold/hysteresis angle
     4. Set the debouce rate
  // For more info check out this app note: http://cache.freescale.com/files/sensors/doc/app_note/AN4068.pdf */
  writeRegister(0x11, 0x40);  // 1. Enable P/L
  writeRegister(0x13, 0x44);  // 2. 29deg z-lock (don't think this register is actually writable)
  writeRegister(0x14, 0x84);  // 3. 45deg thresh, 14deg hyst (don't think this register is writable either)
  writeRegister(0x12, 0x50);  // 4. debounce counter at 100ms (at 800 hz)
  
  /* Set up single and double tap - 5 steps:
     1. Set up single and/or double tap detection on each axis individually.
     2. Set the threshold - minimum required acceleration to cause a tap.
     3. Set the time limit - the maximum time that a tap can be above the threshold
     4. Set the pulse latency - the minimum required time between one pulse and the next
     5. Set the second pulse window - maximum allowed time between end of latency and start of second pulse
     for more info check out this app note: http://cache.freescale.com/files/sensors/doc/app_note/AN4072.pdf */
  writeRegister(0x21, 0x7F);  // 1. enable single/double taps on all axes
  // writeRegister(0x21, 0x55);  // 1. single taps only on all axes
  // writeRegister(0x21, 0x6A);  // 1. double taps only on all axes
  writeRegister(0x23, 0x20);  // 2. x thresh at 2g, multiply the value by 0.0625g/LSB to get the threshold
  writeRegister(0x24, 0x20);  // 2. y thresh at 2g, multiply the value by 0.0625g/LSB to get the threshold
  writeRegister(0x25, 0x08);  // 2. z thresh at .5g, multiply the value by 0.0625g/LSB to get the threshold
  writeRegister(0x26, 0x30);  // 3. 30ms time limit at 800Hz odr, this is very dependent on data rate, see the app note
  writeRegister(0x27, 0xA0);  // 4. 200ms (at 800Hz odr) between taps min, this also depends on the data rate
  writeRegister(0x28, 0xFF);  // 5. 318ms (max value) between taps max
  
  /* Set up interrupt 1 and 2 */
  writeRegister(0x2C, 0x02);  // Active high, push-pull interrupts
  writeRegister(0x2D, 0x19);  // DRDY, P/L and tap ints enabled
  writeRegister(0x2E, 0x01);  // DRDY on INT1, P/L and taps on INT2
  
  MMA8452Active();  // Set to active to start reading
}

/* Sets the MMA8452 to standby mode.
   It must be in standby to change most register settings */
void MMA8452Standby()
{
  byte c = readRegister(0x2A);
  writeRegister(0x2A, c & ~(0x01));
}

/* Sets the MMA8452 to active mode.
   Needs to be in this mode to output data */
void MMA8452Active()
{
  byte c = readRegister(0x2A);
  writeRegister(0x2A, c | 0x01);
}

/* Read i registers sequentially, starting at address 
   into the dest byte arra */
void readRegisters(byte address, int i, byte * dest)
{
  i2cSendStart();
  i2cWaitForComplete();
  
  i2cSendByte((MMA8452_ADDRESS<<1));	// write 0xB4
  i2cWaitForComplete();
  
  i2cSendByte(address);	// write register address
  i2cWaitForComplete();
  
  i2cSendStart();
  i2cSendByte((MMA8452_ADDRESS<<1)|0x01);	// write 0xB5
  i2cWaitForComplete();
  for (int j=0; j<i; j++)
  {
    i2cReceiveByte(TRUE);
    i2cWaitForComplete();
    dest[j] = i2cGetReceivedByte();	// Get MSB result
  }
  i2cWaitForComplete();
  i2cSendStop();
  
  cbi(TWCR, TWEN);	// Disable TWI
  sbi(TWCR, TWEN);	// Enable TWI
}

/* read a single byte from address and return it as a byte */
byte readRegister(uint8_t address)
{
  byte data;
  
  i2cSendStart();
  i2cWaitForComplete();
  
  i2cSendByte((MMA8452_ADDRESS<<1));	// write 0xB4
  i2cWaitForComplete();
  
  i2cSendByte(address);	// write register address
  i2cWaitForComplete();
  
  i2cSendStart();
  
  i2cSendByte((MMA8452_ADDRESS<<1)|0x01);	// write 0xB5
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  
  data = i2cGetReceivedByte();	// Get MSB result
  i2cWaitForComplete();
  i2cSendStop();
  
  cbi(TWCR, TWEN);	// Disable TWI
  sbi(TWCR, TWEN);	// Enable TWI
  
  return data;
}

/* Writes a single byte (data) into address */
void writeRegister(unsigned char address, unsigned char data)
{
  i2cSendStart();
  i2cWaitForComplete();
  
  i2cSendByte((MMA8452_ADDRESS<<1));// write 0xB4
  i2cWaitForComplete();
  
  i2cSendByte(address);	// write register address
  i2cWaitForComplete();
  
  i2cSendByte(data);
  i2cWaitForComplete();
  
  i2cSendStop();
}


