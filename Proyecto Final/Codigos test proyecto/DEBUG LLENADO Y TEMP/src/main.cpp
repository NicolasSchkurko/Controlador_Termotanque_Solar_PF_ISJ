//Libs
#include <Arduino.h>
#include "AT24CX.h"
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"

//████████████████████████████████████████████████████████████████████
//Defines
  //Control de temp
  #define sumador_temperatura 5 
  #define maxi_cast 80           //puede hacerse un DEFINE porque no se modifica
  #define temp_threshold 5
  #define min_temp 40    
  #define tiempo_para_temperatura 5000 

  //Control de nivel
  #define sumador_nivel 25
  #define min_nivel 25     //can be a DEFINE because doesnt change (used in manual)
  #define max_nivel 100 //can be a DEFINE because doesnt change (used in manual)
  #define tiempo_para_nivel 3000
  //Entradas y salidas
  #define Pote A2
  #define nivel_del_tanque A0 
  #define electrovalvula 10
  #define resistencia 11
  #define onewire 9 // pin del onewire

  //Datos del Menu
  #define maxY_menu1 7
  #define maxY_menu2 5
  #define tiempo_de_parpadeo 700
  #define tiempo_de_espera_menu 3000 
  //Datos horas
  #define hora_max 24
  #define minuto_max 60 //I kit actual time because in used in other sites and here didnt work  SUCK MY DIK JEREMAIAS BRTOLSIC na mentira oka

//█████████████████████████████████████████████████████████████████████████████████
//Prototipos
  // funciones del menu (ordenadas segun su lugar de inicio)
  void standby();
  void menu_basico();
  void menu_de_calefaccion_auto();
  void menu_de_calefaccion_manual();
  void menu_de_llenado_auto();
  void menu_de_llenado_manual();
  void menu_de_auto_por_hora();
  void menu_avanzado();
  void menu_modificar_hora_rtc();
  void menu_farenheit_celsius();
  void menu_seteo_wifi();
  void menu_activar_bomba();  
  //Funciones control
  void Controltemp();
  void Controllvl();
  // funcion actualizar sensores
  void Actualizar_entradas();
  //Funciones conexion arduino/esp
  void Serial_Read_UNO();
  void Serial_Send_UNO(uint8_t);
  //Funciones extra
  String String_de_hora(uint8_t,uint8_t);
  uint8_t CharToUINT(uint8_t,uint8_t);
  uint8_t StringToChar(uint8_t,const String);
  uint8_t ReturnToCero(int8_t , uint8_t);
  char Character_Return(uint8_t , bool);
  void guardado_para_menus(bool);

//█████████████████████████████████████████████████████████████████████████████████
//Declaraciones de libs
  AT24C32 eep;
  RTC_DS1307 rtc;
  LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);
  OneWire sensor_t(onewire);
  DallasTemperature Sensor_temp(&sensor_t);

