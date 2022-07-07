//optimizar
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"
/* para ordenar declaraciones usar el siguiente orden Definitons
                                                      Functions
                                                      Library declarations
                                                      Type def 
                                                      Const declarations
                                                      Variable declarations according to value
no sean cripticos la concha de su madre*/

//sensor de temperatura
#define onewire 9
#define TEMP_RESOLUTION 9
void tomar_temperatura ();
void limpiar_pantalla_y_escribir ();
void control_de_temp_auto();
int control_de_temp_por_menu_manual(int temp_a_alcanzar);
void menu_de_calefaccion_manual();
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 
uint16_t tiempo_para_temperatura = 5000; // 2 bytes mas 60k si necesitan mas cambien a 36 (4 bytes), le puse unsigned si necesitan negativos saquen la u
uint8_t temperatura_inicial = 40; // byte 0-255 ¿Para que chota usamos int si no necesitamos 60k opciones? solo 0-100 
uint8_t temperatura_final = 40;
uint8_t min_temp_ini = 40;
uint8_t maxima_temp_fin = 80;
uint8_t milis_para_temperatura = 0;
uint8_t umbral_de_temperatura = 5;
uint8_t sumador_temperatura = 5;
bool confirmar = false;
bool encendido_de_temp_auto = true;

//nivel de agua
void sensar_nivel_de_agua();
void sensar_nivel_actual();
void nivel_auto();
typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_seteado;
niveles nivel_actual;
const uint8_t nivel_del_tanque = A0; 
const uint8_t electrovalvula = 10;
const uint8_t resistencia = 11;
uint8_t  nivel = 0;
uint8_t  mili_segundos = 0;
uint8_t sumador_nivel = 25;
uint8_t  milis_para_nivel = 0;
uint16_t tiempo_para_nivel = 3000;
bool confirmar_nivel = false;
//

//cosas del menu princial
void menu_basico();
void standby();
void imprimir_en_pantalla();
void carga_por_nivel();
void limpiar_pantalla_y_escribir_nivel();
uint16_t tiempo_de_standby = 10000;
//

//cosas del mennu avanzado
void menu_avanzado();
//

//funciones para el RTC se cionecta directamente a los pines SCL Y SDA
/*RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now; */
void imprimir_hora ();
//

//Cosas necesarias para el menu
LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x20,20,4);
typedef enum{estado_standby,estado_inicial,calefaccion_manual,calefaccion_auto_senstemp,carga_auto_hora,carga_agua_por_nivel,llenado_agua_manual} estadoMEF;  
estadoMEF Menu_principal = estado_inicial;
typedef enum{menu_inicio,set_wifi,activar_bomba,cambio_unidad,set_hora,momento_standby} estadoMEF2;
estadoMEF2 Menu_secundario = menu_inicio;
const uint8_t pulsador1 = 2;
const uint8_t pulsador2 = 3;
const uint8_t pulsador3 = 4;
const uint8_t pulsador4 = 5;
const uint8_t pulsador5 = 6;
const uint8_t pulsador6 = 7; //pulsador de retorno
const uint8_t pulsador7 = 8;
bool flag_menu_avanzado = false;
bool borrar_display = false;

// Cosas rtc y reajustes 
RTC_DS1307 rtc;
uint16_t anio;
uint8_t mes,dia,hora,minutos,segundos;
uint8_t correccionh,correccionmin, correccionseg;


