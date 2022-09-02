// Libs
#include <Arduino.h>
#include <AT24CX.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"

#include "SubMenues.h"
#include "menu.h"
#include "FuncionesSoporte.h"
#include "comunicacion.h"

//████████████████████████████████████████████████████████████████████
// Defines
// Control de temp
// Entradas y salidas
#define encoder0PinA 2
#define encoder0PinB 3
#define nivel_del_tanque A3
#define onewire 9 // pin del onewire
#define tiempo_para_temperatura 5000
#define tiempo_para_nivel 3000

void Controltemp();
void Controllvl();
void ControlPorHora();
void Actualizar_entradas();
void Sum_Encoder();
void doEncodeA();
void doEncodeB();

AT24C32 eep;
RTC_DS1307 rtc;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t);
LiquidCrystal_I2C lcd(0x27, 20, 4); // LiquidCrystal_I2C lcd(0x27,20,4);

typedef enum
{
  posicion_inicial,
  llenado_manual,
  calefaccion_manual,
  funcion_menu_de_auto_por_hora,
  llenado_auto,
  calefaccion_auto,
  funcion_de_menu_modificar_hora_rtc,
  funcion_farenheit_celsius,
  funcion_activar_bomba,
  funcion_de_menu_seteo_wifi
} Seleccionar_Funciones;
typedef enum
{
  estado_standby,
  estado_inicial,
  menu1,
  menu2,
  funciones
} estadoMEF;

Seleccionar_Funciones funcionActual = posicion_inicial;
estadoMEF Estadoequipo = estado_inicial;

char WIFISSID[20];
char WIFIPASS[20];
char LCDMessage[20];

uint8_t encoder0Pos;
uint16_t mili_segundos = 0;
uint16_t tiempo_sensores;
uint8_t nivel_actual;
uint8_t temperatura_a_calentar;
uint8_t nivel_a_llenar;
uint8_t hora, minutos;
int8_t temperatura_actual; // temp actual
uint16_t Tiempo_encoder;

bool last_encoder_state;
bool Resistencia;
bool Valvula;
bool calentar;
bool Activar_bomba;
bool llenar;
bool use_farenheit;
//█████████████████████████████████████████████████████████████████████████████████
// Codigo
void setup()
{
  // Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2 | 0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //
  // pulsadores pra manejar los menus//
  DDRD &= B00001111; // setea input
  DDRB &= B00011100;

  PORTD |= B11110000; // setea pull up o pull down

  pinMode(nivel_del_tanque, INPUT); // pines  nivel
  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncodeA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncodeB, CHANGE);
  // encoder pin on interrupt 0 (pin 2)
  Wire.begin();
  Serial.begin(9600); // inicializacion del serial arduino-esp
  rtc.begin();        // inicializacion del rtc arduino-esp
  Serial.setTimeout(200);
  lcd.init(); // Iniciacion del LCD
  // Sensr de temperatura
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);

  for (uint8_t i = 0; i < 19; i++)
  {
    WIFIPASS[i] = eep.read(14 + i);
    WIFISSID[i] = eep.read(34 + i);
  }

  mili_segundos = 0;
  while (mili_segundos <= 3000)
  {
    if (mili_segundos == 1)
      Serial_Send_UNO(1, 0);
    if (mili_segundos == 500)
      Serial_Send_UNO(1, 1);
    if (mili_segundos == 500)
      Serial_Send_UNO(1, 2);
    if (mili_segundos == 1500)
      Serial_Send_UNO(1, 3);
    if (mili_segundos == 2000)
      Serial_Send_UNO(1, 4);
    if (mili_segundos == 2500)
      Serial_Send_UNO(1, 5);
  }
  tiempo_sensores = mili_segundos;
}
void loop()
{
  Actualizar_entradas();
  Controllvl();
  Controltemp();
  ControlPorHora();

  if (Serial.available() > 0)
    Serial_Read_UNO(); // si recibe un dato del serial lo lee

  switch (Estadoequipo)
  {
  case estado_standby:
    standby(use_farenheit, nivel_actual, temperatura_actual, hora, minutos, encoder0Pos); // sin backlight
    break;
  case estado_inicial:
    standby(use_farenheit, nivel_actual, temperatura_actual, hora, minutos, encoder0Pos); // con backlight
    break;
  case menu1:
    menu_basico(encoder0Pos);
    break;
  case menu2:
    menu_avanzado(encoder0Pos);
    break;
  case funciones:
    if (funcionActual == llenado_manual)
      nivel_a_llenar = menu_de_llenado_manual(nivel_actual, encoder0Pos);
    if (funcionActual == calefaccion_manual)
      temperatura_a_calentar = menu_de_calefaccion_manual(temperatura_actual, use_farenheit, encoder0Pos);
    if (funcionActual == funcion_menu_de_auto_por_hora)
      menu_de_auto_por_hora(hora, minutos, use_farenheit, encoder0Pos);
    if (funcionActual == llenado_auto)
      menu_de_llenado_auto(encoder0Pos); // Menu principal
    if (funcionActual == calefaccion_auto)
      menu_de_calefaccion_auto(use_farenheit, encoder0Pos); // Menu principal
    if (funcionActual == funcion_de_menu_modificar_hora_rtc)
      menu_modificar_hora_rtc(hora, minutos, encoder0Pos); // Menu avanzado
    if (funcionActual == funcion_farenheit_celsius)
      menu_farenheit_celsius(Activar_bomba, encoder0Pos); // Menu avanzado
    if (funcionActual == funcion_activar_bomba)
      menu_activar_bomba(use_farenheit, encoder0Pos); // Menu avanzado
    if (funcionActual == funcion_de_menu_seteo_wifi)
      menu_seteo_wifi(WIFISSID, WIFIPASS, encoder0Pos); // Menu avanzado
    break;
  }
}

