#include <Arduino.h>
#include <Wire.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);
//LiquidCrystal lcd(12,11,10,9,8,7);
//tiene que cambiar de estado cada 1.25v
const int sensado_de_nivel = A0;
int mili_segundos = 0;

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel;

void setup() {
  Serial.begin(9600);
  //lcd.begin(16,2);
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  lcd.init();
  lcd.backlight();
  pinMode(sensado_de_nivel, INPUT);
}

void loop() {

  lcd.setCursor(0,4);
  lcd.print(mili_segundos);
  lcd.setCursor(0,0);
  switch (nivel)
  {
    case tanque_vacio:
      lcd.print("tanque al: 0%");
      break;
    case tanque_al_25:
      lcd.print("tanque al: 25%"); 
      break;  
    case tanque_al_50:
      lcd.print("tanque al: 50%");
      break;
    case tanque_al_75:
      lcd.print("tanque al: 75%");
      break;
    case tanque_al_100:
      lcd.print("tanque al: 100%");
      break;
  }
    if (mili_segundos >= 5000)
    {
      lcd.clear();
      if (analogRead(sensado_de_nivel) >= 100 && analogRead(sensado_de_nivel) < 256)    nivel = tanque_al_25;
      if (analogRead(sensado_de_nivel) >= 256 && analogRead(sensado_de_nivel) < 512)    nivel = tanque_al_50;
      if (analogRead(sensado_de_nivel) >=512  && analogRead(sensado_de_nivel) < 768)    nivel = tanque_al_75;
      if (analogRead(sensado_de_nivel) >= 768 && analogRead(sensado_de_nivel) <= 1024)    nivel = tanque_al_100;
      if (analogRead(sensado_de_nivel) < 100) nivel = tanque_vacio;  
      
      mili_segundos = 0;
    }
}

ISR(TIMER2_OVF_vect){
    mili_segundos++;
}