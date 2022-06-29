//optimizar
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"

//sensr de temperatura
#define onewire 7
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 
void tomar_temperatura ();
void limpiar_pantalla_y_escribir ();
void control_de_temp_auto(int temperatura_inicial,int temperatura_final);
int control_de_temp_por_menu_manual(int temp_a_alcanzar);
int temperatura_inicial = 40;
int temperatura_final = 40;
int min_temp_ini = 40;
int maxima_temp_fin = 80;
int sumador = 5;
bool confirmar = false;
//

//nivel de agua
const int nivel_del_tanque = A0;
const int electrovalvula = 10;
const int resistencia = 8;
typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_seteado;
niveles nivel_actual;
void sensar_nivel_de_agua();
void sensar_nivel_actual();
int nivel = 0;
int mili_segundos = 0;
//

//cosas del menu princial
void menu_basico();
void standby();
void imprimir_en_pantalla();
//

//cosas del mennu avanzado
void menu_avanzado();
//

//funciones para el RTC se cionecta directamente a los pines SCL Y SDA
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now; 
void imprimir_hora ();
//



//Cosas necesarias para el menu
bool borrar_display = false;
const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;
const int pulsador7 = 6;
bool flag_menu_avanzado = false;
typedef enum{estado_standby,estado_inicial,calefaccion_manual,calefaccion_auto_senstemp,carga_auto_hora,carga_agua_por_nivel,llenado_agua_manual} estadoMEF;  
estadoMEF Menu_principal = estado_inicial;
typedef enum{menu_inicio,set_wifi,activar_bomba,cambio_unidad,set_hora,momento_standby} estadoMEF2;
estadoMEF2 Menu_secundario = menu_inicio;

LiquidCrystal_I2C lcd(0x27,20,4);
//LiquidCrystal_I2C lcd(0x20,20,4);
//

void setup() 
{
  //Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //
  Wire.begin();
  
  //iniccializacion del RTC 
  RTC.begin();
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //la compu, despues comentar para subirlo bien

  //Iniciacion del LCD//
  lcd.init();
  lcd.backlight();
  //
  Serial.begin(9600);

  //pines para el sensor de nivel
  pinMode(nivel_del_tanque, INPUT);
  pinMode(electrovalvula, OUTPUT);
  pinMode(resistencia, OUTPUT);
  //

  //Sensr de temperatura
  Sensor_temp.begin();
  //

  //pulsadores pra manejar los menus//
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  pinMode(pulsador7, INPUT_PULLUP);
  //

}

void loop() 
{
  now = RTC.now();

  if(flag_menu_avanzado == true)
  {
    menu_avanzado();
    Menu_principal = estado_inicial;
  }

  if(flag_menu_avanzado == false)
  {
    menu_basico();
    Menu_secundario = menu_inicio;
  } 

  if(digitalRead(pulsador7) == LOW)
  {
    while(digitalRead(pulsador7) == LOW){}
    lcd.clear();
    mili_segundos = 0;
    flag_menu_avanzado = !flag_menu_avanzado;
  } 

  if(digitalRead(pulsador6) == LOW) 
  {
    while (digitalRead(pulsador6) == LOW){}
    mili_segundos = 0;
    Menu_principal = estado_inicial;
    Menu_secundario = menu_inicio;
    lcd.clear();
  }
  if(mili_segundos >= 10000 && digitalRead(pulsador1) == HIGH && digitalRead(pulsador2) == HIGH && digitalRead(pulsador3) == HIGH && digitalRead(pulsador4) == HIGH && digitalRead(pulsador5) == HIGH && digitalRead(pulsador6) == HIGH)
  {
    Menu_principal = estado_standby;
    Menu_secundario = momento_standby; 
    mili_segundos = 0;
  }
}