ISR(TIMER2_OVF_vect)
{
  mili_segundos++;
}

void Actualizar_entradas()
{ // Sexo y adaptarlo para no usar delay farenheit
  if (mili_segundos >= tiempo_sensores + tiempo_para_temperatura)
  {
    Sensor_temp.requestTemperatures();
    temperatura_actual = Sensor_temp.getTempCByIndex(0);

    if (analogRead(nivel_del_tanque) < 100)
      nivel_actual = 0;
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)
      nivel_actual = 25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)
      nivel_actual = 50;
    if (analogRead(nivel_del_tanque) >= 512 && analogRead(nivel_del_tanque) < 768)
      nivel_actual = 75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)
      nivel_actual = 100;
    tiempo_sensores = mili_segundos;
  }
  DateTime now = rtc.now(); // iguala la variable datetime al valor del rtc
  hora = now.hour();
  minutos = now.minute();
}

void Controltemp()
{
  if (temperatura_actual <= eep.read(10) || temperatura_actual < temperatura_a_calentar)
    Resistencia = true;
  if (temperatura_actual > eep.read(11) || temperatura_actual >= temperatura_a_calentar)
    Resistencia = false;
  PrintOutput(10, Resistencia);
}

void Controllvl()
{
  if (nivel_actual <= eep.read(12) || nivel_actual < nivel_a_llenar)
    Valvula = true;
  if (nivel_actual > eep.read(13) || nivel_actual >= nivel_a_llenar)
    Valvula = false;
  PrintOutput(11, Valvula);
  if (Activar_bomba)
    PrintOutput(12, Valvula);
}

void ControlPorHora()
{
  char Array_hora[6];
  Printhora(Array_hora, hora, minutos);
  for (uint8_t i; i < 3; i++)
  {
    if (ArrayToChar(1, Array_hora) == eep.read(i + 1))
    {
      temperatura_a_calentar = eep.read(i + 2);
      nivel_a_llenar = eep.read(i + 3);
    }
  }
}

void doEncodeA()
{
  if (mili_segundos > Tiempo_encoder + 25)
  {
    if (PressedButton(40) == PressedButton(41))
      encoder0Pos++;
    else
      encoder0Pos--;
    Tiempo_encoder = mili_segundos;
  }
}
void doEncodeB()
{
  if (mili_segundos > Tiempo_encoder + 25)
  {
    if (PressedButton(40) != PressedButton(41))
      encoder0Pos++;
    else
      encoder0Pos--;
    Tiempo_encoder = mili_segundos;
  }
}


