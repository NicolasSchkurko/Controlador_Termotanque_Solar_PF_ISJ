#include <LiquidCrystal.h>
const int sensor1=2;
const int sensor2=1;
const int led=13;
unsigned long t=0;
unsigned long t1=0;
int val=0;
LiquidCrystal lcd (12,11,10,9,8,7);
void setup() {
  lcd.begin(16,2);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(led, OUTPUT);
}

void loop() {
  if(digitalRead(sensor1)==HIGH and digitalRead(sensor2)==LOW and val==0){
    t=millis();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("tomando velocidad auto");
    val=1;
  }
  if(digitalRead(sensor2)==HIGH and val==1 ){
    t=millis()-t;
    t1=t/1000;
  if(t>=8000){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("vel. permitida");
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(t1);
    }
  if(t<=8000){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Se tomo foto");
      digitalWrite(led,HIGH);
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(t1);
  }  
    val=0;
  }
}



  
 
