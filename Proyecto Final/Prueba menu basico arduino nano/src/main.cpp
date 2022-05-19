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

typedef enum{estado_inicial,calefaccion_manual,calefaccion_auto_senstemp,carga_auto_hora,carga_agua_por_nivel,llenado_agua_manual} estadoMEF;  
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
  if(digitalRead(pulsador1) == LOW) estadoActual = calefaccion_manual;
  if(digitalRead(pulsador1) == LOW) lcd.clear();
  if(digitalRead(pulsador2) == LOW) estadoActual = calefaccion_auto_senstemp;
  if(digitalRead(pulsador2) == LOW) lcd.clear();
  if(digitalRead(pulsador3) == LOW) estadoActual = carga_auto_hora;
  if(digitalRead(pulsador3) == LOW) lcd.clear();
  if(digitalRead(pulsador4) == LOW) estadoActual = carga_agua_por_nivel;
  if(digitalRead(pulsador4) == LOW) lcd.clear();
  if(digitalRead(pulsador5) == LOW) estadoActual = llenado_agua_manual;
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
      break;
    case calefaccion_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "Calefaccion man");
      lcd.print(variable_para_imprimir_en_el_lcd);
      //llenado_manual();
      break;
    case calefaccion_auto_senstemp:
      strcpy(variable_para_imprimir_en_el_lcd, "Calef auto temp");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // carga_por_sensor();
      break;
    case carga_auto_hora:
      strcpy(variable_para_imprimir_en_el_lcd, "Carga auto hora");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calentamiento_por_hora();
      break;
    case carga_agua_por_nivel:
      strcpy(variable_para_imprimir_en_el_lcd, "carga por nivel");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calienta_por_sensor();
      break;
    case llenado_agua_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "llenado manual");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calefaccion_manual();
      break;
  }
}

/*void imprimir_en_pantalla(){
  lcd.print(variable_para_imprimir_en_el_lcd);
}*/
