
// Libs
#include <Arduino.h>
#include <SoftwareSerial.h>
SoftwareSerial portOne(2,3);
void setup()
{
  Serial.begin(9600);
  portOne.begin(9600);
}

void loop()
{
    char Mensaje;
    Mensaje = Serial.read();
    Serial.print(Mensaje);
}
