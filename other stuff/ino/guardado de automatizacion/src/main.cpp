#include <Arduino.h>
void convercionhora1(int modo,char hora,char tempylvl);
void convercionhora(int horas, int minuto,int nivel, int temp);
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
  23:45=094             /                   95%   18x
    q                   /                   100%  20x
    */    
void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
void desconvercionhora(char hora,char tempylvl)
{
int minuto=0;
int horas=0;
int aux=0;

minuto=(hora*15)-aux;
if (minuto==60)
  {
    aux=60;
    horas++;
  }

int temp=0;
int nivel=0;
int aux2=0;
int digito=0;
int digito_cal=0;

switch (tempylvl)
{
  case 255:
    int temp=0;
    int nivel=0;
    int minuto=0;
    int horas=0;
    break;

  default:
   while(tempylvl!=0)
{
  digito=tempylvl%10;
  tempylvl=tempylvl/10;
  aux2++;
  if(aux2==3)aux2=0;
}
if(aux2>=0 & aux2<=1)
{
  switch (aux2)
  {
    case 0:
    digito_cal=digito*10;
      break;
    case 1:
    digito_cal=digito+digito_cal;
      break;
  }
digito_cal=digito_cal/2;
nivel=50+(digito_cal*5);
}
if(aux2==2)
{
temp=40+(digito*5);
}

    break;
}

}
void convercionhora(int horas, int minuto,int nivel, int temp)
{
char hora=0;
char tempylvl=0;
int nivel_temp=0;
int temp_temp=0;

hora=(minuto/15)+(horas/15);
nivel_temp=((nivel-50)/5)*20;
temp_temp=(temp-40)/5;
tempylvl=nivel_temp+temp_temp;
}
