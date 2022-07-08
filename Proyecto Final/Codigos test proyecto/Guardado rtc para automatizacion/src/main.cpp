#include <Arduino.h>
#include <AT24CX.h>
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 
String desconvercionhora(uint8_t,uint8_t,uint8_t,uint8_t);
uint8_t convercionhora(uint8_t, String);
/*HORARIOS EN BITS HORA /TEMP 40-80(5en5) Y LVL 50 a 100 (5en5)
  00:00=000             / 40° xx0           50%   00x                   null==255
  00:15=001             / 45° xx1           55%   02x
  00:30=002             / 50° xx2           60%   04x
  00:45=003             / 55° xx3           65%   06x
  01:00=004             / 60° xx4           70%   18x 
  01:15=005             / 65° xx5           75%   10x 
  01:30=006             / 70° xx6           80%   12x
  01:45=007             / 75° xx7           85%   14x 
  . . . .               / 80° xx8           90%   16x
  23:45=095             /                   95%   18x
    q                   /                   100%  20x
    */    
void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
String desconvercionhora(uint8_t function,uint8_t savehora,uint8_t temp, uint8_t lvl)
{
  uint8_t resto_decovert=0;
  uint8_t hora_decovert=0;
  uint8_t minuto_decovert=0;
  String returned;

  switch (function)
    {
      case 1:
        resto_decovert= savehora % 4;
        hora_decovert= (savehora-resto_decovert)/4;
        minuto_decovert= 15* resto_decovert;
        returned= String(hora_decovert)+':'+String(minuto_decovert);
        return returned;
      break;
      case 2:
        returned= String(temp);
        return returned;
      break;
      case 3:
        returned= String(lvl);
        return returned;
      break;
      default:
        break;
    }
}


uint8_t convercionhora(uint8_t function, String toconvert)
{
  uint8_t hora_convert=0;
  uint8_t minuto_convert=0;
  uint8_t resto=0;
  switch (function)
  {
    case 1:
      hora_convert=(toconvert.charAt(0)-'0')*10+(toconvert.charAt(1)-'0'); //agarra los primeros char (hora)
      minuto_convert=(toconvert.charAt(3)-'0')*10+(toconvert.charAt(4)-'0'); //agarra los dos ultimos char (minutos)
      hora_convert=hora_convert*4;//multiplica la hora x 4 (la igualacion esta en la documentacion boludin)
      resto= minuto_convert%15; //saca el resto ejemplo 7/5 resto 2
      if(resto<8) minuto_convert= minuto_convert-resto; //redondeapara abajo
      else minuto_convert= minuto_convert+15-resto;// redondea para arriba
      return hora_convert+minuto_convert;
      break;
    case 2:

    default:
      break;
  }
}
