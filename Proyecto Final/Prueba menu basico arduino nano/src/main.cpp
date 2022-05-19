#include <Arduino.h>
#include <LiquidCrystal.h>

void actualizar_MEF();
void imprimir_en_pantalla();
//optimizar
LiquidCrystal lcd(12,11,10,9,8,7);
//Pines que usa el menu para manejarse
const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;

typedef enum{estado_inicial,Carga_x_sensor,Calienta_x_hora,Calienta_x_sensor,Calienta_manual,llenado_manual} estadoMEF;  
estadoMEF estadoActual;

char variable_para_imprimir_en_el_lcd [20];

void actualizar_MEF();

void setup() 
{
  lcd.begin(16,2);
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
}

void loop() 
{
  if(digitalRead(pulsador1) == LOW) estadoActual = Carga_x_sensor;
  if(digitalRead(pulsador1) == LOW) lcd.clear();
  if(digitalRead(pulsador2) == LOW) estadoActual = Calienta_x_hora;
  if(digitalRead(pulsador2) == LOW) lcd.clear();
  if(digitalRead(pulsador3) == LOW) estadoActual = Calienta_x_sensor;
  if(digitalRead(pulsador3) == LOW) lcd.clear();
  if(digitalRead(pulsador4) == LOW) estadoActual = Calienta_manual;
  if(digitalRead(pulsador4) == LOW) lcd.clear();
  if(digitalRead(pulsador5) == LOW) estadoActual = llenado_manual;
  if(digitalRead(pulsador5) == LOW) lcd.clear();
  
  if(digitalRead(pulsador6) == LOW) estadoActual = estado_inicial;
  if(digitalRead(pulsador6) == LOW) lcd.clear();
  
  actualizar_MEF();
}

void actualizar_MEF()
{
  lcd.setCursor(0,0);
  switch (estadoActual)
  {
    case estado_inicial:
      lcd.print("1       2      3"); lcd.setCursor(0,2); lcd.print("4              5");
    case llenado_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "Llenado manual");
      lcd.print(variable_para_imprimir_en_el_lcd);
      //llenado_manual();
      break;
    case Carga_x_sensor:
      strcpy(variable_para_imprimir_en_el_lcd, "Carga x sensor");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // carga_por_sensor();
      break;
    case Calienta_x_hora:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta x hora");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calentamiento_por_hora();
      break;
    case Calienta_x_sensor:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta x sensor");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calienta_por_sensor();
      break;
    case Calienta_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta manual");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calefaccion_manual();
      break;
  }
}

/*void imprimir_en_pantalla(){
  lcd.print(variable_para_imprimir_en_el_lcd);
}*/
