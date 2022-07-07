#include <Arduino.h>
#include <stdio.h>

//SOLO EN TEST
String Device= "ARDUINO";
//==================================
void Serial_Read_UNO();
String Serial_Input;
String ActualValue;
String OTROVALIU;
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

    break;

  default:

    break;
  }
}
}