void menu_basico()
{
  switch (Menu_principal)
  {
    case estado_standby:
      standby();
      if(digitalRead(pulsador1) == LOW) Menu_principal = estado_inicial;
      if(digitalRead(pulsador2) == LOW) Menu_principal = estado_inicial;
      if(digitalRead(pulsador3) == LOW) Menu_principal = estado_inicial;
      if(digitalRead(pulsador4) == LOW) Menu_principal = estado_inicial;
      if(digitalRead(pulsador5) == LOW) Menu_principal = estado_inicial;
      break;
    case estado_inicial:
      lcd.setCursor(0,0); lcd.print("Calef manual"); lcd.setCursor(0,1); lcd.print("Calef auto"); lcd.setCursor(0,2); lcd.print("Carga hora"); lcd.setCursor(0,3); lcd.print("Carga nivel"); //lcd.setCursor(16,4); lcd.print(Sensor_temp.getTempCByIndex(0));
      if(digitalRead(pulsador1) == LOW )
      { 
        while (digitalRead(pulsador1) == LOW){}
        mili_segundos = 0;
        Menu_principal = calefaccion_manual;
        lcd.clear();
      }
      if(digitalRead(pulsador2) == LOW) 
      {
        while (digitalRead(pulsador2) == LOW){}
        mili_segundos = 0;
        Menu_principal = calefaccion_auto_senstemp;
        lcd.clear();
      }
      if(digitalRead(pulsador3) == LOW) 
      {
        while (digitalRead(pulsador3) == LOW){}
        mili_segundos = 0;
        Menu_principal = carga_auto_hora;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW) 
      {
        while (digitalRead(pulsador4) == LOW){}
        mili_segundos = 0;
        Menu_principal = carga_agua_por_nivel;
        lcd.clear();
      }
      if(digitalRead(pulsador5) == LOW) 
      {
        while (digitalRead(pulsador5) == LOW){}
        mili_segundos = 0;
        Menu_principal = llenado_agua_manual;
        lcd.clear();
      }
      break;
    case calefaccion_manual:
      lcd.setCursor(0,0); lcd.print("Calefaccion man");
      control_de_temp_auto(temperatura_inicial,temperatura_final);
    case calefaccion_auto_senstemp:
      if(digitalRead(pulsador1) == LOW)
      {
        while(digitalRead(pulsador1) == LOW){}
        temperatura_inicial += sumador;
        mili_segundos = 0;
        confirmar = false;
        lcd.clear();
      }
      if (temperatura_inicial > maxima_temp_fin) temperatura_inicial = maxima_temp_fin;

      if(digitalRead(pulsador2) == LOW)
      {
        while(digitalRead(pulsador2) == LOW){}
        temperatura_inicial -= sumador;
        mili_segundos = 0;
        confirmar = false;
        lcd.clear();
      }
      if (temperatura_inicial < min_temp_ini) temperatura_inicial = min_temp_ini;

      if(digitalRead(pulsador3) == LOW)
      {
        while(digitalRead(pulsador3) == LOW){}
        temperatura_final += sumador;
        mili_segundos = 0;
        confirmar = false;
        lcd.clear();
      }
      if (temperatura_final < temperatura_inicial) temperatura_final = temperatura_inicial;

      if(digitalRead(pulsador4) == LOW)
      {
        while(digitalRead(pulsador4) == LOW){}
        temperatura_final -= sumador;
        mili_segundos = 0;
        confirmar = false;
        lcd.clear();
      }
      if (temperatura_final > maxima_temp_fin) temperatura_final = maxima_temp_fin;

      if(digitalRead(pulsador5) == LOW)
      {
        while(digitalRead(pulsador5) == LOW){}
        confirmar = true;
        mili_segundos = 0;
        limpiar_pantalla_y_escribir ();
      }
        if(confirmar == false)
      {
        lcd.setCursor(0,0);
        lcd.print("Minima guardada:");
        lcd.print(temperatura_inicial);
        lcd.setCursor(0,1);
        lcd.print("Maxima guardada:");
        lcd.print(temperatura_final);
        control_de_temp_auto(temperatura_inicial,temperatura_final);
      }
      break;
    case carga_auto_hora:
      lcd.setCursor(0,0); lcd.print("Carga auto hora");
      // calentamiento_por_hora();
      break;
    case carga_agua_por_nivel:
      lcd.setCursor(0,0); lcd.print("Carga por nivel");
      // calienta_por_sensor();
      break;
    case llenado_agua_manual:
      lcd.setCursor(0,0); lcd.print("Llenado manual");
      // calefaccion_manual();
      break;
  }
}

