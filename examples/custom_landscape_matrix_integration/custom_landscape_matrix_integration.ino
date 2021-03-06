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

// ToneAC library

#include <toneAC.h>

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

//Define a few of the registers that we will be accessing on the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I   0x0D
#define CTRL_REG1  0x2A

// Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
#define GSCALE 2

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
 left_up[] =
  { B00000100,
    B00001000,
    B00011111,
    B00001000,
    B00100100,
    B01110000,
    B10101000,
    B00100000 },   
  right_down[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 };


// vars for smoothing
// ------------------

const int numReadings = 50;
int index = 0;                  // the index of the current reading

int y_readings[numReadings];      // the readings from the analog input
float y_total = 0;                  // the running total
float y_average = 0;                // the average

int z_readings[numReadings];
float z_total = 0;
float z_average = 0;

int x_readings[numReadings];
float x_total = 0;
float x_average = 0;

int averages[3];

int freefalled = 0;

void setup()
{
    matrix.begin(LEDMATRIX_ADDRESS);  // pass in the address  
    
    	Serial.begin(57600);
	Serial.println("Start MMA8452 routine");

	Wire.begin(); //Join the bus as a master
	initMMA8452(); //Test and intialize the MMA8452

	// set all readings to 0
	for (int thisReading = 0; thisReading < numReadings; thisReading++) {
		y_readings[thisReading] = 0;
		z_readings[thisReading] = 0;
		x_readings[thisReading] = 0;
	}
    
}


