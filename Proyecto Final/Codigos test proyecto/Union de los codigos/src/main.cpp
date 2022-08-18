//Libs
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
//Defines
  //Control de temp
  //Entradas y salidas
#define Pote A2
#define nivel_del_tanque A0 
#define electrovalvula 10
#define resistencia 11
#define onewire 9 // pin del onewire
#define tiempo_para_temperatura 5000 
#define tiempo_para_nivel 3000

void Controltemp();
void Controllvl();
void Actualizar_entradas();

AT24C32 eep;
RTC_DS1307 rtc;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t);

uint16_t tiempo_envio_mensajes=0;
uint8_t nivel_actual;
uint32_t tiempo_sensores;
uint32_t  mili_segundos=0;

//Variables menu
typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 
  
Seleccionar_Funciones funcionActual = posicion_inicial;
estadoMEF Estadoequipo = estado_inicial;

  //Variables Comunicacion esp/arduino


uint8_t Auxiliar1;
bool Resistencia;
bool Valvula;

uint8_t MessagePoss=0;
bool calentar;
char WIFISSID [20];
char WIFIPASS [20];
char LCDMessage[20];
int8_t temperatura_a_calentar; 
int8_t nivel_a_llenar; 
int8_t temperatura_actual; // temp actual
bool Activar_bomba;
bool llenar;
bool Can_Read=true;
uint8_t hora,minutos;
LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);
uint16_t tiempo_de_standby=0;
//█████████████████████████████████████████████████████████████████████████████████
//Codigo
void setup() 
{
  //Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //
  Wire.begin();  
  Serial.begin(9600); //inicializacion del serial arduino-esp
  rtc.begin();//inicializacion del rtc arduino-esp

  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); //subirlo solo una unica vez y despues subirlo nuevamente pero comentando (sino cuando reinicia borra config hora)
  Serial.setTimeout(200);
  lcd.init();//Iniciacion del LCD
  pinMode(nivel_del_tanque, INPUT); //pines  nivel
  //Sensr de temperatura
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);
  
  //pulsadores pra manejar los menus//
  DDRD = B11000011; // setea input
  PORTD |= B00111100;
  DDRB &= B111101; 
//

  tiempo_sensores=mili_segundos;

  for (uint8_t i = 0; i < 19; i++){
    WIFIPASS[i] = eep.read(14 + i);
    WIFISSID[i] = eep.read(34 + i);
  }

  mili_segundos=0;

  while(mili_segundos<=3000){
    if(mili_segundos==1)Serial_Send_UNO(1,0);
    if(mili_segundos==500)Serial_Send_UNO(1,1);
    if(mili_segundos==500)Serial_Send_UNO(1,2);
    if(mili_segundos==1500)Serial_Send_UNO(1,3);
    if(mili_segundos==2000)Serial_Send_UNO(1,4);
    if(mili_segundos==2500)Serial_Send_UNO(1,5);
  }  
}
void loop() 
{
  Actualizar_entradas();
  Controllvl();
  Controltemp();
  if(Can_Read==true && Serial.available()>0)Serial_Read_UNO(); // si recibe un dato del serial lo lee

  switch (Estadoequipo){
    case estado_standby:
      standby(); // sin backlight
      break;
    case estado_inicial:
      standby(); // con backlight
      break;
    case menu1:
      menu_basico();
      break;
    case menu2: 
      menu_avanzado();
      break;
    case funciones:
      if(funcionActual==llenado_manual)menu_de_llenado_manual();             //Menu principal
      if(funcionActual==calefaccion_manual)menu_de_calefaccion_manual();     //Menu principal
      if(funcionActual==funcion_menu_de_auto_por_hora)menu_de_auto_por_hora();                     //Menu principal
      if(funcionActual==llenado_auto)menu_de_llenado_auto();                 //Menu principal 
      if(funcionActual==calefaccion_auto)menu_de_calefaccion_auto();         //Menu principal
      if(funcionActual==funcion_de_menu_modificar_hora_rtc)menu_modificar_hora_rtc();  //Menu avanzado
      if(funcionActual==funcion_farenheit_celsius)menu_farenheit_celsius();  //Menu avanzado
      if(funcionActual==funcion_activar_bomba)menu_activar_bomba();        //Menu avanzado
      if(funcionActual==funcion_de_menu_seteo_wifi)menu_seteo_wifi();    //Menu avanzado
      break;
  }
 
}

ISR(TIMER2_OVF_vect){
  mili_segundos++;
  tiempo_de_standby++;
}

void Actualizar_entradas (){ //Sexo y adaptarlo para no usar delay farenheit
  if(mili_segundos>=tiempo_sensores+tiempo_para_temperatura){
    Sensor_temp.requestTemperatures();
    temperatura_actual = Sensor_temp.getTempCByIndex(0);
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = 0;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = 25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = 50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = 75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)  nivel_actual = 100;
    tiempo_sensores=mili_segundos;
    if (Can_Read==false)Can_Read=true;
  }
  DateTime now = rtc.now(); //iguala la variable datetime al valor del rtc
  hora=now.hour();
  minutos=now.minute();
}

void Controltemp()
{
  // control temp min to max
  if(temperatura_actual <= eep.read(10) && Resistencia == false && PORTD !=(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=true;PORTB |= B000010;}
  if(temperatura_actual > eep.read(11) && Resistencia == true && PORTD ==(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=false;PORTB &= B111101;}
  //=========Compara nivel actual con el minimo seteado============
  if (temperatura_actual < temperatura_a_calentar && Resistencia == false && PORTD !=(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=true;PORTB |= B000010;}
  if (temperatura_actual >= temperatura_a_calentar && Resistencia == true && PORTD ==(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=false;PORTB &= B111101;}
  //=================================================================
}

void Controllvl(){
  // control lvl min to max
  if(nivel_actual <= eep.read(12) && Activar_bomba == true && Valvula == false && PORTD !=(1<<PD7)){PORTD ^=(1<<PD7);Valvula=true; PORTB |= B000001;}
  if(nivel_actual > eep.read(13) && Valvula == true && PORTD ==(1<<PD7)){PORTD ^=(1<<PD7);Valvula=true; PORTB &= B111110;}
  //======Compara temperatura actual con el minimo seteado=========
  if(nivel_actual < nivel_a_llenar && Valvula == false && PORTD !=(1<<PD7)) {PORTD ^=(1<<PD7);Valvula=true;PORTB |= B000001;}
  if(nivel_actual >= nivel_a_llenar && Valvula == true && PORTD ==(1<<PD7)) {PORTD ^=(1<<PD7);Valvula=true;PORTB &= B111110;}
  //=================================================================
}

