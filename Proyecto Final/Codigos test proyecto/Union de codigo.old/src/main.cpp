//Libs
#include <Arduino.h>
#include "AT24CX.h"
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"

//█████████████████████████████████████████████████████████████████████████████████
//Defines
  //Control de temp
  #define sumador_temperatura 5 
  #define maxima_temp 90           //puede hacerse un DEFINE porque no se modifica
  #define temp_threshold 5
  #define min_temp 40    
  #define tiempo_para_temperatura 5000 

  //Control de nivel
  #define sumador_nivel 25
  #define min_nivel 25     //can be a DEFINE because doesnt change (used in manual)
  #define max_nivel 100 //can be a DEFINE because doesnt change (used in manual)
  #define tiempo_para_nivel 3000
  //Entradas y salidas
  #define nivel_del_tanque A0 
  #define electrovalvula 10
  #define resistencia 11
  #define onewire 9 // pin del onewire
  #define pulsador1 2
  #define pulsador2 3
  #define pulsador3 4
  #define pulsador4 5

  //Datos del Menu
  #define maxY_menu1 7
  #define maxY_menu2 5
  #define tiempo_de_parpadeo 700
  #define tiempo_de_espera_menu 5000 
  //Datos horas
  #define hora_max 24
  #define minuto_max 59 //I kit actual time because in used in other sites and here didnt work  SUCK MY DIK JEREMAIAS BRTOLSIC na mentira oka

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

  //Funciones control temperatura
  void Controltemp();

  //Funciones control nivel
  void Controllvl();
  // funcion actualizar sensores
  void Actualizar_entradas();
  //Funciones conexion arduino/esp
  void Serial_Read_UNO();
  void Serial_Send_UNO(uint8_t);

  //Funciones hora
  String String_de_hora(uint8_t,uint8_t);
  String CharToString(uint8_t,uint8_t);
  uint8_t StringToChar(uint8_t, String);

  //Otras Funciones
  uint8_t ReturnToCero(int8_t , uint8_t);
  
  char Character_Return(uint8_t , bool);

  void writeStringToEEPROM(uint8_t , const String);
  String readStringFromEEPROM(uint8_t);
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
  bool flag_farenheit = false; 
  bool calentar;
  int8_t temperatura_a_calentar; //se usa en calefaccion manual para guardar que temperatura se necesita
  uint8_t temperatura_inicial,temperatura_final;  // Guardado en memoria
  int8_t temperatura_actual; // temp actual
  //Variables de nivel          Para teo: vals that share in level funciones //Jere: ?? // Chuco: ??
  bool llenar;
  int8_t nivel_a_llenar; //se usa en llenado manual para guardar que temperatura se necesita  
  uint8_t nivel_inicial,nivel_final;// Guardado en memoria 
  uint8_t nivel_actual;

  //Variables de hora
  uint32_t tiempo_menues;             //(used in temperature funcions and in llenado auto to showw a confirm output)
  uint32_t tiempo_sensores;
  uint32_t  mili_segundos = 0;
  uint8_t hora_to_modify, minuto_to_modify;
  uint8_t sumador_hora, sumador_minuto;
  uint8_t hora,minutos;


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
  String Menuprincipal[maxY_menu1] = {
    "C manual",
    "H manual",
    "H & F in H",
    "C segun lleno",
    "H segun temp",
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
  Seleccionar_Funciones funcionActual = posicion_inicial;
  estadoMEF Estadoequipo = estado_inicial;

  //Variables Comunicacion esp/arduino
  bool Take_Comunication_Data=false;
  bool ComunicationError=false;
  bool InitComunication;  
  bool mayusculas=false;
  uint8_t StringLength=0;
  uint8_t ActualIndividualDataPos=0;
  char Actualchar=0;
  char input=0;
  String Serial_Input;
  String Individualdata[4];
  String IndividualValue;
  String WIFISSID,WIFIPASS;

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
  pinMode(electrovalvula, OUTPUT);
  pinMode(resistencia, OUTPUT);
  
  //Sensr de temperatura
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);
  
  //pulsadores pra manejar los menus//
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  //
  InitComunication=true;
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
  temperatura_inicial=eep.read(10);
  temperatura_final=eep.read(11);
  nivel_inicial=eep.read(12);
  nivel_final=eep.read(13);
  WIFISSID=readStringFromEEPROM(14); //32B + uno que indica largo
  WIFIPASS=readStringFromEEPROM(48); //64B + uno que indica largo
}

void loop() 
{
  Actualizar_entradas();
  Controllvl();
  Controltemp();

  if (Serial.available()>0)Serial_Read_UNO(); // si recibe un dato del serial lo lee

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
      //if(funcionActual==funcion_activar_bomba)menu_activar_bomba();        //Menu avanzado
      if(funcionActual==funcion_de_menu_seteo_wifi)menu_seteo_wifi();    //Menu avanzado
      break;
  }
 
}

