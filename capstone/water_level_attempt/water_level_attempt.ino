/* 
 * This is an attempt to obtain water level derived from a SparkFun 
 * water pressure transducer an Adafruit barometric pressure transducer
 * and an Adafruit temperature sensor for the CLFLWD continuous groundwater 
 * monitor project. I (Evan Host) am adapting code for each of the sensors
 * from test sketches provided by the companies for their sensors
 * (Adafruit MPL115A2 and SparkFun MS5803) and from EnviroDIY.
 * Here goes nothing!
  */
// Call up libraries
#include <Wire.h> // One Wire library
#include <Adafruit_MPL115A2.h> // MPL115A2 barometric pressure sensor library
#include <SparkFun_MS5803_I2C.h> // MS5803 pressure transducer (used for water pressure) with I2C library
#include <SPI.h> // SPI library
#include <SD.h> // MicroSD card library
#include <OneWire.h> // OneWire library for DS18B20 Temp sensor
#include <DallasTemperature.h> // DallasTemperature library for DS18B20 Temp sensor

// Data wire for DS18B20 temp sensor is plugged into port 4 on the Mayfly Datalogger board (D4-D5 grove port)
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Define variables
// ADDRESS_HIGH = 0X76
// ADDRESS_LOW  = 0X77

MS5803 sensor(ADDRESS_HIGH);

Adafruit_MPL115A2 mpl115a2;

// Define the SD pin: digital pin 12 is the MicroSD slave select pin on the Mayfly.
#define SD_SS_PIN 12

// Define the data log file for storing data on the SD card.
#define FILE_NAME "DATALOG.txt" // For some reason, the program only writes data to the SD card when this is the name of the file.

// Data header  (these lines get written to the beginning of a file when it's created)
#define LOGGERNAME "Water Level"
#define DATA_HEADER "Sample Number, Water Pressure Difference, Water Level, Water Temperature"

int samplenum = 1; // sample number starts at 1
                       
// define variable types
// I am defining the variable types in the SparkFun test sketch even though we may not use them.
float H2O_temperature_c, H20_temperature_f; // Floats are precise to 6 or 7 digits.
double baro_pressure_kPa; // Measured in kPa
double H2O_pressure_mbar; // SparkFun pressure in mbar
double H2O_pressure_kPa; // used for converting from mbar to kPa
double H2O_pressure_relative, H2O_altitude_delta, H2O_pressure_baseline; // Doubles are precise to 15 digits. Might not use these variables.
double reference_altitude = 0; // reference_altitude is currently equal to 0, or sea level (measured in m).
double H2O_pressure; // water pressure calculation (water pressure minus barometric pressure)
double H2O_level; // (1000 * pressure diff) / (9.80 * 1) 1000 times pressure (kPa) over gravity (9.8 m/s^2) times density (1 kg/m^3)
double WaterTemp;
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
  sensors.begin();
  
  /* H2O_pressure_baseline = sensor.getPressure(ADC_4096); // This pressure_baseline variable might only be
                                                             // used for change in atmpospheric elevation. We 
                                                             // may not be able to use this for groundwater elevation. */

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

H2O_pressure_mbar = sensor.getPressure(ADC_4096);

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
H2O_pressure_relative = sealevel(H2O_pressure_mbar, reference_altitude);

// Report values
Serial.print("water pressure (kPa) = ");
Serial.println(H2O_pressure_mbar / 10);

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
  baro_pressure_kPa = mpl115a2.getPressure();  
  Serial.print("Barometric Pressure (kPa) = ");
  Serial.println(baro_pressure_kPa, 4); 

  WaterTemp = sensors.getTempCByIndex(0);
  sensors.requestTemperatures(); // request temp from DS18B20
  Serial.print("Temperature for the device 1 (index 0) = ");
  Serial.println(WaterTemp); // Print temperature reading from DS18B20
  
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
  // Sample Number, Water Pressure, Water Level, Water Temperature
  String data = "";
  data += samplenum;      // Creates a string called "data" and enters in the sample number
  data += ",";
  data += H2O_pressure = (H2O_pressure_mbar / 10) - baro_pressure_kPa; // recording the calculated water pressure from above formula ^ .
  data += ",";
  data += H2O_level = ((1000 * H2O_pressure) / (9.80 * 1)); // calculate and record water level from water pressure (formula in comment way up there ^ )
  data += ",";
  data += WaterTemp; // record water temp to SD card
  samplenum++;            // Increment the sample number
  return data;
}
