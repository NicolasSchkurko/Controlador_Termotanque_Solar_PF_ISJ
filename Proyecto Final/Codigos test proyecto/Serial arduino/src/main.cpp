#include <Arduino.h>
#include <stdio.h>

//SOLO EN TEST
String Device= "ARDUINO";
//==================================
void Serial_Read_UNO();
String Serial_Input;
String ActualValue;
char input=0;
uint8_t StringLength=0;

uint8_t InfoPos=0;
bool ResetString=true;
char Actualchar=0;
uint8_t hora=0;
uint8_t min=0;
uint8_t horasave=0;
uint8_t tempsave=0;
uint8_t lvlsave=0;
uint8_t savestruct=0;

void setup() {
Serial.begin(9600);
}

void loop() {
  Serial_Read_UNO();
}

void Serial_Read_UNO(){
  Serial_Input=Serial.readString();
  StringLength= Serial_Input.length();
  input=Serial_Input.charAt(0);
  for (uint8_t CharPos = 2; CharPos < StringLength; CharPos++)
  {
    if(CharPos==2)ActualValue=Serial_Input.charAt(CharPos);
    else ActualValue+=Serial_Input.charAt(CharPos);
  }

  switch (input)
  {
  case 'H':
    hora=ActualValue.toInt();
    Serial.println(hora);
    break;

  case 'M':
    min=ActualValue.toInt();
    Serial.println(min);
    break;

  case 'K':
    
    for (uint8_t CharPos2 = 2; CharPos2 < StringLength; CharPos2++)
    {
      Actualchar=Serial_Input.charAt(CharPos2);
      if (Actualchar != ':')
      {
        if (ResetString==true){ActualValue=Actualchar; ResetString=false;}
        else ActualValue+=Actualchar; 
        Serial.println(Actualchar);
      }
      else 
      {
        switch (InfoPos)
        {
        case 0:
        horasave=ActualValue.toInt();
        Serial.println(horasave);
        ResetString=true;
          break;

        case 1:
        tempsave=ActualValue.toInt();
        Serial.println(tempsave);
        ResetString=true;
          break;

        case 2:
        lvlsave=ActualValue.toInt();
        Serial.println(lvlsave);
        ResetString=true;
          break;

        case 3:
        savestruct=ActualValue.toInt();
        Serial.println(savestruct);
        ResetString=true;
          break;
        }
        InfoPos++;
      }

    break;

  default:

    break;
  }
}
}