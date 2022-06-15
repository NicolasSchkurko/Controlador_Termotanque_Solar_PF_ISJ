#include <Arduino.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
float poteA0=0;
const int led1=1;
const int led2=2;
const int led3=3;
const int led4=4;
const int led5=5;
const int pulsador=0;
float tension=0;
unsigned long tiempo=0;
char estado_test=0;

void tension_lcd();
void led_voltaje();
void led_test();
int temp_delay();

void setup() {
lcd.begin(16,2);
pinMode(led1,OUTPUT);
pinMode(led2,OUTPUT);
pinMode(led3,OUTPUT);
pinMode(led4,OUTPUT);
pinMode(led5,OUTPUT); 
pinMode(pulsador,INPUT); 
}

void loop() {
  poteA0= analogRead(A0);
  tension= poteA0*5/1023;
  tension_lcd();
  led_voltaje();
  if (digitalRead(pulsador)==HIGH || estado_test==1)
  {
  led_test();
  }
}

void led_voltaje(){
  if (tension >= 1) digitalWrite(led1, HIGH);
  if (tension < 1) digitalWrite(led1, LOW);
  if (tension >= 2) digitalWrite(led2, HIGH);
  if (tension < 2) digitalWrite(led2, LOW);
  if (tension >= 3) digitalWrite(led3, HIGH);
  if (tension < 3) digitalWrite(led3, LOW);
  if (tension >= 4) digitalWrite(led4, HIGH);
  if (tension < 4) digitalWrite(led4, LOW);
  if (tension >= 5) digitalWrite(led5, HIGH);
  if (tension < 5) digitalWrite(led5, LOW);
}

void tension_lcd(){
  lcd.setCursor(0,0);
  lcd.print("Tension");
  lcd.setCursor(0,1);
  lcd.print(tension);
}

void led_test(){
if(estado_test==0){
estado_test=1;
tiempo=millis();
}
if (millis()>=tiempo+500)digitalWrite(led1, HIGH);
if (millis()>=tiempo+1000)digitalWrite(led2, HIGH);
if (millis()>=tiempo+1500)digitalWrite(led3, HIGH);
if (millis()>=tiempo+2000)digitalWrite(led4, HIGH);
if (millis()>=tiempo+2500)digitalWrite(led5, HIGH);
if (millis()>=tiempo+3000){
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);
  digitalWrite(led5,LOW);
  tiempo=millis();
  estado_test=0;
}
}