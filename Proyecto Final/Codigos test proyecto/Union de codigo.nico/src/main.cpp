//optimizar
#include <Arduino.h>
#include "AT24CX.h"
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


AT24C32 eep;
RTC_DS1307 rtc;


void menu_basico();
void menu_avanzado();
void standby();
void imprimir_en_pantalla();
void limpiar_pantalla_y_escribir_nivel();
void tomar_temperatura ();
void control_de_temp_auto();
void sensar_nivel_de_agua();
void nivel_auto(bool);
void Serial_Read_UNO();
void Serial_Send_UNO(uint8_t);
void menu_de_calefaccion_auto();
void menu_de_calefaccion_manual();
void menu_de_llenado_auto();
void Actualizar_hora ();
void modificar_hora_rtc();
uint8_t menuposY(uint8_t , uint8_t);
int control_de_temp_por_menu_manual(int temp_a_alcanzar);
String desconvercionhora(uint8_t,uint8_t,uint8_t,uint8_t);
uint8_t convercionhora(uint8_t, String);
void sensar_nivel_actual();

uint16_t tiempo_para_temperatura = 5000; // 2 bytes mas 60k si necesitan mas cambien a 36 (4 bytes), le puse unsigned si necesitan negativos saquen la u
uint8_t temperatura_inicial = 40; // byte 0-255 ¿Para que chota usamos int si no necesitamos 60k opciones? solo 0-100 
uint8_t temperatura_final = 40;
int16_t temperatura_del_sensor=0;
uint16_t milis_para_temperatura = 0;
//nivel de agua
uint8_t nivel_inicial;
uint8_t nivel_final; 
uint8_t nivel_actual;
const uint8_t nivel_del_tanque = A0; 
const uint8_t electrovalvula = 10;
const uint8_t resistencia = 11;
uint8_t  nivel = 0;
uint64_t  mili_segundos = 0;// ayy milii BOCHA DE SEGUNDOS LOL 
uint16_t  milis_para_nivel = 0;
uint16_t tiempo_para_nivel = 3000;
//cosas guardado y rtc
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 
uint8_t ActualStruct=0;
//cosas del menu princial
uint8_t funcionActual=0;
uint16_t tiempo_de_standby = 0;
uint8_t opcionmenu1=0;
uint8_t opcionmenu2=0;
uint8_t Flag=0;
// Comunicacion esp/arduino
String Serial_Input;
String Individualdata[4];
String IndividualValue;
uint16_t Send_time=0;
uint8_t StringLength=0;
uint8_t ActualIndividualDataPos=0;
char Actualchar=0;
char input=0;
bool ConvertString=false;
bool StringTaked=false;
bool stateheating=false;
bool ComunicationError=false;
bool InitComunication;
//cosas del mennu avanzado
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

LiquidCrystal_I2C lcd(0x20,20,4);//LiquidCrystal_I2C lcd(0x27,20,4);
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF;  
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
  Wire.begin();  //inicializacion del i2C
  Serial.begin(9600); //inicializacion del serial arduino-esp
  rtc.begin();//inicializacion del rtc arduino-esp
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); //subirlo solo una unica vez y despues subirlo nuevamente pero comentando (sino cuando reinicia borra config hora)
  lcd.init();//Iniciacion del LCD
  lcd.backlight();
  pinMode(nivel_del_tanque, INPUT); //pines  nivel
  pinMode(electrovalvula, OUTPUT);
  pinMode(resistencia, OUTPUT);
  //Sensr de temperatura
  /*Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_del_sensor = Sensor_temp.getTempCByIndex(0);
  */
  //pulsadores pra manejar los menus//
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  pinMode(pulsador7, INPUT_PULLUP);
  //
  InitComunication=true;
}

