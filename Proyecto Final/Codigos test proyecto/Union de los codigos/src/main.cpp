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
#define encoder0PinA A0
#define encoder0PinB A1
#define nivel_del_tanque A3 
#define electrovalvula 10
#define resistencia 11
#define onewire 9 // pin del onewire
#define tiempo_para_temperatura 5000 
#define tiempo_para_nivel 3000

void Controltemp();
void Controllvl();
void ControlPorHora();
void Actualizar_entradas();
void Sum_Encoder();



AT24C32 eep;
RTC_DS1307 rtc;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t);
LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);

typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 
  
Seleccionar_Funciones funcionActual = posicion_inicial;
estadoMEF Estadoequipo = estado_inicial;

char WIFISSID [20];
char WIFIPASS [20];
char LCDMessage[20];

uint8_t encoder0Pos;
uint8_t last_encoder_state;
uint16_t mili_segundos=0;
uint16_t tiempo_sensores;
uint16_t Tiempo_encoder;
uint8_t nivel_actual;
uint8_t temperatura_a_calentar; 
uint8_t nivel_a_llenar; 
uint8_t hora,minutos;
int8_t temperatura_actual; // temp actual

bool Resistencia;
bool Valvula;
bool calentar;
bool Activar_bomba;
bool llenar;
bool use_farenheit;
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
  pinMode(encoder0PinA, INPUT_PULLUP); 
  pinMode(encoder0PinB, INPUT_PULLUP);
  // encoder pin on interrupt 0 (pin 2)
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
  tiempo_sensores=mili_segundos; 
  last_encoder_state=digitalRead(encoder0PinA);
}
void loop() 
{
  Sum_Encoder();
  Actualizar_entradas();
  Controllvl();
  Controltemp();
  ControlPorHora();

  if(Serial.available()>0)Serial_Read_UNO(); // si recibe un dato del serial lo lee

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
  }
  DateTime now = rtc.now(); //iguala la variable datetime al valor del rtc
  hora=now.hour();
  minutos=now.minute();
}

void Controltemp()
{
  if(temperatura_actual <= eep.read(10)|| temperatura_actual < temperatura_a_calentar )Resistencia=true;
  if(temperatura_actual > eep.read(11) ||temperatura_actual >= temperatura_a_calentar)Resistencia=false;

  if(Resistencia)PORTD = PORTD | 0b01000000; // make bit 2 of PORT D a 0 (clear the bit), leaving rest alone
  else PORTD = PORTD & 0b10111111;
}

void Controllvl(){
  if(nivel_actual <= eep.read(12) || nivel_actual < nivel_a_llenar)Valvula=true;
  if(nivel_actual > eep.read(13) ||nivel_actual >= nivel_a_llenar)Valvula=false;
  
  if(Valvula)PORTD = PORTD | 0b10000000; // make bit 2 of PORT D a 0 (clear the bit), leaving rest alone
  else PORTD = PORTD & 0b01111111;
}

void ControlPorHora(){
  uint8_t Byte_hora_actual;
  char Array_hora[6];

  Printhora(Array_hora,hora,minutos);
  Byte_hora_actual=ArrayToChar(1,Array_hora);

  for(uint8_t i; i<3;i++)
  if(Byte_hora_actual==eep.read(i+1)){
    temperatura_a_calentar=eep.read(i+2); 
    nivel_a_llenar=eep.read(i+3); 
  }
}

void Sum_Encoder(){
    if(mili_segundos > Tiempo_encoder+20){
      if(digitalRead(encoder0PinA)!=last_encoder_state){
        if (digitalRead(encoder0PinB) == digitalRead(encoder0PinA))encoder0Pos--;
        else encoder0Pos++;
        last_encoder_state=digitalRead(encoder0PinA);
      }
      Tiempo_encoder=mili_segundos;
    }
  }