void setup() 
{
  //Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //
  Wire.begin();  //iniccializacion del i2C
  Serial.begin(9600); //iniccializacion del serial arduino-esp
  rtc.begin();//iniccializacion del rtc arduino-esp
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); //subirlo solo una unica vez y despues subirlo nuevamente pero comentando (sino cuando reinicia borra config hora)
  //Iniciacion del LCD//
  lcd.init();
  lcd.backlight();
  //
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
  //now = RTC.now();
  /*if (milis_para_nivel == tiempo_para_nivel)//sujeto a cambios
  {
    milis_para_nivel= 0;
  }
*/
  if (milis_para_temperatura == 5000)
  {
    control_de_temp_auto();
    milis_para_temperatura = 0;
  }
  // nico: QUE CHOTA ES ESTO AYUDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
  // Jere: pusimos una variable flag para que un pulsador alterne entre menus
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
  if(mili_segundos >= 2000 && digitalRead(pulsador1) == HIGH && digitalRead(pulsador2) == HIGH && digitalRead(pulsador3) == HIGH && digitalRead(pulsador4) == HIGH && digitalRead(pulsador5) == HIGH && digitalRead(pulsador6) == HIGH)
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
      menu_de_calefaccion_manual();
      break;
    case calefaccion_auto_senstemp:
      lcd.setCursor(3,2); lcd.print("        "); lcd.setCursor(3,3);lcd.print("     "); lcd.setCursor(16,1); lcd.print("     ");
      lcd.setCursor(0,0); lcd.print("Encender calefaccion");    lcd.setCursor(5,1); lcd.print("automatica"); lcd.setCursor(0,2); lcd.print("-Si"); lcd.setCursor(0,3); lcd.print("-No");
      if(digitalRead(pulsador1) == LOW)//con las variables y el while le doy un delay para que se vea la confirmacion
      {
        while (digitalRead(pulsador1) == LOW){}//con las variables y el while le doy un delay para que se vea la confirmacion
        int espera = 900;
        unsigned long tiempo_ahora = 0;
        tiempo_ahora = millis();
        lcd.clear(); lcd.setCursor(8,1); lcd.print("DONE");
        while (millis()< tiempo_ahora + espera){}
        mili_segundos=0;
        lcd.clear();
        encendido_de_temp_auto = true;
        Menu_principal = estado_inicial;
      }
      if(digitalRead(pulsador2) == LOW)//con las variables y el while le doy un delay para que se vea la confirmacion
      {
        while (digitalRead(pulsador2) == LOW){}
        int espera = 900;
        unsigned long tiempo_ahora = 0;
        tiempo_ahora = millis();
        lcd.clear(); lcd.setCursor(8,1); lcd.print("DONE");
        while (millis()< tiempo_ahora + espera){}
        mili_segundos=0;
        lcd.clear();
        encendido_de_temp_auto = false;
        Menu_principal = estado_inicial;
      }
      break;
    case carga_auto_hora:
      lcd.setCursor(0,0); lcd.print("Carga auto hora");
      //Todavia no
      break;
    case carga_agua_por_nivel:
      carga_por_nivel();
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


void standby()
{
  lcd.setCursor(0,0); lcd.print("                    "); 
  lcd.setCursor(0,1); lcd.print(" "); lcd.setCursor(18,1); lcd.print("  ");
  lcd.setCursor(0,2); lcd.print(" "); lcd.setCursor(12,2); lcd.print("      "); 
  lcd.setCursor(0,3); lcd.print("        "); lcd.setCursor(13,3); lcd.print("        ");
  lcd.setCursor(1,2); 
  lcd.print("Nivel: ");
  sensar_nivel_actual();
  switch (nivel_actual)
  {
    case tanque_vacio:
      lcd.print("0%  ");
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
  //imprimir_hora();
  lcd.setCursor(1,1);
  tomar_temperatura();
  control_de_temp_auto();
}

void tomar_temperatura ()
{
  lcd.print("Temperatura:");
  Sensor_temp.requestTemperatures();
  lcd.print(Sensor_temp.getTempCByIndex(0));
}
/*
void imprimir_hora (){//ver que pasa
  lcd.setCursor(8,3); 
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(" ");
}
*/

ISR(TIMER2_OVF_vect){
    mili_segundos++;
    milis_para_nivel++;
    if(encendido_de_temp_auto == true) milis_para_temperatura++;
}

void sensar_nivel_actual(){//bien
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
}

void control_de_temp_auto(){//modificar para que se haga y no bloquee el codigo
  int temperatura_actual = 0;
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);
  if(temperatura_actual < temperatura_inicial) digitalWrite(resistencia, HIGH);
  if(temperatura_actual >= temperatura_final + umbral_de_temperatura) digitalWrite(resistencia, LOW);
}

