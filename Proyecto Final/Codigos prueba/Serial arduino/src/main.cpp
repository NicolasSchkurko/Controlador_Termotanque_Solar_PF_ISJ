#include <Arduino.h>
#include <stdio.h>

//SOLO EN TEST
String Device= "ARDUINO";
//==================================
void Serial_Read_UNO();
void Serial_Send_UNO(uint8_t);
String Serial_Input;
String Individualdata[4];
String IndividualValue;

uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
uint8_t temp_a_calentar=0;
uint8_t nivel_a_llenar=0;
uint8_t ActualStruct=0;
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 

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
}
