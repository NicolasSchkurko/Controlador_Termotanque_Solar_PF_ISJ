#include <Arduino.h>
#include <AT24CX.h>

char conversionSave(char,const char*);
String unconversionPrint(int,int,int);

struct guardado{ char hour; char level; char temp;};
guardado[5]; // 3 de save normal, 2 de temp y lvl min y max
String imprimir;

void setup() {
Serial.begin(9600);
conversionSave(1,"12:45");
}

void loop() {
delay(1000);
}

//convierte string to char
char conversionSave(char func,const char* txt){
/*SI entra un 1 convierte solo la hora
 con un 2 convierte los minutos
 con un 3 se convierte el temp
 con un 4 se convierte en level*/

String data=txt;
int hora,minutos,tempylvl;
  
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

if (func==3){
    if (func==3)tempylvl = data.toInt();// convierte el string de temperatura a int
}

switch (func){
    case 1:
      return hora;
      break;
    case 2:
      return minutos;
      break;
    case 3:
      return tempylvl;
      break;
    default:
      return 0;
      break;
  }
}
//Char to string
String unconversionPrint(int func,int Num1,int Num2){
  /* Func 1 permite convertir el primer int y segundo en horas y minutos
     el segundo conviertr temperatura y nivel el primer int */
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
    default:
      return ("ERROR");
      break;
  }
  }