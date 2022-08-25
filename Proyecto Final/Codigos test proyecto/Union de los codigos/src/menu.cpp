#include "menu.h"
#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "FuncionesSoporte.h"

#define maxY_menu1 7
#define maxY_menu2 5
#define tiempo_de_parpadeo 700

typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora_display, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_display_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 

extern uint16_t  mili_segundos;
extern Seleccionar_Funciones funcionActual;
extern estadoMEF Estadoequipo;
extern LiquidCrystal_I2C lcd;

const char* Menuprincipal[maxY_menu1] = {
  "C manual",
  "H manual", 
  "H & F in H",
  "C segun lleno",
  "H segun temp",
  "menu avanzado",
  "volver"
};
const char* menuavanzado[maxY_menu2] = {
  "Setear hora_display",
  "c° o F°",
  "Activar la bomba",
  "conexion wifi",
  "volver"
};

char imprimir_lcd[20];
uint16_t tiempo_de_standby;
uint16_t tiempo_parpadeo;
int8_t Ypos;
uint8_t Posicion_menu=0;  

void standby(bool Display_farenheit,uint8_t nivel_display,int8_t temperatura_display, uint8_t hora_display, uint8_t minutos_display, uint8_t posicion_encoder)
{ 
  if(Display_farenheit == false)sprintf(imprimir_lcd, "T:%d%cC",temperatura_display,(char)223);
  if(Display_farenheit == true) sprintf(imprimir_lcd, "T:%d%cF",((9*temperatura_display)/5)+32,(char)223);
  PrintLCD (imprimir_lcd,0,0);
  sprintf(imprimir_lcd, "N:%d%c",nivel_display,'%'); PrintLCD (imprimir_lcd,12,0);
  Printhora(imprimir_lcd,hora_display,minutos_display);            PrintLCD (imprimir_lcd,6,1);
  
  if(PressedButton (1) || Ypos!=posicion_encoder){
    switch (Estadoequipo)
    {
      case estado_standby:
        Estadoequipo = estado_inicial;
        tiempo_de_standby=mili_segundos;
        lcd.backlight();
        break;
      case estado_inicial:
        lcd.backlight();
        Estadoequipo = menu1;
        tiempo_de_standby=mili_segundos;
        lcd.clear();
        break;
      default:
        Estadoequipo = estado_standby;
        break;
    }   
    Ypos=posicion_encoder;
  }
  if(mili_segundos>=tiempo_de_standby+tiempo_de_espera_menu){
    Estadoequipo = estado_standby;
    lcd.noBacklight();
    tiempo_de_standby=mili_segundos;
  }
}

void menu_basico(uint8_t posicion_encoder)
{
  switch (Posicion_menu){
    case 0:
      tiempo_de_standby=mili_segundos;
      tiempo_parpadeo=mili_segundos;
      lcd.clear();
      Posicion_menu=1;
      posicion_encoder=0;
      break;
    case 1: //¿Por que hago tiempo actual -=100? en el lcd se ve mejor cuando el tiempo de "apagado" es menor al de encendio
      sprintf(imprimir_lcd, "%c", char(62));PrintLCD (imprimir_lcd,0,0);
      sprintf(imprimir_lcd, "%s",Menuprincipal[ReturnToCero(Ypos,maxY_menu1)]);   PrintLCD (imprimir_lcd,1,0);
      sprintf(imprimir_lcd, "%s",Menuprincipal[ReturnToCero(Ypos+1,maxY_menu1)]); PrintLCD (imprimir_lcd,1,1);
      sprintf(imprimir_lcd, "%s",Menuprincipal[ReturnToCero(Ypos+2,maxY_menu1)]); PrintLCD (imprimir_lcd,1,2);
      sprintf(imprimir_lcd, "%s",Menuprincipal[ReturnToCero(Ypos+3,maxY_menu1)]); PrintLCD (imprimir_lcd,1,3);

      posicion_encoder=ReturnToCero(posicion_encoder,maxY_menu1*2);

      if(posicion_encoder/2!=Ypos){
        tiempo_de_standby=mili_segundos;
        lcd.clear();
        Ypos=posicion_encoder/2;
      }

      if (PressedButton(1))posicion_encoder-=2; // suma 1 a Ypos
      if (PressedButton(2))posicion_encoder+=2; // resta 1 a Ypos
      if (PressedButton(3)){Posicion_menu=Ypos+2;lcd.clear();} //confirmacion

      if(mili_segundos>=tiempo_parpadeo+tiempo_de_parpadeo){
        tiempo_parpadeo=mili_segundos;
        sprintf(imprimir_lcd, " ");
        PrintLCD (imprimir_lcd,0,0);
      } //prende o apaga la flechita

      if(mili_segundos>=tiempo_de_standby+tiempo_de_espera_menu){
        Estadoequipo = estado_standby;
        tiempo_de_standby=mili_segundos;
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
      funcionActual=funcion_menu_de_auto_por_hora_display;
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

void menu_avanzado(uint8_t posicion_encoder)
{
  switch (Posicion_menu)
  { 
    case 0:
      Ypos=0;
      tiempo_de_standby=mili_segundos;
      lcd.clear();
      Posicion_menu=1;
      posicion_encoder=0;
      break;
    case 1:
      sprintf(imprimir_lcd,"%c",char(62));                                        PrintLCD (imprimir_lcd,0,0);
      sprintf(imprimir_lcd, "%s",menuavanzado[ReturnToCero(Ypos,maxY_menu2)]);    PrintLCD (imprimir_lcd,1,0);
      sprintf(imprimir_lcd, "%s",menuavanzado[ReturnToCero(Ypos+1,maxY_menu2)]);  PrintLCD (imprimir_lcd,1,1);
      sprintf(imprimir_lcd, "%s",menuavanzado[ReturnToCero(Ypos+2,maxY_menu2)]);  PrintLCD (imprimir_lcd,1,2);
      sprintf(imprimir_lcd, "%s",menuavanzado[ReturnToCero(Ypos+3,maxY_menu2)]);  PrintLCD (imprimir_lcd,1,3);

      posicion_encoder=ReturnToCero(posicion_encoder,maxY_menu2*2);

      if(posicion_encoder/2!=Ypos){
        tiempo_de_standby=mili_segundos;
        lcd.clear();
        Ypos=posicion_encoder/2;
      }
      
      if (PressedButton(1))posicion_encoder-=2; // suma 1 a Ypos
      if (PressedButton(2))posicion_encoder+=2; // resta 1 a Ypos
      if (PressedButton(3)){Posicion_menu=Ypos+2;lcd.clear();}//confirmacion
      
      if(mili_segundos>=tiempo_parpadeo+tiempo_de_parpadeo){
        tiempo_parpadeo=mili_segundos;
        sprintf(imprimir_lcd, " ");
        PrintLCD (imprimir_lcd,0,0);
        } //prende o apaga la flechita

      if(mili_segundos>=tiempo_de_standby+tiempo_de_espera_menu){
        Estadoequipo = estado_standby;
        tiempo_de_standby=mili_segundos;
        lcd.clear();
      }
      break;
    case 2:
      Estadoequipo=funciones;
      funcionActual=funcion_de_menu_modificar_hora_display_rtc;
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