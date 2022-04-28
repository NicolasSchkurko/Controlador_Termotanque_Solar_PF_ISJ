#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
float sensor=0;
float voltsens=0;
int temp=0;
int led1=7;
int led2=6;
int puls1=5;
int puls2=4;
int ledstate=0;
int state=0;
int statelcd=0;

void setup(){
lcd.begin(16,2);
pinMode(led1,OUTPUT);
pinMode(led2,OUTPUT);
pinMode(puls1,INPUT);
pinMode(puls2,INPUT);
}
void loop() 
{

if(digitalRead(puls1)==HIGH&&state==0){
  digitalWrite(led1,HIGH);
  digitalWrite(led2,HIGH);  
  state=1;
}
if(digitalRead(puls2)==HIGH&&state>=1){
  state=0;
  lcd.clear();
}
if (state==1){
   sensor = analogRead(A0);
   voltsens=(sensor*100)/1023;
   temp=voltsens;
   lcd.setCursor(0,0);
   lcd.print("t:");
   lcd.setCursor(3,0);
   lcd.print(temp);
  if(temp>=45&&ledstate==0){
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    ledstate=1;
  }
  if(temp>=35&&ledstate==0){
    digitalWrite(led2,LOW);
    
  }
  if(temp<30&&ledstate==1){
    digitalWrite(led1,HIGH);
    digitalWrite(led2,HIGH);
    ledstate=0;
  }
  
  }
}
