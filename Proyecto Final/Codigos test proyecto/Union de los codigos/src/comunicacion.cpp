#include <Arduino.h>
#include "comunicacion.h"
#include "FuncionesSoporte.h"
#include "AT24CX.h"
#include <SPI.h>

void Serial_Send_UNO(uint8_t,uint8_t);

extern uint16_t tiempo_envio_mensajes;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern char WIFISSID [20];
extern char WIFIPASS [20];
extern int8_t temperatura_a_calentar; 
extern int8_t nivel_a_llenar; 
extern int8_t temperatura_actual; // temp actual
extern  bool Activar_bomba;
extern bool calentar;
extern bool llenar;
extern bool Can_Read;
extern uint32_t  mili_segundos;
extern AT24C32 eep;
extern uint8_t hora,minutos;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
char Serial_Input [60];
char Individualdata[4][20];
char input=0;
uint8_t Auxiliar2;
uint8_t ActualStruct=0; 

bool Take_Comunication_Data=false;
bool ComunicationError=false;
bool InitComunication=true;  

void Serial_Read_UNO(){
  uint8_t gap;
  StringLength = Serial.available();
  for(int i=0; i<StringLength; i++)
  {
      Serial_Input[i] = Serial.read();
  }// iguala el string del serial a un string input
  input=Serial_Input[0]; // toma el char del comando a realizar (usualmente una letra)
  // Separador del string en variables:
  for (Auxiliar2 = 2; Auxiliar2 <=StringLength; Auxiliar2++){ // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
    if(Serial_Input[Auxiliar2]==':'){
      ActualIndividualDataPos++;
      gap=Auxiliar2;
    } //si hay : divide los datos

    if(Serial_Input[Auxiliar2]!=':'){// si no es nungun caracter especial:
      Individualdata[ActualIndividualDataPos][Auxiliar2-gap]=Serial_Input[Auxiliar2];//copia al individual
    }
    if(Auxiliar2==80)Take_Comunication_Data=true; //comienza a igualar variables
  } 
  
  if(Take_Comunication_Data==true){
    switch (input){//dependiendo del char de comando
    case 'H':
      temperatura_a_calentar=atoi(Individualdata[0]);
      if(Individualdata[1]=="ON")calentar=true;
      if(Individualdata[1]=="Off")calentar=false;
      Take_Comunication_Data=false;
      break;

    case 'C':
      nivel_a_llenar=atoi(Individualdata[0]);
      if(Individualdata[1]=="ON")llenar=true;
      if(Individualdata[1]=="Off")llenar=false;
      Take_Comunication_Data=false;
      break;

    case 'K':
      ActualStruct=atoi(Individualdata[3]);
      if (ActualStruct<=2 && ActualStruct>=0){
          eep.write((ActualStruct*3)+1,atoi(Individualdata[0]));
          eep.write((ActualStruct*3)+2,atoi(Individualdata[1]));
          eep.write((ActualStruct*3)+3,atoi(Individualdata[2]));
          ActualIndividualDataPos=0;
          Take_Comunication_Data=false;
        }
      break;

    case 'J':
      eep.write(10,atoi(Individualdata[0]));
      eep.write(11,atoi(Individualdata[1]));
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'V':
      eep.write(12,atoi(Individualdata[0]));
      eep.write(13,atoi(Individualdata[1]));
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'E':
      Serial_Send_UNO(6,0);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'O':
      if(Individualdata[0]=="OK"){
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      }
      break;
    }
  }
}

void Serial_Send_UNO(uint8_t WhatSend,uint8_t What_slot){
  uint8_t HourChar;
  if (ComunicationError==false){
    switch (WhatSend){
      case 1:
        if (InitComunication==true){
            switch (What_slot){
              case 0:
                sprintf( Serial_Input, "S_%s:%s:",WIFISSID,WIFIPASS);
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                break;
              case 1:
                sprintf( Serial_Input, "K_%d:%d:%d",eep.read(1),eep.read(2),eep.read(3));
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                break;
              case 2:
                sprintf( Serial_Input, "K_%d:%d:%d",eep.read(4),eep.read(5),eep.read(6));
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                break;
              case 3:
                sprintf( Serial_Input, "K_%d:%d:%d",eep.read(7),eep.read(8),eep.read(9));
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                break;
              case 4:
                sprintf( Serial_Input, "J_%d:%d",eep.read(10),eep.read(11));
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                break;
              case 5:
                sprintf( Serial_Input, "V_%d:%d",eep.read(12),eep.read(13));
                Serial.println( Serial_Input);
                tiempo_envio_mensajes=mili_segundos;
                InitComunication=false;
                Can_Read=false;
                 
                break;
          }  
        }
      break;
      case 2:
        if(InitComunication==false){
          sprintf( Serial_Input, "K_%d:%d:%d",eep.read((What_slot*3)+1),eep.read((What_slot*3)+2),eep.read((What_slot*3)+3));
          Serial.println( Serial_Input);
          Can_Read=false;
           
          }
        break;
      case 3:
        if(InitComunication==false){
          sprintf( Serial_Input, "J_%d:%d",eep.read(10),eep.read(11));
          Serial.println( Serial_Input);
          Can_Read=false;
           
          }
        break;
      case 4:
        if(InitComunication==false){                
          sprintf( Serial_Input, "V_%d:%d",eep.read(12),eep.read(13));
          Serial.println( Serial_Input);
          Can_Read=false;
           
          }
        break;
        case 5:
        if(InitComunication==false){
          Printhora( Serial_Input,hora,minutos);
          HourChar=ArrayToChar(1, Serial_Input);
          sprintf( Serial_Input, "U_%d:%d:%d",HourChar,nivel_actual,temperatura_actual);
          Can_Read=false;
           
        }
          Serial.println( Serial_Input);
        break;
      case 6:
        if(InitComunication==false){
          sprintf( Serial_Input, "S_%s:%s:",WIFISSID,WIFIPASS);
          Serial.println( Serial_Input);;
          Can_Read=false;
           
          }
      default:
        Can_Read=false;
         
        break;
    }
  }
  if (ComunicationError==true){Serial.println(F("?_RESET"));}//resetea esp 
}
