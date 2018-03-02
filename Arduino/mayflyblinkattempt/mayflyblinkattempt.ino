int delayTime = 5000; //miliseconds
int delayTime2 = 500;

void setup ()
{
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  Serial.begin(57600);
  delay(1000);
  Serial.println("Mayfly: Blink demo 1");
  delay(1000);
}

void loop ()
{
  // turn on green light
    digitalWrite(8, HIGH);
    digitalWrite(9, LOW);

    delay(delayTime);

//turn on red light
    digitalWrite(8, LOW);
    digitalWrite(9, HIGH);

    delay (delayTime2);

//turn off lights
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);

    delay (delayTime2);
//turn on red light
    digitalWrite(8, LOW);
    digitalWrite(9, HIGH);

    delay(delayTime2);
}

