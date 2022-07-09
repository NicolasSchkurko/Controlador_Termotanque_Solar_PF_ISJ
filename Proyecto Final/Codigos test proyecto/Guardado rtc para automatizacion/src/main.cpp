#include <Arduino.h>
#include <AT24CX.h>
#define i2c_address 0x50
AT24C32 eep;

struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 
String desconvercionhora(uint8_t,uint8_t,uint8_t,uint8_t);
uint8_t convercionhora(uint8_t, String);

uint8_t hora=91;
uint8_t temp=34;
uint8_t lvl=50;

String horas;
String temps;
String lvls;
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
  22:30=090
  22:45=091
  23:00=092
  23:15=093
  23:30=094
  23:45=095             /                   95%   18x
    q                   /                   100%  20x
    */    
void setup() {
Serial.begin(9600);
eep.write(1, 22);
}
/* 5*3= 15
  Savestring ssid
  Savestring rtc
  */
void loop() {
hora=eep.read(1);
Serial.println(hora);
horas=desconvercionhora(1,hora,temp,lvl);
Serial.println(horas);
}
String desconvercionhora(uint8_t function,uint8_t savehora,uint8_t temp, uint8_t lvl)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t var2_deconvert=0;
  uint8_t resto_deconvert=0;
  String returned;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯
  switch (function)
    {
      case 1:
        resto_deconvert= (savehora) % 4;
        var1_deconvert= (savehora-resto_deconvert)/4;
        var2_deconvert=resto_deconvert*15;
        returned= String(var1_deconvert)+':'+String(var2_deconvert);
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
}
