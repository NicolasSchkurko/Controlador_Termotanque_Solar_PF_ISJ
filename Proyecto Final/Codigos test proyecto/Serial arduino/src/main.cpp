#include <Arduino.h>
#include <stdio.h>

//SOLO EN TEST
String Device= "ARDUINO";
//==================================
void Serial_Read_UNO();
void Serial_Send_UNO(uint8_t);
String Serial_Input;
<<<<<<< HEAD
String ActualValue;
String OTROVALIU;
char input=0;
uint8_t StringLength=0;
=======
String Individualdata[4];
String IndividualValue;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
uint8_t temp_a_calentar=0;
uint8_t nivel_a_llenar=0;
uint8_t ActualStruct=0;
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 
>>>>>>> 2d554643a7d21654259823a4e190981be575d8e5

char Actualchar=0;
char input=0;

bool ConvertString=false;
bool StringTaked=false;
bool stateheating=false;
bool ComunicationError=false;
bool InitComunication=true;

void setup() {
Serial.begin(9600);
}

void loop() {
  while(Serial.available() > 0){Serial_Read_UNO();}
}

void Serial_Read_UNO(){
  
  Serial_Input=Serial.readString();// iguala el string del serial a un string imput
  StringLength= Serial_Input.length();// saca el largo del string
  Serial.println(Serial_Input);// Solo de verificacion (eliminar en el final)
  input=Serial_Input.charAt(0); // toma el char de comando (el primer char usualmente una letra)

  for (uint8_t CharPos = 2; CharPos <= StringLength; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
      if(Serial_Input.charAt(CharPos)==':')ActualIndividualDataPos++; //si hay : divide los datos
      if(Serial_Input.charAt(CharPos)!=':' && Serial_Input.charAt(CharPos)!='_' && Serial_Input.charAt(CharPos)!='/')// si no es nungun caracter especial:
        {
          if(Serial_Input.charAt(CharPos-1)==':' || Serial_Input.charAt(CharPos-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(CharPos);//si es el primer digito lo iguala
          else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(CharPos);//si es el segundo en adelante lo suma
        }
      if(CharPos==StringLength)ConvertString=true;// activa el comando final (flag)
  }

  switch (input)//dependiendo del char de comando
  {
  case 'H':
    if (ConvertString==true)
      {
        temp_a_calentar=Individualdata[0].toInt();
        if(Individualdata[1].toInt()>=1)Serial.println("on");//save ycalentado manual();
        else Serial.println("off");//only save
      }
    break;

  case 'C':
    if (ConvertString==true)
      {
        nivel_a_llenar=Individualdata[0].toInt();
        if(Individualdata[1].toInt()>=1)Serial.println("on");//save ycalentado manual();
        else Serial.println("off");//only save
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;

  case 'K':
<<<<<<< HEAD
    
    for (uint8_t CharPos2 = 0; CharPos2 < StringLength-2; CharPos2++)
    {
      Actualchar=ActualValue.charAt(CharPos2);
      if (Actualchar != ':')
      {
        if (ResetString==true){OTROVALIU=Actualchar; ResetString=false;}
        else OTROVALIU+=Actualchar; 
        Serial.println(Actualchar);
      }
      else 
      {
        switch (InfoPos)
        {
        case 0:
        horasave=OTROVALIU.toInt();
        Serial.println(horasave);
        ResetString=true;
          break;

        case 1:
        tempsave=OTROVALIU.toInt();
        Serial.println(tempsave);
        ResetString=true;
          break;

        case 2:
        lvlsave=OTROVALIU.toInt();
        Serial.println(lvlsave);
        ResetString=true;
          break;

        case 3:
        savestruct=OTROVALIU.toInt();
        Serial.println(savestruct);
        ResetString=true;
          break;
        }
        InfoPos++;
      }

=======
    if (ConvertString==true)
<<<<<<< HEAD
      {
        ActualStruct=Individualdata[3].toInt();
        if (ActualStruct<=2 && ActualStruct>=0)
          {
            save[ActualStruct].hour=Individualdata[0].toInt();
            save[ActualStruct].temp=Individualdata[1].toInt();
            save[ActualStruct].level=Individualdata[2].toInt();
            ActualIndividualDataPos=0;
            StringTaked=false;
            ConvertString=false;
            ComunicationError=false;
          }
        else Serial.println("error");
      }
=======
    {
      ActualStruct=Individualdata[3].toInt();;
      save[ActualStruct].hour=Individualdata[0].toInt();
      Serial.println(save[ActualStruct].hour);
      save[ActualStruct].temp=Individualdata[1].toInt();
      Serial.println(save[ActualStruct].temp);
      save[ActualStruct].level=Individualdata[2].toInt();
      Serial.println(save[ActualStruct].level);
      ActualIndividualDataPos=0;
      StringTaked=false;
      ConvertString=false;
    }
>>>>>>> 2d554643a7d21654259823a4e190981be575d8e5
>>>>>>> e152052007854d2010991178542efd8ca8dc65d9
    break;

  case 'J':
    if (ConvertString==true)
      {
        save[3].hour=255;
        save[4].temp=Individualdata[1].toInt();// tempmin
        save[4].level=Individualdata[2].toInt();// tempmax
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;
      case 'V':
    if (ConvertString==true)
      {
        save[4].hour=255;
        save[4].temp=Individualdata[1].toInt();// lvlmin
        save[4].level=Individualdata[2].toInt();// lvlmax
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;
    case 'E':
    if (ConvertString==true)
      {
        if(Individualdata[0]=="ERROR")
        {
              Serial.println("?_RESET");
              ComunicationError=true;
        }
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
      }
    break;
    case 'O':
    if (ConvertString==true)
      {
        if(Individualdata[0]=="OK")
        {
              ComunicationError=false;
        }
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
      }
    break;
  default:
    Serial.println("?_NOTHING TO READ");
    break;
  }
}

void Serial_Send_UNO(uint8_t WhatSend)
  {
    uint8_t MessagePoss;
    if (InitComunication==true)MessagePoss=0;
    switch (WhatSend){
      case 1:
        if (ComunicationError==false && InitComunication==true && MessagePoss<=5)//&& Send_time>=1000)
        {
            switch (MessagePoss)
            {
              case 0:
                Serial.println("S_""SSID"":""Pass");
                 MessagePoss++;
                break;
              case 1:
                Serial.println("K_HORA:TEMP:LVL:0");
                MessagePoss++;
                break;
              case 2:
                Serial.println("K_HORA:TEMP:LVL:1");
                MessagePoss++;
                break;
              case 3:
                Serial.println("K_HORA:TEMP:LVL:2");
                MessagePoss++;
                break;
              case 4:
                Serial.println("J_255:TEMPMIN:TEMPMAX:3");
                MessagePoss++;
                break;
              case 5:
                Serial.println("V_255:TEMPMIN:TEMPMAX:3");
                InitComunication=false;
                MessagePoss=0;
                break;
            // delay de 1 seg
            }
    
            //Send_time =0;
        }
        break;
      case 2:
        if (ComunicationError==false && InitComunication==false)//&& Send_time>=2000)
          {
              Serial.println("U_TEMP:LVL:HORA:STATE");
              //Send_time =0;
          }
      case 3:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("K_HORA:TEMP:LVL:STRUCTPOS");
          }
      case 4:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("J_255:TEMPMIN:TEMPMAX:3");
          }
      case 5:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("V_255:LVLMIN:LVLMAX:4");
          }
    }
    
  }