void menu_avanzado()
{
  switch (Menu_secundario)
  {
    case momento_standby:
      standby();
      if(digitalRead(pulsador1) == LOW) Menu_secundario = menu_inicio;
      if(digitalRead(pulsador2) == LOW) Menu_secundario = menu_inicio;
      if(digitalRead(pulsador3) == LOW) Menu_secundario = menu_inicio;
      if(digitalRead(pulsador4) == LOW) Menu_secundario = menu_inicio;
      if(digitalRead(pulsador5) == LOW) Menu_secundario = menu_inicio;
    break;  
    
    case menu_inicio:
      lcd.setCursor(0,0); lcd.print("Setear wifi"); lcd.setCursor(0,1); lcd.print("Activar bomba"); lcd.setCursor(0,2); lcd.print("Cambio unidad"); lcd.setCursor(0,3); lcd.print("Setear hora");
      if(digitalRead(pulsador1) == LOW )
      { 
        while (digitalRead(pulsador1) == LOW){}
        mili_segundos = 0;
        Menu_secundario = set_wifi;
        lcd.clear();
      }
      if(digitalRead(pulsador2) == LOW) 
      {
        while (digitalRead(pulsador2) == LOW){}
        mili_segundos = 0;
        Menu_secundario = activar_bomba;
        lcd.clear();
      }
      if(digitalRead(pulsador3) == LOW) 
      {
        while (digitalRead(pulsador3) == LOW){}
        mili_segundos = 0;
        Menu_secundario = cambio_unidad;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW) 
      {
        while (digitalRead(pulsador4) == LOW){}
        mili_segundos = 0;
        Menu_secundario = set_hora;
        lcd.clear();
      }
    break;

    case set_wifi:
      lcd.setCursor(0,0); lcd.print("Seteo WIFI");
    //setea wifi, falta programar
    break;

    case activar_bomba:
      lcd.setCursor(0,0); lcd.print("Activar bomba");
    //activa bomba de agua por relay, falta programar
    break;

    case cambio_unidad:
      lcd.setCursor(0,0); lcd.print("Cambio de unidad");
    //cambia unidad de temperatura entre Celsius y Fahrenheit
    break;

    case set_hora:
      lcd.setCursor(0,0); lcd.print("Seteo de hora");
    //Seteo hora del RTC mediante pulsadores
    break;
  }
}

void tomar_temperatura ()
{
  lcd.print("Temperatura:");
  Sensor_temp.requestTemperatures();
  lcd.print(Sensor_temp.getTempCByIndex(0));
}

void standby()
{
  lcd.setCursor(0,0); lcd.print("                    "); lcd.setCursor(0,1); lcd.print(" "); lcd.setCursor(0,2); lcd.print(" "); lcd.setCursor(0,3); lcd.print("        ");
  lcd.setCursor(18,1); lcd.print("  ");
  lcd.setCursor(1,2); 
  lcd.print("Nivel: ");
  sensar_nivel_actual();
  switch (nivel_actual)
  {
    case tanque_vacio:
      lcd.print("0% ");
      break;
    case tanque_al_25:
      lcd.print("25%");
      break;  
    case tanque_al_50:
      lcd.print("50%");
      break;
    case tanque_al_75:
      lcd.print("75% ");
      break;
    case tanque_al_100:
      lcd.print("100%");
      break;
  }
  imprimir_hora();
  lcd.setCursor(1,1);
  tomar_temperatura();
}

void imprimir_hora (){
  lcd.setCursor(8,3); 
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
}

ISR(TIMER2_OVF_vect){
    mili_segundos++;
}

void sensar_nivel_actual(){
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
}

void control_de_temp_auto(int temp_a_alcanzar,int temp_minima){
  int umbral_de_temperatura = 5;
  int temperatura_actual = 0;
  if( temperatura_actual < temp_a_alcanzar + umbral_de_temperatura)
  {
    Sensor_temp.requestTemperatures();
    temperatura_actual = Sensor_temp.getTempCByIndex(0);
    digitalWrite(resistencia, HIGH);
  }
  if(temperatura_actual >= temp_a_alcanzar + umbral_de_temperatura)digitalWrite(resistencia, LOW);

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



