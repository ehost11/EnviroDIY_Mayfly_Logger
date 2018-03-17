#include <Wire.h>
#include <Adafruit_MPL115A2.h>
Adafruit_MPL115A2 mpl115a2;
int delaytime = 10000;
void setup() 
{
  Serial.begin(9600);
  mpl115a2.begin();
}
void loop() 
{
  float pressureKPA = 0;
  pressureKPA = mpl115a2.getPressure();  
  Serial.print("Pressure (kPa): ");
  Serial.print(pressureKPA, 4);
  Serial.println(" kPa ");
  delay(delaytime);
}