void loop() 
{
  tomar_temperatura();
  Actualizar_hora ();
  sensar_nivel_actual();
  if (Serial.available()>0)Serial_Read_UNO();
  /*control_de_temp_auto();
  sensar_nivel_actual();*/
  /*if (milis_para_nivel == tiempo_para_nivel)//sujeto a cambios
  {
    control_de_temp_auto();
    milis_para_nivel= 0;
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
    case funciones:

      if(funcionActual==2)menu_de_calefaccion_manual();
      if(funcionActual==4)menu_de_llenado_auto();
      if(funcionActual==5)menu_de_calefaccion_auto();
      if(funcionActual==6)modificar_hora_rtc();
      break;
  }
 
}

void standby()
{ 
  lcd.setCursor(0,0);
  lcd.print("Nivel: ");
  lcd.setCursor(8,0);
  lcd.print(nivel_actual);
  lcd.setCursor(0,1);
  lcd.print("Temperatura:");
  tomar_temperatura();
  lcd.setCursor(13,1);
  lcd.print(temperatura_del_sensor);
  lcd.print((char)223);
  lcd.print("C");
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
      break;
    case 2:
      Estadoequipo=funciones;
      funcionActual=2;
      break;
    case 3:
      // cal y carg segun hora
      break;
    case 4:
      Estadoequipo=funciones;
      funcionActual=4;
      break;
    case 5:
   Estadoequipo=funciones;
      funcionActual=5;
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
      Estadoequipo=funciones;
      funcionActual=6;
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

void menu_de_calefaccion_manual()
{
  const uint8_t sumador_temperatura = 5;
  const uint16_t tiempo_de_espera = 5000;
  const uint8_t min_temp = 40;
  const uint8_t maxima_temp = 80;
  uint8_t temperatura_a_calentar;
  uint64_t tiempo_actual;

    
    switch (Flag)
    {
      case 2:
        if (temperatura_del_sensor < min_temp)temperatura_a_calentar=min_temp;
        else temperatura_a_calentar=temperatura_del_sensor;
        Flag=3;
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("Temp. a calentar:");
        lcd.print(temperatura_a_calentar);
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        if(digitalRead(pulsador1) == LOW && temperatura_a_calentar<maxima_temp)
          {
            while(digitalRead(pulsador1) == LOW){}
            temperatura_a_calentar += sumador_temperatura;
            lcd.setCursor(17,0); lcd.print("    ");
          }
        
        if(digitalRead(pulsador2) == LOW && temperatura_a_calentar>min_temp || temperatura_a_calentar>temperatura_del_sensor)
          {
            while(digitalRead(pulsador2) == LOW){}
            temperatura_a_calentar -= sumador_temperatura;
            lcd.setCursor(17,0); lcd.print("    ");
          }

        if(digitalRead(pulsador3) == LOW)
          {
            while(digitalRead(pulsador3) == LOW){}
            Flag=4;
            lcd.setCursor(17,0); lcd.print("    ");
          }
        break; 

        case 4:
          lcd.setCursor(0,0);
          lcd.print("Calentar hasta:");
          lcd.print(temperatura_a_calentar);
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");
          if(digitalRead(pulsador3) == LOW)
            {
              while(digitalRead(pulsador3) == LOW){}
              Flag=5;
              tiempo_actual=mili_segundos;
              lcd.clear();
            }
          break;

        case 5:
          lcd.setCursor(0,0);
          lcd.print("Guardando...");
          if(mili_segundos>=tiempo_actual+tiempo_de_espera){Estadoequipo=estado_inicial;Flag=0;funcionActual=0;}
          break;
          
    }
}

void menu_de_calefaccion_auto()
  {
  const uint8_t sumador_temperatura = 5;
  const uint16_t tiempo_de_espera = 5000;
  const uint8_t min_temp = 40;
  const uint8_t maxima_temp = 80;
  uint64_t tiempo_actual;

    
    switch (Flag)
    {
      case 2:
        temperatura_inicial=60;
        Flag=3;
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("Temperatura min:");
        lcd.print(temperatura_inicial);
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        if(digitalRead(pulsador1) == LOW && temperatura_inicial<maxima_temp-sumador_temperatura)
          {
            while(digitalRead(pulsador1) == LOW){}
            temperatura_inicial += sumador_temperatura;
            mili_segundos = 0;
            lcd.setCursor(17,0); lcd.print("    ");
          }
        
        if(digitalRead(pulsador2) == LOW && min_temp<temperatura_inicial)
          {
            while(digitalRead(pulsador2) == LOW){}
            temperatura_inicial -= sumador_temperatura;
            mili_segundos = 0;
            lcd.setCursor(17,0); lcd.print("    ");
          }

        if(digitalRead(pulsador3) == LOW)
          {
            while(digitalRead(pulsador3) == LOW){}
            Flag=4;
            lcd.setCursor(17,0); lcd.print("    ");
            temperatura_final=temperatura_inicial+sumador_temperatura;
          }
        break; 
      case 4:
        lcd.setCursor(0,0);
        lcd.print("Temperatura max:");
        lcd.print(temperatura_final);
        lcd.setCursor(0,1);
        lcd.print("Sumar 5 con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("Restar 5 con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        if(digitalRead(pulsador1) == LOW && temperatura_final < maxima_temp)
          {
            while(digitalRead(pulsador1) == LOW){}
            temperatura_final += sumador_temperatura;
            mili_segundos = 0;
            lcd.setCursor(17,0); lcd.print("    ");
          }
        
        if(digitalRead(pulsador2) == LOW && temperatura_final>temperatura_inicial+sumador_temperatura)
          {
            while(digitalRead(pulsador2) == LOW){}
            temperatura_final -= sumador_temperatura;
            mili_segundos = 0;
            lcd.setCursor(17,0); lcd.print("    ");
          }

        if(digitalRead(pulsador3) == LOW){
            while(digitalRead(pulsador3) == LOW){}
            Flag=5;
            lcd.clear();
          }
          break;

        case 5:
          lcd.setCursor(0,0);
          lcd.print("Minima guardada:");
          lcd.print(temperatura_inicial);
          lcd.setCursor(0,1);
          lcd.print("Maxima guardada:");
          lcd.print(temperatura_final);
          lcd.setCursor(0,3);
          lcd.print("     Confirmar?    ");
          if(digitalRead(pulsador3) == LOW)
            {
              while(digitalRead(pulsador3) == LOW){}
              Flag=6;
              eep.write(10,temperatura_inicial);
              eep.write(11,temperatura_final);
              tiempo_actual=mili_segundos;
              lcd.clear();
            }
          break;

        case 6:
          lcd.setCursor(0,0);
          lcd.print("Guardando...");
          if(mili_segundos>=tiempo_actual+tiempo_de_espera){Estadoequipo=estado_inicial;Flag=0;funcionActual=0;}
          break;
          
    }
  }

void menu_de_llenado_auto(){
const uint8_t sumador_nivel = 5;
const uint16_t tiempo_de_espera = 5000;
const uint8_t min_nivel = 20;
const uint8_t maxima_nivel = 100;
uint64_t tiempo_actual;

  
  switch (Flag)
  {
  case 2:
    nivel_inicial=60;
    Flag=3;
    break;
  case 3:
    lcd.setCursor(0,0);
    lcd.print("Nivel min:");
    lcd.print(nivel_inicial);
    lcd.setCursor(0,1);
    lcd.print("Sumar 5 con: 1 ");
    lcd.setCursor(0,2);
    lcd.print("Restar 5 con: 2");
    lcd.setCursor(0,3);
    lcd.print("Confirmar con 3");

    if(digitalRead(pulsador1) == LOW && nivel_inicial<maxima_nivel-sumador_nivel)
      {
        while(digitalRead(pulsador1) == LOW){}
        nivel_inicial += sumador_nivel;
        mili_segundos = 0;
        lcd.setCursor(11,0); lcd.print("       ");
      }
    
    if(digitalRead(pulsador2) == LOW && min_nivel<nivel_inicial)
      {
        while(digitalRead(pulsador2) == LOW){}
        nivel_inicial -= sumador_nivel;
        mili_segundos = 0;
        lcd.setCursor(11,0); lcd.print("       ");
      }

    if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=4;
        nivel_final=nivel_inicial+sumador_nivel;
        lcd.clear();
      }
    break;
    
    case 4:
          lcd.setCursor(0,0);
    lcd.print("Temperatura max:");
    lcd.print(nivel_final);
    lcd.setCursor(0,1);
    lcd.print("Sumar 5 con: 1 ");
    lcd.setCursor(0,2);
    lcd.print("Restar 5 con: 2");
    lcd.setCursor(0,3);
    lcd.print("Confirmar con 3");


    if(digitalRead(pulsador1) == LOW && nivel_final < maxima_nivel)
      {
        while(digitalRead(pulsador1) == LOW){}
        nivel_final += sumador_nivel;
        mili_segundos = 0;
        lcd.setCursor(11,0); lcd.print("       ");
      }
    
    if(digitalRead(pulsador2) == LOW && nivel_final>nivel_inicial+sumador_nivel)
      {
        while(digitalRead(pulsador2) == LOW){}
        nivel_final -= sumador_nivel;
        mili_segundos = 0;
        lcd.setCursor(11,0); lcd.print("       ");
      }

    if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=5;
        lcd.clear();
      }
      break;
    case 5:
  
      lcd.setCursor(0,0);
      lcd.print("Minima guardada:");
      lcd.print(nivel_inicial);
      lcd.setCursor(0,1);
      lcd.print("Maxima guardada:");
      lcd.print(nivel_final);
      lcd.setCursor(0,3);
      lcd.print("     Confirmar?    ");
      if(digitalRead(pulsador3) == LOW){
        while(digitalRead(pulsador3) == LOW){}
        Flag=6;
        eep.write(12,nivel_inicial);
        eep.write(13,nivel_final);
        tiempo_actual=mili_segundos;
        lcd.clear();
      }
      break;

    case 6:
      lcd.setCursor(0,0);
      lcd.print("Guardando...");
      if(mili_segundos>=tiempo_actual+tiempo_de_espera){Estadoequipo=estado_inicial;Flag=0;funcionActual=0;lcd.clear();}
      break;
      
  }
}

//==============================FUNCIONES ESCONDIDAS===================
void Actualizar_hora ()
  {
    DateTime now = rtc.now(); //iguala la variable datetime al valor del rtc
    anio=now.year(),
    mes=now.month();
    dia=now.day();
    hora=now.hour();
    minutos=now.minute();
    segundos=now.second();
  }

void Serial_Read_UNO(){
  
  Serial_Input=Serial.readString();// iguala el string del serial a un string imput
  StringLength= Serial_Input.length();// saca el largo del string
  Serial.println(Serial_Input);// Solo de verificacion (eliminar en el final)
  input=Serial_Input.charAt(0); // toma el char de comando (el primer char usualmente una letra)

  for (uint8_t CharPos = 2; CharPos <= StringLength; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
      if(Serial_Input.charAt(CharPos)==':')ActualIndividualDataPos++; //si hay : divide los datos
      if(Serial_Input.charAt(CharPos)!=':' && Serial_Input.charAt(CharPos)!='_' && Serial_Input.charAt(CharPos)!='/')// si no es nungun caracter especial:
        {
          if(Serial_Input.charAt(CharPos-1)==':' || Serial_Input.charAt(CharPos-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(CharPos);//si es el primer digito lo iguala
          else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(CharPos);//si es el segundo en adelante lo suma
        }
      if(CharPos==StringLength)ConvertString=true;// activa el comando final (flag)
  } 
  
  switch (input)//dependiendo del char de comando
  {
  case 'H':
    if (ConvertString==true)
      {
        //temp_a_calentar=Individualdata[0].toInt();
        if(Individualdata[1].toInt()>=1)Serial.println("on");//save ycalentado manual();
        else Serial.println("off");//only save
      }
    break;

  case 'C':
    if (ConvertString==true)
      {
        //nivel_a_llenar=Individualdata[0].toInt();
        if(Individualdata[1].toInt()>=1)Serial.println("on");//save ycalentado manual();
        else Serial.println("off");//only save
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;

  case 'K':
    if (ConvertString==true)
      {
        ActualStruct=Individualdata[3].toInt();
        if (ActualStruct<=2 && ActualStruct>=0)
          {
            input=ActualStruct*3;
            eep.write(input+1, Individualdata[0].toInt());
            eep.write(input+2, Individualdata[1].toInt());
            eep.write(input+3, Individualdata[2].toInt());
            ActualIndividualDataPos=0;
            StringTaked=false;
            ConvertString=false;
            ComunicationError=false;
          }
        else Serial.println("error");
      }
    break;

  case 'J':
    if (ConvertString==true)
      {
        eep.write(10, Individualdata[0].toInt());
        eep.write(11, Individualdata[1].toInt());// tempmax
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;
      case 'V':
    if (ConvertString==true)
      {
        eep.write(12, Individualdata[0].toInt());// lvlmin
        eep.write(13, Individualdata[1].toInt());// lvlmax
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;
    case 'E':
    if (ConvertString==true)
      {
        if(Individualdata[0]=="ERROR")
        {
              Serial.println("?_RESET");
              ComunicationError=true;
        }
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
      }
    break;
    case 'O':
    if (ConvertString==true)
      {
        if(Individualdata[0]=="OK")
        {
              ComunicationError=false;
        }
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
      }
    break;
  default:
    Serial.println("?_NOTHING TO READ");
    break;
  }
}

void Serial_Send_UNO(uint8_t WhatSend)
  {
    uint8_t MessagePoss;
    if (InitComunication==true)MessagePoss=0;
    switch (WhatSend){
      case 1:
        if (ComunicationError==false && InitComunication==true && MessagePoss<=5 &&Send_time>=1000)
        {
            switch (MessagePoss)
            {
              case 0:
                Serial.println("S_""SSID"":""Pass");
                 MessagePoss++;
                 Send_time=0;
                break;
              case 1:
                Serial.println("K_HORA:TEMP:LVL:0");
                MessagePoss++;
                Send_time=0;
                break;
              case 2:
                Serial.println("K_HORA:TEMP:LVL:1");
                MessagePoss++;
                Send_time=0;
                break;
              case 3:
                Serial.println("K_HORA:TEMP:LVL:2");
                MessagePoss++;
                Send_time=0;
                break;
              case 4:
                Serial.println("J_255:TEMPMIN:TEMPMAX:3");
                MessagePoss++;
                Send_time=0;
                break;
              case 5:
                Serial.println("V_255:TEMPMIN:TEMPMAX:3");
                InitComunication=false;
                MessagePoss=0;
                Send_time=0;
                break;
            // delay de 1 seg
            }

            //Send_time =0;
        }
        break;
      case 2:
        if (ComunicationError==false && InitComunication==false && Send_time>=2000 )
          {
              Serial.println("U_TEMP:LVL:HORA:STATE");
              Send_time=0;
          }
      case 3:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("K_HORA:TEMP:LVL:STRUCTPOS");
          }
      case 4:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("J_255:TEMPMIN:TEMPMAX:3");
          }
      case 5:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("V_255:LVLMIN:LVLMAX:4");
          }
    }
    
  }


//===============================FUNCIONES DE SOPORTE===================

ISR(TIMER2_OVF_vect){
  mili_segundos++;
  milis_para_nivel++;
  tiempo_de_standby++;
  milis_para_temperatura++;
  Send_time++;
}

uint8_t menuposY (uint8_t actualpos, uint8_t maxpos){
uint8_t realvalue;

if (actualpos>=maxpos){
  realvalue = actualpos-maxpos;
  return realvalue;
}
else{
  if (actualpos < 0) return maxpos;
  else return actualpos;
}
}

String desconvercionhora(uint8_t function,uint8_t savehora,uint8_t temp, uint8_t lvl)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t var2_deconvert=0;
  uint8_t resto_deconvert=0;
  String returned;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯
  switch (function)
    {
      case 1:
        resto_deconvert= (savehora) % 4;
        var1_deconvert= (savehora-resto_deconvert)/4;
        var2_deconvert=resto_deconvert*15;
        returned= String(var1_deconvert)+':'+String(var2_deconvert);
        return returned;
      break;
      case 2:
        returned= String(temp);
        return returned;
      break;
      case 3:
        returned= String(lvl);
        return returned;
      break;
      default:
        break;
    }
}

uint8_t convercionhora(uint8_t function, String toconvert)
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


/*=======================POR PROBAR=============================*/

void modificar_hora_rtc()
  {
  uint8_t sumador_hora;
  uint8_t sumador_minuto;
  const uint16_t tiempo_de_espera = 5000;
  const uint8_t hora_max = 23;
  const uint8_t min_max = 45;
  uint64_t tiempo_actual;
  uint8_t hora_to_modify;
  uint8_t minuto_to_modify;

    switch (Flag)
    {
      case 4:
        sumador_hora=0;
        sumador_minuto=0;
        Flag=5;
        break;
      case 5:
        lcd.setCursor(0,0);
        lcd.print("Hour:");
        lcd.print(String(hora_to_modify) + ":" + String(minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("disminuir con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        hora_to_modify= menuposY(hora+sumador_hora,hora_max);

        if(digitalRead(pulsador1) == LOW){ while(digitalRead(pulsador1) == LOW){}sumador_hora++;}
        if(digitalRead(pulsador2) == LOW){while(digitalRead(pulsador2) == LOW){}sumador_hora--;}
        if(digitalRead(pulsador3) == LOW){while(digitalRead(pulsador3) == LOW){}Flag=6;}

        break;
      
      case 6:
        lcd.setCursor(0,0);
        lcd.print("Hour:");
        lcd.print(String(hora_to_modify) + ":" + String(minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("aumentar minuto con: 1 ");
        lcd.setCursor(0,2);
        lcd.print("disminuir minuto con: 2");
        lcd.setCursor(0,3);
        lcd.print("Confirmar con 3");

        minuto_to_modify = menuposY(minutos+sumador_minuto,min_max);

        if(digitalRead(pulsador1) == LOW){ while(digitalRead(pulsador1) == LOW){}sumador_minuto+=15;}
        if(digitalRead(pulsador2) == LOW){while(digitalRead(pulsador2) == LOW){}sumador_minuto-=15;}
        if(digitalRead(pulsador3) == LOW){while(digitalRead(pulsador3) == LOW){}Flag=7;}
        break;
      case 7:
        lcd.setCursor(0,0);
        lcd.print("hora guardada:");
        lcd.setCursor(0,1);
        lcd.print(String(hora_to_modify) + ":" + String(minuto_to_modify));
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");
        if(digitalRead(pulsador3) == LOW){
          while(digitalRead(pulsador3) == LOW){}
          Flag=8;
          rtc.adjust(DateTime(anio,mes,dia,hora_to_modify,minuto_to_modify,segundos));
          tiempo_actual=mili_segundos;
          lcd.clear();
        }
        break;

      case 8:
        lcd.setCursor(0,0);
        lcd.print("Guardando...");
        if(mili_segundos>=tiempo_actual+tiempo_de_espera){Estadoequipo=estado_inicial;Flag=0;funcionActual=0;}
        break;
  
    }
  }

/*======================PARA ADAPTAR=========================*/


void tomar_temperatura () //Sexo y adaptarlo para no usar delay
{
  /*if (milis_para_temperatura >= tiempo_para_temperatura)
  {
    Sensor_temp.requestTemperatures();
    temperatura_del_sensor = Sensor_temp.getTempCByIndex(0);
    milis_para_temperatura = 0;
  }*/
}

void sensar_nivel_actual(){
   /* if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;*/
}
void nivel_auto (){//modificar
    if (nivel_actual <= nivel_inicial)    digitalWrite(electrovalvula, HIGH);
    if (nivel_actual == nivel_final)    digitalWrite(electrovalvula, LOW);
}

void control_de_temp_auto(){

}

/*=====================================LABURANDOLO==============================================*/
void configuracionwifi(){  
  
}

void login()
{
  uint8_t pulsaciones1 = 0;
  bool mayus = false;
  if(digitalRead(pulsador1) == LOW) pulsaciones1++;
  if(digitalRead(pulsador2) == LOW) pulsaciones1--;
  if(digitalRead(pulsador3) == LOW) mayus = !mayus;
  Letra(pulsaciones1, mayus);
  lcd.setCursor(0,0);
  lcd.print("Network SSID:");
  lcd.setCursor(0,1);
  
}

char Letra(uint8_t letranum, bool mayus)
{
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


