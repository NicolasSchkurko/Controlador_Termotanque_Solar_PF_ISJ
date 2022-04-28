#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
float sensor=0;
float voltsens=0;
int temp=0;
unsigned long tiempo=0;
int state=0;
void setup(){
  lcd.begin(16,2);

}
void loop() 
{
  if (state==0){
    tiempo=millis();
    state=1;
  }
 sensor = analogRead(A0);
 voltsens=(sensor*5)/1023;
 temp=100*voltsens;
 if(millis()-tiempo>=1000){
 lcd.setCursor(0,0);
 lcd.print("c:");
  lcd.setCursor(2,0);
 lcd.print(sensor);
  lcd.setCursor(0,1);
  lcd.print("v:");
  lcd.setCursor(2,1);
 lcd.print(voltsens);
  lcd.setCursor(8,1);
   lcd.print("t:");
   lcd.setCursor(10,1);
 lcd.print(temp);
 tiempo=millis();
 if(temp<99&&state==1){
 lcd.setCursor(10,1);
 lcd.print("              ");
 state=2;
 }
  if(temp<9&&state==2){
 lcd.setCursor(10,1);
 lcd.print("              ");
 state=3;
 }
  if(temp>99)state=1;
  if(temp>9&&temp<99)state=2;
 }
}
