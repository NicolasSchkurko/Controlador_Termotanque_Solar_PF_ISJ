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
  #define maxi_cast 90           //puede hacerse un DEFINE porque no se modifica
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
  #define tiempo_de_espera_menu 5000 
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
  void menu_activar_bomba();        //Menu avanzado


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
  uint8_t PotenciometerOutput(uint8_t, uint8_t, uint8_t);

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

void standby()
{ 

  lcd.setCursor(0,0); lcd.print("T:"); 
  if(use_farenheit == false) {lcd.print(temperatura_actual); lcd.print((char)223); lcd.print("C");}
  if(use_farenheit == true) {lcd.print(((9*temperatura_actual)/5)+32);lcd.print((char)223); lcd.print("F  ");}
  lcd.setCursor(12,0);lcd.print("N:");  lcd.print(nivel_actual); lcd.print("% ");
  lcd.setCursor(6,1);
  lcd.print(String_de_hora(hora,minutos)+"hs");

  if((PIND & (1<<PD2)) == 0 || (PIND & (1<<PD3)) == 0 || (PIND & (1<<PD4)) == 0 || (PIND & (1<<PD5)) == 0){
    while((PIND & (1<<PD2)) == 0 || (PIND & (1<<PD3)) == 0 || (PIND & (1<<PD4)) == 0 || (PIND & (1<<PD5)) == 0){}
    switch (Estadoequipo)
    {
      case estado_standby:
        Estadoequipo = estado_inicial;
        lcd.backlight();
        tiempo_de_standby=0;
        break;
      case estado_inicial:
        Flag=1;
        lcd.backlight();
        Estadoequipo = menu1;
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
    opcionmenu1= 0;
    Flag=2;
    tiempo_de_standby = 0;
    tiempo_menues=mili_segundos;
  }
  if (Flag==2){
    if ((PIND & (1<<PD2)) == 0 ){while ((PIND & (1<<PD2)) == 0 ){} Ypos=ReturnToCero(Ypos-1,maxY_menu1); lcd.clear(); Blink = true; tiempo_de_standby = 0;} // suma 1 a Ypos
    if ((PIND & (1<<PD3)) == 0 ){while ((PIND & (1<<PD3)) == 0 ){}  Ypos=ReturnToCero(Ypos+1,maxY_menu1); lcd.clear(); Blink = true; tiempo_de_standby = 0;}// resta 1 a Ypos
    if ((PIND & (1<<PD4)) == 0 ){while ((PIND & (1<<PD4)) == 0 ){}  opcionmenu1=Ypos+1; lcd.clear(); } //confirmacion
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
  if(tiempo_de_standby>=5000){
    Estadoequipo = estado_standby;
    tiempo_de_standby = 0;
    Flag=1;
    lcd.clear();
  }
  }
}

void menu_de_llenado_manual(){
    switch (Flag)
    {
      case 2:
        nivel_a_llenar=nivel_a_llenar+25-(nivel_a_llenar%25);
        Flag=3;
        Auxiliar1=map(analogRead(A2), 0, 1023,min_nivel,max_nivel);
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("Nivel a llenar: ");
        lcd.print(nivel_a_llenar);
        if (nivel_a_llenar<100)lcd.print("% ");
        else lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con 1");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        nivel_a_llenar = map(analogRead(A2), 0, 1023,min_nivel,max_nivel);
        if(Auxiliar1!=map(analogRead(A2), 0, 1023,min_nivel,max_nivel)){
          nivel_a_llenar=ReturnToCero(Ypos+map(analogRead(A2), 0, 1023,min_nivel,max_nivel)-Auxiliar1,maxY_menu1);
          tiempo_de_standby=0;
          lcd.clear();
          Auxiliar1=map(analogRead(A2), 0, 1023,0,maxY_menu1);
        }
        if (nivel_a_llenar>max_nivel)nivel_a_llenar=max_nivel;
        if (nivel_a_llenar<min_nivel)nivel_a_llenar=min_nivel;
        if((PIND & (1<<PD2)) == 0 && nivel_a_llenar<max_nivel){
            while((PIND & (1<<PD2)) == 0){}
            nivel_a_llenar += sumador_nivel;
          }
        if((PIND & (1<<PD3)) == 0 && nivel_a_llenar>min_nivel){
            while((PIND & (1<<PD3)) == 0){}
            nivel_a_llenar -= sumador_nivel;
          }
        if((PIND & (1<<PD4)) == 0){
            while((PIND & (1<<PD4)) == 0){}
            Flag=4;
            lcd.clear();
          }
        if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            Auxiliar1=0;
            lcd.clear();
          }
        break; 

        case 4:
          lcd.setCursor(0,0);
          lcd.print("Llenar hasta:"); lcd.print(nivel_a_llenar);lcd.print("%");
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");

          if((PIND & (1<<PD4)) == 0){
              while((PIND & (1<<PD4)) == 0){}
              tiempo_menues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
              lcd.clear();
              Flag=5;
            }
          if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=3; lcd.clear();}
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
        lcd.print("T. a calentar:");
        if(use_farenheit == false) {
          lcd.print(temperatura_a_calentar); 
          if (temperatura_a_calentar<100){lcd.print((char)223); lcd.print("C ");}
          else{lcd.print((char)223); lcd.print("C");}
        }
        if(use_farenheit == true) {
          lcd.print(((9*temperatura_a_calentar)/5)+32);
          if (temperatura_a_calentar<100){lcd.print((char)223); lcd.print("F ");}
          else{lcd.print((char)223); lcd.print("F");}
        }
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con 1");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        temperatura_a_calentar = map(analogRead(A2), 0, 1023,min_temp,maxi_cast);
        if (temperatura_a_calentar>maxi_cast)temperatura_a_calentar=maxi_cast;
        if (temperatura_a_calentar<min_temp)temperatura_a_calentar=min_temp;
        if((PIND & (1<<PD2)) == 0 && temperatura_a_calentar<maxi_cast){
            while((PIND & (1<<PD2)) == 0){}
            temperatura_a_calentar += sumador_temperatura;
          }
        
        if((PIND & (1<<PD3)) == 0 && temperatura_a_calentar>min_temp){
            while((PIND & (1<<PD3)) == 0){}
            temperatura_a_calentar -= sumador_temperatura;
          }

        if((PIND & (1<<PD4)) == 0){
            while((PIND & (1<<PD4)) == 0){}
            Flag=4;
            lcd.setCursor(17,0); lcd.print("   ");
          }
        if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            lcd.clear();
          }
        break; 

        case 4:
          lcd.setCursor(0,0);
          lcd.print("Calentar hasta: ");
          if(use_farenheit == false) {lcd.print(temperatura_a_calentar); lcd.print((char)223); lcd.print("C");}
          if(use_farenheit == true) {lcd.print(((9*temperatura_a_calentar)/5)+32);lcd.print((char)223); lcd.print("F  ");}
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");
          if((PIND & (1<<PD4)) == 0){
              while((PIND & (1<<PD4)) == 0){}
              Flag=5;
              tiempo_menues=mili_segundos;
              lcd.clear();
            }
          if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=3; lcd.clear();}
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
      lcd.print("1:");lcd.print(String_de_hora(CharToUINT(1,save[0].hour),CharToUINT(2,save[0].hour)));
      lcd.setCursor(0,2);
      lcd.print("2:");lcd.print(String_de_hora(CharToUINT(1,save[1].hour),CharToUINT(2,save[1].hour)));
      lcd.setCursor(0,3);
      lcd.print("3:");lcd.print(String_de_hora(CharToUINT(1,save[2].hour),CharToUINT(2,save[2].hour)));

      ActualStruct = ReturnToCero(Ypos,3);
      Ypos += map(analogRead(A2), 0, 1023,0,2);
      if((PIND & (1<<PD2)) == 0){
          while((PIND & (1<<PD2)) == 0){}
          Ypos=ReturnToCero(Ypos+1,3);
        }
      if((PIND & (1<<PD3)) == 0){
          while((PIND & (1<<PD3)) == 0){}
          Ypos=ReturnToCero(Ypos-1,3);
      }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          save[ActualStruct].temp=min_temp;
          save[ActualStruct].level=min_nivel;
          Flag=4;
        }
      if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Estadoequipo=menu1;
            Flag=1;
            funcionActual=posicion_inicial;
            lcd.clear();
            save[ActualStruct].temp = eep.read(3*(ActualStruct+1));
            save[ActualStruct].level = eep.read(2*(ActualStruct+1));
          }
      break; 

    case 4:
      lcd.setCursor(0,0);
      lcd.print("Temperatura max:");
      if(use_farenheit == false) {
        lcd.print(save[ActualStruct].temp); 
        if (save[ActualStruct].temp<100){lcd.print((char)223); lcd.print("C ");}
        else{lcd.print((char)223); lcd.print("C");}
      }
      if(use_farenheit == true) {
        lcd.print(((9*save[ActualStruct].temp)/5)+32);
        if (save[ActualStruct].temp<100){lcd.print((char)223); lcd.print("F ");}
        else{lcd.print((char)223); lcd.print("F");}
      }

      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if (save[ActualStruct].temp>maxi_cast)save[ActualStruct].temp=maxi_cast;
      if (save[ActualStruct].temp<min_temp)save[ActualStruct].temp=min_temp;
      save[ActualStruct].temp = map(analogRead(A2), 0, 1023,min_temp,maxi_cast);
      if((PIND & (1<<PD2)) == 0 && save[ActualStruct].temp < maxi_cast){
          while((PIND & (1<<PD2)) == 0){}
          save[ActualStruct].temp += sumador_temperatura;
        }
      if((PIND & (1<<PD3)) == 0 && save[ActualStruct].temp> min_temp){
          while((PIND & (1<<PD3)) == 0){}
          save[ActualStruct].temp -= sumador_temperatura;
        }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Flag=5;
          lcd.clear();
        }
      if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=3; lcd.clear();}
        break;

      case 5:
       lcd.setCursor(0,0);
      lcd.print("Nivel max:"); 
      lcd.print(save[ActualStruct].level);
      if (save[ActualStruct].level<100)lcd.print("% ");
      else lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if (save[ActualStruct].level>max_nivel)save[ActualStruct].level=max_nivel;
      if (save[ActualStruct].level<min_nivel)save[ActualStruct].level=min_nivel;
      save[ActualStruct].level = map(analogRead(A2), 0, 1023,min_nivel,max_nivel);
      if((PIND & (1<<PD2)) == 0 && save[ActualStruct].level<max_nivel){
          while((PIND & (1<<PD2)) == 0){}
          save[ActualStruct].level += sumador_nivel;
        }
      if((PIND & (1<<PD3)) == 0 && save[ActualStruct].level>min_nivel){
          while((PIND & (1<<PD3)) == 0){}
          save[ActualStruct].level -= sumador_nivel;
        }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Flag=6;
          sumador_hora=0;
          sumador_minuto=0;
          lcd.clear();
        }
      if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=4; lcd.clear();}
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

        hora_to_modify = map(analogRead(A2), 0, 1023, 0, hora_max-1);
        hora_to_modify = ReturnToCero((hora+sumador_hora),hora_max);
        if((PIND & (1<<PD2)) == 0){while((PIND & (1<<PD2)) == 0){}sumador_hora++;}
        if((PIND & (1<<PD3)) == 0){while((PIND & (1<<PD3)) == 0){}sumador_hora--;}
        if((PIND & (1<<PD4)) == 0){while((PIND & (1<<PD4)) == 0){}Flag=7;lcd.clear();}
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=5; lcd.clear();}
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

        minuto_to_modify = map(analogRead(A2), 0, 1023, 0, minuto_max-1);
        minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max);
        if((PIND & (1<<PD2)) == 0){while((PIND & (1<<PD2)) == 0){}sumador_minuto++;}
        if((PIND & (1<<PD3)) == 0){while((PIND & (1<<PD3)) == 0){}sumador_minuto--;}
        if((PIND & (1<<PD4)) == 0){while((PIND & (1<<PD4)) == 0){}Flag=8;lcd.clear();}
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=6; lcd.clear();}
        break;

      case 8:
        lcd.setCursor(0,0);
        lcd.print("A las:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("Calentar:");
        if(use_farenheit == false) {lcd.print(save[ActualStruct].temp); lcd.print((char)223); lcd.print("C");}
        if(use_farenheit == true) {lcd.print(((9*save[ActualStruct].temp)/5)+32);lcd.print((char)223); lcd.print("F  ");}
        lcd.setCursor(0,2);
        lcd.print("Llenar:"); lcd.print(save[ActualStruct].level);lcd.print("%");
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");

        if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          tiempo_menues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
          lcd.clear();
          Flag=9;
          save[ActualStruct].hour= StringToChar(1,String_de_hora(hora_to_modify,minuto_to_modify));
        }
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Flag=7; lcd.clear();}
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
      Valorinicial=eep.read(12);
      Valorfinal=eep.read(13);
      eep.write(12,min_nivel);
      Flag=3;
      break;

    case 3:
      lcd.setCursor(0,0);
      lcd.print("Nivel min: "); 
      lcd.print(eep.read(12));
      if (eep.read(12)<100)lcd.print("% ");
      else lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      eep.write(12,map(analogRead(A2), 0, 1023, min_nivel, max_nivel-sumador_nivel));
      if (eep.read(12)>max_nivel-sumador_nivel)eep.write(12,max_nivel-sumador_nivel);
      if (eep.read(12)<min_nivel)eep.write(12,min_nivel);
      if((PIND & (1<<PD2)) == 0 && eep.read(12)<max_nivel-sumador_nivel){
        while((PIND & (1<<PD2)) == 0){}
        eep.write(12,eep.read(12)+sumador_nivel);
      }
      if((PIND & (1<<PD3)) == 0 && eep.read(12)>min_nivel){
        while((PIND & (1<<PD3)) == 0){}
        eep.write(12,eep.read(12)-sumador_nivel);
      }
      if((PIND & (1<<PD4)) == 0){
        while((PIND & (1<<PD4)) == 0){}
        Flag=4;
        eep.write(13,eep.read(12)+sumador_nivel);
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        eep.write(12,Valorinicial);
        eep.write(13,Valorfinal);
        Estadoequipo=menu1;
        Flag=1;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break;
      
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Nivel max: "); 
      lcd.print(eep.read(13));
      if (eep.read(13)<100)lcd.print("% ");
      else lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      eep.write(13,map(analogRead(A2), 0, 1023, eep.read(12)+sumador_nivel, maxi_cast));
      if (eep.read(13)>max_nivel)eep.write(13,max_nivel);
      if (eep.read(13)<eep.read(12)+sumador_nivel)eep.write(13,eep.read(12)+sumador_nivel);
      if((PIND & (1<<PD2)) == 0 && eep.read(13)<max_nivel){
        while((PIND & (1<<PD2)) == 0){}
        eep.write(13,eep.read(13)+sumador_nivel); 
      }
      if((PIND & (1<<PD3)) == 0 && eep.read(13)>eep.read(12)+sumador_nivel){
        while((PIND & (1<<PD3)) == 0){}
         eep.write(13,eep.read(13)-sumador_nivel); 
      }
      if((PIND & (1<<PD4)) == 0){
        while((PIND & (1<<PD4)) == 0){}
        Flag=5;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Flag=3;
        lcd.clear();
      }
      break;

    case 5: 
      lcd.setCursor(0,0);
      lcd.print("Minimo:");lcd.print(eep.read(12));
      lcd.setCursor(0,1);
      lcd.print("Llenar:");lcd.print(eep.read(13));
      lcd.setCursor(0,3);
      lcd.print("     Confirmar?    ");

      if((PIND & (1<<PD4)) == 0){
        while((PIND & (1<<PD4)) == 0){}
        Flag=6;
        tiempo_menues=mili_segundos;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Flag=4;
        lcd.clear();
      }
      break;

    case 6:
      guardado_para_menus(true);
      break; 
  }
}

