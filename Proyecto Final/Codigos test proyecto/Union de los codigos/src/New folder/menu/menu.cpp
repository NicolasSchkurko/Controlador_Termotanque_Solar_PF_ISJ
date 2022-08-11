#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "FuncionesSoporte.h"

#define maxY_menu1 7
#define maxY_menu2 5
#define tiempo_de_parpadeo 700
#define tiempo_de_espera_menu 3000 

extern bool use_farenheit;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern uint16_t tiempo_de_standby;
extern uint32_t  mili_segundos;
extern uint8_t hora,minutos;
extern uint16_t tiempo_de_standby;

bool Blink;
uint8_t Flog;
int8_t Ypos;
uint8_t opcionmenu;
uint16_t tiempo_menues;   

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

typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 

extern Seleccionar_Funciones funcionActual;
extern estadoMEF Estadoequipo;

extern LiquidCrystal_I2C lcd;

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
        Flog=1;
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
  if(Flog==1){ // inicializa variables utiles para el menu
    Blink=false;
    lcd.clear();
    opcionmenu= 0;
    Flog=2;
    tiempo_de_standby = 0;
    tiempo_menues=mili_segundos;
  }
  if (Flog==2){
    if ((PIND & (1<<PD2)) == 0 ){while ((PIND & (1<<PD2)) == 0 ){} Ypos=ReturnToCero(Ypos-1,maxY_menu1); lcd.clear(); Blink = true; tiempo_de_standby = 0;} // suma 1 a Ypos
    if ((PIND & (1<<PD3)) == 0 ){while ((PIND & (1<<PD3)) == 0 ){}  Ypos=ReturnToCero(Ypos+1,maxY_menu1); lcd.clear(); Blink = true; tiempo_de_standby = 0;}// resta 1 a Ypos
    if ((PIND & (1<<PD4)) == 0 ){while ((PIND & (1<<PD4)) == 0 ){}  opcionmenu=Ypos+1; lcd.clear(); } //confirmacion
    if(mili_segundos>=(tiempo_menues+tiempo_de_parpadeo)){tiempo_menues=mili_segundos;Blink=!Blink;} //prende o apaga la flechita

    switch (opcionmenu){
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
        Flog=2; // la declaracion de flag no es del todo necesaria pero se vuelve a declarar x seguridad
        break;
      case 2:
        Estadoequipo=funciones;
        funcionActual=calefaccion_manual;
        Flog=2;
        break;
      case 3:
        Estadoequipo=funciones;
        funcionActual=funcion_menu_de_auto_por_hora;
        Flog=2;
        break;
      case 4:
        Estadoequipo=funciones;
        funcionActual=llenado_auto;
        Flog=2;
        break;
      case 5:
        Estadoequipo=funciones;
        funcionActual=calefaccion_auto;
        Flog=2;
        break;
      case 6:
        Estadoequipo=menu2;
        Flog=3; // aumenta pq pasa al menu avanzado
        break;
      case 7:
        Estadoequipo=estado_inicial;
        tiempo_de_standby=0;
        Flog=0; // disminuye pq pasa al standby
        break;
    }
  if(tiempo_de_standby>=5000){
    Estadoequipo = estado_standby;
    tiempo_de_standby = 0;
    Flog=1;
    lcd.clear();
  }
  }
}

void menu_avanzado()
{
  if (Flog==3){
    Blink=false;
    lcd.clear();
    Ypos=0;
    opcionmenu=0;
    Flog=4;
    tiempo_de_standby = 0;
  }
  
  if (Flog==4){
  if ((PIND & (1<<PD2)) == 0 ){ while ((PIND & (1<<PD2)) == 0){} Ypos=ReturnToCero(Ypos-1,maxY_menu2); lcd.clear(); Blink = true;tiempo_de_standby = 0;}
  if ((PIND & (1<<PD3)) == 0 ){ while ((PIND & (1<<PD3)) == 0){} Ypos=ReturnToCero(Ypos+1,maxY_menu2); lcd.clear(); Blink = true;tiempo_de_standby = 0;}
  if ((PIND & (1<<PD4)) == 0 ){ while ((PIND & (1<<PD4)) == 0){} opcionmenu=ReturnToCero(Ypos,maxY_menu2)+1; lcd.clear(); }
  if(mili_segundos>=(tiempo_menues+tiempo_de_parpadeo)){tiempo_menues=mili_segundos;Blink=!Blink;}
    
    switch (opcionmenu)
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
        Flog=1;
        opcionmenu=0;
        break;
    }
  if(tiempo_de_standby>=5000){
    Estadoequipo = estado_standby;
    tiempo_de_standby = 0;
    Flog=1;
    lcd.clear();
  }
  }
}

void guardado_para_menus(bool Menu){
  lcd.setCursor(4,0);
  lcd.print("Guardando...");
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