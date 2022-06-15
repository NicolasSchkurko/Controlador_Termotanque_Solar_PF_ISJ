#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void actualizar_MEF();
void imprimir_en_pantalla();
//optimizar
LiquidCrystal_I2C lcd(0x27,16,2);
//Pines que usa el menu para manejarce
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;
//
typedef enum{estado_inicial,Carga_x_sensor,Calienta_x_hora,Calienta_x_sensor,Calienta_manual,estado_5} estadoMEF;  
estadoMEF estadoActual;

char variable_para_imprimir_en_el_lcd [20];

void actualizar_MEF();

void setup() 
{
  lcd.init();
  lcd.backlight();
  pinMode(pulsador1, INPUT);
  pinMode(pulsador2, INPUT);
  pinMode(pulsador3, INPUT);
  pinMode(pulsador4, INPUT);
}

void loop() 
{
  if(digitalRead(pulsador1)== HIGH) estadoActual = Carga_x_sensor;
  if(digitalRead(pulsador2)== HIGH) estadoActual = Calienta_x_hora;
  if(digitalRead(pulsador3)== HIGH) estadoActual = Calienta_x_sensor;
  if(digitalRead(pulsador1)== HIGH) estadoActual = Calienta_manual;
  actualizar_MEF();
}

void actualizar_MEF()
{
  lcd.setCursor(0,0);
  switch (estadoActual)
  {
    case estado_inicial:
      strcpy(variable_para_imprimir_en_el_lcd, "Llenado manual");
      //llenado_manual();
      break;
    case Carga_x_sensor:
      strcpy(variable_para_imprimir_en_el_lcd, "Carga x sensor");
      // carga_por_sensor();
      break;
    case Calienta_x_hora:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta x hora");
      // calentamiento_por_hora();
    case Calienta_x_sensor:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta x sensor");
      // calienta_por_sensor();
    case Calienta_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "Calienta manual");
      // calefaccion_manual();
  }
}

void imprimir_en_pantalla(){
  lcd.print(variable_para_imprimir_en_el_lcd);
}
