//optimizar
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"
// nico: el menu nuevo esta en union.nico
/* para ordenar declaraciones usar el siguiente orden Definitons
                                                      Functions
                                                      Library declarations
                                                      Type def 
                                                      Const declarations
                                                      Variable declarations according to value
no sean cripticos la concha de su madre*/

//sensor de temperatura
#define onewire 9
void tomar_temperatura ();
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
void nivel_auto(bool);
typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_seteado;
niveles nivel_actual;
const uint8_t nivel_del_tanque = A0; 
const uint8_t electrovalvula = 10;
const uint8_t resistencia = 11;
uint8_t  nivel = 0;
uint16_t  mili_segundos = 0;
uint8_t sumador_nivel = 25;
uint16_t  milis_para_nivel = 0;
uint16_t tiempo_para_nivel = 3000;
bool confirmar_nivel = false;
//

//cosas del menu princial
uint8_t menuposY(uint8_t , uint8_t);
void menu_basico();
void standby();
void imprimir_en_pantalla();
void carga_por_nivel();
void limpiar_pantalla_y_escribir_nivel();
uint16_t tiempo_de_standby = 0;
uint8_t opcionmenu1=0;
uint8_t opcionmenu2=0;
uint8_t Flag=0;
//

//cosas del mennu avanzado
void menu_avanzado();
//

//funciones para el RTC se cionecta directamente a los pines SCL Y SDA
/*RTC_DS1307 RTC; //variable que se usa para comunicarse con el Sensor DS1307 via I2C 
DateTime now; */
void imprimir_hora ();
//
// cosas del wifi
void configuracionwifi();
char Letra(uint8_t , bool);
String WIFISSID;
String WIFIPASS;
//Cosas necesarias para el menu
LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);
typedef enum{estado_standby,estado_inicial,menu1,menu2} estadoMEF;  
estadoMEF Estadoequipo = estado_inicial;
const uint8_t maxY_menu1=7;
const uint8_t maxY_menu2=5;
const uint8_t pulsador1 = 2;
const uint8_t pulsador2 = 3;
const uint8_t pulsador3 = 4;
const uint8_t pulsador4 = 5;
const uint8_t pulsador5 = 6;
const uint8_t pulsador6 = 7; //pulsador de retorno
const uint8_t pulsador7 = 8;
String Menuprincipal[maxY_menu1] = {
  "C manual",
  "H manual",
  "H & F in H",
  "H segun temp",
  "C segun lleno",
  "menu avanzado",
  "volver"
};
String menuavanzado[maxY_menu2] = {
    "Setear hora",
    "c° o F°",
    "Activar la bomba",
    "conexion wifi",
    "volver"
};
bool flag_menu_avanzado = false;
bool borrar_display = false;
uint8_t Ypos;

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
  WIFISSID=" ";
}

void loop() 
{
  /*tomar_temperatura();
  control_de_temp_auto();
  sensar_nivel_actual();*/
  sensar_nivel_actual();
  /*if (milis_para_nivel == tiempo_para_nivel)//sujeto a cambios
  {
    control_de_temp_auto();
    milis_para_nivel= 0;
  }
  if (milis_para_temperatura == tiempo_para_temperatura)
  {
    control_de_temp_auto();
    milis_para_temperatura = 0;
  }
  */
  switch (Estadoequipo)
  {
    case estado_standby:
      standby();
      break;
    case estado_inicial:
      standby();
      opcionmenu1=0; //reinicia la posicion del menu1
      opcionmenu2=0; //reinicia la posicion del menu2
      break;
    case menu1:
      menu_basico();
      break;
    case menu2:
      menu_avanzado();
      break;
  }
 
}
void standby()
{ 
  lcd.setCursor(0,0);
  lcd.print("Nivel: ");
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
  if(digitalRead(pulsador1)==LOW || digitalRead(pulsador2)==LOW || digitalRead(pulsador3)==LOW || digitalRead(pulsador4)==LOW || digitalRead(pulsador5)==LOW || digitalRead(pulsador6)==LOW || digitalRead(pulsador7)==LOW ){
    while (digitalRead(pulsador1)==LOW || digitalRead(pulsador2)==LOW || digitalRead(pulsador3)==LOW || digitalRead(pulsador4)==LOW || digitalRead(pulsador5)==LOW || digitalRead(pulsador6)==LOW || digitalRead(pulsador7)==LOW){}
    switch (Estadoequipo)
    {
    case estado_standby:
      Estadoequipo = estado_inicial;
      lcd.backlight();
      tiempo_de_standby=0;
      break;
    case estado_inicial:
      Flag=1;
      Estadoequipo =menu1;
      lcd.backlight();
      break;
    default:
      Estadoequipo = estado_standby;
      break;
    }   
    }
  if(tiempo_de_standby>=5000 && Estadoequipo  == estado_inicial)
  {
    Estadoequipo = estado_standby;
    lcd.noBacklight();
    tiempo_de_standby = 0;
  }


}

