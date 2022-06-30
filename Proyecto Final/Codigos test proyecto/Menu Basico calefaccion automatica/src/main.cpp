#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// este es un menu de vefiracion donde al confirmanr
// se debe enviar la infomacion guardada para que se ponga la resistencia a calentar el agua
void limpiar_pantalla_y_escribir();

const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;

int sumador = 5;
int temperatura_inicial = 40;
int temperatura_final = 40;
int min_temp_ini = 40;
int maxima_temp_fin = 80;
bool confirmar = false;

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() 
{
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Seleccione temp");
  lcd.setCursor(0,1);
  lcd.print("minima");
  //delay(1000);                      //Usar timer integrado
  lcd.clear();
}

void loop() 
{
  if(digitalRead(pulsador1) == LOW)
  {
    while(digitalRead(pulsador1) == LOW){}
    temperatura_inicial += sumador;
    confirmar = false;
    lcd.clear();
  }
    if (temperatura_inicial > maxima_temp_fin) temperatura_inicial = maxima_temp_fin;

  if(digitalRead(pulsador2) == LOW)
  {
    while(digitalRead(pulsador2) == LOW){}
    temperatura_inicial -= sumador;
    confirmar = false;
    lcd.clear();
  }
  if (temperatura_inicial < min_temp_ini) temperatura_inicial = min_temp_ini;

  if(digitalRead(pulsador3) == LOW)
  {
    while(digitalRead(pulsador3) == LOW){}
    temperatura_final += sumador;
    confirmar = false;
    lcd.clear();
  }
  if (temperatura_final < temperatura_inicial) temperatura_final = temperatura_inicial;

  if(digitalRead(pulsador4) == LOW)
  {
    while(digitalRead(pulsador4) == LOW){}
    temperatura_final -= sumador;
    confirmar = false;
    lcd.clear();
  }
  if(temperatura_final > maxima_temp_fin) temperatura_final = maxima_temp_fin;

  if(digitalRead(pulsador5) == LOW)
  {
    while(digitalRead(pulsador5) == LOW){}
    confirmar = true;
    limpiar_pantalla_y_escribir ();
  }
  if(confirmar == false)
  {
    lcd.setCursor(0,0);
    lcd.print("Minima:");
    lcd.print(temperatura_inicial);
    lcd.setCursor(0,1);
    lcd.print("Maxima:");
    lcd.print(temperatura_final);
  }
}

void limpiar_pantalla_y_escribir (){
  lcd.clear();
  if (confirmar == true)
  {
    lcd.clear();
    lcd.print("Temp minima: ");
    lcd.setCursor(12,0);
    lcd.print(temperatura_inicial);
    lcd.setCursor(0,1);
    lcd.print("Temp maxima: ");
    lcd.setCursor(12,1);
    lcd.print(temperatura_final);
  }
}