void menu_de_calefaccion_auto(){
  switch (Flag)
  {
    case 2:
      Valorinicial=eep.read(10);
      Valorfinal=eep.read(11);
      eep.write(10,min_temp);
      Ypos=0;
      Flag=3;
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("Temp. min:");
      if(use_farenheit == false) {
        lcd.print(eep.read(10)); 
        if (eep.read(10)<100){lcd.print((char)223); lcd.print("C ");}
        else{lcd.print((char)223); lcd.print("C");}
      }
      if(use_farenheit == true) {
        lcd.print(((9*eep.read(10))/5)+32);
        if (eep.read(10)<100){lcd.print((char)223); lcd.print("F ");}
        else{lcd.print((char)223); lcd.print("F");}
      }
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      eep.write(10,map(analogRead(A2), 0, 1023, min_temp, maxi_cast-sumador_temperatura));
      if (eep.read(10)>maxi_cast-sumador_temperatura)eep.write(10,maxi_cast-sumador_temperatura);
      if (eep.read(10)<min_temp)eep.write(10,min_temp);

      if((PIND & (1<<PD2)) == 0 && eep.read(10)<maxi_cast-sumador_temperatura){
        while((PIND & (1<<PD2)) == 0){}
        eep.write(10,eep.read(10)+sumador_temperatura);
      }
      if((PIND & (1<<PD3)) == 0 && min_temp<eep.read(10)){
        while((PIND & (1<<PD3)) == 0){}
        eep.write(10,eep.read(10)-sumador_temperatura);
      }
      if((PIND & (1<<PD4)) == 0){
        while((PIND & (1<<PD4)) == 0){}
        Flag=4;
        eep.write(11,eep.read(10)+sumador_temperatura);
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        eep.write(10,Valorinicial);
        eep.write(11,Valorfinal);
        Estadoequipo=menu1;
        Flag=1;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break; 
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Temp. max:");
      if(use_farenheit == false) {
          lcd.print(eep.read(11)); 
          if (eep.read(11)<100){lcd.print((char)223); lcd.print("C ");}
          else{lcd.print((char)223); lcd.print("C");}
      }
      if(use_farenheit == true) {
          lcd.print(((9*eep.read(11))/5)+32);
          if (eep.read(11)<100){lcd.print((char)223); lcd.print("F ");}
          else{lcd.print((char)223); lcd.print("F");}
      }
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      eep.write(11,map(analogRead(A2), 0, 1023, eep.read(10)+sumador_temperatura, maxi_cast));
      if (eep.read(11)>maxi_cast)eep.write(11,maxi_cast);
      if (eep.read(11)<eep.read(10)+sumador_temperatura)eep.write(11,eep.read(10)+sumador_temperatura);
      if((PIND & (1<<PD2)) == 0 && eep.read(11)<maxi_cast){
        while((PIND & (1<<PD2)) == 0){}
        eep.write(11,eep.read(11)+sumador_temperatura);
      }
      if((PIND & (1<<PD3)) == 0 && eep.read(11)>eep.read(10)+sumador_temperatura){
        while((PIND & (1<<PD3)) == 0){}
        eep.write(11,eep.read(11)+sumador_temperatura);
      }
      if((PIND & (1<<PD4)) == 0){
        while((PIND & (1<<PD4)) == 0){}
        Flag=5;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Flag=3;
        lcd.clear();
      }
      
      break;

      case 5:
        lcd.setCursor(0,0);
        lcd.print("Minima guardada:");
        if(use_farenheit == false){ lcd.print(eep.read(10)); lcd.print((char)223); lcd.print("C");}
        if(use_farenheit == true){ lcd.print(((9*eep.read(10))/5)+32);lcd.print((char)223); lcd.print("F");lcd.print(eep.read(10));}
        lcd.setCursor(0,1);
        lcd.print("Maxima guardada:");
        if(use_farenheit == false){ lcd.print(eep.read(11)); lcd.print((char)223); lcd.print("C");}
        if(use_farenheit == true){ lcd.print(((9*eep.read(11))/5)+32);lcd.print((char)223); lcd.print("F");lcd.print(eep.read(10));}
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");

        if((PIND & (1<<PD4)) == 0){
            while((PIND & (1<<PD4)) == 0){}
            Flag=6;
            tiempo_menues=mili_segundos;
            lcd.clear();
          }
          if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Flag=4;
            lcd.clear();
          }
          break;

      case 6:
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
    tiempo_de_standby = 0;
  }
  
  if (Flag==4){
  Ypos = map(analogRead(A2), 0, 1023, 0, maxY_menu2);
  if ((PIND & (1<<PD2)) == 0 ){ while ((PIND & (1<<PD2)) == 0){} Ypos=ReturnToCero(Ypos-1,maxY_menu2); lcd.clear(); Blink = true;tiempo_de_standby = 0;}
  if ((PIND & (1<<PD3)) == 0 ){ while ((PIND & (1<<PD3)) == 0){} Ypos=ReturnToCero(Ypos+1,maxY_menu2); lcd.clear(); Blink = true;tiempo_de_standby = 0;}
  if ((PIND & (1<<PD4)) == 0 ){ while ((PIND & (1<<PD4)) == 0){} opcionmenu2=ReturnToCero(Ypos,maxY_menu2)+1; lcd.clear(); }
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
        Estadoequipo=funciones;
        funcionActual=funcion_activar_bomba;
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
  if(tiempo_de_standby>=5000){
    Estadoequipo = estado_standby;
    tiempo_de_standby = 0;
    Flag=1;
    lcd.clear();
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

        sumador_hora=map(analogRead(A2), 0, 1023, 0, hora_max-1);
        
        if((PIND & (1<<PD2)) == 0){ 
          while((PIND & (1<<PD2)) == 0){}
          sumador_hora++;
        }
        if((PIND & (1<<PD3)) == 0){
          while((PIND & (1<<PD3)) == 0){}
          sumador_hora--;
        }
        if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Flag=6;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
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

        sumador_hora=map(analogRead(A2), 0, 1023, 0, minuto_max-1);

        if((PIND & (1<<PD2)) == 0){ 
          while((PIND & (1<<PD2)) == 0){}
          sumador_minuto++;
        }
        if((PIND & (1<<PD3)) == 0){
          while((PIND & (1<<PD3)) == 0){}
          sumador_minuto--;
        }
        if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Flag=7;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Flag=5;
          lcd.clear();
        }
        break;

      case 7:
        lcd.setCursor(0,0);
        lcd.print("hora guardada:");lcd.print(String(hora_to_modify) + ":" + String(minuto_to_modify));
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");
        if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Flag=8;
          DateTime now = rtc.now();
          rtc.adjust(DateTime(now.year(),now.month(),now.day(),hora_to_modify,minuto_to_modify,now.second()));
          tiempo_menues=mili_segundos;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Flag=6;
          lcd.clear();
        }
        break;

      case 8:
        guardado_para_menus(false);
        break; 
    }
  }

