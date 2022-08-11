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
  // funciones del menu (ordenadas segun su lugar de inicio
         //Menu avanzado


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
  uint8_t PotenciometerOutput(uint8_t, uint8_t, uint8_t);

//█████████████████████████████████████████████████████████████████████████████████
//Declaraciones de libs
  AT24C32 eep;
  RTC_DS1307 rtc;
  OneWire sensor_t(onewire);
  DallasTemperature Sensor_temp(&sensor_t);

//█████████████████████████████████████████████████████████████████████████████████
//Variables 
  //Variables de temp (ordenados segun peso)
  bool use_farenheit = false; 
  bool calentar;
  int8_t temperatura_a_calentar; //se usa en calefaccion manual para guardar que temperatura se necesita
  extern int8_t temperatura_actual; // temp actual
  //Variables de nivel          Para teo: vals that share in level funciones //Jere: ?? // Chuco: ??
  bool Activar_bomba;
  bool llenar;
  int8_t nivel_a_llenar; //se usa en llenado manual para guardar que temperatura se necesita  
  uint8_t nivel_actual;
  uint8_t MessagePoss=0;
  //Variables de hora         
  uint32_t tiempo_sensores;
  extern uint32_t  mili_segundos = 0;
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



  uint8_t Flag=0;
  int8_t Ypos;

  extern Seleccionar_Funciones funcionActual = posicion_inicial;
  extern estadoMEF Estadoequipo = estado_inicial;

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
  LiquidCrystal_I2C lcd(0x27,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);


  extern uint16_t tiempo_de_standby=0;
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
        if (InitComunication==true && mili_segundos>=tiempo_menues+2000){
            switch (MessagePoss){
              case 0:
                Serial.print(("S_"));Serial.print(String(WIFISSID));Serial.print(F(":"));Serial.println(String(WIFIPASS)+":");
                tiempo_menues=mili_segundos;
                MessagePoss++;
                break;
              case 1:
                Serial.print(("K_"));Serial.print(save[0].hour);Serial.print(F(":"));Serial.print(save[0].temp);Serial.print(F(":"));Serial.println(save[0].level);
                tiempo_menues=mili_segundos;
                MessagePoss++;
                break;
              case 2:
                Serial.print(("K_"));Serial.print(save[1].hour);Serial.print(F(":"));Serial.print(save[1].temp);Serial.print(F(":"));Serial.println(save[1].level);
                tiempo_menues=mili_segundos;
                MessagePoss++;
                break;
              case 3:
                Serial.print(("K_"));Serial.print(save[2].hour);Serial.print(F(":"));Serial.print(save[2].temp);Serial.print(F(":"));Serial.println(save[2].level);
                tiempo_menues=mili_segundos;
                MessagePoss++;
                break;
              case 4:
                Serial.print(("J_"));Serial.print(eep.read(10));Serial.print(F(":"));Serial.println(eep.read(11));
                tiempo_menues=mili_segundos;
                MessagePoss++;
                break;
              case 5:
                Serial.print(("V_"));Serial.print(eep.read(12));Serial.print(F(":"));Serial.println(eep.read(13));
                tiempo_menues=mili_segundos;
                MessagePoss++;
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

uint8_t ReturnToCero (int8_t actualpos, uint8_t maxpos)
{ 
  uint8_t realvalue;
  if (actualpos>=maxpos){
    realvalue = 0 + actualpos % maxpos;       
    return realvalue;
  }
  if (actualpos<0){
    realvalue = maxpos + actualpos;          
    return realvalue;
  }
  if (actualpos>0 && actualpos<maxpos) return actualpos;
}

uint8_t CharToUINT(uint8_t function,uint8_t save)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t resto_deconvert=0;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ 
  switch (function)
    {
      case 1:
        resto_deconvert= (save) % 4;
        var1_deconvert= (save-resto_deconvert)/4;
        return var1_deconvert;
      break;
      case 2:
        resto_deconvert= (save) % 4;
        var1_deconvert=resto_deconvert*15;
        return var1_deconvert;
      break;
      case 3:
        return save;
      break;
      default:
        break;
    }
}

uint8_t StringToChar(uint8_t function, String toconvert) //// ya arregle lo de colver
{
  uint8_t var1_convert; // solo una variable (_convert  nos evita modificar variables globales como bldos)
  uint8_t var2_convert; // solo una variable
  uint8_t resto_convert; //guarda el resto del calculo de tiempo
  switch (function)
  {
    case 1:
      var1_convert=(toconvert.charAt(0)- '0')*10;// toma el valor del primer digito del string y lo convierte en int (numero de base 10)
      var1_convert+=(toconvert.charAt(1)-'0');// toma el valor del segundo digito
      var1_convert=var1_convert*4;//multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

      var2_convert=(toconvert.charAt(3)- '0')*10;//lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
      var2_convert+=(toconvert.charAt(4)-'0');//lo mismo que en el var 1 pero con minutos
      resto_convert=var2_convert%15; //saca el resto (ejemplo 7/5 resto 2)
      if(resto_convert<8) var2_convert= var2_convert-resto_convert; //utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
      else var2_convert=var2_convert+15-resto_convert;// utiliza el resto para redondear arriba
      var2_convert=var2_convert/15;// convierte los minutos en la proporcion del char (1 entero = 15 minutos)

      var1_convert+=var2_convert;// suma horas y minutos
      if(var1_convert>=96)var1_convert=0;
      return var1_convert;
      break;
    case 2:
      var1_convert= toconvert.toInt();// hace magia y despues de tirar magia convierte el string en un int (fua ta dificil la conversion de ete)
      return var1_convert;
      break;
    default:
      var2_convert= toconvert.toInt();// mismo sistema
      return var2_convert;
      break;
  }
}

String String_de_hora (uint8_t hora_entrada, uint8_t minuto_entrada){
  if(hora_entrada > 9 && minuto_entrada>9) return(String(hora_entrada)+":"+String(minuto_entrada));
  if(hora_entrada <= 9 && minuto_entrada>9) return("0"+String(hora_entrada)+":"+String(minuto_entrada));
  if(hora_entrada > 9 && minuto_entrada<=9) return(String(hora_entrada)+":0"+String(minuto_entrada));
  if(hora_entrada <= 9 && minuto_entrada<=9) return("0"+String(hora_entrada)+":0"+String(minuto_entrada));
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

char Character_Return(uint8_t Character_pos, bool mayus)
{
  switch (mayus)
  {
    case true:
      if (Character_pos>=0 && Character_pos<=25)return Character_pos+65;
      if (Character_pos>=26 && Character_pos<=29) return Character_pos+9;
      if (Character_pos==30) return 47;
      if (Character_pos==31) return 40;
      if (Character_pos==32) return 41;
      if (Character_pos==33) return 61;
      if (Character_pos==34) return 59;
      if (Character_pos==35) return 58;
      if (Character_pos==36) return 94;
      if (Character_pos==37) return 63;
      if (Character_pos==38) return 95;
      if (Character_pos==39) return 27;
      if (Character_pos>39) return 0;
    break;

    case false:
      if (Character_pos>=0 && Character_pos<=25)return Character_pos+97;
      if (Character_pos>=26 && Character_pos<=35)return Character_pos+22;
      if (Character_pos==36) return 33;
      if (Character_pos==37) return 45;
      if (Character_pos==38) return 64;
      if (Character_pos==39) return 0;
      if (Character_pos>39) return 0;
    break;
  }
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

