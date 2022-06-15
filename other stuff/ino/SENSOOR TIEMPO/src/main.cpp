#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SPI.h>

LiquidCrystal lcd(12,11,10,9,8,7);
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 

void setup () {
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien
  Serial.begin(9600);
}

void loop(){
DateTime now = RTC.now();       // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
lcd.setCursor(0,0);
lcd.print(now.hour()); // Horas
lcd.print(':');
lcd.print(now.minute(), DEC); // Minutos
}