void standby()
{ 

  lcd.setCursor(0,0); lcd.print("T:"); lcd.print(temperatura_actual);lcd.print((char)223); //imprime  el simbolo de °
  if(flag_farenheit == false) lcd.print("C");
  else lcd.print("F  ");
  lcd.setCursor(12,0);lcd.print("N:");  lcd.print(nivel_actual); lcd.print("% ");
  lcd.setCursor(6,1);
  lcd.print(String_de_hora(hora,minutos)+"hs");

  if(digitalRead(pulsador1)==LOW || digitalRead(pulsador2)==LOW || digitalRead(pulsador3)==LOW || digitalRead(pulsador4)==LOW){
    while (digitalRead(pulsador1)==LOW || digitalRead(pulsador2)==LOW || digitalRead(pulsador3)==LOW || digitalRead(pulsador4)==LOW){}
    switch (Estadoequipo)
    {
      case estado_standby:
        Estadoequipo = estado_inicial;
        lcd.backlight();
        tiempo_de_standby=0;
        break;
      case estado_inicial:
        Flag=1;
        Estadoequipo = menu1;
        lcd.backlight();
        break;
      default:
        Estadoequipo = estado_standby;
        break;
    }   
  }

  if(tiempo_de_standby>=5000){
    Estadoequipo = estado_standby;
    lcd.noBacklight();
    tiempo_de_standby = 0;
  }
}

void menu_basico()
{
  if(Flag==1){ // inicializa variables utiles para el menu
    Blink=false;
    lcd.clear();
    Ypos=0;
    opcionmenu1= 0;
    Flag=2;
    tiempo_menues=mili_segundos;
  }

  if (Flag==2){
    if (digitalRead(pulsador1) == LOW ){ while (digitalRead(pulsador1) == LOW){} Ypos=ReturnToCero(Ypos-1,maxY_menu1); lcd.clear(); Blink = true;} // suma 1 a Ypos
    if (digitalRead(pulsador2) == LOW ){ while (digitalRead(pulsador2) == LOW){} Ypos=ReturnToCero(Ypos+1,maxY_menu1); lcd.clear(); Blink = true; }// resta 1 a Ypos
    if (digitalRead(pulsador3) == LOW ){ while (digitalRead(pulsador3) == LOW){} opcionmenu1=Ypos+1; lcd.clear(); } //confirmacion
    if(mili_segundos>=(tiempo_menues+tiempo_de_parpadeo)){tiempo_menues=mili_segundos;Blink=!Blink;} //prende o apaga la flechita

    switch (opcionmenu1){
      case 0:
        lcd.setCursor(0,0);
        if(Blink == false){lcd.print(" "); tiempo_menues-=100;} //¿Por que hago tiempo actual -=100? en el lcd se ve mejor cuando el tiempo de "apagado" es menor al de encendio
        if(Blink == true)lcd.print(char(62));
        lcd.setCursor(1,0); 
        lcd.print(Menuprincipal[ReturnToCero(Ypos,maxY_menu1)]);
        lcd.setCursor(1,1); 
        lcd.print(Menuprincipal[ReturnToCero(Ypos+1,maxY_menu1)]);
        lcd.setCursor(1,2); 
        lcd.print(Menuprincipal[ReturnToCero(Ypos+2,maxY_menu1)]);
        lcd.setCursor(1,3); 
        lcd.print(Menuprincipal[ReturnToCero(Ypos+3,maxY_menu1)]);
        break;
      case 1:
        Estadoequipo=funciones;
        funcionActual=llenado_manual;
        Flag=2; // la declaracion de flag no es del todo necesaria pero se vuelve a declarar x seguridad
        break;
      case 2:
        Estadoequipo=funciones;
        funcionActual=calefaccion_manual;
        Flag=2;
        break;
      case 3:
        Estadoequipo=funciones;
        funcionActual=funcion_menu_de_auto_por_hora;
        Flag=2;
        break;
      case 4:
        Estadoequipo=funciones;
        funcionActual=llenado_auto;
        Flag=2;
        break;
      case 5:
        Estadoequipo=funciones;
        funcionActual=calefaccion_auto;
        Flag=2;
        break;
      case 6:
        Estadoequipo=menu2;
        Flag=3; // aumenta pq pasa al menu avanzado
        break;
      case 7:
        Estadoequipo=estado_inicial;
        tiempo_de_standby=0;
        Flag=0; // disminuye pq pasa al standby
        break;
    }
  }
}

