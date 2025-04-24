#include <Arduino.h>
uint8_t LargoDatos;
uint8_t i=0;
char LetraString[24];
bool iniciar;
void setup() {
  Serial.begin(9600);
}

void loop() {
  
    if(Serial.available()>0 && iniciar==false){
      LargoDatos=Serial.available();
      iniciar=true;
    }

    if(iniciar){
    if(i<LargoDatos){
      LetraString[i]=Serial.read();
      i++;
    }
    if(i==LargoDatos){
      Serial.print(LetraString);
    }
    if(Serial.available()==0){
      i=0;
      iniciar=false;
    }
    }
  }