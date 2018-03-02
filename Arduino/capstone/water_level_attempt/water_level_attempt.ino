/* 
 * This is an attempt to obtain water level derived from a SparkFun 
 * water pressure transducer and a Adafruit barometric pressure transducer
 * for the CLFLWD continuous groundwater monitor project.
 * I (Evan Host) am adapting code for each of the sensors from test sketches
 * provided by the company for their sensors (Adafruit MPL115A2 and SparkFun MS5803).
 * Here goes nothing!
  */

// Call up libraries
#include <Wire.h>
#include <Adafruit_MPL115A2.h>
#include <SparkFun_MS5803_I2C.h>

// Define variables
// ADDRESS_HIGH = 0X76
// ADDRESS_LOW  = 0X77

MS5803 sensor(ADDRESS_HIGH);

Adafruit_MPL115A2 mpl115a2;

int time_conversion_factor = 1000; // Use this to convert easily from miliseconds to seconds (just multiply by desired interval in seconds)
int delaytime = 5 * time_conversion_factor; // delay time defines the delay interval between data collections.
                       // Delay time currently set at 5 seconds
                       // Delay time variable should be spelled exactly the same as
                       // The delay time variable in other scripts to be used.
                       
// I am defining the variables in the SparkFun test sketch even though we may not use them.
float H2O_temperature_c, H20_temperature_f; // Floats are precise to 6 or 7 digits.
float BaroPressureKPA = 0;
double H2O_pressure_abs, H2O_pressure_relative, H2O_altitude_delta, H2O_pressure_baseline; // Doubles are precise to 15 digits.
double reference_altitude = 0; // reference_altitude is currently equal to 0, or sea level (measured in m)

/*
 * SparkFun Comment
 "Thanks to Mike Grusin for letting me borrow the functions below from 
 the BMP180 example code."
 functions from the BMP180 example code are the following two variable definitions (sealevel and altitude).
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
  
void setup() 
{
  // Define baud number
  Serial.begin(9600);// Serial baud number (is this the correct one?)
                     // (what is the difference between baud numbers?)
                     // (it seems like this is the one used by all the programs I've used so far.
                     
  // Retrieve calibration constants for calculation math.
  sensor.reset();
  sensor.begin(); // This shouldn't mess things up, I think these are separate "begin" syntaxes for
                  // the two libraries.
  mpl115a2.begin();

  // This pressure_baseline variable might only be used for change in atmpospheric
  // elevation. We may not be able to use this for groundwater elevation.
  H2O_pressure_baseline = sensor.getPressure(ADC_4096);
}

void loop() {

  // I am omitting the temperature calculations and sensing because we will have a separate sensor for temperature.

/* Read pressure from the sensor in mbar.
* The way I read this is "pressure_abs equals (a reading from the sensor 
* at ADC_4096 resolution, whatever that means.)"
*/

H2O_pressure_abs = sensor.getPressure(ADC_4096);

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

H2O_pressure_relative = sealevel(H2O_pressure_abs, reference_altitude);

// Report values
Serial.print("pressure abs (mbar)= ");
Serial.println(H2O_pressure_abs);

Serial.print("Pressure relative (mbar)= ");
Serial.println(H2O_pressure_relative);

Serial.print("Altitude change (m)= ");
Serial.println(H2O_altitude_delta);

  BaroPressureKPA = mpl115a2.getPressure();  
  Serial.print("Barometric Pressure (kPa): ");
  Serial.print(BaroPressureKPA, 4);
  Serial.println(" kPa");

  

// Keep delay at the bottom of the loop code.
  delay(delaytime);
  
}
