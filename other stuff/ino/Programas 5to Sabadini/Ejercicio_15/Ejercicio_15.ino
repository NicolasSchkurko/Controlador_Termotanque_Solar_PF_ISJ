#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
const int pulsador1=1;
const int pulsador2=2;
const int pulsadorint=3;
int validacion=0;
int activador=0;
int i=0;
void setup() {
attachInterrupt(digitalPinToInterrupt(3),INTERRUPT,LOW);
lcd.begin(16,2);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
pinMode(pulsadorint,INPUT);
}

void loop() {
  lcd.setCursor(0,0);
  lcd.print("pulse para");
  lcd.setCursor(0,1);
  lcd.print("iniciar");
  
  if(digitalRead(pulsador1)==HIGH || activador==1)
  {
    for(i=0; i<=9;i++)
    {
      if(i==9)activador=0;
      if(validacion==1 )
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("estado de");
        lcd.setCursor(0,1);
        lcd.print("interrupcion");
        delay(1000);
        validacion=0;
        activador=1;
      }
      if(validacion==0)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(i);
        delay(1000);
      }
    }
  }
  
  if(digitalRead(pulsador2)==HIGH || activador==2)
  {
    for(i=9; i>=0;i--)
    {
      if(i==0)activador=0;
      if(validacion==1)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("estado de");
        lcd.setCursor(0,1);
        lcd.print("interrupcion");
        delay(1000);
        validacion=0;
        activador=2;
      }
      if(validacion==0)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(i);
        delay(1000);
      }
    }
  }
}

void INTERRUPT(){
  validacion=1;
}