void menu_de_llenado_manual(){
    switch (Flag)
    {
      case 2:
        if (min_nivel > nivel_actual) nivel_a_llenar=min_nivel;
        else nivel_a_llenar=nivel_actual;
        Flag=3;
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("Nivel a llenar: ");
        lcd.print(nivel_a_llenar );
        lcd.print(" ");
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con 1");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        if(digitalRead(pulsador1) == LOW){
            while(digitalRead(pulsador1) == LOW && nivel_a_llenar<max_nivel){}
            nivel_a_llenar += sumador_nivel;
          }
        if(digitalRead(pulsador2) == LOW && nivel_a_llenar>min_nivel){
            while(digitalRead(pulsador2) == LOW){}
            nivel_a_llenar -= sumador_nivel;
          }
        if(digitalRead(pulsador3) == LOW){
            while(digitalRead(pulsador3) == LOW){}
            Flag=4;
            lcd.clear();
          }
        if(digitalRead(pulsador4) == LOW){
            while(digitalRead(pulsador4) == LOW){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            lcd.clear();
          }
        break; 

        case 4:
          lcd.setCursor(0,0);
          lcd.print("Llenar hasta:"); lcd.print(nivel_a_llenar);lcd.print("%");
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");

          if(digitalRead(pulsador3) == LOW){
              while(digitalRead(pulsador3) == LOW){}
              tiempo_menues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
              lcd.clear();
              Flag=5;
            }
          if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=3; lcd.clear();}
          break;

        case 5:
          guardado_para_menus(true);
          break;
    }
}

void menu_de_calefaccion_manual(){
    switch (Flag)
    {
      case 2:
        if (min_temp>temperatura_actual) temperatura_a_calentar=min_temp;
        else  temperatura_a_calentar = temperatura_actual;
        Flag=3;
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("Temp. a calentar:");lcd.print(temperatura_a_calentar);
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con 1");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        if(digitalRead(pulsador1) == LOW && temperatura_a_calentar<maxima_temp){
            while(digitalRead(pulsador1) == LOW){}
            temperatura_a_calentar += sumador_temperatura;
          }
        
        if(digitalRead(pulsador2) == LOW && temperatura_a_calentar>min_temp){
            while(digitalRead(pulsador2) == LOW){}
            temperatura_a_calentar -= sumador_temperatura;
          }

        if(digitalRead(pulsador3) == LOW){
            while(digitalRead(pulsador3) == LOW){}
            Flag=4;
            lcd.setCursor(17,0); lcd.print("   ");
          }
        if(digitalRead(pulsador4) == LOW){
            while(digitalRead(pulsador4) == LOW){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            lcd.clear();
          }

        break; 

        case 4:
          lcd.setCursor(0,0);
          lcd.print("Calentar hasta: ");
          lcd.print(temperatura_a_calentar);
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");
          if(digitalRead(pulsador3) == LOW){
              while(digitalRead(pulsador3) == LOW){}
              Flag=5;
              tiempo_menues=mili_segundos;
              lcd.clear();
            }
          if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=3; lcd.clear();}
          break;

        case 5:
          guardado_para_menus(true);
          break;
    }
}

