#include "SubMenues.h"
#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "menu.h"
#include "FuncionesSoporte.h"
#include <AT24CX.h>
#include "RTClib.h"
#include "comunicacion.h"

#define sumador_temperatura 5 
#define maxi_cast 80           
#define min_temp 40    
//Control de nivel
#define sumador_nivel 25
#define min_nivel 25   
#define max_nivel 100 

#define hora_max 24
#define minuto_max 60

typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 

struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};  
          //guardado 1/hora/level/temp                                   //guardado 2/hora/level/temp
extern save_data save[3];  
extern Seleccionar_Funciones funcionActual;
extern estadoMEF Estadoequipo;
extern LiquidCrystal_I2C lcd;//LiquidCrystal_I2C lcd(0x27,20,4);
extern int8_t temperatura_a_calentar; 
extern int8_t nivel_a_llenar; 
extern char WIFISSID [20];
extern char WIFIPASS [20];
extern uint32_t  mili_segundos;
extern bool use_farenheit;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern uint8_t hora,minutos;
extern  bool Activar_bomba;
extern AT24C32 eep;
extern RTC_DS1307 rtc;
 
uint8_t Flag=0;

bool use_farenheit = false; 
uint8_t hora_to_modify, minuto_to_modify;
uint8_t sumador_hora, sumador_minuto;
uint8_t Valorinicial,Valorfinal;
char Actualchar=0;
bool mayusculas=false;
int8_t Aux;
uint8_t ActualSlot=0;     

extern char LCDMessage[20];

void menu_de_llenado_manual(){
    switch (Flag)
    {
      case 0:
        nivel_a_llenar=min_nivel;
        lcd.clear();
        Flag=1;
        break;
      case 1:
        sprintf(LCDMessage, "Nivel a llenar: %d%c",nivel_a_llenar,'%');     PrintLCD (LCDMessage,0,0);
        memcpy(LCDMessage, "Sumar 25 con 1", 15);                           PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 25 con 2", 16);                          PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                          PrintLCD (LCDMessage,0,3);

        if (nivel_a_llenar>max_nivel)nivel_a_llenar=max_nivel;
        if (nivel_a_llenar<min_nivel)nivel_a_llenar=min_nivel;

        if(PressedButton(1) == true && nivel_a_llenar<max_nivel)nivel_a_llenar += sumador_nivel;
        if(PressedButton(2) == true  && nivel_a_llenar>min_nivel)nivel_a_llenar -= sumador_nivel;
        if(PressedButton(3)){
            Flag=2;
            lcd.clear();
          }
        if(PressedButton(4)){
            while(PressedButton(4)){}
            Estadoequipo=menu1;
            funcionActual=posicion_inicial;
            lcd.clear();
          }
        break; 

        case 2:
          sprintf(LCDMessage, "Llenar hasta: %d%c",nivel_a_llenar,'%'); PrintLCD (LCDMessage,0,0);
          memcpy(LCDMessage, "Confirmar?", 11);                         PrintLCD (LCDMessage,5,3);

          if(PressedButton(3)){
              lcd.clear();
              Flag=3;
            }
          if(PressedButton(4)){
              Flag=1; 
              lcd.clear();
            }
          break;

        case 3:
          guardado_para_menus(true);
          Flag=0;
          break;
    }
}

