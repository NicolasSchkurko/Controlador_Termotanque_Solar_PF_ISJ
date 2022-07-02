#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
 
RTC_DS1307 rtc;
uint64_t ano;
uint8_t mes,dia,hora,minutos,segundos;
uint8_t correccionh,correccionmin, correccionseg;

void setup () {
Serial.begin(9600);
rtc.begin();

}
 
void loop () {

DateTime now = rtc.now(); //iguala la variable datetime al valor del rtc
ano=now.year(),
mes=now.month();
dia=now.day();
hora=now.hour();
minutos=now.minute();
segundos=now.second();

if (Serial.available())
{
  if (Serial.read() == 's')
  {
      correccionseg+=5;
  }
  if (Serial.read() == 'm')
  {
      correccionmin+=5;
  }
  if (Serial.read() =='p')
  {
      rtc.adjust(DateTime(ano,mes,dia,hora,minutos,segundos)); //actualiza la variable datetime al valor del rtc
      correccionmin=0;
      correccionseg=0;
      Serial.println("saved");
  }
}
delay(2000);
Serial.print(hora, DEC);
Serial.print(":"); 
Serial.print(minutos);
Serial.print(":"); 
Serial.print(segundos);
Serial.println();
}
