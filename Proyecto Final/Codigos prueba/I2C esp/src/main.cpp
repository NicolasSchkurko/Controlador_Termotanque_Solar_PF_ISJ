#include <Arduino.h>
#include <iostream>
#include <string>

char conversionSave(char,const char*);
String unconversionPrint(int,int,int);

struct guardado{ char pos; char hour; char level; char temp;};
String imprimir;

void setup() {
  Serial.begin(9600);
conversionSave(1,"12:45");
}

void loop() {
int i=0;
for(i=1;i<5;i++){
  if (i==1)Serial.println((int)conversionSave(1,"01:45"));
  if (i==2)Serial.println((int)conversionSave(2,"01:05"));
  if (i==3)Serial.println((int)conversionSave(3,"100"));
  if (i==3)Serial.println((int)conversionSave(4,"7"));
}
Serial.println(unconversionPrint(1,23,0));
Serial.println(unconversionPrint(2,37,0));
Serial.println(unconversionPrint(3,90,0));
Serial.println(unconversionPrint(1,99,99));
delay(1000);
}

char conversionSave(char func,const char* txt){
  String data=txt;
  int hora,minutos,temp,lvl;
  if (func==1||func==2){
  int stringpos=0;
  for(stringpos=0;stringpos<sizeof(data);stringpos++){
    switch (stringpos){
    // separa el string de hora en los primeros dos digitos (hora)
      case 0:
        hora= 10 * (data[stringpos]- 48);
        break;
      case 1:
        hora+= data[stringpos] - 48;
        break;
    // separa el string de hora en los primeros dos digitos (minutos)
      case 3:
        minutos= 10 * (data[stringpos] - 48);
        break;
      case 4:
        minutos+= data[stringpos]- 48;
        break;
    }
 }
}
  if (func==3 || func==4){
  if (func==3)temp= data.toInt();// convierte el string de temperatura a int
  if (func==4)lvl= data.toInt(); // convierte el string de nivel a int
}
  switch (func){
    case 1:
      return hora;
      break;
    case 2:
      return minutos;
      break;
    case 3:
      return temp;
      break;
    case 4:
      return lvl;
      break;
    default:
      return 0;
      break;
  }
}
String unconversionPrint(int func,int Num1,int Num2){
  String hh;
  String mm;
  String str;
  switch (func){
    case 1:
      if(Num1<10)hh = "0" + (String)Num1;
      else hh= (String)Num1;
      if(Num2<10)mm= "0" + (String)Num2;
      else mm= (String)Num2;
      str= hh + ":" + mm;
      return (str);
      break;
    case 2:
      str = String(Num1);
      return (str);
      break;
    case 3:
      str = String(Num1);
      return (str);
      break;
    default:
      return ("ERROR");
      break;
  }
  }