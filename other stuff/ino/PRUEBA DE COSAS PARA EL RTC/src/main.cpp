#include <Arduino.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>   

LiquidCrystal_I2C lcd(0x20,20,4);

void imresion_de_hora_del_dia();
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now = RTC.now();       // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
//
const int pulsador6 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;
const int pulsador5 = 6;
byte hora;
byte minutos;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  //iniccializacion del RTC 
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien
  //
  lcd.init();
  lcd.backlight();
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
}

void loop() {
  //delay(500);
  lcd.setCursor(0,0); 
  lcd.print(now.hour()); // Horas
  lcd.print(':');
  lcd.print(now.minute()); // Minutos
  lcd.print(':');
  lcd.print(now.second());  //Segundos

  if (pulsador1 == LOW)
  {
    while (digitalRead(pulsador2) == LOW){}
    hora++;
    lcd.setCursor(3,0); 
    lcd.print(hora);
  }
  if (pulsador2 == LOW)
  {
    while (digitalRead(pulsador2) == LOW){}
    hora--;
    lcd.setCursor(3,0); 
    lcd.print(hora);
  }
  if (pulsador3 == LOW)
  {
    while (digitalRead(pulsador2) == LOW){}
    minutos++;
    lcd.setCursor(4,0); 
    lcd.print(minutos);
  }
  if (pulsador4 == LOW)
  {
    while (digitalRead(pulsador2) == LOW){}
    minutos--;
    lcd.setCursor(4,0); 
    lcd.print(minutos);
  }

  if (pulsador6 == LOW)
  {
    while (digitalRead(pulsador2) == LOW){}
    RTC.adjust(DateTime());
  }
}

