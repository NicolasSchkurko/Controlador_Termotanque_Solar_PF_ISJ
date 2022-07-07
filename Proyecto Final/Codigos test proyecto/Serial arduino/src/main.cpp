#include <Arduino.h>
#include <stdio.h>

//SOLO EN TEST
String Device= "ARDUINO";
//==================================
void Serial_Read_UNO();
String Serial_Input;
String Individualdata[4];
String IndividualValue;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
uint8_t temp_a_calentar=0;
uint8_t nivel_a_llenar=0;
uint8_t ActualStruct=0;
struct save_data{ char hour; char level; char temp;};
save_data save[5]; 

char Actualchar=0;
char input=0;

bool ConvertString=false;
bool StringTaked=false;

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
      //calentado manual();
    }
    break;

  case 'C':
    if (ConvertString==true)
    {
      nivel_a_llenar=Individualdata[0].toInt();
      //llenadomanual();
    }
    break;

  case 'K':
    if (ConvertString==true)
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
    break;

  default:

    break;
  }
}