void menu_de_calefaccion_manual(){
    switch (Flag)
    {
      case 0:
        if (min_temp>temperatura_actual) temperatura_a_calentar=min_temp;
        else  temperatura_a_calentar = temperatura_actual;
        lcd.clear();
        Flag=1;
        break;
      case 1:
        if(use_farenheit == false)sprintf(LCDMessage, "T. a calentar: %d%cC",nivel_a_llenar,(char)223);
        if(use_farenheit == true)sprintf(LCDMessage, "T. a calentar: %d%cF",((9*temperatura_a_calentar)/5)+32,(char)223);
        PrintLCD (LCDMessage,0,0);
        memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

        if (temperatura_a_calentar>maxi_cast)temperatura_a_calentar=maxi_cast;
        if (temperatura_a_calentar<min_temp)temperatura_a_calentar=min_temp;

        if(PressedButton(1)==true && temperatura_a_calentar<maxi_cast)temperatura_a_calentar += sumador_temperatura;
        if(PressedButton(2)==true && temperatura_a_calentar>min_temp)temperatura_a_calentar -= sumador_temperatura;
        if(PressedButton(3)){
            Flag=2;
            lcd.clear();
          }
        if(PressedButton(4)){
            Estadoequipo=menu1;
            Flag=0;
            funcionActual=posicion_inicial;
            lcd.clear();
          }
        break; 

        case 2:
          if(use_farenheit == false)sprintf(LCDMessage, "Calentar hasta: %d%cC",nivel_a_llenar,(char)223);
          if(use_farenheit == true)sprintf(LCDMessage, "Calentar hasta: %d%cF",((9*temperatura_a_calentar)/5)+32,(char)223);
          PrintLCD (LCDMessage,0,0);
          memcpy(LCDMessage, "Confirmar?", 11);                                     PrintLCD (LCDMessage,5,3);
          if(PressedButton(3)){
              Flag=3;
              lcd.clear();
            }
          if(PressedButton(4)){
            Flag=3; 
            lcd.clear();
            }
          break;

        case 3:
          guardado_para_menus(true);
          Flag=0;
          break;
    }
}

