#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
void INTERRUPT();
const int parada=0;
const int led=1;
int lectorA0=0;
float tensionA0=0;
int lectorA1=0;
float tensionA1=0;
int interrupcion=0;
int i=0;
int a=0;


void setup() {
attachInterrupt(digitalPinToInterrupt (2), INTERRUPT, RISING);
lcd.begin(16,2);
pinMode(led,OUTPUT);

}

void loop() {
  lectorA0= analogRead(A0);
  tensionA0= lectorA0*5/1023;

  lectorA1= analogRead(A1);
  tensionA1= lectorA1*5/1023;
  
if (tensionA0<4 || tensionA1<4)
{
  if(interrupcion==0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tension1: ");
    lcd.setCursor(9,0);
    lcd.print(tensionA0);
    lcd.setCursor(0,1);
    lcd.print("Tension2: ");
    lcd.setCursor(9,1);
    lcd.print(tensionA1);
    digitalWrite(led, LOW);
    delay(200);
  }
  if(interrupcion==1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Parada de");
    lcd.setCursor(0,1);
    lcd.print("emergencia");
    delay(1000);
    interrupcion=0;
  }
}
if (tensionA0>=4 && tensionA1>=4)
{
  if(interrupcion==0)
  {
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("RIEGO ACTIVADO");
    digitalWrite(led, HIGH);
    delay(200);
  }
  if(interrupcion==1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Parada de");
    lcd.setCursor(0,1);
    lcd.print("emergencia");
    delay(1000);
    interrupcion=0;
  }
}
}
void INTERRUPT(){
  interrupcion=1;
}