void loop()
{
/*  
  matrix.clear();
  matrix.drawBitmap(0, 0, right_up, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(500);
  
 matrix.clear();
 matrix.drawBitmap(0, 0, right_down, 8, 8, LED_ON);
 matrix.writeDisplay();
 delay(500);
*/

	// READ ACCELERATION OBSERVATIONS
	// ==============================

	int accelCount[3];  // Stores the 12-bit signed value
	readAccelData(accelCount);  // Read the x/y/z adc values

	// Now we'll calculate the accleration value into actual g's
	float accelG[3];  // Stores the real accel value in g's
	float conversion[3];  // Stores the real accel value in g's

	for (int i = 0 ; i < 3 ; i++)
	{
		// get actual g value, this depends on scale being set
		// multiplied by 10 for convenience, gives resting range [-10, 10]
		accelG[i] = (float) accelCount[i] * 10  / ((1<<12)/(2*GSCALE));
	}


        
	// Print out values
	for (int i = 0 ; i < 3 ; i++)
	{
		Serial.print(accelG[i], 4);  // Print g values
		Serial.print("\t");  // tabs in between axes
	}
        Serial.println();
        
  
  	// SMOOTHING OF ACCELERATION OBSERVATIONS USING AVERAGES
	// =====================================================

	x_total= x_total - x_readings[index];
	x_readings[index] = accelG[0];
	x_total= x_total + x_readings[index];

	y_total= y_total - y_readings[index];
	y_readings[index] = accelG[1];
	y_total= y_total + y_readings[index];

	z_total= z_total - z_readings[index];
	z_readings[index] = accelG[2];
	z_total= z_total + z_readings[index];

	// advance to the next position in the array:
	index = index + 1;

	// if we're at the end of the array...
	if (index >= numReadings)
		// ...wrap around to the beginning:
		index = 0;

	// calculate the average:
	y_average = y_total / numReadings;
	z_average = z_total / numReadings;
	x_average = x_total / numReadings;

	averages[0] = x_average;
	averages[1] = y_average;
	averages[2] = z_average;



	// DETECT FREEFALL BASED ON SMOOTHED OBSERVATIONS
	// ==============================================

        Serial.print(x_average);
        Serial.print("\t");
        Serial.print(y_average);
        Serial.print("\t");
        Serial.print(z_average);
        Serial.print("\t");
        Serial.println();

	if ( x_average <= -9 ) {
		Serial.println("TILT RIGHT");
                matrix.clear();
                //matrix.drawBitmap(0, 0, right_up, 8, 8, LED_ON);
                matrix.drawChar(0, 1, 'A', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(500); // 
	}

	if ( x_average >= 9 ) {
		Serial.println("TILT LEFT");
                matrix.clear();
                //matrix.drawBitmap(0, 0, left_up, 8, 8, LED_ON);
                matrix.drawChar(0, 1, 'B', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(1000); // Play the frequency (150 Hz to 15 kHz).
                //delay(1);     // Wait 1 ms so you can hear it.
                //toneAC(0); // Turn off toneAC, can also use noToneAC().
                
	}

	if ( y_average <= -9 ) {
		Serial.println("TILT UP");
                matrix.clear();
                matrix.drawChar(0, 1, 'C', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(1500); // 
                
	}

	if ( y_average >= 9 ) {
		Serial.println("TILT DOWN");
                matrix.clear();
                matrix.drawChar(0, 1, 'D', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(2000); // 
	}
        
	if ( z_average <= -9 ) {
		Serial.println("FACE DOWN");
                matrix.clear();
                matrix.drawChar(0, 1, 'E', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(2500); // 
	}

	if ( z_average >= 9 ) {
		Serial.println("FACE UP");
                matrix.clear();
                matrix.drawChar(0, 1, 'F', 1, 1, 1);
                matrix.writeDisplay();
                
                toneAC(0);
                
	}

        
        
        
        /*
	if ( y_average <= -13.0 | y_average >= 13) {
		Serial.println("FREEFALL ON Y-AXIS");
		freefalled = 1;
	}

	if ( z_average <= -13.0 | z_average >= 13) {
		Serial.println("FREEFALL ON Z-AXIS");
		freefalled = 1;
	}
        */
  
  
}



void readAccelData(int *destination)
{
	byte rawData[6];  // x/y/z accel register data stored here

	readRegisters(OUT_X_MSB, 6, rawData);  // Read the six raw data registers into data array

	// Loop to calculate 12-bit ADC and g value for each axis
	for(int i = 0; i < 3 ; i++)
	{
		int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1];  //Combine the two 8 bit registers into one 12-bit number
		gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

		// If the number is negative, we have to make it so manually (no 12-bit data type)
		if (rawData[i*2] > 0x7F)
		{
			gCount = ~gCount + 1;
			gCount *= -1;  // Transform into negative 2's complement #
		}

		destination[i] = gCount; //Record this gCount into the 3 int array
	}
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
void initMMA8452()
{
	byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
	if (c == 0x2A) // WHO_AM_I should always be 0x2A
	{
		Serial.println("MMA8452Q is online...");
	}
	else
	{
		Serial.print("Could not connect to MMA8452Q: 0x");
		Serial.println(c, HEX);
		while(1) ; // Loop forever if communication doesn't happen
	}

	MMA8452Standby();  // Must be in standby to change registers

	// Set up the full scale range to 2, 4, or 8g.
	byte fsr = GSCALE;
	if(fsr > 8) fsr = 8; //Easy error check
	fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
	writeRegister(XYZ_DATA_CFG, fsr);

	//The default data rate is 800Hz and we don't modify it in this example code

	MMA8452Active();  // Set to active to start reading
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
	byte c = readRegister(CTRL_REG1);
	writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
	byte c = readRegister(CTRL_REG1);
	writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
	Wire.beginTransmission(MMA8452_ADDRESS);
	Wire.write(addressToRead);
	Wire.endTransmission(false); //endTransmission but keep the connection active

	Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); //Ask for bytes, once done, bus is released by default

//      dit was oude delay:
//	while(Wire.available() < bytesToRead); //Hang out until we get the # of bytes we expect

//      zet er een ruwe time-out functie rond, anders bleef loop soms hangen (slecht contact/rate?)
//      betere oplossing is niet continue pollen, maar via interupts werken
        int x = 0;
        while(Wire.available() < bytesToRead && x < 1000){
        x++;
        continue; // TODO nu negeren we gewoon gemiste metingen -> niet echt een probleem voor ruwe gemiddelde -> orientatie omzetting
        }

	for(int x = 0 ; x < bytesToRead ; x++)
		dest[x] = Wire.read();
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead)
{
	Wire.beginTransmission(MMA8452_ADDRESS);
	Wire.write(addressToRead);
	Wire.endTransmission(false); //endTransmission but keep the connection active

	Wire.requestFrom(MMA8452_ADDRESS, 1); //Ask for 1 byte, once done, bus is released by default

	while(!Wire.available()) ; //Wait for the data to come back
	return Wire.read(); //Return this one byte
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
	Wire.beginTransmission(MMA8452_ADDRESS);
	Wire.write(addressToWrite);
	Wire.write(dataToWrite);
	Wire.endTransmission(); //Stop transmitting
}
