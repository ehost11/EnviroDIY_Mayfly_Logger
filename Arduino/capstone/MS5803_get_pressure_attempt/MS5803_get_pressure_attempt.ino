
/******************************************************************************
SparkFun_MS5803_Demo.ino
Demo Program for MS5803 pressure sensors.
Casey Kuhns @ SparkFun Electronics
7/20/2014
https://github.com/sparkfun/MS5803-14BA_Breakout/

The MS58XX MS57XX and MS56XX by Measurement Specialties is a low cost I2C pressure
sensor.  This sensor can be used in weather stations and for altitude
estimations. It can also be used underwater for water depth measurements. 

Resources:
This library uses the Arduino Wire.h to complete I2C transactions.

Development environment specifics:
  IDE: Arduino 1.0.5
  Hardware Platform: Arduino Pro 3.3V/8MHz
  T5403 Breakout Version: 1.0

**Updated for Arduino 1.6.4 5/2015**
  
This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

/* This is an attempt to get the MS5803 14BA water pressure sensor to work 
 * for the CLFLWD continuous groundwater monitor project.
 * I (Evan Host) am pulling parts of code from the SparkFun_MS5803_Demo
 * sketch to work together for our purposes (deriving groundwater level
 * from water pressure, air pressure, and air temperature).
 * Here goes nothing!
 *********************************************************************/

// inculding libraries used
 #include <Wire.h>
#include <SparkFun_MS5803_I2C.h>

// ADDRESS_HIGH = 0X76
// ADDRESS_LOW  = 0X77

MS5803 sensor(ADDRESS_HIGH);

// define variables


// delay time defines the delay between data collections. very important
int delaytime = 1000; // delay time currently set at 10 seconds.

// Floats are precise to 6 or 7 digits.
// Doubles are precise to 15 digits.
// I am defining the variables in the SparkFun test code even though we may not use them.
float temperature_c, temperature_f;
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;

// base_altitude is a reference used. In this test sketch, I will use sea level.
// In St. Paul, elevation above sea level should be around 214 meters.
// For the final datalogger, we should probably use sea level.
double base_altitude = 0.0; // Sea level in m.

void setup() {
Serial.begin(9600); // Serial baud number (is this the correct one?)
                    // (what is the difference between baud numbers?)
                    // (it seems like this is the one used by all the programs I've used so far.
  // Retrieve calibration constants for calculation math.
  sensor.reset();
  sensor.begin();

  // This pressure_baseline variable may only be used for change in atmpospheric
  // elevation. We may not be able to use this for groundwater elevation.
  pressure_baseline = sensor.getPressure(ADC_4096);
  
}

void loop() {

// I am omitting the temperature calculations and sensing because we will have a separate sensor for temperature.

/* Read pressure from the sensor in mbar.
* The way I read this is "pressure_abs equals (a reading from the sensor 
* at ADC_4096 resolution, whatever that means.)"
*/

pressure_abs = sensor.getPressure(ADC_4096);


/* Specifying the degree of precision
   *  according to the SparkFun test, it looks like higher ADC values mean higher precision.
   *  SparkFun says:   
   *  "To measure to higher degrees of precision use the following sensor settings:
   * ADC_256 
   * ADC_512 
   * ADC_1024
   * ADC_2048
   * ADC_4096"
   * 
   * I am reading this as "higher ADC values means higher precision"
   */

// Include a conversion from absolute pressure into relative
// pressure (AKA, meters above sea level).

pressure_relative = sealevel(pressure_abs, base_altitude);


// Report values
Serial.print("pressure abs (mbar)= ");
Serial.println(pressure_abs);

Serial.print("Pressure relative (mbar)= ");
Serial.println(pressure_relative);

Serial.print("Altitude change (m)= ");
Serial.println(altitude_delta);


// Delay is the amount of time between data collection.
// Change this when you want to alter time between data collection.
// Keep delay at the bottom of the loop code.
  delay(delaytime);
}
/*
 * SparkFun Comment
 Thanks to Mike Grusin for letting me borrow the functions below from 
 the BMP180 example code. 
*/

 double sealevel(double P, double A)
// Given a pressure P (mbar) taken at a specific altitude (meters),
// return the equivalent pressure (mbar) at sea level.
// This produces pressure readings that can be used for weather measurements.


{
  return(P/pow(1-(A/44330.0),5.255));
}


double altitude(double P, double P0)
// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
{
  return(44330.0*(1-pow(P/P0,1/5.255)));
}

