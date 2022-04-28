#include <TimerOne.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,9,8);
void miliseg();
void ganador();

unsigned long milisegundos1=0;
unsigned long milisegundos2=0;
char pulsador1_estado=0;
char pulsador2_estado=0;
char juego1=0;
char flag=0;
int contador1;
int contador2;


const int pulsador1=1;
const int pulsador2=2;
const int pulsador3=3;
const int pulsador4=4;

void setup() {
Timer1.initialize(1000);
Timer1.attachInterrupt(miliseg);
pinMode(pulsador1,INPUT_PULLUP);
pinMode(pulsador2,INPUT_PULLUP);
pinMode(pulsador3,INPUT_PULLUP);
pinMode(pulsador4,INPUT_PULLUP);
lcd.begin(16,2);
}

void loop() {
  
if(digitalRead(pulsador1)==LOW|| juego1==1)
{
  if (flag==0)
  {
    juego1=1;
    lcd.setCursor(0,0);
    lcd.print("Bienvenidos");
  
    lcd.setCursor(0,1);
    lcd.print("al juego");
    if(digitalRead(pulsador2)==LOW && flag==0)
    {
      lcd.clear();
      flag=1;
    }
  }
}

if(digitalRead(pulsador2)==LOW|| juego1==2)
{ 
  if (flag==1)
  {
    juego1=2;
    lcd.setCursor(0,0);
    lcd.print(milisegundos1);
    lcd.setCursor(9,0);
    lcd.print(milisegundos2);
    
    lcd.setCursor(0,1);
    lcd.print("P1");
    lcd.setCursor(3,1);
    lcd.print(contador1);
    
    lcd.setCursor(10,1);
    lcd.print("P2");
    lcd.setCursor(13,1);
    lcd.print(contador2);
    
    if(digitalRead(pulsador3)==LOW && pulsador1_estado==0)
    {
      pulsador1_estado=1;
      contador2++;
    }
    if(digitalRead(pulsador3)==HIGH && pulsador1_estado==1)
    {
      pulsador1_estado=0;
    }
    if(digitalRead(pulsador4)==LOW && pulsador2_estado==0)
    {
      pulsador2_estado=1;  
      contador1++;
    }
    if(digitalRead(pulsador4)==HIGH && pulsador2_estado==1)
    {
      pulsador2_estado=0;  
    }
    if (contador1>=20)ganador(1);
    if (contador2>=20)ganador(2);
    
  }
 }
}
void miliseg(){
  if (juego1==2)
  {
    if (contador1<20)
    {
      milisegundos1++;
    }
    if (contador2<20)
    {
      milisegundos2++;
    }
}
}
void ganador(int jugador){
  if(flag==1)lcd.clear();
  flag=2;
  lcd.setCursor(0,0);
  lcd.print("El ganador es");
  if (jugador==1)
  {
    lcd.setCursor(0,1);
    lcd.print("P1 en:");
    lcd.setCursor(7,1);
    lcd.print(milisegundos1);
    lcd.setCursor(12,1);
    lcd.print("mS");
  }
  if (jugador==2){
    lcd.setCursor(0,1);
    lcd.print("P2 en:");
    lcd.setCursor(7,1);
    lcd.print(milisegundos2);
    lcd.setCursor(12,1);
    lcd.print("mS");
  } 
}
