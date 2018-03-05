/* 
 * This is an attempt to obtain water level derived from a SparkFun 
 * water pressure transducer and a Adafruit barometric pressure transducer
 * for the CLFLWD continuous groundwater monitor project.
 * I (Evan Host) am adapting code for each of the sensors from test sketches
 * provided by the companies for their sensors (Adafruit MPL115A2 and SparkFun MS5803)
 * and from EnviroDIY.
 * Here goes nothing!
  */

// Call up libraries
#include <Wire.h> // One Wire library
#include <Adafruit_MPL115A2.h> // MPL115A2 barometric pressure sensor library
#include <SparkFun_MS5803_I2C.h> // MS5803 pressure transducer (used for water pressure) with I2C library
#include <SPI.h> // SPI library
#include <SD.h> // MicroSD card library

// Define variables
// ADDRESS_HIGH = 0X76
// ADDRESS_LOW  = 0X77

MS5803 sensor(ADDRESS_HIGH);

Adafruit_MPL115A2 mpl115a2;

//Define the SD pin: digital pin 12 is the MicroSD slave select pin on the Mayfly.
#define SD_SS_PIN 12

// Define the data log file for storing data on the SD card.
#define FILE_NAME "WaterLevel.txt"

// Data header  (these lines get written to the beginning of a file when it's created)
#define LOGGERNAME "Water Level attempt"
#define DATA_HEADER "Sample Number, Water Level"

int samplenum = 1; // sample number starts at 1
                       
// I am defining the variables in the SparkFun test sketch even though we may not use them.
float H2O_temperature_c, H20_temperature_f; // Floats are precise to 6 or 7 digits.
float BaroPressure_mbar; // Measured in kpa; will need to be converted to mbar (just multiply by 10).
double H2O_pressure_abs, H2O_pressure_relative, H2O_altitude_delta, H2O_pressure_baseline; // Doubles are precise to 15 digits.
double reference_altitude = 0; // reference_altitude is currently equal to 0, or sea level (measured in m).

/*
 * SparkFun Comment
 "Thanks to Mike Grusin for letting me borrow the functions below from 
 the BMP180 example code."
 functions from the BMP180 example code are the following two variable definitions 
 (sealevel and altitude) and the two return functions. Not sure if we will need these.
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
int time_conversion_factor = 1000; // Use this to convert easily from miliseconds to seconds 
                                   // (just multiply it by the desired interval in seconds).
int delaytime = 5 * time_conversion_factor; // delay time defines the delay interval between data collections.
                                            // Delay time currently set at 5 seconds.
                                            // Delay time variable should be spelled exactly the same as
                                            // The delay time variable in other scripts to be used.

void setup() 
{
  // Define baud number
  Serial.begin(9600);  
  
  // Retrieve calibration constants for calculation math.
  sensor.reset();
  sensor.begin(); 
  mpl115a2.begin(); // This shouldn't mess things up, I think these are separate "begin" syntaxes for the two libraries.

  
  H2O_pressure_baseline = sensor.getPressure(ADC_4096); // This pressure_baseline variable might only be
                                                        // used for change in atmpospheric elevation. We 
                                                        // may not be able to use this for groundwater elevation.

//Initialise log file
  setupLogFile();
  //print a line for data headers before starting to report data.
  Serial.println(DATA_HEADER);
}

void loop() {

  // I am omitting the temperature calculations and sensing because we will have a separate sensor for temperature.

/* Read pressure from the sensor in mbar.
 *  ADC stands for Analog to Digital Converter.
 *  "A circuit that converts an analog voltage into a digital number representing that voltage. 
 *  This circuit is built-in to the microcontroller, and is connected to the analog input pins 
 *  of the Arduino board."
* The way I read this is "H2O_pressure_abs equals (a reading from the sensor 
* at ADC_4096 resolution.)"
*/

H2O_pressure_abs = sensor.getPressure(ADC_4096);

/* Specifying the degree of precision
   *  according to the SparkFun test, it looks like higher ADC values mean higher precision.
   *  SparkFun says:   
   *  "To measure to higher degrees of precision use the following sensor settings:
   * ADC_256 (must be 1 mbar)
   * ADC_512 (...0.6 mbar)
   * ADC_1024 (...0.4 mbar)
   * ADC_2048 (...0.3 mbar)
   * ADC_4096" (...0.2 mbar)
   * 
   * I am reading this as "higher ADC values means higher precision"
   * 
   * Beth comment: And if 1 bar of pressure ~= 30 feet or 10 m, the approximate accuracy of the sensor is
   * 10 m * 0.2 mbar = 0.002 meters (or 2 mm). I think we can live with that for this project. 
   * (1 bar = 10.19 m of fresh water if we wish to be more precise.)
   */

// Include a conversion from absolute pressure into relative pressure (AKA, meters above sea level).
// reference altitude here might be the barometric pressure from the Adafruit MPL115A2.
H2O_pressure_relative = sealevel(H2O_pressure_abs, reference_altitude);

// Report values
Serial.print("absolute water pressure = ");
Serial.println(H2O_pressure_abs);

/*
 * not desired until water level calculations from presure is done.
 * when desired, just copy the following quotation and paste into the script.
 * 
 * "

Serial.print("Pressure relative (mbar)= ");
Serial.println(H2O_pressure_relative);

Serial.print("Altitude change (m)= ");
Serial.println(H2O_altitude_delta);

"
*/
  BaroPressure_mbar = mpl115a2.getPressure();  
  Serial.print("Barometric Pressure (mbar): ");
  Serial.println(BaroPressure_mbar * 10, 4); //*10 because 1 Kilopascal (baro sensor) = 10 milibar (water pressure)

String dataRec = createDataRecord();
  
  //Save the data record to the log file
  logData(dataRec);
  
  // print data reports to the serial monitor.
  Serial.println(dataRec);
  /* This SD writing code needs to be kept at the bottom of the loop, but not after the delay.
  // Needs to be kept at the bottom in order to make "sample 1" match up with the correct pressure
  // or water level readings.
  */

// Keep delay at the bottom of the loop code.
  delay(delaytime);
  
}

// Code for saving data to the SD card.
void setupLogFile()
{
  //Initialise the SD card
  if (!SD.begin(SD_SS_PIN))
  {
    Serial.println("Error: SD card failed to initialise or is missing.");
    //Hang
  //  while (true) 
  }
  
  //Check if the file already exists
  bool oldFile = SD.exists(FILE_NAME);  
  
  //Open the file in write mode
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  //Add header information if the file did not already exist
  if (!oldFile)
  {
    logFile.println(LOGGERNAME);
    logFile.println(DATA_HEADER);
  }
  
  // Close the file to save it
  logFile.close();  
}

void logData(String rec)
{
  //Re-open the file
  File logFile = SD.open(FILE_NAME, FILE_WRITE);
  
  //Write the CSV data
  logFile.println(rec);
  
  //Close the file to save it
  logFile.close();  
}


String createDataRecord()
{
  // Create a String type data record in CSV format
  // SampleNumber, WaterLevel
  String data = "";
  data += samplenum;      // Creates a string called "data" and enters in the sample number
  data += ",";            // Add a comma after sample number
  data += (H2O_pressure_abs - (BaroPressure_mbar * 10));     // This is recording the calculated water level to the file.
  samplenum++;            // Increment the sample number
  return data;
}

