
// Libs
#include <Arduino.h>


void setup()
{
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    String Mensaje;
    Mensaje = Serial.readString();
    Serial.print(Mensaje);
  }
}
