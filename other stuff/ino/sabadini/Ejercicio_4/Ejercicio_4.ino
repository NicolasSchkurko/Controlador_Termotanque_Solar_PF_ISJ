#include <LiquidCrystal.h>
LiquidCrystal lcd(9,8,7,6,5,4);
const int pulsador=1;

void setup() {
  // put your setup code here, to run once:
lcd.begin(16,2);
pinMode(pulsador,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
lcd.home();
if(digitalRead(pulsador)==HIGH){
lcd.print("hola mundo");
}
if(digitalRead(pulsador)==LOW){
lcd.clear();
}
}
