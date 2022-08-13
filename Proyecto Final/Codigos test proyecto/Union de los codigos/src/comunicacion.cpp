#include <Arduino.h>
#include "comunicacion.h"
#include "FuncionesSoporte.h"
#include "AT24CX.h"
#include <SPI.h>

void Serial_Send_UNO(uint8_t);

struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;}; 

uint16_t tiempo_envio_mensajes;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern save_data save[3];
extern char WIFISSID [20];
extern char WIFIPASS [20];
extern char LCDMessage[20];
extern int8_t temperatura_a_calentar; 
extern int8_t nivel_a_llenar; 
extern int8_t temperatura_actual; // temp actual
extern  bool Activar_bomba;
extern bool calentar;
extern bool llenar;
extern uint8_t MessagePoss;
extern uint32_t  mili_segundos;
extern AT24C32 eep;
extern uint8_t hora,minutos;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
String Serial_Input;
String Individualdata[4];
char input=0;
uint8_t Auxiliar2;
uint8_t ActualStruct=0; 

bool Take_Comunication_Data=false;
bool ComunicationError=false;
bool InitComunication = true;  

void Serial_Read_UNO(){
  Serial_Input=Serial.readString();// iguala el string del serial a un string input
  StringLength= Serial_Input.length();// saca el largo del string
  input=Serial_Input.charAt(0); // toma el char del comando a realizar (usualmente una letra)
  // Separador del string en variables:
  for (Auxiliar2 = 2; Auxiliar2 <= StringLength; Auxiliar2++){ // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
    if(Serial_Input.charAt(Auxiliar2)==':') ActualIndividualDataPos++; //si hay : divide los datos
    else{// si no es nungun caracter especial:
      if(Serial_Input.charAt(Auxiliar2-1)==':' || Serial_Input.charAt(Auxiliar2-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(Auxiliar2);//si es el primer digito lo iguala
      else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(Auxiliar2);//si es el segundo en adelante lo suma al string
    }
    if(Auxiliar2==StringLength)Take_Comunication_Data=true; //comienza a igualar variables
  } 
  
  if(Take_Comunication_Data==true){
    switch (input){//dependiendo del char de comando
    case 'H':
      temperatura_a_calentar=Individualdata[0].toInt();
      if(Individualdata[1]=="ON")calentar=true;
      if(Individualdata[1]=="Off")calentar=false;
      Take_Comunication_Data=false;
      break;

    case 'C':
      nivel_a_llenar=Individualdata[0].toInt();
      if(Individualdata[1]=="ON")llenar=true;
      if(Individualdata[1]=="Off")llenar=false;
      Take_Comunication_Data=false;
      break;

    case 'K':
      ActualStruct=Individualdata[3].toInt();
      if (ActualStruct<=2 && ActualStruct>=0){
          eep.write((ActualStruct*3)+1, Individualdata[0].toInt());
          save[ActualStruct].hour=Individualdata[0].toInt();
          eep.write((ActualStruct*3)+2, Individualdata[1].toInt());
          save[ActualStruct].temp=Individualdata[0].toInt();
          eep.write((ActualStruct*3)+3, Individualdata[2].toInt());
          save[ActualStruct].level=Individualdata[0].toInt();
          ActualIndividualDataPos=0;
          Take_Comunication_Data=false;
        }
      break;

    case 'J':
      eep.write(10,Individualdata[0].toInt());
      eep.write(11,Individualdata[1].toInt());
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'V':
      eep.write(12,Individualdata[0].toInt());
      eep.write(13,Individualdata[1].toInt());
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'E':
      Serial_Send_UNO(6);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'O':
      if(Individualdata[0]=="OK"){
      if (InitComunication==true)Serial_Send_UNO(1);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      }
      break;
    }
  }
}

void Serial_Send_UNO(uint8_t WhatSend){
  if (ComunicationError==false){
    switch (WhatSend){
      case 1:
        if (InitComunication==true && mili_segundos>=tiempo_envio_mensajes+2000){
            switch (MessagePoss){
              case 0:
                Serial.print(("S_"));Serial.print(String(WIFISSID));Serial.print(F(":"));Serial.println(String(WIFIPASS)+":");
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                break;
              case 1:
                Serial.print(("K_"));Serial.print(save[0].hour);Serial.print(F(":"));Serial.print(save[0].temp);Serial.print(F(":"));Serial.println(save[0].level);
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                break;
              case 2:
                Serial.print(("K_"));Serial.print(save[1].hour);Serial.print(F(":"));Serial.print(save[1].temp);Serial.print(F(":"));Serial.println(save[1].level);
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                break;
              case 3:
                Serial.print(("K_"));Serial.print(save[2].hour);Serial.print(F(":"));Serial.print(save[2].temp);Serial.print(F(":"));Serial.println(save[2].level);
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                break;
              case 4:
                Serial.print(("J_"));Serial.print(eep.read(10));Serial.print(F(":"));Serial.println(eep.read(11));
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                break;
              case 5:
                Serial.print(("V_"));Serial.print(eep.read(12));Serial.print(F(":"));Serial.println(eep.read(13));
                tiempo_envio_mensajes=mili_segundos;
                MessagePoss++;
                InitComunication=false;
                break;
          }  
        }
      break;
      case 2:
        if(InitComunication==false){Serial.print(("K_"));Serial.print(save[0].hour);Serial.print(F(":"));Serial.print(save[0].temp);Serial.print(F(":"));Serial.println(save[0].level);}
        break;
      case 3:
        if(InitComunication==false){Serial.print(("J_"));Serial.print(eep.read(10));Serial.print(F(":"));Serial.println(eep.read(11));}
        break;
      case 4:
        if(InitComunication==false){Serial.print(("V_"));Serial.print(eep.read(12));Serial.print(F(":"));Serial.println(eep.read(13));}
        break;
      case 5:
        Printhora(LCDMessage,hora,minutos);
        if(InitComunication==false){Serial.print(("U_"));Serial.print(ArrayToChar(1,LCDMessage));Serial.print(F(":"));Serial.print(nivel_actual);Serial.print(F(":"));Serial.println(temperatura_actual);}
        break;
      case 6:
        if(InitComunication==false){Serial.print(("S_"));Serial.print(WIFISSID);Serial.print(F(":"));Serial.println(WIFIPASS);}
    }
  }
  if (ComunicationError==true){Serial.println(F("?_RESET"));}//resetea esp 
}