void menu_basico()
{
  if(Flag==1){
    Flag=2;
    lcd.clear();
    Ypos=0;
    opcionmenu1=0;
  }
  if (Flag==2){
  if (digitalRead(pulsador1) == LOW ){ while (digitalRead(pulsador1) == LOW){} Ypos=Ypos+1; lcd.clear(); }
  if (digitalRead(pulsador2) == LOW ){ while (digitalRead(pulsador2) == LOW){} Ypos=Ypos-1; lcd.clear(); }
  if (digitalRead(pulsador3) == LOW ){ while (digitalRead(pulsador3) == LOW){}  opcionmenu1=Ypos+1; lcd.clear(); }
  if (Ypos>=7)Ypos=0;

  switch (opcionmenu1)
  {
    case 0:
      lcd.setCursor(1,0); 
      lcd.print(Menuprincipal[menuposY(Ypos,maxY_menu1)]); 
      lcd.setCursor(1,1); 
      lcd.print(Menuprincipal[menuposY(Ypos+1,maxY_menu1)]); 
      lcd.setCursor(1,2); 
      lcd.print(Menuprincipal[menuposY(Ypos+2,maxY_menu1)]); 
      lcd.setCursor(1,3); 
      lcd.print(Menuprincipal[menuposY((Ypos+3),maxY_menu1)]);
      break;
    case 1:
      //menu_de_llenado_manual();
      break;
    case 2:
      //menu_de_calefaccion_manual();
      break;
    case 3:
      // cal y carg segun hora
      break;
    case 4:
      //calef segun tempmin
      break;
    case 5:
      //carga_por_nivel();
      break;
    case 6:
      Estadoequipo=menu2;
      Flag=3;
      break;
    case 7:
      Estadoequipo=estado_inicial;
      tiempo_de_standby=0;
      Flag=0;
      break;
  }
  }
}

void menu_avanzado()
{
  if (Flag==3){
    Ypos=0;
    opcionmenu2=0;
    lcd.clear();
    Flag=4;
  }
  if (Flag==4){
  if (digitalRead(pulsador1) == LOW ){ while (digitalRead(pulsador1) == LOW){} Ypos=Ypos+1; lcd.clear(); }
  if (digitalRead(pulsador2) == LOW ){ while (digitalRead(pulsador2) == LOW){} Ypos=Ypos-1; lcd.clear(); }
  if (digitalRead(pulsador3) == LOW ){ while (digitalRead(pulsador3) == LOW){}  opcionmenu2=Ypos+1; lcd.clear(); }
  switch (opcionmenu2)
  {
    case 0:
      lcd.setCursor(1,0); 
      lcd.print(menuavanzado[Ypos]); 
      lcd.setCursor(1,1); 
      lcd.print(menuavanzado[menuposY(Ypos+1,maxY_menu2)]); 
      lcd.setCursor(1,2); 
      lcd.print(menuavanzado[menuposY(Ypos+2,maxY_menu2)]); 
      lcd.setCursor(1,3); 
      lcd.print(menuavanzado[menuposY(Ypos+3,maxY_menu2)]);
      break;
    case 1:
      //setear hora
      break;
    case 2: 
      // formato grados
      break;
    case 3:
      // Activar bomba
      break;
    case 4:
      configuracionwifi();
      break;
    case 5:
      Estadoequipo= menu1;
      Flag=1;
      break;

  }
  }
}

uint8_t menuposY (uint8_t actualpos, uint8_t maxpos){
uint8_t realvalue;
if (actualpos>=maxpos){
  realvalue = actualpos-maxpos;
  return realvalue;
}
else{
  return actualpos;
}
}

ISR(TIMER2_OVF_vect){
  mili_segundos++;
  milis_para_nivel++;
  tiempo_de_standby++;
  if(encendido_de_temp_auto == true) milis_para_temperatura++;
}
/*======================PARA ADAPTAR=========================*/

void tomar_temperatura ()
{
  lcd.print("Temperatura:");
  Sensor_temp.requestTemperatures();
  lcd.print(Sensor_temp.getTempCByIndex(0));
}

void sensar_nivel_actual(){
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
}

void nivel_auto (bool use){
    if (nivel_actual <= tanque_al_25)    digitalWrite(electrovalvula, HIGH);
    if (nivel_actual == tanque_al_100)    digitalWrite(electrovalvula, LOW);
}

void control_de_temp_auto(){
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



/*=====================================LABURANDOLO==============================================*/

void configuracionwifi(){
  
}

char Letra(uint8_t letranum, bool mayus){
switch (mayus)
{
case true:
  if (letranum>=0 && letranum<=13)return letranum+65;
  if (letranum==14)return 165;
  if (letranum>=15 && letranum<=26)return letranum+64;
  if (letranum==27) return 36;
  if (letranum==28) return 37;
  if (letranum==29) return 38;
  if (letranum==30) return 47;
  if (letranum==31) return 40;
  if (letranum==32) return 41;
  if (letranum==33) return 61;
  if (letranum==34) return 59;
  if (letranum==35) return 58;
  if (letranum==36) return 94;
  if (letranum==36) return 63;
  if (letranum==37) return 168;
  if (letranum==38) return 95;
  if (letranum==39) return 35;
  if (letranum==40) return 27;
  if (letranum>40) return 0;
  break;

case false:
  if (letranum>=0 && letranum<=13)return letranum+97;
  if (letranum==14)return 164;
  if (letranum>=15 && letranum<=26)return letranum+96;
  if (letranum>=26 && letranum<=35)return letranum+22;
  if (letranum==36) return 33;
  if (letranum==37) return 173;
  if (letranum==38) return 45;
  if (letranum==39) return 64;
  if (letranum==40) return 0;
  if (letranum>40) return 0;
  break;
}
}