void menu_de_auto_por_hora()
{
  switch (Flag)
  {
    case 2:
      temperatura_inicial=60;
      Flag=3;
      sumador_hora=0;
      sumador_minuto=0;
      Ypos=0;
      hora_to_modify=hora;
      minuto_to_modify=minutos;
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("Seleccionar:"); lcd.print(ActualStruct+1); lcd.print(" slot");
      lcd.setCursor(0,1);
      lcd.print("1:");lcd.print(CharToString(1,save[0].hour));
      lcd.setCursor(0,2);
      lcd.print("2:");lcd.print(CharToString(1,save[1].hour));
      lcd.setCursor(0,3);
      lcd.print("3:");lcd.print(CharToString(1,save[2].hour));
      lcd.setCursor(0,4);
      ActualStruct = ReturnToCero(Ypos,3);

      if(digitalRead(pulsador1) == LOW){
          while(digitalRead(pulsador1) == LOW){}
          Ypos=ReturnToCero(Ypos+1,3);
        }
      if(digitalRead(pulsador2) == LOW){
          while(digitalRead(pulsador2) == LOW){}
          Ypos=ReturnToCero(Ypos-1,3);
      }
      if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          save[ActualStruct].temp=min_temp;
          save[ActualStruct].level=min_nivel;
          Flag=4;
        }
      if(digitalRead(pulsador4) == LOW){
            while(digitalRead(pulsador4) == LOW){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            lcd.clear();
            save[ActualStruct].temp = min_temp;
            save[ActualStruct].level = min_nivel;
          }
      break; 

    case 4:
      lcd.setCursor(0,0);
      lcd.print("Temperatura max:");
      lcd.print(save[ActualStruct].temp);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && save[ActualStruct].temp < maxima_temp){
          while(digitalRead(pulsador1) == LOW){}
          save[ActualStruct].temp += sumador_temperatura;
        }
      if(digitalRead(pulsador2) == LOW && save[ActualStruct].temp> min_temp){
          while(digitalRead(pulsador2) == LOW){}
          save[ActualStruct].temp -= sumador_temperatura;
        }
      if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=5;
          lcd.clear();
        }
      if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=3; lcd.clear();}
        break;

      case 5:
       lcd.setCursor(0,0);
      lcd.print("Nivel max:"); lcd.print(save[ActualStruct].level);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && save[ActualStruct].level<max_nivel){
          while(digitalRead(pulsador1) == LOW){}
          save[ActualStruct].level += sumador_nivel;
        }
      if(digitalRead(pulsador2) == LOW && save[ActualStruct].level>min_nivel){
          while(digitalRead(pulsador2) == LOW){}
          save[ActualStruct].level -= sumador_nivel;
        }
      if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=6;
          sumador_hora=0;
          sumador_minuto=0;
          lcd.clear();
        }
      if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=4; lcd.clear();}
        break;

      case 6:
        lcd.setCursor(0,0);
        lcd.print("Setear hora:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar con 1 ");
        lcd.setCursor(0,2);
        lcd.print("disminuir con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");
        hora_to_modify = ReturnToCero((hora+sumador_hora),hora_max);

        if(digitalRead(pulsador1) == LOW){while(digitalRead(pulsador1) == LOW){}sumador_hora++;}
        if(digitalRead(pulsador2) == LOW){while(digitalRead(pulsador2) == LOW){}sumador_hora--;}
        if(digitalRead(pulsador3) == LOW){while(digitalRead(pulsador3) == LOW){}Flag=7;lcd.clear();}
        if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=5; lcd.clear();}
        break;

      case 7:
        lcd.setCursor(0,0);
        lcd.print("Setear minuto:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar con 1  ");
        lcd.setCursor(0,2);
        lcd.print("disminuir con 2  ");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3  ");
        minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max+1);
        if(digitalRead(pulsador1) == LOW){while(digitalRead(pulsador1) == LOW){}sumador_minuto++;}
        if(digitalRead(pulsador2) == LOW){while(digitalRead(pulsador2) == LOW){}sumador_minuto--;}
        if(digitalRead(pulsador3) == LOW){while(digitalRead(pulsador3) == LOW){}Flag=8;lcd.clear();}
        if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=6; lcd.clear();}
        break;

      case 8:
        lcd.setCursor(0,0);
        lcd.print("A las:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("Calentar:"); lcd.print(save[ActualStruct].temp); lcd.print((char)223); lcd.print("c");
        lcd.setCursor(0,2);
        lcd.print("Llenar:"); lcd.print(save[ActualStruct].level);lcd.print("%");
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");

        if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          tiempo_menues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
          lcd.clear();
          Flag=9;
          save[ActualStruct].hour= StringToChar(1,String_de_hora(hora_to_modify,minuto_to_modify));
        }
        if(digitalRead(pulsador4) == LOW){while(digitalRead(pulsador4) == LOW){} Flag=7; lcd.clear();}
        break;

      case 9:
        eep.write((ActualStruct*3)+1, save[ActualStruct].hour);
        eep.write((ActualStruct*3)+2, save[ActualStruct].level);
        eep.write((ActualStruct*3)+3, save[ActualStruct].temp);
        guardado_para_menus(true);
      break;
    }
  }

void menu_de_llenado_auto()
{
  switch (Flag)
  {
    case 2:
      nivel_inicial=min_nivel;
      Flag=3;
      break;

    case 3:
      lcd.setCursor(0,0);
      lcd.print("Nivel min: "); lcd.print(nivel_inicial);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && nivel_inicial<max_nivel){
        while(digitalRead(pulsador1) == LOW){}
        nivel_inicial += sumador_nivel;
      }
      if(digitalRead(pulsador2) == LOW && nivel_inicial>min_nivel){
        while(digitalRead(pulsador2) == LOW){}
        nivel_inicial -= sumador_nivel;
      }
      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=4;
        nivel_final=nivel_inicial+sumador_nivel;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW){
        while(digitalRead(pulsador4) == LOW){}
        nivel_inicial = eep.read(12);
        nivel_final = eep.read(13);
        Estadoequipo=menu1;
        Flag=1;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break;
      
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Nivel max: "); lcd.print(nivel_final);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && nivel_final<max_nivel){
        while(digitalRead(pulsador1) == LOW){}
        nivel_final += sumador_nivel;
      }
      if(digitalRead(pulsador2) == LOW && nivel_final>nivel_inicial+sumador_nivel){
        while(digitalRead(pulsador2) == LOW){}
        nivel_final -= sumador_nivel;
      }
      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=5;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW){
        while(digitalRead(pulsador4) == LOW){}
        Flag=3;
        lcd.clear();
      }
      break;

    case 5: 
      lcd.setCursor(0,0);
      lcd.print("Minimo:");lcd.print(nivel_inicial);
      lcd.setCursor(0,1);
      lcd.print("Llenar:");lcd.print(nivel_final);
      lcd.setCursor(0,3);
      lcd.print("     Confirmar?    ");

      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=6;
        tiempo_menues=mili_segundos;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW){
        while(digitalRead(pulsador4) == LOW){}
        Flag=4;
        lcd.clear();
      }
      break;

    case 6:
      eep.write(12,nivel_inicial);
      eep.write(13,nivel_final);
      guardado_para_menus(true);
      break; 
  }
}