void menu_de_auto_por_hora()
{
switch (Flag)
{
  case 0:
    Flag=1;
    sumador_hora=0;
    sumador_minuto=0;
    Aux=0;
    hora_to_modify=hora;
    minuto_to_modify=minutos;
    break;
  case 1:
    sprintf(LCDMessage, "Seleccionar:%d slot",ActualSlot+1);                   PrintLCD (LCDMessage,0,0);
    memcpy(LCDMessage, "1:", 2);                                                  PrintLCD (LCDMessage,0,1);
    Printhora(LCDMessage,CharToUINT(1,save[0].hour),CharToUINT(2,save[0].hour));  PrintLCD (LCDMessage,3,1);
    memcpy(LCDMessage, "2:", 2);                                                  PrintLCD (LCDMessage,0,2);
    Printhora(LCDMessage,CharToUINT(1,save[1].hour),CharToUINT(2,save[1].hour));  PrintLCD (LCDMessage,3,2);
    memcpy(LCDMessage, "3:", 2);                                                  PrintLCD (LCDMessage,0,3);
    Printhora(LCDMessage,CharToUINT(1,save[2].hour),CharToUINT(2,save[2].hour));  PrintLCD (LCDMessage,3,3);

    ActualSlot = ReturnToCero(Aux,3);

    if(PressedButton(1))Aux=ReturnToCero(Aux+1,3);
    if(PressedButton(2))Aux=ReturnToCero(Aux-1,3);
    if(PressedButton(3)){
        save[ActualSlot].temp=min_temp;
        save[ActualSlot].level=min_nivel;
        Flag=2;
      }
    if(PressedButton(4)){
          Estadoequipo=menu1;
          Flag=0;
          funcionActual=posicion_inicial;
          lcd.clear();
          save[ActualSlot].temp = eep.read(3*(ActualSlot+1));
          save[ActualSlot].level = eep.read(2*(ActualSlot+1));
        }
    break; 

  case 2:        
    if(use_farenheit == false)sprintf(LCDMessage, "Temp. max: %d%cC",nivel_a_llenar,(char)223);
    if(use_farenheit == true)sprintf(LCDMessage, "Temp. max: %d%cF",((9*temperatura_a_calentar)/5)+32,(char)223);
    PrintLCD (LCDMessage,0,0);
    memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);
    
    if (save[ActualSlot].temp>maxi_cast)save[ActualSlot].temp=maxi_cast;
    if (save[ActualSlot].temp<min_temp)save[ActualSlot].temp=min_temp;

    if(PressedButton(1)==true && save[ActualSlot].temp < maxi_cast)save[ActualSlot].temp += sumador_temperatura;
    if(PressedButton(2)==true && save[ActualSlot].temp> min_temp)save[ActualSlot].temp -= sumador_temperatura;
    if(PressedButton(3)){
        Flag=3;
        lcd.clear();
      }
    if(PressedButton(4)){
      Flag=1; 
      lcd.clear();
    }
      break;

    case 3:
      sprintf(LCDMessage, "Nivel max:%d%c",save[ActualSlot].level,'%');       PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

      if (save[ActualSlot].level>max_nivel)save[ActualSlot].level=max_nivel;
      if (save[ActualSlot].level<min_nivel)save[ActualSlot].level=min_nivel;

      if(PressedButton(1)==true && save[ActualSlot].level<max_nivel)save[ActualSlot].level += sumador_nivel;
      if(PressedButton(2)==true  && save[ActualSlot].level>min_nivel)save[ActualSlot].level -= sumador_nivel;
      if(PressedButton(3)){
          Flag=4;
          sumador_hora=0;
          sumador_minuto=0;
          lcd.clear();
      }
      if(PressedButton(4)){
        Flag=2; 
        lcd.clear();
      }
      break;

    case 4:
      memcpy(LCDMessage, "Setear hora:", 13);                                    PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

      hora_to_modify = ReturnToCero((hora+sumador_hora),hora_max);

      if(PressedButton(1))sumador_hora++;
      if(PressedButton(2))sumador_hora--;
      if(PressedButton(3)){Flag=5; lcd.clear();}
      if(PressedButton(4)){Flag=3; lcd.clear();}
      break;

    case 5:
      memcpy(LCDMessage, "Setear minuto:", 15);                                  PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

      minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max);

      if(PressedButton(1))sumador_minuto++;
      if(PressedButton(2))sumador_minuto--;
      if(PressedButton(3)){Flag=6; lcd.clear();}
      if(PressedButton(4)){Flag=4; lcd.clear();}
      break;

    case 6:
      memcpy(LCDMessage, "A las:", 7);                                          PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                    PrintLCD (LCDMessage,7,0);
      if(use_farenheit == false) {sprintf(LCDMessage, "Calentar: %d%cC",save[ActualSlot].temp,(char)223);}
      if(use_farenheit == true) {sprintf(LCDMessage, "Calentar: %d%cF",((9*save[ActualSlot].temp)/5)+32,(char)223);}
      PrintLCD (LCDMessage,0,1);
      sprintf(LCDMessage, "Llenar: %d%c",save[ActualSlot].level,'%');        PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar?", 11);                                     PrintLCD (LCDMessage,5,3);


      if(PressedButton(3)){lcd.clear(); Flag=7; Printhora(LCDMessage,hora_to_modify,minuto_to_modify); save[ActualSlot].hour= ArrayToChar(1,LCDMessage);}
      if(PressedButton(4)){Flag=5; lcd.clear();}
      break;

    case 7:
      eep.write((ActualSlot*3)+1, save[ActualSlot].hour);
      eep.write((ActualSlot*3)+2, save[ActualSlot].level);
      eep.write((ActualSlot*3)+3, save[ActualSlot].temp);
      guardado_para_menus(true);
      Flag=0;
      break;
  }
}

