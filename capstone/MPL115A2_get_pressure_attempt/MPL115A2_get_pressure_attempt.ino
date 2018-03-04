
// Snagged from Adafruit MPL115A2 test sketch

// Define libraries
#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

int delaytime = 10000; // Delay time currently set at 10 seconds
                       // Delay time variable should be spelled exactly the same as
                       // The delay time variable in other scripts to be used
 
void setup() 
{
  // Define baud number
  Serial.begin(9600);
  mpl115a2.begin();
}

void loop() 
{
  // Define presure variable
  float pressureKPA = 0;    


  
  pressureKPA = mpl115a2.getPressure();  
  Serial.print("Pressure (kPa): ");
  Serial.print(pressureKPA, 4);
  Serial.println(" kPa");



  
  delay(delaytime);
}