void menu_de_calefaccion_auto(){
  switch (Flag)
  {
    case 2:
      temperatura_inicial=min_temp;
      Ypos=0;
      Flag=3;
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("Temperatura min:");lcd.print(temperatura_inicial);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && temperatura_inicial<maxima_temp){
        while(digitalRead(pulsador1) == LOW){}
        temperatura_inicial += sumador_temperatura;
      }
      
      if(digitalRead(pulsador2) == LOW && min_temp<temperatura_inicial){
        while(digitalRead(pulsador2) == LOW){}
        temperatura_inicial -= sumador_temperatura;
      }

      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=4;
        temperatura_final=temperatura_inicial+sumador_temperatura;
      }
      
      if(digitalRead(pulsador4) == LOW){
        while(digitalRead(pulsador4) == LOW){}
        Estadoequipo=menu1;
        Flag=1;
        funcionActual=posicion_inicial;
        temperatura_inicial = eep.read(10);
        temperatura_final = eep.read(11);
        lcd.clear();
      }
      break; 
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Temperatura max:");lcd.print(temperatura_final);
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if(digitalRead(pulsador1) == LOW && temperatura_final < maxima_temp){
        while(digitalRead(pulsador1) == LOW){}
        temperatura_final += sumador_temperatura;
      }
      if(digitalRead(pulsador2) == LOW && temperatura_final>temperatura_inicial+sumador_temperatura){
        while(digitalRead(pulsador2) == LOW){}
        temperatura_final -= sumador_temperatura;
      }
      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=5;
        lcd.clear();
      }
      if(digitalRead(pulsador4) == LOW){
        while(digitalRead(pulsador4) == LOW){}
        Flag=3;
        lcd.clear();
      }
      
      break;

      case 5:
        lcd.setCursor(0,0);
        lcd.print("Minima guardada:");lcd.print(temperatura_inicial);
        lcd.setCursor(0,1);
        lcd.print("Maxima guardada:");lcd.print(temperatura_final);
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");

        if(digitalRead(pulsador3) == LOW){
            while(digitalRead(pulsador3) == LOW){}
            Flag=6;
            tiempo_menues=mili_segundos;
            lcd.clear();
          }
          if(digitalRead(pulsador4) == LOW){
            while(digitalRead(pulsador4) == LOW){}
            Flag=4;
            lcd.clear();
          }
          break;

      case 6:
        eep.write(10,temperatura_inicial);
        eep.write(11,temperatura_final);
        guardado_para_menus(true);
        break; 
  }
}

void menu_avanzado()
{
  if (Flag==3){
    Blink=false;
    lcd.clear();
    Ypos=0;
    opcionmenu2=0;
    Flag=4;
  }
  
  if (Flag==4){
  if (digitalRead(pulsador1) == LOW ){ while (digitalRead(pulsador1) == LOW){} Ypos=ReturnToCero(Ypos+1,maxY_menu2); lcd.clear(); Blink = true; }
  if (digitalRead(pulsador2) == LOW ){ while (digitalRead(pulsador2) == LOW){} Ypos=ReturnToCero(Ypos-1,maxY_menu2); lcd.clear(); Blink = true;}
  if (digitalRead(pulsador3) == LOW ){ while (digitalRead(pulsador3) == LOW){} opcionmenu2=ReturnToCero(Ypos,maxY_menu2)+1; lcd.clear(); }
  if(mili_segundos>=(tiempo_menues+tiempo_de_parpadeo)){tiempo_menues=mili_segundos;Blink=!Blink;}
    
    switch (opcionmenu2)
    {
      case 0:
        lcd.setCursor(0,0);
        if(Blink == false){lcd.print(" "); tiempo_menues-=100;} //¿Por que hago tiempo actual -=100? en el lcd se ve mejor cuando el tiempo de "apagado" es menor al de encendio
        if(Blink == true)lcd.print(char(62));
        lcd.setCursor(1,0); 
        lcd.print(menuavanzado[ReturnToCero(Ypos,maxY_menu2)]); 
        lcd.setCursor(1,1); 
        lcd.print(menuavanzado[ReturnToCero(Ypos+1,maxY_menu2)]); 
        lcd.setCursor(1,2); 
        lcd.print(menuavanzado[ReturnToCero(Ypos+2,maxY_menu2)]); 
        lcd.setCursor(1,3); 
        lcd.print(menuavanzado[ReturnToCero(Ypos+3,maxY_menu2)]);
        break;
      case 1:
        Estadoequipo=funciones;
        funcionActual=funcion_de_menu_modificar_hora_rtc;
        break;
      case 2: 
        Estadoequipo=funciones;
        funcionActual=funcion_farenheit_celsius;
        break;
      case 3:
        // funcionActual=funcion_activar_bomba:
        break;
      case 4:
        Estadoequipo=funciones;
        funcionActual=funcion_de_menu_seteo_wifi;
        break;
      case 5:
        Estadoequipo=menu1;
        tiempo_de_standby=0;
        Flag=1;
        opcionmenu1=0;
        break;
    }
  }
}

