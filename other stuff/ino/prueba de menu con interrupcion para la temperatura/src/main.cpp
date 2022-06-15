#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <DallasTemperature.h>

void actualizar_MEF();
void imprimir_en_pantalla();

//optimizar
LiquidCrystal_I2C lcd(0x20,16,2);
//Pines que usa el menu para manejarse
const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;

//Variables para que funcione la interrupcion//
int milisegundos_para_sensar = 5000;
int cuenta = 0;

//RTCds1307//
void actualizar_hora();
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now = RTC.now();       // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)

//Sensor de temperatura//
const int onewire = 6;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 

//Estados y variables para que funcione el menu//
typedef enum{estado_inicial,calefaccion_manual,calefaccion_auto_senstemp,carga_auto_hora,carga_agua_por_nivel,llenado_agua_manual} estadoMEF;  
estadoMEF estadoActual = estado_inicial;
char variable_para_imprimir_en_el_lcd [20];

//Sensor de nivel//
void nivel_de_tanque();
typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel;
const int sensado_de_nivel = A0;

void setup() 
{
  Serial.begin(9600);
  //RTC//
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien
  //LCD con I2C//
  lcd.init();
  lcd.backlight();
  //Pines para manejo de menu//
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  pinMode(sensado_de_nivel, INPUT);
  //Sensor de temperatura//
  Sensor_temp.begin();

  //interrupcion 1 milisegundo//
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;  
}

void loop() 
{
  actualizar_MEF();
  if(digitalRead(pulsador6) == LOW) 
  {
    while (digitalRead(pulsador6) == LOW){}
    estadoActual = estado_inicial;
    lcd.clear();
  }
}

void actualizar_MEF()
{
  lcd.setCursor(0,0);
  switch (estadoActual)
  {
    case estado_inicial:
    lcd.print("1       2      3"); lcd.setCursor(0,2); lcd.print("4"); lcd.setCursor(6,2); lcd.print(Sensor_temp.getTempCByIndex(0)); lcd.setCursor(11,2); lcd.print("  "); lcd.setCursor(15,2); lcd.print("5");
    if(digitalRead(pulsador1) == LOW )
    { 
      while (digitalRead(pulsador1) == LOW){}
      estadoActual = calefaccion_manual;
      lcd.clear();
    }
    if(digitalRead(pulsador2) == LOW) 
    {
      while (digitalRead(pulsador2) == LOW){}
      estadoActual = calefaccion_auto_senstemp;
      lcd.clear();
    }
    if(digitalRead(pulsador3) == LOW) 
    {
      while (digitalRead(pulsador3) == LOW){}
      estadoActual = carga_auto_hora;
      lcd.clear();
    }
    if(digitalRead(pulsador4) == LOW) 
    {
      while (digitalRead(pulsador4) == LOW){}
      estadoActual = carga_agua_por_nivel;
      lcd.clear();
    }
    if(digitalRead(pulsador5) == LOW) 
    {
      while (digitalRead(pulsador5) == LOW){}
      estadoActual = llenado_agua_manual;
      lcd.clear();
    }
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
      strcpy(variable_para_imprimir_en_el_lcd, "Carga por nivel");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calienta_por_sensor();
      break;
    case llenado_agua_manual:
      strcpy(variable_para_imprimir_en_el_lcd, "Llenado manual");
      lcd.print(variable_para_imprimir_en_el_lcd);
      // calefaccion_manual();
      break;
  }
}

ISR(TIMER2_OVF_vect){
  cuenta++;
  if (cuenta == milisegundos_para_sensar)
  {
    Sensor_temp.requestTemperatures(); 
    nivel_de_tanque();
    cuenta = 0;
  }  
}

void nivel_de_tanque(){
  if (analogRead(sensado_de_nivel) >= 100 && analogRead(sensado_de_nivel) < 256)    nivel = tanque_al_25;
  if (analogRead(sensado_de_nivel) >= 256 && analogRead(sensado_de_nivel) < 512)    nivel = tanque_al_50;
  if (analogRead(sensado_de_nivel) >=512  && analogRead(sensado_de_nivel) < 768)    nivel = tanque_al_75;
  if (analogRead(sensado_de_nivel) >= 768 && analogRead(sensado_de_nivel) <= 1024)    nivel = tanque_al_100;
  if (analogRead(sensado_de_nivel) < 100) nivel = tanque_vacio;  
  
  lcd.setCursor(0,3);
  switch (nivel)
  {     //ver en donde esta ubicado en el menu para setear el cursor
    case tanque_vacio:
      lcd.print("tanque al: 0%");
      break;
    case tanque_al_25:
      lcd.print("tanque al: 25%");
      break;  
    case tanque_al_50:
      lcd.print("tanque al: 50%");
      break;
    case tanque_al_75:
      lcd.print("tanque al: 75%");
      break;
    case tanque_al_100:
      lcd.print("tanque al: 100%");
      break;
  }
}

void actualizar_hora(){
  lcd.setCursor(0,4);
  lcd.print(now.hour()); // Horas
  lcd.print(':');
  lcd.print(now.minute(), DEC); // Minutos
}