void menu_de_llenado_auto()
{
  switch (Flag)
  {
    case 0:
      Valorinicial=eep.read(12);
      Valorfinal=eep.read(13);
      eep.write(12,min_nivel);
      Flag=1;
      break;

    case 1:
      sprintf(LCDMessage, "Nivel min:%d%c",eep.read(12),'%');                    PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

      if (eep.read(12)>max_nivel-sumador_nivel)eep.write(12,max_nivel-sumador_nivel);
      if (eep.read(12)<min_nivel)eep.write(12,min_nivel);

      if(PressedButton(1)==true && eep.read(12)<max_nivel-sumador_nivel)eep.write(12,eep.read(12)+sumador_nivel);
      if(PressedButton(2)==true&& eep.read(12)>min_nivel)eep.write(12,eep.read(12)-sumador_nivel);
      if(PressedButton(3)==true){
        Flag=2;
        eep.write(13,eep.read(12)+sumador_nivel);
        lcd.clear();
      }
      if(PressedButton(1)==true){
        eep.write(12,Valorinicial);
        eep.write(13,Valorfinal);
        Estadoequipo=menu1;
        Flag=0;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break;
      
    case 2:
      sprintf(LCDMessage, "Nivel max: %d%c",eep.read(13),'%');                  PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

      if (eep.read(13)>max_nivel)eep.write(13,max_nivel);
      if (eep.read(13)<eep.read(12)+sumador_nivel)eep.write(13,eep.read(12)+sumador_nivel);

      if(PressedButton(1)==true && eep.read(13)<max_nivel)eep.write(13,eep.read(13)+sumador_nivel); 
      if(PressedButton(2)==true && eep.read(13)>eep.read(12)+sumador_nivel)eep.write(13,eep.read(13)-sumador_nivel); 
      if(PressedButton(3)){Flag=3;  lcd.clear();}
      if(PressedButton(4)){Flag=1;  lcd.clear();}
      break;

    case 3: 
      sprintf(LCDMessage, "Al %d%c",eep.read(12),'%');                          PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "Calentar hasta %d%c",eep.read(14),'%');              PrintLCD (LCDMessage,0,1);

      memcpy(LCDMessage, "Confirmar?", 11);                                     PrintLCD (LCDMessage,5,3);

      if(PressedButton(3)){Flag=4;  lcd.clear();}
      if(PressedButton(4)){Flag=2;lcd.clear();}
      break;

    case 4:
      guardado_para_menus(true);
      Flag=0;
      break; 
  }
}

void menu_de_calefaccion_auto(){
  switch (Flag)
  {
    case 0:
      Valorinicial=eep.read(10);
      Valorfinal=eep.read(11);
      eep.write(10,min_temp);
      Aux=0;
      Flag=1;
      break;
    case 1:
      if(use_farenheit == false)sprintf(LCDMessage, "Temp. min: %d%cC",eep.read(10),(char)223);
      if(use_farenheit == true)sprintf(LCDMessage, "Temp. min: %d%cF",((9*eep.read(10))/5)+32,(char)223);
      PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);


      if (eep.read(10)>maxi_cast-sumador_temperatura)eep.write(10,maxi_cast-sumador_temperatura);
      if (eep.read(10)<min_temp)eep.write(10,min_temp);

      if(PressedButton(1)==true && eep.read(10)<maxi_cast-sumador_temperatura)eep.write(10,eep.read(10)+sumador_temperatura);
      if(PressedButton(2)==true && min_temp<eep.read(10))eep.write(10,eep.read(10)-sumador_temperatura);
      if(PressedButton(3)){Flag=2;  eep.write(11,eep.read(10)+sumador_temperatura);}
      if(PressedButton(4)){
        eep.write(10,Valorinicial);
        eep.write(11,Valorfinal);
        Estadoequipo=menu1;
        Flag=0;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break; 

    case 2:
      if(use_farenheit == false)sprintf(LCDMessage, "Temp. max: %d%cC",eep.read(11),(char)223);
      if(use_farenheit == true)sprintf(LCDMessage, "Temp. max: %d%cF",((9*eep.read(11))/5)+32,(char)223);
      PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);
  
      if (eep.read(11)>maxi_cast)eep.write(11,maxi_cast);
      if (eep.read(11)<eep.read(10)+sumador_temperatura)eep.write(11,eep.read(10)+sumador_temperatura);

      if(PressedButton(1)==true && eep.read(11)<maxi_cast)eep.write(11,eep.read(11)+sumador_temperatura);
      if(PressedButton(2)==true && eep.read(11)>eep.read(10)+sumador_temperatura)eep.write(11,eep.read(11)+sumador_temperatura);
      if(PressedButton(3)){Flag=3;  lcd.clear();}
      if(PressedButton(4)){Flag=1;  lcd.clear();}
      break;

    case 3:
      if(use_farenheit == false){
        sprintf(LCDMessage, "Minima guardada: %d%cC",eep.read(10),(char)223);             PrintLCD (LCDMessage,0,0);
        sprintf(LCDMessage, "Maxima guardada: %d%cC",eep.read(11),(char)223);             PrintLCD (LCDMessage,0,1);
      }
      if(use_farenheit == true){
        sprintf(LCDMessage, "Minima guardada: %d%cC",((9*eep.read(10))/5)+32,(char)223);  PrintLCD (LCDMessage,0,0);
        sprintf(LCDMessage, "Maxima guardada: %d%cC",((9*eep.read(11))/5)+32,(char)223);  PrintLCD (LCDMessage,0,1);
      }
      memcpy(LCDMessage, "Confirmar con 3", 16);                                          PrintLCD (LCDMessage,0,3);

      if(PressedButton(3)){Flag=4;  lcd.clear();}
      if(PressedButton(4)){Flag=2;  lcd.clear();}
        break;

    case 4:
      guardado_para_menus(true);
      Flag=0;
      break; 
  }
}

