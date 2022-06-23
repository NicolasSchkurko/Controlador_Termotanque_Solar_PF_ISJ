//optimizar
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//sensr de temperatura
#define onewire 7
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 
void tomar_temperatura ();
//

//nivel de agua
const int nivel_del_tanque = A0;
const int electrovalvula = 10;
int nivel = 0;
int mili_segundos = 0;

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_seteado;
niveles nivel_actual;
void sensar_nivel_de_agua();
void sensar_nivel_actual();
//

//cosas del menu princial
void menu_basico();
void standby();
void imprimir_en_pantalla();
//

//
void menu_avanzado();
//

//funciones para el RTC se cionecta directamente a los pines SCL Y SDA
void imprimir_de_hora_del_dia();
RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now = RTC.now();    // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
//

//LiquidCrystal_I2C lcd(0x27,20,4);
LiquidCrystal_I2C lcd(0x20,20,4);

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
//

void setup() 
{
  //Serial.begin(9600);
  //Interrupcion ca 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //

  //iniccializacion del RTC 
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien
  //

  //Iniciacion del LCD//
  lcd.init();
  lcd.backlight();
  Wire.begin();
  //

  //pines para el sensor de nivel
  pinMode(nivel_del_tanque, INPUT);
  pinMode(electrovalvula, OUTPUT);
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
      lcd.setCursor(0,0); lcd.print("Calefaccion man");
      //llenado_manual();
      break;
    case calefaccion_auto_senstemp:
      lcd.setCursor(0,0); lcd.print("Calef auto temp");
      // carga_por_sensor();
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
      sensar_nivel_actual();
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

/*void imprimir_de_hora_del_dia(){
lcd.print("       ");
lcd.print(now.hour(), DEC); // Horas
lcd.print(':');
lcd.print(now.minute(),DEC); // Minutos
Serial.print(now.hour()); // Horas
Serial.print(':');
Serial.print(now.minute()); // Minutos
Serial.println();
}*/

void tomar_temperatura ()
{
  lcd.print("Temperatura:");
  Sensor_temp.requestTemperatures();
  lcd.print(Sensor_temp.getTempCByIndex(0));
}

void standby()
{
  lcd.setCursor(0,0); lcd.print("                    "); lcd.setCursor(0,1); lcd.print(" "); lcd.setCursor(0,2); lcd.print(" "); lcd.setCursor(0,3); lcd.print(" ");
  lcd.setCursor(18,1); lcd.print("  ");
  lcd.setCursor(1,2); 
  lcd.print("Nivel: ");
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
  lcd.setCursor(0,3); 
  lcd.print("       ");
  lcd.print(now.hour(), DEC); // Horas
  lcd.print(':');
  lcd.print(now.minute(),DEC); // Minutos
  Serial.print(now.hour()); // Horas
  Serial.print(':');
  Serial.print(now.minute()); // Minutos
  Serial.println();
  lcd.setCursor(1,1);
  tomar_temperatura();
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
