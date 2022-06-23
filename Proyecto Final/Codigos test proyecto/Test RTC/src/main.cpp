#include <Arduino.h>
#include <SPI.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,20,4);
//LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 

void setup () {
  Wire.begin();
  RTC.begin();
  lcd.init();
  lcd.backlight();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien
  Serial.begin(9600);
}

void loop(){
DateTime now = RTC.now();       // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
lcd.setCursor(0,0);
lcd.print("Tiempo: ");
lcd.print(now.hour()); // Horas
lcd.print(':');
lcd.print(now.minute()); // Minutos
Serial.print(now.hour()); // Horas
Serial.print(':');
Serial.print(now.minute()); // Minutos
Serial.println();
}
