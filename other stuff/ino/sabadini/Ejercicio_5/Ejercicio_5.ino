#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
const int pulsador1=1;
const int pulsador2=2;
int state=0;
unsigned long tiempo1=0;
int tiempo2=0;
int tiempo=0;
int i=0;

void setup() {
lcd.begin(16,2);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
}

void loop() {

  if (state==0){
lcd.home();
lcd.print("comienze a");
lcd.setCursor(0,2);
lcd.print("llenar el tanque");
  }
if (digitalRead(pulsador1)==HIGH && state==0 && digitalRead(pulsador2)==LOW){
  lcd.clear();
  lcd.home();
  lcd.print("ready");
  lcd.setCursor(0,2);
  tiempo1=millis();
if (i<=100 && digitalRead(pulsador2)==LOW ){
  while(i<100&& digitalRead(pulsador2)==LOW){
    if (millis()-tiempo1>=500){
    lcd.setCursor(0,2);
    lcd.print(i);
    lcd.setCursor(3,2);
    lcd.print("%");
    tiempo1=millis();
    i++;
    state=1;
    }
    }
    }

if (i==100&&state==1&& digitalRead(pulsador2)==LOW){
  while(digitalRead(pulsador2 && digitalRead(pulsador2)==LOW)==LOW && state==1){
  if (millis()-tiempo1>=200){
  lcd.setCursor(0,2);
  lcd.print("FULL");
  tiempo1=millis();
  }
  if (millis()-tiempo1>=200){
    lcd.setCursor(0,2);
    lcd.print("        ");
    tiempo1=millis();
  }
  }

}
}

if (digitalRead(pulsador2)==HIGH && i>0){
  
  if (state==1){
    lcd.clear();
  }
  lcd.home();
  lcd.print("full al");
  lcd.setCursor(0,2);
  lcd.print(i);
  lcd.setCursor(3,2);
  lcd.print("%");
  state=3;
}
if (digitalRead(pulsador2)==HIGH && i==0){
  
  if (state==0){
    lcd.clear();
  }
  lcd.home();
  lcd.print("reset");
  lcd.setCursor(0,2);
  state=3;
}
}
