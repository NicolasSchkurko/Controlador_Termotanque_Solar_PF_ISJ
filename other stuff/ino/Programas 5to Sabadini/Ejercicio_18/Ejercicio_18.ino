#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
const int led=7;
const int interruptorwait=3;
const int sensorin=2;
const int sensorout=1;
int contador=0;
int wait=0;
void setup() {
attachInterrupt(digitalPinToInterrupt(interruptorwait), WAIT, CHANGE);  
lcd.begin(16,2);
pinMode(sensorin,INPUT);
pinMode(sensorout,INPUT);
pinMode(led,OUTPUT);

}

void loop() {
  
if(contador>=0 && contador<20 && wait==0)
{
  lcd.setCursor(0,0);
  lcd.print("hay un total de");
  lcd.setCursor(0,1);
  lcd.print(contador);
  if (digitalRead(sensorin)==HIGH && contador<20)
  {
    contador++;
    delay(400); 
  }
  if (digitalRead(sensorout)==HIGH && contador>0)
  {
    contador--;
    delay(400); 
  }
}

if (contador==20 && wait==0)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("no hay lugar");
  
  if (digitalRead(sensorout)==HIGH && contador>0)
  {
    contador--;
    delay(400); 
  }
  delay(200);  
  
}

if (wait==1)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("espere");
  delay(1000);  
  wait=0;
}
}



void WAIT(){
wait=1;
}

