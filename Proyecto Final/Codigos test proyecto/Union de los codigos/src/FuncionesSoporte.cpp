#include "FuncionesSoporte.h"
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

extern LiquidCrystal_I2C lcd;

uint8_t ReturnToCero (int8_t actualpos, uint8_t maxpos)
{ 
  uint8_t Aux1;
  if (actualpos>=maxpos){
    Aux1 = 0 + actualpos % maxpos;       
    return Aux1;
  }
  if (actualpos<0){
    Aux1 = maxpos + actualpos;          
    return Aux1;
  }
  if (actualpos>0 && actualpos<maxpos) return actualpos;
  return (0);
}

uint8_t CharToUINT(uint8_t function,uint8_t save)
{
  uint8_t Aux1=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t resto=0;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ 
  switch (function)
    {
      case 1:
        resto= (save) % 4;
        Aux1= (save-resto)/4;
        return Aux1;
      break;
      case 2:
        resto= (save) % 4;
        Aux1=resto*15;
        return Aux1;
      break;
      case 3:
        return save;
      break;
      default:
        break;
    }
    return (0);
}

int8_t ArrayToChar(uint8_t function,  char buffer[20]) //// ya arregle lo de colver
{
  uint8_t Aux1, resto; // solo una variable (_convert  nos evita modificar variables globales como bldos)
  int8_t NumeroFinal;
  switch (function)
  {
    case 1:
      Aux1=(buffer[0]- '0')*10;// toma el valor del primer digito del string y lo convierte en int (numero de base 10)
      Aux1+=(buffer[1]-'0');// toma el valor del segundo digito
      NumeroFinal=Aux1*4;//multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

      Aux1=(buffer[3]- '0')*10;//lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
      Aux1+=(buffer[4]-'0');//lo mismo que en el var 1 pero con minutos
      resto=Aux1%15; //saca el resto (ejemplo 7/5 resto 2)
      if(resto<8) Aux1= Aux1-resto; //utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
      else Aux1=Aux1+15-resto;// utiliza el resto para redondear arriba
      Aux1=Aux1/15;// convierte los minutos en la proporcion del char (1 entero = 15 minutos)

      NumeroFinal+=Aux1;// suma horas y minutos
      if(Aux1>=96)NumeroFinal=0;
      return NumeroFinal;
      break;
    default:
      Aux1= atoi(buffer);// mismo sistema
      return Aux1;
      break;
  }
  return (0);
}

void Printhora (char buffer[6],uint8_t hora_entrada, uint8_t minuto_entrada){
  uint8_t Aux1;
  for(Aux1=0; Aux1<20; Aux1++)buffer[Aux1]='\0';
  Aux1=hora_entrada/10;
  buffer[0]=Aux1+'0';
  hora_entrada=hora_entrada-(Aux1*10);
  buffer[1]=hora_entrada+'0';
  buffer[2]=':';
  Aux1=minuto_entrada/10;
  buffer[3]=Aux1+'0';
  minuto_entrada=minuto_entrada-(Aux1*10);
  buffer[4]=minuto_entrada+'0';
  buffer[5]='h';
  buffer[6]='s';
}

char Character_Return(uint8_t Character_pos, bool mayus)
{
  switch (mayus)
  {
    case true:
      if (Character_pos>=0 && Character_pos<=25)return Character_pos+65;
      if (Character_pos>=26 && Character_pos<=29) return Character_pos+9;
      if (Character_pos==30) return 47;
      if (Character_pos==31) return 40;
      if (Character_pos==32) return 41;
      if (Character_pos==33) return 61;
      if (Character_pos==34) return 59;
      if (Character_pos==35) return 58;
      if (Character_pos==36) return 94;
      if (Character_pos==37) return 63;
      if (Character_pos==38) return 95;
      if (Character_pos==39) return 27;
      if (Character_pos>39) return 0;
    break;

    case false:
      if (Character_pos>=0 && Character_pos<=25)return Character_pos+97;
      if (Character_pos>=26 && Character_pos<=35)return Character_pos+22;
      if (Character_pos==36) return 33;
      if (Character_pos==37) return 45;
      if (Character_pos==38) return 64;
      if (Character_pos==39) return 0;
      if (Character_pos>39) return 0;
    break;
  }
  return (0);
}

bool PressedButton (uint8_t Wich_Button){
switch (Wich_Button)
{
case 1:
  if ((PIND & (1<<PD2)) == 0){while((PIND & (1<<PD2)) == 0){}return true;}
  else return false;
  break;
case 2:
  if ((PIND & (1<<PD3)) == 0){while((PIND & (1<<PD3)) == 0){} return true;}
  else return false;
  break;
case 3:
  if ((PIND & (1<<PD4)) == 0){while((PIND & (1<<PD4)) == 0){} return true;}
  else return false;
  break;
case 4:
  if ((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} return true;}
  else return false;
  break;
default:
  if ((PIND & (1<<PD5)) == 0 || (PIND & (1<<PD4)) == 0 || (PIND & (1<<PD3)) == 0 || (PIND & (1<<PD2)) == 0)return true;
  else return false;
  break;
}
}

void PrintLCD (char buffer[20], uint8_t Column, uint8_t Row){
lcd.setCursor(Column,Row); 
lcd.print(buffer);
}