void menu_de_calefaccion_manual(){
  if(confirmar == false)
  {
    lcd.setCursor(0,0);
    lcd.print("Temperatua minima:");
    lcd.print(temperatura_inicial);
    lcd.setCursor(0,1);
    lcd.print("Temperatua maxima:");
    lcd.print(temperatura_final);
    
    lcd.setCursor(0,2);
    lcd.print("Sumar 5 con: 1 y 3");
    lcd.setCursor(0,3);
    lcd.print("Restar 5 con: 2 y 4");
  }
  if(digitalRead(pulsador1) == LOW)
  {
    while(digitalRead(pulsador1) == LOW){}
    temperatura_inicial += sumador_temperatura;
    mili_segundos = 0;
    confirmar = false;
    lcd.clear();
  }
  if (temperatura_inicial > maxima_temp_fin) temperatura_inicial = maxima_temp_fin;

  if(digitalRead(pulsador2) == LOW)
  {
    while(digitalRead(pulsador2) == LOW){}
    temperatura_inicial -= sumador_temperatura;
    mili_segundos = 0;
    confirmar = false;
    lcd.clear();
  }
  if (temperatura_inicial < min_temp_ini) temperatura_inicial = min_temp_ini;

  if(digitalRead(pulsador3) == LOW)
  {
    while(digitalRead(pulsador3) == LOW){}
    temperatura_final += sumador_temperatura;
    mili_segundos = 0;
    confirmar = false;
    lcd.clear();
  }
  if (temperatura_final < temperatura_inicial) temperatura_final = temperatura_inicial;

  if(digitalRead(pulsador4) == LOW)
  {
    while(digitalRead(pulsador4) == LOW){}
    temperatura_final -= sumador_temperatura;
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
}

void limpiar_pantalla_y_escribir (){
  lcd.clear();
  if (confirmar == true)//con las variables y el while le doy un delay para que se vea la confirmacion
  {
    lcd.clear();
    lcd.print("Minima guardada:");
    lcd.print(temperatura_inicial);
    lcd.setCursor(0,1);
    lcd.print("Maxima guardada:");
    lcd.print(temperatura_final);
    confirmar = false;
    int espera = 1500;
    unsigned long tiempo_ahora = 0;
    tiempo_ahora = millis();
    while (millis()< tiempo_ahora + espera){}
    lcd.clear();
    Menu_principal = estado_inicial;
    control_de_temp_auto();
  }
}

void carga_por_nivel()
{
  if(digitalRead(pulsador1) == LOW)
  {
    while(digitalRead(pulsador1) == LOW){}
    nivel += sumador_nivel;
    confirmar_nivel = false;
    mili_segundos = 0;
    lcd.clear();
  }

  if(digitalRead(pulsador2) == LOW)
  {
    while(digitalRead(pulsador2) == LOW){}
    nivel -= sumador_nivel;
    confirmar_nivel = false;
    mili_segundos = 0;
    lcd.clear();
  }

  if(digitalRead(pulsador5) == LOW)
  {
    while(digitalRead(pulsador5) == LOW){}
    confirmar_nivel = true;
    mili_segundos = 0;
    limpiar_pantalla_y_escribir_nivel ();
  }

  if(nivel < 0) nivel = 0;
  if(nivel > 100) nivel = 100;

  if(confirmar_nivel == false)
  {
    lcd.setCursor(0,0);
    lcd.print("Nivel:");
    if(nivel < 10)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(7,0);
      lcd.print("%");
    }
    if(nivel > 9 && nivel < 100)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(8,0);
      lcd.print("%");
    }
    if(nivel == 100)
    {
      lcd.setCursor(6,0);
      lcd.print(nivel);
      lcd.setCursor(9,0);
      lcd.print("%");
    } 
  }
}

void limpiar_pantalla_y_escribir_nivel ()
{
  if (confirmar_nivel == true)//con las variables y el while le doy un delay para que se vea la confirmacion
  {
    lcd.setCursor(0,0);
    lcd.print("Nivel maximo:");
    lcd.print(nivel);
    if(nivel < 10)
    {
      lcd.setCursor(14,0);
      lcd.print("%");
    }
    if(nivel > 9 && nivel < 100)
    {
      lcd.setCursor(15,0);
      lcd.print("%");
    }
    if(nivel == 100)
    {
      lcd.setCursor(16,0);
      lcd.print("%");
    }
    confirmar_nivel = false;
    int espera = 1500;
    unsigned long tiempo_ahora = 0;
    tiempo_ahora = millis();
    while (millis()< tiempo_ahora + espera){}
    lcd.clear();
    Menu_principal = estado_inicial;
    nivel_auto();
  }
}

void nivel_auto (){
  if (nivel_actual <= tanque_al_25)    digitalWrite(electrovalvula, HIGH);
  if (nivel_actual == tanque_al_100)    digitalWrite(electrovalvula, LOW);
}