void menu_activar_bomba(){
switch (Flag)
  {
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Activar bomba");
      lcd.setCursor(5,2);
      lcd.print("Si");
      lcd.setCursor(11,2);
      lcd.print("No");
      lcd.setCursor(5,3);
      lcd.print("1");
      lcd.setCursor(11,3);
      lcd.print("2");
      if((PIND & (1<<PD3)) == 0 ){
          while((PIND & (1<<PD3)) == 0){}
          Activar_bomba = false;
          Flag=5;
        }
      if((PIND & (1<<PD2)) == 0 ){
          while((PIND & (1<<PD2)) == 0){}
          Activar_bomba = true;
          Flag=5;
        }
      if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
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
      if((PIND & (1<<PD3)) == 0 ){
          while((PIND & (1<<PD3)) == 0){}
          use_farenheit = false;
          Flag=5;
        }
      if((PIND & (1<<PD2)) == 0 ){
          while((PIND & (1<<PD2)) == 0){}
          use_farenheit = true;
          Flag=5;
        }
      if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
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
    lcd.print("SSID:");lcd.print(WIFISSID);
    lcd.setCursor(0,1);
    lcd.print("PASS:");lcd.print(WIFIPASS);
    lcd.setCursor(0,2);
    lcd.print("modificar?");
    lcd.setCursor(0,3);
    lcd.print("P3: SI   P4: NO");

    if((PIND & (1<<PD4)) == 0 ){
        while((PIND & (1<<PD4)) == 0){}
        lcd.clear();
        Flag=5;
      }
    if((PIND & (1<<PD5)) == 0 ){
        while((PIND & (1<<PD5)) == 0){}
        Estadoequipo=menu2;
        Flag=3;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
    break;
  case 5:
    for (Auxiliar1=0;Auxiliar1<=19;Auxiliar1++){ 
    WIFISSID[Auxiliar1]='\0';
    WIFIPASS[Auxiliar1]='\0';}
    Ypos=0;
    Actualchar=0;
    Flag=6;
    break;
  case 6:
    lcd.setCursor(0,0);
    lcd.print("Nombre Wifi:");
    lcd.setCursor(0,1);
    for (uint8_t i = 0; i < 20; i++){
    if (WIFISSID[i]=='\0' ){lcd.print(" ");}
    else lcd.print(WIFISSID[i]);
    }
    Actualchar=ReturnToCero(Actualchar, 40);
    Ypos=ReturnToCero(Ypos,20);
    WIFISSID[Ypos]=Character_Return(Actualchar, mayusculas);
 
    Actualchar=map(analogRead(A2), 0, 1023, 0, 39);
    if((PIND & (1<<PD2)) == 0 )
      {
        while((PIND & (1<<PD2)) == 0){}
        mayusculas=!mayusculas;
      }
    if((PIND & (1<<PD3)) == 0 && Ypos <= 19)
      {
        while((PIND & (1<<PD3)) == 0){}
        Ypos++;
        Actualchar=0;
      }
    if((PIND & (1<<PD4)) == 0 )
      {
        while((PIND & (1<<PD4)) == 0){}
        WIFISSID[Ypos]='\0';
        lcd.clear();
        Flag=7;
        Ypos=0;
        Actualchar=0;
      }
    if((PIND & (1<<PD5)) == 0 )
      {
        while((PIND & (1<<PD5)) == 0){}
        WIFISSID[Ypos]='\0';
        lcd.clear();
        Flag=4;
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
    Ypos=ReturnToCero(Ypos,20);
    WIFIPASS[Ypos]=Character_Return(Actualchar, mayusculas);
    Actualchar=map(analogRead(A2), 0, 1023, 0, 39);
    if((PIND & (1<<PD2)) == 0 )
      {
        while((PIND & (1<<PD2)) == 0){}
        mayusculas=!mayusculas;
      }
    if((PIND & (1<<PD3)) == 0 )
      {
        while((PIND & (1<<PD3)) == 0){}
        Ypos ++;
        Actualchar=0;
      }
    if((PIND & (1<<PD4)) == 0 )
      {
        while((PIND & (1<<PD4)) == 0){}
        WIFIPASS[Ypos]='\0';
        lcd.clear();
        tiempo_menues=mili_segundos;
        Flag=8;
        Ypos=0;
      }
    if((PIND & (1<<PD5)) == 0 )
      {
        while((PIND & (1<<PD5)) == 0){}
        WIFISSID[Ypos]='\0';
        lcd.clear();
        Flag=4;
        Ypos=0;
        Actualchar=0;
      }
    break;
      case 8:
      for (uint8_t i = 0; i < 19; i++){
      eep.write(14 + i, WIFIPASS[i]);
      eep.write(34 + i, WIFISSID[i]);
      }
      Serial_Send_UNO(6);
        guardado_para_menus(false);
      break;
  }

}

void guardado_para_menus(bool Menu){
  lcd.setCursor(4,0);
  lcd.print("Guardando...");
  if(mili_segundos>=tiempo_menues+tiempo_de_espera_menu){
    if(Menu == true){
      Estadoequipo=menu1;
      Flag=1;
    }
  if(Menu == false){
    Estadoequipo=menu2; 
    Flag=3; 
  }
  funcionActual=posicion_inicial;
  lcd.clear();
  tiempo_de_standby = 0;
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

