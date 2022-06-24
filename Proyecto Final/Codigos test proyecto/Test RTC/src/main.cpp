#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <DallasTemperature.h>
LiquidCrystal_I2C lcd(0x20,20,4);
//LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 

int mili_segundos = 0;
bool mostrar_hora = false;

void imprimir (DateTime now);

void setup () {
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  Wire.begin();
  RTC.begin();
  lcd.init();
  lcd.backlight();
  RTC.adjust(DateTime (F(__DATE__), F(__TIME__))); //saca la data de la compu, despues comentar para subirlo bien
  Serial.begin(9600);
}

void loop(){
  //DateTime now =  RTC.now();
  char buf1[20];
  DateTime now = RTC.now(); 
  sprintf(buf1, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  
  Serial.print(F("Date/Time: "));
  Serial.println(buf1);
  //mostrar_hora = true;
  //imprimir(now);       // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
}

void imprimir (DateTime date){
  if (mostrar_hora == true)
  {
    lcd.setCursor(1,3); 
    lcd.print("Hora: ");
    lcd.print(date.hour()); // Horas
    lcd.print(':');
    lcd.print(date.minute()); // Minutos
    Serial.print(date.hour(), DEC);
    Serial.print(':');
    Serial.print(date.minute(), DEC);
    Serial.print(':');
    Serial.print(date.second(), DEC);
    Serial.println();
  }
}

ISR(TIMER2_OVF_vect){
    mili_segundos++;
}