#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador5 = 1;
int nivel = 0;
int sumador = 25;
bool confirmar = false;

LiquidCrystal_I2C lcd(0x27,16,2);

void limpiar_pantalla_y_escribir();

void setup() 
{
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
}

void loop() 
{
  if(digitalRead(pulsador1) == LOW)
  {
    while(digitalRead(pulsador1) == LOW){}
    nivel += sumador;
    confirmar = false;
    lcd.clear();
  }

  if(digitalRead(pulsador2) == LOW)
  {
    while(digitalRead(pulsador2) == LOW){}
    nivel -= sumador;
    confirmar = false;
    lcd.clear();
  }
  
  if(digitalRead(pulsador5) == LOW)
  {
    while(digitalRead(pulsador5) == LOW){}
    confirmar = true;
    limpiar_pantalla_y_escribir ();
  }

  if(nivel < 0) nivel = 0;
  if(nivel > 100) nivel = 100;

  if(confirmar == false)
  {
    lcd.setCursor(0,0);
    lcd.print("Nivel:");
    if(nivel < 10)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(7,0);
      lcd.print("%");
    }
    if(nivel > 9 && nivel < 100)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(8,0);
      lcd.print("%");
    }
    if(nivel == 100)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(9,0);
      lcd.print("%");
    } 
  }
}

void limpiar_pantalla_y_escribir ()
{
  lcd.clear();
  if (confirmar == true)
  {
    lcd.clear();
    lcd.print("Nivel maximo:");
    lcd.print(nivel);
    if(nivel < 10)
    {
      lcd.setCursor(14,0);
      lcd.print("%");
    }
    if(nivel > 9 && nivel < 100)
    {
      lcd.setCursor(15,0);
      lcd.print("%");
    }
    if(nivel == 100)
    {
      lcd.setCursor(16,0);
      lcd.print("%");
    }
  }
}