//█████████████████████████████████████████████████████████████████████████████████
//Variables 
  //Variables de temp (ordenados segun peso)
  bool use_farenheit = false; 
  bool calentar;
  int8_t temperatura_a_calentar; //se usa en calefaccion manual para guardar que temperatura se necesita
  int8_t temperatura_actual; // temp actual
  //Variables de nivel          Para teo: vals that share in level funciones //Jere: ?? // Chuco: ??
  bool Activar_bomba;
  bool llenar;
  int8_t nivel_a_llenar; //se usa en llenado manual para guardar que temperatura se necesita  
  uint8_t nivel_actual;
  uint8_t MessagePoss=0;
  //Variables de hora
  uint32_t tiempo_menues;             //(used in temperature funcions and in llenado auto to showw a confirm output)
  uint32_t tiempo_sensores;
  uint32_t  mili_segundos = 0;
  uint8_t hora_to_modify, minuto_to_modify;
  uint8_t sumador_hora, sumador_minuto;
  uint8_t hora,minutos;

  uint8_t Valorinicial,Valorfinal;// Guardado en memoria 
  //Variables EEPROM
  struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};            //guardado 1/hora/level/temp
  uint8_t ActualStruct=0;                                                   //guardado 2/hora/level/temp
  save_data save[3];                                                        //guardado 3/hora/level/temp
                                                                            //guarda auto temp = 255/temp_min/temp_max
                                                                            //guarda auto lvl = 255/level_min/level_max

  //Variables menu
  typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
  typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 
  bool Blink;
  uint8_t opcionmenu1=0;
  uint8_t opcionmenu2=0;
  uint8_t Flag=0;
  int8_t Ypos;
  uint16_t tiempo_de_standby = 0;
  const String Menuprincipal[maxY_menu1] = {
    "C manual",
    "H manual",
    "H & F in H",
    "C segun lleno",
    "H segun temp",
    "menu avanzado",
    "volver"
  };
  const String menuavanzado[maxY_menu2] = {
      "Setear hora",
      "c° o F°",
      "Activar la bomba",
      "conexion wifi",
      "volver"
  };
  Seleccionar_Funciones funcionActual = posicion_inicial;
  estadoMEF Estadoequipo = estado_inicial;

  //Variables Comunicacion esp/arduino
  bool Take_Comunication_Data=false;
  bool ComunicationError=false;
  bool InitComunication = true;  
  bool mayusculas=false;
  uint8_t StringLength=0;
  uint8_t ActualIndividualDataPos=0;
  char Actualchar=0;
  char input=0;
  String Serial_Input;
  String Individualdata[4];
  char WIFISSID [19];
  char WIFIPASS [19];
  uint8_t Auxiliar1;
  bool Resistencia;
  bool Valvula;

  int16_t Valor_Pote;

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
  //

  // bomba tucumana
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
  Serial_Send_UNO(1);
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


