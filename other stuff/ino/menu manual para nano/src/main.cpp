#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;

int sumador_5 = 5;
bool confirmar = false;
int final_temp = 40;

//LiquidCrystal_I2C lcd(0x20,20,4);

void limpiar_pantalla_y_escribir();
LiquidCrystal lcd(12,11,10,9,8,7);
void setup() 
{
  lcd.begin(16,2);
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  Serial.begin(9600);
  //lcd.init();
  //lcd.backlight();
  limpiar_pantalla_y_escribir();
}

void loop() 
{
  //while(1)
  //{
    if(digitalRead(pulsador1) == LOW)
    {
      while(digitalRead(pulsador1) == LOW){}
      final_temp += sumador_5;
      if (final_temp > 80)    final_temp = 80;
      confirmar = false;
      limpiar_pantalla_y_escribir();
    }
    if(digitalRead(pulsador2) == LOW)
    {
      while(digitalRead(pulsador2) == LOW){}
      final_temp -= sumador_5;
      if (final_temp < 40)    final_temp = 40;
      confirmar = false;
      limpiar_pantalla_y_escribir();
    }
    if(digitalRead(pulsador5) == LOW)
    {
      while(digitalRead(pulsador5) == LOW){}
      confirmar = true;
      limpiar_pantalla_y_escribir();
    }
  //}
}
void limpiar_pantalla_y_escribir (){
  
  lcd.clear();
  if (confirmar == false)
  {
    lcd.setCursor(0,0);
    lcd.print(final_temp);
  }
  if (confirmar == true)
  {
    lcd.print("Temp: ");
    lcd.setCursor(6,0);
    lcd.print(final_temp);
  }
}
