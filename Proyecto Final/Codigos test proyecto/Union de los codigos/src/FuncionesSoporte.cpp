#include "FuncionesSoporte.h"
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

extern LiquidCrystal_I2C lcd;
extern char LCDMessage[20];

uint8_t ReturnToCero (int8_t actualpos, uint8_t maxpos)
{ 
  uint8_t realvalue;
  if (actualpos>=maxpos){
    realvalue = 0 + actualpos % maxpos;       
    return realvalue;
  }
  if (actualpos<0){
    realvalue = maxpos + actualpos;          
    return realvalue;
  }
  if (actualpos>0 && actualpos<maxpos) return actualpos;
  return (0);
}

uint8_t CharToUINT(uint8_t function,uint8_t save)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t resto_deconvert=0;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ 
  switch (function)
    {
      case 1:
        resto_deconvert= (save) % 4;
        var1_deconvert= (save-resto_deconvert)/4;
        return var1_deconvert;
      break;
      case 2:
        resto_deconvert= (save) % 4;
        var1_deconvert=resto_deconvert*15;
        return var1_deconvert;
      break;
      case 3:
        return save;
      break;
      default:
        break;
    }
    return (0);
}

uint8_t StringToChar(uint8_t function, String toconvert) //// ya arregle lo de colver
{
  uint8_t var1_convert; // solo una variable (_convert  nos evita modificar variables globales como bldos)
  uint8_t var2_convert; // solo una variable
  uint8_t resto_convert; //guarda el resto del calculo de tiempo
  switch (function)
  {
    case 1:
      var1_convert=(toconvert.charAt(0)- '0')*10;// toma el valor del primer digito del string y lo convierte en int (numero de base 10)
      var1_convert+=(toconvert.charAt(1)-'0');// toma el valor del segundo digito
      var1_convert=var1_convert*4;//multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

      var2_convert=(toconvert.charAt(3)- '0')*10;//lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
      var2_convert+=(toconvert.charAt(4)-'0');//lo mismo que en el var 1 pero con minutos
      resto_convert=var2_convert%15; //saca el resto (ejemplo 7/5 resto 2)
      if(resto_convert<8) var2_convert= var2_convert-resto_convert; //utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
      else var2_convert=var2_convert+15-resto_convert;// utiliza el resto para redondear arriba
      var2_convert=var2_convert/15;// convierte los minutos en la proporcion del char (1 entero = 15 minutos)

      var1_convert+=var2_convert;// suma horas y minutos
      if(var1_convert>=96)var1_convert=0;
      return var1_convert;
      break;
    case 2:
      var1_convert= toconvert.toInt();// hace magia y despues de tirar magia convierte el string en un int (fua ta dificil la conversion de ete)
      return var1_convert;
      break;
    default:
      var2_convert= toconvert.toInt();// mismo sistema
      return var2_convert;
      break;
  }
  return (0);
}

String String_de_hora (uint8_t hora_entrada, uint8_t minuto_entrada){
  if(hora_entrada > 9 && minuto_entrada>9) return(String(hora_entrada)+":"+String(minuto_entrada));
  if(hora_entrada <= 9 && minuto_entrada>9) return("0"+String(hora_entrada)+":"+String(minuto_entrada));
  if(hora_entrada > 9 && minuto_entrada<=9) return(String(hora_entrada)+":0"+String(minuto_entrada));
  if(hora_entrada <= 9 && minuto_entrada<=9) return("0"+String(hora_entrada)+":0"+String(minuto_entrada));
}

void Printhora (uint8_t hora_entrada, uint8_t minuto_entrada){
  if(hora_entrada > 9 && minuto_entrada>9)sprintf(LCDMessage,"%d:%dhs",hora_entrada,minuto_entrada);
  if(hora_entrada <= 9 && minuto_entrada>9)sprintf(LCDMessage,"0%d:%dhs",hora_entrada,minuto_entrada);
  if(hora_entrada > 9 && minuto_entrada<=9)sprintf(LCDMessage,"%d:0%dhs",hora_entrada,minuto_entrada);
  if(hora_entrada <= 9 && minuto_entrada<=9)sprintf(LCDMessage,"0%d:0%dhs",hora_entrada,minuto_entrada);
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
  if ((PIND & (1<<PD5)) == 0 || (PIND & (1<<PD4)) == 0 || (PIND & (1<<PD3)) == 0 || (PIND & (1<<PD2)) == 0){while((PIND & (1<<PD5)) == 0 || (PIND & (1<<PD4)) == 0 || (PIND & (1<<PD3)) == 0 || (PIND & (1<<PD2)) == 0){} return true;} 
  else return false;
  break;
}
}

void PrintLCD (char buffer[20], uint8_t Column, uint8_t Row){
lcd.setCursor(Column,Row); 
lcd.print(buffer);
}