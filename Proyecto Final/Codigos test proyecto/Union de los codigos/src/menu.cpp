#include "menu.h"
#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "FuncionesSoporte.h"

#define maxY_menu1 7
#define maxY_menu2 5
#define tiempo_de_parpadeo 700
#define tiempo_de_espera_menu 3000 

typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 

const char* Menuprincipal[maxY_menu1] = {
  "C manual          ",
  "H manual          ", 
  "H & F in H        ",
  "C segun lleno     ",
  "H segun temp      ",
  "menu avanzado     ",
  "volver            "
};
const char* menuavanzado[maxY_menu2] = {
  "Setear hora       ",
  "c° o F°           ",
  "Activar la bomba  ",
  "conexion wifi     ",
  "volver            "
};

extern char LCDMessage[20];
extern uint32_t  mili_segundos;
extern uint16_t tiempo_de_standby;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern uint8_t hora,minutos;
extern bool use_farenheit;

extern Seleccionar_Funciones funcionActual;
extern estadoMEF Estadoequipo;
extern LiquidCrystal_I2C lcd;

uint16_t tiempo_menues;
int8_t Ypos;
uint8_t Posicion_menu=0; 
bool Blink;  

void standby()
{ 
  if(use_farenheit == false)sprintf(LCDMessage, "T:%d%cC",temperatura_actual,(char)223);
  if(use_farenheit == true) sprintf(LCDMessage, "T:%d%cF",((9*temperatura_actual)/5)+32,(char)223);
  PrintLCD (LCDMessage,0,0);
  sprintf(LCDMessage, "N:%d%c",nivel_actual,'%'); PrintLCD (LCDMessage,12,0);
  Printhora (LCDMessage,hora,minutos);            PrintLCD (LCDMessage,6,1);

  if(PressedButton (254)){
    switch (Estadoequipo)
    {
      case estado_standby:
        Estadoequipo = estado_inicial;
        tiempo_de_standby=0;
        lcd.backlight();
        break;
      case estado_inicial:
        lcd.backlight();
        Estadoequipo = menu1;
        tiempo_de_standby=0;
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
  switch (Posicion_menu){
    case 0:
      tiempo_de_standby = 0;
      tiempo_menues=mili_segundos;
      Blink=false;
      lcd.clear();
      Posicion_menu=1;
      break;
    case 1:
      if(Blink == false){sprintf(LCDMessage, " "); tiempo_menues-=100;} //¿Por que hago tiempo actual -=100? en el lcd se ve mejor cuando el tiempo de "apagado" es menor al de encendio
      if(Blink == true)sprintf(LCDMessage, "%c", char(62));
      PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "%s",Menuprincipal[ReturnToCero(Ypos,maxY_menu1)]);   PrintLCD (LCDMessage,1,0);
      sprintf(LCDMessage, "%s",Menuprincipal[ReturnToCero(Ypos+1,maxY_menu1)]); PrintLCD (LCDMessage,1,1);
      sprintf(LCDMessage, "%s",Menuprincipal[ReturnToCero(Ypos+2,maxY_menu1)]); PrintLCD (LCDMessage,1,2);
      sprintf(LCDMessage, "%s",Menuprincipal[ReturnToCero(Ypos+3,maxY_menu1)]); PrintLCD (LCDMessage,1,3);

      if (PressedButton(1)){Ypos=ReturnToCero(Ypos-1,maxY_menu1);Blink = true; tiempo_de_standby = 0;} // suma 1 a Ypos
      if (PressedButton(2)){Ypos=ReturnToCero(Ypos+1,maxY_menu1);Blink = true; tiempo_de_standby = 0;} // resta 1 a Ypos
      if (PressedButton(3))Posicion_menu=ReturnToCero(Ypos,maxY_menu1)+2; //confirmacion

      if(mili_segundos>=tiempo_menues+tiempo_de_parpadeo){tiempo_menues=mili_segundos;Blink=!Blink;} //prende o apaga la flechita

      if(tiempo_de_standby>=5000){
        Estadoequipo = estado_standby;
        tiempo_de_standby = 0;
        lcd.clear();
      }
      break;
    case 2:
      Estadoequipo=funciones;
      funcionActual=llenado_manual;
      Posicion_menu=0; // la declaracion de flag no es del todo necesaria pero se vuelve a declarar x seguridad
      break;
    case 3:
      Estadoequipo=funciones;
      funcionActual=calefaccion_manual;
      Posicion_menu=0;
      break;
    case 4:
      Estadoequipo=funciones;
      funcionActual=funcion_menu_de_auto_por_hora;
      Posicion_menu=0;
      break;
    case 5:
      Estadoequipo=funciones;
      funcionActual=llenado_auto;
      Posicion_menu=0;
      break;
    case 6:
      Estadoequipo=funciones;
      funcionActual=calefaccion_auto;
      Posicion_menu=0;
      break;
    case 7:
      Posicion_menu=0; // aumenta pq pasa al menu avanzado
      Ypos=0;
      lcd.clear();
      tiempo_de_standby=0;
      Estadoequipo=menu2;
      break;
    case 8:
      tiempo_de_standby=0;
      Posicion_menu=0; // disminuye pq pasa al standby
      lcd.clear();
      Estadoequipo=estado_inicial;
      break;
  }
}

void menu_avanzado()
{
  switch (Posicion_menu)
  { 
    case 0:
      Ypos=0;
      tiempo_de_standby = 0;
      Blink=false;
      lcd.clear();
      Posicion_menu=1;
      break;
    case 1:
      if(Blink == false){sprintf(LCDMessage, " "); tiempo_menues-=100;} //¿Por que hago tiempo actual -=100? en el lcd se ve mejor cuando el tiempo de "apagado" es menor al de encendio
      if(Blink == true)sprintf(LCDMessage,"%c",char(62));
      PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "%s",menuavanzado[ReturnToCero(Ypos,maxY_menu2)]);    PrintLCD (LCDMessage,1,0);
      sprintf(LCDMessage, "%s",menuavanzado[ReturnToCero(Ypos+1,maxY_menu2)]);  PrintLCD (LCDMessage,1,1);
      sprintf(LCDMessage, "%s",menuavanzado[ReturnToCero(Ypos+2,maxY_menu2)]);  PrintLCD (LCDMessage,1,2);
      sprintf(LCDMessage, "%s",menuavanzado[ReturnToCero(Ypos+3,maxY_menu2)]);  PrintLCD (LCDMessage,1,3);

      if (PressedButton(1)){Ypos=ReturnToCero(Ypos-1,maxY_menu2); Blink = true; tiempo_de_standby = 0;}// suma 1 a Ypos
      if (PressedButton(2)){Ypos=ReturnToCero(Ypos+1,maxY_menu2); Blink = true; tiempo_de_standby = 0;}// resta 1 a Ypos
      if (PressedButton(3))Posicion_menu=ReturnToCero(Ypos,maxY_menu2)+2; //confirmacion
      
      if(mili_segundos>=tiempo_menues+tiempo_de_parpadeo){tiempo_menues=mili_segundos;Blink=!Blink;} //prende o apaga la flechita

      if(tiempo_de_standby>=5000){
        Estadoequipo = estado_standby;
        tiempo_de_standby = 0;
        lcd.clear();
      }
      break;
    case 2:
      Estadoequipo=funciones;
      funcionActual=funcion_de_menu_modificar_hora_rtc;
      Posicion_menu=0;
      break;
    case 3: 
      Estadoequipo=funciones;
      funcionActual=funcion_farenheit_celsius;
      Posicion_menu=0;
      break;
    case 4:
      Estadoequipo=funciones;
      funcionActual=funcion_activar_bomba;
      Posicion_menu=0;
      break;
    case 5:
      Estadoequipo=funciones;
      funcionActual=funcion_de_menu_seteo_wifi;
      Posicion_menu=0;
      break;
    case 6:
      tiempo_de_standby=0;
      Posicion_menu=0;
      Ypos=0;
      lcd.clear();
      Estadoequipo=menu1;
      break;
  }
}

void guardado_para_menus(bool Menu){
  memcpy(LCDMessage, "Guardando...", 13);                                   PrintLCD (LCDMessage,4,0);
  if(mili_segundos>=tiempo_menues+tiempo_de_espera_menu){
  if(Menu == true){
      Estadoequipo=menu1;
  }
  if(Menu == false){
    Estadoequipo=menu2;  
  }
  funcionActual=posicion_inicial;
  lcd.clear();
  tiempo_de_standby = 0;
  }
}