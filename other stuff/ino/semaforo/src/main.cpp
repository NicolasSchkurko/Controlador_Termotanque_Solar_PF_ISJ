#include <Arduino.h>
const int rojo=1;
const int amarillo=2;
const int verde=3;
int ColorActual;
unsigned long tiempo;

void setup() {
pinMode(rojo,OUTPUT);
pinMode(amarillo,OUTPUT);
pinMode(verde,OUTPUT);
tiempo=millis();
ColorActual=0;
}

void loop() {
switch (ColorActual)
{
  case 0:
  {
    digitalWrite(rojo, HIGH);
    digitalWrite(amarillo, LOW);
    digitalWrite(verde, LOW);
    if(millis()>=tiempo+20000)
    {
      ColorActual=1;
      tiempo=millis();
    }
  }
  break;

  case 1:
  {
    digitalWrite(amarillo, HIGH);
    if(millis()>=tiempo+5000)
    {
      ColorActual=2;
      tiempo=millis();
    }
  }
  break;

  case 2:
  {
    digitalWrite(rojo, LOW);
    digitalWrite(amarillo, LOW);
    digitalWrite(verde, HIGH);
    if(millis()>=tiempo+30000)
    {
      ColorActual=3;
      tiempo=millis();
    }
  }
  break;

  case 3:
  {
    digitalWrite(amarillo, HIGH);
    digitalWrite(verde, LOW);
    if(millis()>=tiempo+5000)
    {
      ColorActual=0;
      tiempo=millis();
    }
  }
  break;


}
}