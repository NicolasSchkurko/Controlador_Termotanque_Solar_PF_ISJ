#include <Arduino.h>
struct automatizacion
{
  char hora; 
  char tempylvl;
};
/*HORARIOS EN BITS HORA /TEMP 40-80(5en5) Y LVL 50 a 100 (5en5)
  00:00=000             / 40° xx0           50%   00x                   null==255
  00:15=001             / 45° xx1           55%   02x
  00:30=002             / 50° xx2           60%   05x
  00:45=003             / 55° xx3           65%   09x
  01:00=004             / 60° xx4           70%   10x 
  01:15=005             / 65° xx5           75%   12x 
  01:30=006             / 70° xx6           80%   15x
  01:45=007             / 75° xx7           85%   19x 
  . . . .               / 80° xx8           90%   20x
  23:45=094             /                   95%   22x
    q                   /                   100%  24x
    */    
void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}