void menu_modificar_hora_rtc()
  {
    switch (Flag)
    {
      case 4:
        sumador_hora=0;
        sumador_minuto=0;
        hora_to_modify=hora;
        minuto_to_modify=minutos;
        Flag=5;
        lcd.clear();
        break;
      case 5:
        lcd.setCursor(0,0);
        lcd.print("Setear hora:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar con 1 ");
        lcd.setCursor(0,2);
        lcd.print("disminuir con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        hora_to_modify = ReturnToCero(hora+sumador_hora,hora_max);
        minuto_to_modify=minutos;

        if(digitalRead(pulsador1) == LOW){ 
          while(digitalRead(pulsador1) == LOW){}
          sumador_hora++;
        }
        if(digitalRead(pulsador2) == LOW){
          while(digitalRead(pulsador2) == LOW){}
          sumador_hora--;
        }
        if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=6;
          lcd.clear();
        }
        if(digitalRead(pulsador4) == LOW){
          while(digitalRead(pulsador4) == LOW){}
          Estadoequipo=menu2;
          Flag=3;
          funcionActual=posicion_inicial;
          lcd.clear();
        }
        break;          

      case 6:
        lcd.setCursor(0,0);
        lcd.print("Setear minuto:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("disminuir con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max);

        if(digitalRead(pulsador1) == LOW){ 
          while(digitalRead(pulsador1) == LOW){}
          sumador_minuto++;
        }
        if(digitalRead(pulsador2) == LOW){
          while(digitalRead(pulsador2) == LOW){}
          sumador_minuto--;
        }
        if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=7;
          lcd.clear();
        }
        if(digitalRead(pulsador4) == LOW){
          while(digitalRead(pulsador4) == LOW){}
          Flag=5;
          lcd.clear();
        }
        break;

      case 7:
        lcd.setCursor(0,0);
        lcd.print("hora guardada:");lcd.print(String(hora_to_modify) + ":" + String(minuto_to_modify));
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");
        if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=8;
          DateTime now = rtc.now();
          rtc.adjust(DateTime(now.year(),now.month(),now.day(),hora_to_modify,minuto_to_modify,now.second()));
          tiempo_menues=mili_segundos;
          lcd.clear();
        }
        if(digitalRead(pulsador4) == LOW){
          while(digitalRead(pulsador4) == LOW){}
          Flag=6;
          lcd.clear();
        }
        break;

      case 8:
        guardado_para_menus(false);
        break; 
    }
  }

void menu_farenheit_celsius()
{
  switch (Flag)
  {
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Seleccione Farenheit");
      lcd.setCursor(5,1);
      lcd.print(" o Celsius");
      lcd.setCursor(5,2);
      lcd.print((char)223); //imprime °
      lcd.print("F");
      lcd.setCursor(11,2);
      lcd.print((char)223); //imprime °
      lcd.print("C");
      lcd.setCursor(5,3);
      lcd.print("1");
      lcd.setCursor(11,3);
      lcd.print("2");
      if(digitalRead(pulsador2) == LOW ){
          while(digitalRead(pulsador2) == LOW){}
          flag_farenheit = false;
          Flag=5;
        }
      if(digitalRead(pulsador1) == LOW ){
          while(digitalRead(pulsador1) == LOW){}
          flag_farenheit = true;
          Flag=5;
        }
      if(digitalRead(pulsador4) == LOW){
          while(digitalRead(pulsador4) == LOW){}
          Estadoequipo=menu2;
          Flag=3;
          funcionActual=posicion_inicial;
          lcd.clear();
        }
    break;

    case 5:
      lcd.clear();
      tiempo_menues=mili_segundos;
      Flag=6;
    break;

    case 6:
      guardado_para_menus(false);
    break;
  }
  
}

