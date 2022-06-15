//optimizar
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//sensr de temperatura
#define onewire 6
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 
void tomar_temperatura ();
//

//nivel de agua
typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel;
const int sensado_de_nivel = A0;
int mili_segundos = 0;
void colectar_nivel_de_agua();
//

//cosas del menu princial
void actualizar_MEF();
void imprimir_en_pantalla();
//

//funciones para el RTC
//void imresion_de_hora_del_dia();
/*RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now = RTC.now(); */      // guarda la fecha y hora del RTC en la variable (es una maquina de estado que guarda año,mes,dia,hora,minutos,segundos en ese orden)
//

LiquidCrystal_I2C lcd(0x27,20,4);
//Cosas necesarias para el menu
bool borrar_display = false;
const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;
typedef enum{estado_standby,estado_inicial,calefaccion_manual,calefaccion_auto_senstemp,carga_auto_hora,carga_agua_por_nivel,llenado_agua_manual} estadoMEF;  
estadoMEF Menu_principal = estado_inicial;
//

void setup() 
{
  Serial.begin(9600);
  //Interrupcion ca 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  //
  void actualizar_MEF();

  //iniccializacion del RTC 
 /* RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__)); //saca la data de la compu, despues comentar para subirlo bien*/
  //

  //Iniciacion del LCD//
  lcd.init();
  lcd.backlight();
  Wire.begin();
  //

  //pines para el sensor de nivel
  pinMode(sensado_de_nivel, INPUT);
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
  //

}

void loop() 
{
  actualizar_MEF();
  if(digitalRead(pulsador6) == LOW) 
  {
    while (digitalRead(pulsador6) == LOW){}
    mili_segundos = 0;
    Menu_principal = estado_inicial;
    lcd.clear();
  }
  if(mili_segundos >= 5000 && digitalRead(pulsador1) == HIGH && digitalRead(pulsador2) == HIGH && digitalRead(pulsador3) == HIGH && digitalRead(pulsador4) == HIGH && digitalRead(pulsador5) == HIGH && digitalRead(pulsador6) == HIGH)
  {
    Menu_principal = estado_standby;   
    mili_segundos = 0; 
  }
}

void actualizar_MEF()
{
  switch (Menu_principal)
  {
    case estado_standby:
      lcd.setCursor(0,1); lcd.print("                    "); lcd.setCursor(0,2); lcd.print("                    "); lcd.setCursor(0,3); lcd.print("                    ");
      tomar_temperatura();
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

/*void imresion_de_hora_del_dia(){
lcd.setCursor(0,0);
lcd.print(now.hour()); // Horas
lcd.print(':');
lcd.print(now.minute(), DEC); // Minutos
}*/

void colectar_nivel_de_agua()
{
  if (analogRead(sensado_de_nivel) >= 100 && analogRead(sensado_de_nivel) < 256)    nivel = tanque_al_25;
  if (analogRead(sensado_de_nivel) >= 256 && analogRead(sensado_de_nivel) < 512)    nivel = tanque_al_50;
  if (analogRead(sensado_de_nivel) >=512  && analogRead(sensado_de_nivel) < 768)    nivel = tanque_al_75;
  if (analogRead(sensado_de_nivel) >= 768 && analogRead(sensado_de_nivel) <= 1024)    nivel = tanque_al_100;
  if (analogRead(sensado_de_nivel) < 100) nivel = tanque_vacio;  
  
  lcd.setCursor(0,3);
  switch (nivel)
  {
    case tanque_vacio:
      lcd.print("Tanque al: 0%");
      break;
    case tanque_al_25:
      lcd.print("Tanque al: 25%");
      break;  
    case tanque_al_50:
      lcd.print("Tanque al: 50%");
      break;
    case tanque_al_75:
      lcd.print("Tanque al: 75%");
      break;
    case tanque_al_100:
      lcd.print("Tanque al: 100%");
      break;
  }
  /*if (mili_segundos >= 5500)
    {
      lcd.clear();
      if (analogRead(sensado_de_nivel) >= 100 && analogRead(sensado_de_nivel) < 256)    nivel = tanque_al_25;
      if (analogRead(sensado_de_nivel) >= 256 && analogRead(sensado_de_nivel) < 512)    nivel = tanque_al_50;
      if (analogRead(sensado_de_nivel) >=512  && analogRead(sensado_de_nivel) < 768)    nivel = tanque_al_75;
      if (analogRead(sensado_de_nivel) >= 768 && analogRead(sensado_de_nivel) <= 1024)    nivel = tanque_al_100;
      if (analogRead(sensado_de_nivel) < 100) nivel = tanque_vacio;  
      
      mili_segundos = 0;
    }*/
}

void tomar_temperatura (){

  lcd.setCursor(0,0);
  lcd.print("Temperatura:");
  Sensor_temp.requestTemperatures();
  lcd.print(Sensor_temp.getTempCByIndex(0));
}

ISR(TIMER2_OVF_vect){
    mili_segundos++;
}