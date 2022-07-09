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

//sensor de temperatura
OneWire sensor_temp(9);
#define TEMP_RESOLUTION 12
DallasTemperature Sensor_temp(&sensor_temp); 
void tomar_temperatura ();
void control_de_temp_auto();
int control_de_temp_por_menu_manual(int temp_a_alcanzar);
void menu_de_calefaccion_manual();
uint16_t tiempo_para_temperatura = 5000; // 2 bytes mas 60k si necesitan mas cambien a 36 (4 bytes), le puse unsigned si necesitan negativos saquen la u
uint8_t temperatura_inicial = 40; // byte 0-255 ¿Para que chota usamos int si no necesitamos 60k opciones? solo 0-100 
uint8_t temperatura_final = 40;
uint8_t min_temp_ini = 40;
int16_t temperatura_del_sensor;
uint8_t maxima_temp_fin = 80;
uint16_t milis_para_temperatura = 0;
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
uint64_t  mili_segundos = 0;// ayy milii BOCHA DE SEGUNDOS LOL 
uint8_t sumador_nivel = 25;
uint16_t  milis_para_nivel = 0;
uint16_t tiempo_para_nivel = 3000;
bool confirmar_nivel = false;
//cosas guardado y rtc
AT24C32 eep;
String desconvercionhora(uint8_t,uint8_t,uint8_t,uint8_t);
uint8_t convercionhora(uint8_t, String);
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[5]; 
uint8_t ActualStruct=0;
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
// Comunicacion esp/arduino
void Serial_Read_UNO();
void Serial_Send_UNO(uint8_t);
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
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_del_sensor = Sensor_temp.getTempCByIndex(0);
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
  InitComunication=true;
}

void loop() 
{
  tomar_temperatura();
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
      //menu_de_llenado_manual();
      break;
    case 2:
      menu_de_calefaccion_manual();
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
            save[ActualStruct].hour=Individualdata[0].toInt();
            save[ActualStruct].temp=Individualdata[1].toInt();
            save[ActualStruct].level=Individualdata[2].toInt();
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
        save[3].hour=255;
        save[4].temp=Individualdata[1].toInt();// tempmin
        save[4].level=Individualdata[2].toInt();// tempmax
        ActualIndividualDataPos=0;
        StringTaked=false;
        ConvertString=false;
        ComunicationError=false;
      }
    break;
      case 'V':
    if (ConvertString==true)
      {
        save[4].hour=255;
        save[4].temp=Individualdata[1].toInt();// lvlmin
        save[4].level=Individualdata[2].toInt();// lvlmax
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

ISR(TIMER2_OVF_vect){
  mili_segundos++;
  milis_para_nivel++;
  tiempo_de_standby++;
  milis_para_temperatura++;
  Send_time++;
  //if(encendido_de_temp_auto == true) 
}


/*======================PARA ADAPTAR=========================*/
void tomar_temperatura () //Sexo y adaptarlo para no usar delay
{
  if (milis_para_temperatura >= tiempo_para_temperatura)
  {
    Sensor_temp.requestTemperatures();
    temperatura_del_sensor = Sensor_temp.getTempCByIndex(0);
    milis_para_temperatura = 0;
  }
}
void sensar_nivel_actual(){
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
}

void nivel_auto (){//modificar
    if (nivel_actual <= nivel_seteado)    digitalWrite(electrovalvula, HIGH);
    if (nivel_actual == tanque_al_100)    digitalWrite(electrovalvula, LOW);
}

void control_de_temp_auto(){

}

void menu_de_calefaccion_manual(){
const uint8_t umbral_de_temperatura = 5;// alta palabra añembui umbral
const uint8_t sumador_temperatura = 5;
const uint8_t tiempo_de_espera = 3000;
uint16_t tiempo_actual;
bool confirmar;
  switch (Flag)
  {
  case 3:
    Flag=4;
    confirmar = false;
    break;
  
  case 4:
    lcd.setCursor(0,0);
    lcd.print("Temperatura min:");
    lcd.print(temperatura_inicial);
    lcd.setCursor(0,1);
    lcd.print("Temperatura max:");
    lcd.print(temperatura_final);
    
    lcd.setCursor(0,2);
    lcd.print("Sumar 5 con: 1 y 3");
    lcd.setCursor(0,3);
    lcd.print("Restar 5 con: 2 y 4");

    if (temperatura_final > maxima_temp_fin) temperatura_final = maxima_temp_fin;
    if (temperatura_inicial > maxima_temp_fin) temperatura_inicial = maxima_temp_fin;
    if (temperatura_inicial < min_temp_ini) temperatura_inicial = min_temp_ini;
    if (temperatura_final < temperatura_inicial) temperatura_final = temperatura_inicial;

    if(digitalRead(pulsador1) == LOW)
      {
        while(digitalRead(pulsador1) == LOW){}
        temperatura_inicial += sumador_temperatura;
        mili_segundos = 0;
        lcd.setCursor(17,0); lcd.print("    ");
      }
    
    if(digitalRead(pulsador2) == LOW)
      {
        while(digitalRead(pulsador2) == LOW){}
        temperatura_inicial -= sumador_temperatura;
        mili_segundos = 0;
        lcd.setCursor(17,0); lcd.print("    ");
      }

    if(digitalRead(pulsador3) == LOW)
      {
        while(digitalRead(pulsador3) == LOW){}
        temperatura_final += sumador_temperatura;
        mili_segundos = 0;
        lcd.setCursor(17,1); lcd.print("    ");
      }

    if(digitalRead(pulsador4) == LOW)
      {
        while(digitalRead(pulsador4) == LOW){}
        temperatura_final -= sumador_temperatura;
        mili_segundos = 0;
        lcd.setCursor(17,1); lcd.print("    ");
      }  

    if(digitalRead(pulsador5) == LOW){
        while(digitalRead(pulsador5) == LOW){}
        Flag=5;
      }
    break;
    
    case 5:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Minima guardada:");
      lcd.print(temperatura_inicial);
      lcd.setCursor(0,1);
      lcd.print("Maxima guardada:");
      lcd.print(temperatura_final);
      lcd.setCursor(0,3);
      lcd.print("     Confirmar?    ");
      if(digitalRead(pulsador5) == LOW){
        while(digitalRead(pulsador5) == LOW){}
        Flag=6;
        eep.write(10,temperatura_inicial);
        eep.write(11,temperatura_final);
        tiempo_actual=mili_segundos;
      }
      break;

    case 6:
      lcd.clear();
      if(mili_segundos>=tiempo_actual+tiempo_de_espera)Flag=0;
      break;
  }



    control_de_temp_auto();

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


