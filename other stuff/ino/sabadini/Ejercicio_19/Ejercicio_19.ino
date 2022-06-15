
#include <TimerOne.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
void interrupcion();
const int pulsador1=3;
const int pulsador2=2;
int segundos=0;
int milisegundos=0;
int i=0;

void setup() {
Timer1.initialize(1000);
Timer1.attachInterrupt(interrupcion);
lcd.begin(16,2);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
}

void loop() {
  if(digitalRead(pulsador1)==HIGH)
  {
    i=1;
    lcd.clear();
  }
    if(digitalRead(pulsador2)==HIGH)
  {
    i=0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("interrupcion");
    delay(200);
    lcd.clear();
    i=1;
  }
lcd.setCursor(0,0);
lcd.print(segundos);
}

void interrupcion (){
  if (i==1)
  {
    milisegundos++;
    if (milisegundos==1000)
    {
      segundos++;
      milisegundos=0;
    }
    if(segundos >= 60)
    {
      segundos=60;
    }
  }
}
