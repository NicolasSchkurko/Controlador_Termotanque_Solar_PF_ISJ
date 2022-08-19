#include <Arduino.h>
#include "comunicacion.h"
#include "FuncionesSoporte.h"
#include "AT24CX.h"
#include <SPI.h>

void Serial_Send_UNO(uint8_t,uint8_t);

extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern char WIFISSID [20];
extern char WIFIPASS [20];
extern int8_t temperatura_a_calentar; 
extern int8_t nivel_a_llenar;
extern  bool Activar_bomba;
extern bool calentar;
extern bool llenar;
extern uint16_t mili_segundos;
extern AT24C32 eep;
extern uint8_t hora,minutos;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
char OutputMessage [40];
char Individualdata[4];
char input=0;
uint8_t ActualStruct=0; 

bool Take_Comunication_Data=false;
bool ComunicationError=false;
bool InitComunication=true;  

void Serial_Read_UNO(){
  StringLength = Serial.available();
  for(uint8_t i=0; i<StringLength; i++)
  {
    if(i==0)input=Serial.read();
    if(i>=2){
      if(Serial.read()==':'){
        ActualIndividualDataPos++;
      } //si hay : divide los datos

      if(Serial.read()!=':'){// si no es nungun caracter especial:
        Individualdata[ActualIndividualDataPos]=Serial.read();//copia al individual
      }
    }
  }
  
  if(Take_Comunication_Data==true){
    switch (input){//dependiendo del char de comando
    case 'H':
      temperatura_a_calentar=Individualdata[0];
      if(Individualdata[1] == 'O' )calentar=true;
      if(Individualdata[1] == 'I' )calentar=false;
      Take_Comunication_Data=false;
      break;

    case 'C':
      nivel_a_llenar=Individualdata[0];
      if(Individualdata[1] == 'O')llenar=true;
      if(Individualdata[1] == 'I')llenar=false;
      Take_Comunication_Data=false;
      break;

    case 'K':
      ActualStruct=Individualdata[3];
      if (ActualStruct<=2 && ActualStruct>=0){
          eep.write((ActualStruct*3)+1,Individualdata[0]);
          eep.write((ActualStruct*3)+2,Individualdata[1]);
          eep.write((ActualStruct*3)+3,Individualdata[2]);
          ActualIndividualDataPos=0;
          Take_Comunication_Data=false;
        }
      break;

    case 'J':
      eep.write(10,Individualdata[0]);
      eep.write(11,Individualdata[1]);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'V':
      eep.write(12,Individualdata[0]);
      eep.write(13,Individualdata[1]);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'E':
      Serial_Send_UNO(6,0);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'O':
      if(Individualdata[1]=='O' ){
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
                sprintf( OutputMessage, "S_%s:%s",WIFISSID,WIFIPASS);
                Serial.println( OutputMessage);
                break;
              case 1:
                sprintf( OutputMessage, "K_%d:%d:%d:1",eep.read(1),eep.read(2),eep.read(3));
                Serial.println( OutputMessage);
                break;
              case 2:
                sprintf( OutputMessage, "K_%d:%d:%d:2",eep.read(4),eep.read(5),eep.read(6));
                Serial.println( OutputMessage);
                break;
              case 3:
                sprintf( OutputMessage, "K_%d:%d:%d:3",eep.read(7),eep.read(8),eep.read(9));
                Serial.println( OutputMessage);
                break;
              case 4:
                sprintf( OutputMessage, "J_%d:%d",eep.read(10),eep.read(11));
                Serial.println( OutputMessage);
                break;
              case 5:
                sprintf( OutputMessage, "V_%d:%d",eep.read(12),eep.read(13));
                Serial.println( OutputMessage);
                InitComunication=false;
                break;
          }  
        }
      break;
      case 2:
        if(InitComunication==false){
          sprintf( OutputMessage, "K_%d:%d:%d:%d",eep.read((What_slot*3)+1),eep.read((What_slot*3)+2),eep.read((What_slot*3)+3),What_slot);
          Serial.println( OutputMessage);
          }
        break;
      case 3:
        if(InitComunication==false){
          sprintf( OutputMessage, "J_%d:%d",eep.read(10),eep.read(11));
          Serial.println( OutputMessage);
          }
        break;
      case 4:
        if(InitComunication==false){                
          sprintf( OutputMessage, "V_%d:%d",eep.read(12),eep.read(13));
          Serial.println( OutputMessage);
          }
        break;
        case 5:
        if(InitComunication==false){
          Printhora( OutputMessage,hora,minutos);
          HourChar=ArrayToChar(1, OutputMessage);
          sprintf( OutputMessage, "U_%d:%d:%d",HourChar,nivel_actual,temperatura_actual);
        }
          Serial.println( OutputMessage);
        break;
      case 6:
        if(InitComunication==false){
          sprintf( OutputMessage, "S_%s:%s:",WIFISSID,WIFIPASS);
          Serial.println( OutputMessage);;
          }
      default:
        break;
    }
  }
  if (ComunicationError==true){Serial.println(F("?_RESET"));}//resetea esp 
}