void menu_seteo_wifi(){  

  switch (Flag)
  {
  case 4:
    lcd.setCursor(0,0);
    lcd.print("Nombre Wifi:");lcd.print(WIFISSID);
    lcd.setCursor(1,0);
    lcd.print("Contraseña Wifi:");lcd.print(WIFIPASS);
    lcd.setCursor(2,0);
    lcd.print("Desea modificar?");
    lcd.setCursor(3,0);
    lcd.print("P3: SI   P4: NO");

    if(digitalRead(pulsador3) == LOW ){
        while(digitalRead(pulsador3) == LOW){}
        lcd.clear();
        Flag=5;
      }
    if(digitalRead(pulsador4) == LOW ){
        while(digitalRead(pulsador4) == LOW){}
        Estadoequipo=menu2;
        Flag=3;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
    break;
  case 5:
    WIFISSID="                                 ";
    WIFIPASS="                                   ";
    Ypos=0;
    Actualchar=0;
    Flag=6;
    break;
  case 6:
    lcd.setCursor(0,0);
    lcd.print("Nombre Wifi:");
    lcd.setCursor(0,1);
    lcd.print(WIFISSID);
    Actualchar=ReturnToCero(Actualchar, 40);
    WIFISSID.setCharAt(Ypos,Character_Return(Actualchar, mayusculas));

    if(digitalRead(pulsador1) == LOW )
      {
        while(digitalRead(pulsador1) == LOW){}
        Actualchar++;
      }
    if(digitalRead(pulsador2) == LOW )
      {
        while(digitalRead(pulsador2) == LOW){}
        mayusculas=!mayusculas;
      }
    if(digitalRead(pulsador3) == LOW )
      {
        while(digitalRead(pulsador3) == LOW){}
        Ypos ++;
        Actualchar=0;
      }
    if(digitalRead(pulsador4) == LOW )
      {
        while(digitalRead(pulsador4) == LOW){}
        lcd.clear();
        Flag=7;
        Ypos=0;
        Actualchar=0;
      }
    break;

    case 7:
    lcd.setCursor(0,0);
    lcd.print("Pass Wifi:");
    lcd.setCursor(0,1);
    lcd.print(WIFIPASS);
    Actualchar=ReturnToCero(Actualchar, 40);
    WIFIPASS.setCharAt(Ypos,Character_Return(Actualchar, mayusculas));

    if(digitalRead(pulsador1) == LOW )
      {
        while(digitalRead(pulsador1) == LOW){}
        Actualchar++;
      }
    if(digitalRead(pulsador2) == LOW )
      {
        while(digitalRead(pulsador2) == LOW){}
        mayusculas=!mayusculas;
      }
    if(digitalRead(pulsador3) == LOW )
      {
        while(digitalRead(pulsador3) == LOW){}
        Ypos ++;
        Actualchar=0;
      }
    if(digitalRead(pulsador4) == LOW )
      {
        while(digitalRead(pulsador4) == LOW){}
        lcd.clear();
        tiempo_menues=mili_segundos;
        Flag=8;
        Ypos=0;
        Actualchar=0;
      }
    break;
      case 8:
        writeStringToEEPROM(14,WIFISSID);
        writeStringToEEPROM(48,WIFIPASS);
        guardado_para_menus(false);
      break;
  }

}

void Serial_Read_UNO(){
  Serial_Input=Serial.readString();// iguala el string del serial a un string input
  StringLength= Serial_Input.length();// saca el largo del string
  input=Serial_Input.charAt(0); // toma el char del comando a realizar (usualmente una letra)
  // Separador del string en variables:
  for (uint8_t CharPos = 2; CharPos <= StringLength; CharPos++){ // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
    if(Serial_Input.charAt(CharPos)==':') ActualIndividualDataPos++; //si hay : divide los datos
    else{// si no es nungun caracter especial:
      if(Serial_Input.charAt(CharPos-1)==':' || Serial_Input.charAt(CharPos-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(CharPos);//si es el primer digito lo iguala
      else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(CharPos);//si es el segundo en adelante lo suma al string
    }
    if(CharPos==StringLength)Take_Comunication_Data=true; //comienza a igualar variables
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
      temperatura_inicial = Individualdata[0].toInt();
      temperatura_final = Individualdata[1].toInt();// tempmax
      eep.write(10,temperatura_inicial);
      eep.write(11,temperatura_final);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'V':
      nivel_inicial = Individualdata[0].toInt();// lvlmin
      nivel_final = Individualdata[1].toInt();// lvlmax
      eep.write(12,nivel_inicial);
      eep.write(13,nivel_final);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    case 'E':
      Serial_Send_UNO(6);
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      ComunicationError=true;
      break;
    case 'O':
      if(Individualdata[0]=="OK")
      ActualIndividualDataPos=0;
      Take_Comunication_Data=false;
      break;
    default:
      Serial.println("?_NOTHING TO READ");
      break;
    }
  }
}

void Serial_Send_UNO(uint8_t WhatSend){
  if (ComunicationError==false){
    switch (WhatSend){
      case 1:
        if (InitComunication==true){
          for (uint8_t MessagePoss=0; MessagePoss <= 5; MessagePoss++){
            switch (MessagePoss){
              case 0:
                Serial.println("S_"+WIFISSID+":"+WIFIPASS);
                break;
              case 1:
                Serial.println("K_"+String(save[0].hour)+":"+String(save[0].temp)+":"+String(save[0].level));
                break;
              case 2:
                Serial.println("K_"+String(save[1].hour)+":"+String(save[1].temp)+":"+String(save[1].level));
                break;
              case 3:
                Serial.println("K_"+String(save[2].hour)+":"+String(save[2].temp)+":"+String(save[2].level));
                break;
              case 4:
                Serial.println("J_"+String(temperatura_inicial)+":"+String(temperatura_final));
                break;
              case 5:
                Serial.println("V_"+String(temperatura_inicial)+":"+String(temperatura_final));
                InitComunication=false;
                break;
            } 
          }  
        }
      break;
      case 2:
        if (InitComunication==false)Serial.println("K_"+String(save[ActualStruct].hour)+":"+String(save[ActualStruct].temp)+":"+String(save[ActualStruct].level));
        break;
      case 3:
        if (InitComunication==false)Serial.println("J_"+String(temperatura_inicial)+":"+String(temperatura_final));
        break;
      case 4:
        if (InitComunication==false)Serial.println("V_"+String(temperatura_inicial)+":"+String(temperatura_final));
        break;
      case 5:
        if(InitComunication==false)Serial.println("U_"+String(StringToChar(1,String(hora)+":"+String(minutos)))+":"+String(nivel_actual)+":"+String(temperatura_actual));
        break;
      case 6:
        if (InitComunication==false)Serial.println("?_RESET");
        break;
    }
  }
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

String CharToString(uint8_t function,uint8_t save)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t var2_deconvert=0;
  uint8_t resto_deconvert=0;
  String returned;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ 
  switch (function)
    {
      case 1:
        resto_deconvert= (save) % 4;
        var1_deconvert= (save-resto_deconvert)/4;
        var2_deconvert=resto_deconvert*15;
        returned= String(var1_deconvert)+':'+String(var2_deconvert);
        return returned;
      break;
      case 2:
        returned= String(save);
        return returned;
      break;
      case 3:
        returned= String(save);
        return returned;
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
      if (Character_pos==26) return 36;
      if (Character_pos==27) return 37;
      if (Character_pos==28) return 38;
      if (Character_pos==29) return 47;
      if (Character_pos==30) return 40;
      if (Character_pos==31) return 41;
      if (Character_pos==32) return 61;
      if (Character_pos==33) return 59;
      if (Character_pos==34) return 58;
      if (Character_pos==35) return 94;
      if (Character_pos==36) return 63;
      if (Character_pos==37) return 95;
      if (Character_pos==38) return 35;
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
  digitalWrite(resistencia,temperatura_actual < temperatura_inicial ? HIGH : LOW);
  digitalWrite(resistencia,temperatura_actual >= (temperatura_final + temp_threshold) ? LOW : HIGH);
  //=========Compara nivel actual con el minimo seteado============
  digitalWrite(resistencia,temperatura_actual < temperatura_a_calentar ? HIGH : LOW);
  //=================================================================
}

void Controllvl()
{
  // control lvl min to max
  digitalWrite(electrovalvula,nivel_actual <= nivel_inicial ? HIGH:LOW);
  digitalWrite(electrovalvula,nivel_actual == nivel_final ? LOW:HIGH);
  //======Compara temperatura actual con el minimo seteado=========
  digitalWrite(electrovalvula,nivel_actual < min_nivel ? HIGH:LOW);
  //=================================================================
}

void guardado_para_menus( bool Menu){
  lcd.setCursor(4,0);
  lcd.print("Guardando...");
  if(mili_segundos>=tiempo_menues+tiempo_de_espera_menu){
  if(Menu == true){
    Estadoequipo=menu1;
    Flag=1;
    funcionActual=posicion_inicial;
    lcd.clear();
  }
  if(Menu == false){
    Estadoequipo=menu2; 
    Flag=3; 
    funcionActual=posicion_inicial; 
    lcd.clear();
  }
  }
}

void writeStringToEEPROM(uint8_t addrOffset, const String strToWrite){
  uint8_t len = strToWrite.length();
  eep.write(addrOffset, len);
  for (uint8_t i = 0; i < len; i++)
  {
    eep.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

// todo lo que es decumentacion esta en "datos a tener en cuenta"
String readStringFromEEPROM(uint8_t addrOffset){
  uint8_t newStrLen = eep.read(addrOffset);
  char data[newStrLen + 1];
  for (uint8_t i = 0; i < newStrLen; i++)
  {
    data[i] = eep.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}