void Serial_Read_UNO(){

  Serial_Input=Serial.readString();// iguala el string del serial a un string input
  StringLength= Serial_Input.length();// saca el largo del string
  input=Serial_Input.charAt(0); // toma el char del comando a realizar (usualmente una letra)
  // Separador del string en variables:
  for (Auxiliar1 = 2; Auxiliar1 <= StringLength; Auxiliar1++){ // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
    if(Serial_Input.charAt(Auxiliar1)==':') ActualIndividualDataPos++; //si hay : divide los datos
    else{// si no es nungun caracter especial:
      if(Serial_Input.charAt(Auxiliar1-1)==':' || Serial_Input.charAt(Auxiliar1-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(Auxiliar1);//si es el primer digito lo iguala
      else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(Auxiliar1);//si es el segundo en adelante lo suma al string
    }
    if(Auxiliar1==StringLength)Take_Comunication_Data=true; //comienza a igualar variables
  } 
  
  if(Take_Comunication_Data==true){
    switch (input){//dependiendo del char de comando
    case 'H':
      temperatura_a_calentar=Individualdata[0].toInt();
      if(Individualdata[1]=="ON")calentar=true;
      if(Individualdata[1]=="Off")calentar=false;
      Take_Comunication_Data=false;
      break;

    case 'C':
      nivel_a_llenar=Individualdata[0].toInt();
      if(Individualdata[1]=="ON")llenar=true;
      if(Individualdata[1]=="Off")llenar=false;
      Take_Comunication_Data==false;
      break;

    case 'K':
      ActualStruct=Individualdata[3].toInt();
      if (ActualStruct<=2 && ActualStruct>=0){
          eep.write((ActualStruct*3)+1, Individualdata[0].toInt());
          save[ActualStruct].hour=Individualdata[0].toInt();
          eep.write((ActualStruct*3)+2, Individualdata[1].toInt());
          save[ActualStruct].temp=Individualdata[0].toInt();
          eep.write((ActualStruct*3)+3, Individualdata[2].toInt());
          save[ActualStruct].level=Individualdata[0].toInt();
          ActualIndividualDataPos=0;
          Take_Comunication_Data=false;
        }
      break;

    case 'J':
      eep.write(10,Individualdata[0].toInt());
      eep.write(11,Individualdata[1].toInt());
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'V':
      eep.write(12,Individualdata[0].toInt());
      eep.write(13,Individualdata[1].toInt());
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'E':
      Serial_Send_UNO(6);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'O':
      if(Individualdata[0]=="OK"){
      if (InitComunication==true)Serial_Send_UNO(1);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      }
      break;
    }
  }
}

void Serial_Send_UNO(uint8_t WhatSend){
  if (ComunicationError==false){
    switch (WhatSend){
      case 1:
        if (InitComunication==true){
            switch (MessagePoss){
              case 0:
                Serial.print(("S_"));Serial.print(String(WIFISSID));Serial.print(F(":"));Serial.println(String(WIFIPASS)+":");
                MessagePoss++;
                break;
              case 1:
                Serial.print(("K_"));Serial.print(save[0].hour);Serial.print(F(":"));Serial.print(save[0].temp);Serial.print(F(":"));Serial.print(save[0].level);
                break;
              case 2:
                Serial.print(("K_"));Serial.print(save[1].hour);Serial.print(F(":"));Serial.print(save[1].temp);Serial.print(F(":"));Serial.println(save[1].level);
                break;
              case 3:
                Serial.print(("K_"));Serial.print(save[2].hour);Serial.print(F(":"));Serial.print(save[2].temp);Serial.print(F(":"));Serial.println(save[2].level);
                break;
              case 4:
                Serial.print(("J_"));Serial.print(eep.read(10));Serial.print(F(":"));Serial.println(eep.read(11));
                break;
              case 5:
                Serial.print(("V_"));Serial.print(eep.read(12));Serial.print(F(":"));Serial.println(eep.read(13));
                InitComunication=false;
                break;
          }  
        }
      break;
      case 2:
        if(InitComunication==false){Serial.print(("K_"));Serial.print(save[0].hour);Serial.print(F(":"));Serial.print(save[0].temp);Serial.print(F(":"));Serial.println(save[0].level);}
        break;
      case 3:
        if(InitComunication==false){Serial.print(("J_"));Serial.print(eep.read(10));Serial.print(F(":"));Serial.println(eep.read(11));}
        break;
      case 4:
        if(InitComunication==false){Serial.print(("V_"));Serial.print(eep.read(12));Serial.print(F(":"));Serial.println(eep.read(13));}
        break;
      case 5:
        if(InitComunication==false){Serial.print(("U_"));Serial.print(StringToChar(1,String(hora)+":"+String(minutos)));Serial.print(F(":"));Serial.print(nivel_actual);Serial.print(F(":"));Serial.println(temperatura_actual);}
        break;
      case 6:
        if(InitComunication==false){Serial.print(("S_"));Serial.print(WIFISSID);Serial.print(F(":"));Serial.println(WIFIPASS);}
    }
  }
  if (ComunicationError==true){Serial.println(F("?_RESET"));}//resetea esp 
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
  if(temperatura_actual <= eep.read(10) && Resistencia == false && PORTD !=(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=true;}
  if(temperatura_actual > eep.read(11) && Resistencia == true && PORTD ==(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=false;}
  //=========Compara nivel actual con el minimo seteado============
  if (temperatura_actual < temperatura_a_calentar && Resistencia == false && PORTD !=(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=true;}
  if (temperatura_actual >= temperatura_a_calentar && Resistencia == true && PORTD ==(1<<PD6)){PORTD ^=(1<<PD6); Resistencia=false;}
  //=================================================================
}

void Controllvl(){
  // control lvl min to max
  if(nivel_actual <= eep.read(12) && Activar_bomba == true && Valvula == false && PORTD !=(1<<PD7)){PORTD ^=(1<<PD7);Valvula=true; PORTB |= B000001;}
  if(nivel_actual > eep.read(13) && Valvula == true && PORTD ==(1<<PD7)){PORTD ^=(1<<PD7);Valvula=true; PORTB &= B111110;}
  //======Compara temperatura actual con el minimo seteado=========
  if(nivel_actual < nivel_a_llenar && Valvula == false && PORTD !=(1<<PD7)) {PORTD ^=(1<<PD7);Valvula=true;}
  if(nivel_actual >= nivel_a_llenar && Valvula == true && PORTD ==(1<<PD7)) {PORTD ^=(1<<PD7);Valvula=true;}
  //=================================================================
}

