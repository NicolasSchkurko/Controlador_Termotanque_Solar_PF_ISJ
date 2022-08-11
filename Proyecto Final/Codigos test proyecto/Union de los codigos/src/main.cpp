//Libs
#include <Arduino.h>
#include "AT24CX.h"
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


uint8_t nivel_actual;
uint32_t tiempo_sensores;
uint32_t  mili_segundos = 0;

  

// Guardado en memoria 
  //Variables EEPROM
  struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};            //guardado 1/hora/level/temp                                   //guardado 2/hora/level/temp
  save_data save[3];                                                        //guardado 3/hora/level/temp
                                                                            //guarda auto temp = 255/temp_min/temp_max
                                                                            //guarda auto lvl = 255/level_min/level_max

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
char WIFISSID [19];
char WIFIPASS [19];
int8_t temperatura_a_calentar; 
int8_t nivel_a_llenar; 
int8_t temperatura_actual; // temp actual
bool Activar_bomba;
bool llenar;
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

  save[0].hour=eep.read(1);
  save[0].level=eep.read(2);
  save[0].temp=eep.read(3);
  save[1].hour=eep.read(4);
  save[1].level=eep.read(5);
  save[1].temp=eep.read(6);
  save[2].hour=eep.read(7);
  save[2].level=eep.read(8);
  save[2].temp=eep.read(9);
  for (uint8_t i = 0; i < 19; i++){
    WIFIPASS[i] = eep.read(14 + i);
    WIFISSID[i] = eep.read(34 + i);
  }
  while (MessagePoss<6)Serial_Send_UNO(1);
}

void loop() 
{
  Actualizar_entradas();
  Controllvl();
  Controltemp();
  if(Serial.available()>0){Serial_Read_UNO();} // si recibe un dato del serial lo lee

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