void menu_modificar_hora_rtc()
  {
    switch (Flag)
    {
      case 0:
        sumador_hora=0;
        sumador_minuto=0;
        hora_to_modify=hora;
        minuto_to_modify=minutos;
        Flag=1;
        lcd.clear();
        break;
      case 1:
        memcpy(LCDMessage, "Setear hora:", 15);                                    PrintLCD (LCDMessage,0,0);
        Printhora(LCDMessage,hora_to_modify,minutos);                              PrintLCD (LCDMessage,13,0);
        memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

        hora_to_modify = ReturnToCero(hora+sumador_hora,hora_max);

        if(PressedButton(1))sumador_hora++;
        if(PressedButton(2))sumador_hora--;
        if(PressedButton(3)){Flag=2;  lcd.clear();}
        if(PressedButton(4)){
          Estadoequipo=menu2;
          Flag=0;
          funcionActual=posicion_inicial;
          lcd.clear();
        }
        break;          

      case 2:
        memcpy(LCDMessage, "Setear min:", 12);                                     PrintLCD (LCDMessage,0,0);
        Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,12,0);
        memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

        minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max);

        if(PressedButton(1))sumador_minuto++;
        if(PressedButton(2))sumador_minuto--;
        if(PressedButton(3)){Flag=1;  lcd.clear();}
        if(PressedButton(4)){Flag=3;  lcd.clear();}
        break;

      case 3:
        memcpy(LCDMessage, "hora Actual:", 13);                                         PrintLCD (LCDMessage,0,0);
        Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                          PrintLCD (LCDMessage,13,0);
        memcpy(LCDMessage, "Confirmar?", 11);                                           PrintLCD (LCDMessage,5,3);

        if(PressedButton(3)){
          Flag=4;
          DateTime now = rtc.now();
          rtc.adjust(DateTime(now.year(),now.month(),now.day(),hora_to_modify,minuto_to_modify,now.second()));
          lcd.clear();
        }
        if(PressedButton(4)){Flag=2;  lcd.clear();}
        break;

      case 4:
        guardado_para_menus(false);
        Flag=0;
        break; 
    }
  }

void menu_activar_bomba(){
switch (Flag)
  {
    case 0:
      memcpy(LCDMessage, "Activar bomba", 14);                                    PrintLCD (LCDMessage,0,0);
      if(Activar_bomba){memcpy(LCDMessage, "Bomba Activada", 15);                 PrintLCD (LCDMessage,0,1);}
      if(!Activar_bomba){memcpy(LCDMessage, "Bomba Desactivada", 18);             PrintLCD (LCDMessage,0,1);}
      memcpy(LCDMessage, "1:Si", 5);                                              PrintLCD (LCDMessage,3,3);
      memcpy(LCDMessage, "2:No", 5);                                              PrintLCD (LCDMessage,11,3);

      if(PressedButton(1))Activar_bomba = false;
      if(PressedButton(2))Activar_bomba = true;
      if(PressedButton(3)){Flag=1;  lcd.clear();}
    break;
    case 1:
      guardado_para_menus(false);
      Flag=0;
    break;
  }
  
}     

void menu_farenheit_celsius()
{
  switch (Flag)
  {
    case 0:
      memcpy(LCDMessage, "Cambiar unidad", 15);                                    PrintLCD (LCDMessage,0,0);
      if(use_farenheit){sprintf(LCDMessage, "Actualmente %cF",(char)223);          PrintLCD (LCDMessage,0,1);}
      if(!use_farenheit){sprintf(LCDMessage, "Actualmente %cC",(char)223);         PrintLCD (LCDMessage,0,1);}
      sprintf(LCDMessage, "   1:%cC    2:%cF     ",(char)223,(char)223);           PrintLCD (LCDMessage,0,3);
      if(PressedButton(1))use_farenheit = false;
      if(PressedButton(2))use_farenheit = true;
      if(PressedButton(3)){Flag=1;lcd.clear();}
    break;

    case 1:
      guardado_para_menus(false);
      Flag=0;
    break;
  }
}

void menu_seteo_wifi(){  

  switch (Flag)
  {
  case 0:
    sprintf(LCDMessage, "SSID:%s",WIFISSID);                                  PrintLCD (LCDMessage,0,0);
    sprintf(LCDMessage, "PASS:%s",WIFIPASS);                                  PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "modificar?", 11);                                     PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                            PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                            PrintLCD (LCDMessage,11,3);

    if(PressedButton(1)){lcd.clear();Flag=1;}
    if(PressedButton(2)){
        Estadoequipo=menu2;
        Flag=0;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
    break;
  case 1:
    for (uint8_t Auxiliar0=0;Auxiliar0<=19;Auxiliar0++){ 
    WIFISSID[Auxiliar0]='\0';
    WIFIPASS[Auxiliar0]='\0';}
    Aux=0;
    Actualchar=0;
    Flag=2;
    break;
  case 2:
    memcpy(LCDMessage, "Nombre Wifi:", 13);                                   PrintLCD (LCDMessage,0,0);
    sprintf(LCDMessage, "%s",WIFISSID);                                       PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "modificar?", 11);                                     PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                            PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                            PrintLCD (LCDMessage,11,3);

    Actualchar=ReturnToCero(Actualchar, 40);
    Aux=ReturnToCero(Aux,20);
    WIFISSID[Aux]=Character_Return(Actualchar, mayusculas);

    if(PressedButton(1))mayusculas=!mayusculas;
    if(PressedButton(2) == true && Aux <= 19){Aux++; Actualchar=0;}
    if(PressedButton(3)){
        WIFISSID[Aux]='\0';
        lcd.clear();
        Flag=3;
        Aux=0;
        Actualchar=0;
      }
    if(PressedButton(4)){
        WIFISSID[Aux]='\0';
        lcd.clear();
        Flag=0;
        Aux=0;
        Actualchar=0;
      }
    break;

  case 3:
    memcpy(LCDMessage, "Pass Wifi:", 11);                                     PrintLCD (LCDMessage,0,0);
    sprintf(LCDMessage, "%s",WIFIPASS);                                       PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "modificar?", 11);                                     PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                            PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                            PrintLCD (LCDMessage,11,3);

    Actualchar=ReturnToCero(Actualchar, 40);
    Aux=ReturnToCero(Aux,20);
    WIFIPASS[Aux]=Character_Return(Actualchar, mayusculas);

    if(PressedButton(1))mayusculas=!mayusculas;
    if(PressedButton(2)){Aux ++;  Actualchar=0;}
    if(PressedButton(3)){
        WIFIPASS[Aux]='\0';
        lcd.clear();
        Flag=4;
        Aux=0;
      }
    if(PressedButton(4)){
        WIFISSID[Aux]='\0';
        lcd.clear();
        Flag=2;
        Aux=0;
        Actualchar=0;
      }
    break;
  case 4:
    for (uint8_t i = 0; i < 19; i++){
    eep.write(14 + i, WIFIPASS[i]);
    eep.write(34 + i, WIFISSID[i]);
    }
    Serial_Send_UNO(6);
    guardado_para_menus(false);
    Flag=0;
    break;
  }

}