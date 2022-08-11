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
extern char WIFISSID [19];
extern char WIFIPASS [19];
extern uint32_t  mili_segundos;
extern bool use_farenheit;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern uint8_t hora,minutos;
extern  bool Activar_bomba;
extern AT24C32 eep;
extern RTC_DS1307 rtc;

uint16_t tiempo_submenues;   
uint8_t Fleg;
uint8_t Auxiliar0;
bool use_farenheit = false; 
uint8_t hora_to_modify, minuto_to_modify;
uint8_t sumador_hora, sumador_minuto;
uint8_t Valorinicial,Valorfinal;
char Actualchar=0;
bool mayusculas=false;
int8_t Yposo;
uint8_t ActualStructa=0;     

extern char LCDMessage[20];

void menu_de_llenado_manual(){
    switch (Fleg)
    {
      case 2:
        nivel_a_llenar=min_nivel;
        Fleg=3;
        break;
      case 3:
        sprintf(LCDMessage, "Nivel a llenar: %d %", nivel_a_llenar);
        PrintLCD (LCDMessage,0,0);
        sprintf(LCDMessage, "Sumar 5 con 1");
        PrintLCD (LCDMessage,0,1);
        sprintf(LCDMessage, "Restar 5 con 2");
        PrintLCD (LCDMessage,0,2);
        sprintf(LCDMessage, "Confirmar con 3");
        PrintLCD (LCDMessage,0,3);

        if (nivel_a_llenar>max_nivel)nivel_a_llenar=max_nivel;
        if (nivel_a_llenar<min_nivel)nivel_a_llenar=min_nivel;
        if(PressedButton(1)&&nivel_a_llenar<max_nivel){
            while(PressedButton(1)){}
            nivel_a_llenar += sumador_nivel;
          }
        if((PressedButton(2)) == 0 && nivel_a_llenar>min_nivel){
            while(PressedButton(2)){}
            nivel_a_llenar -= sumador_nivel;
          }
        if(PressedButton(3)){
            while(PressedButton(3)){}
            Fleg=4;
            lcd.clear();
          }
        if(PressedButton(4)){
            while(PressedButton(4)){}
            Estadoequipo=menu1;
            Fleg=1;
            funcionActual=posicion_inicial;
            Auxiliar0=0;
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
              tiempo_submenues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
              lcd.clear();
              Fleg=5;
            }
          if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=3; lcd.clear();}
          break;

        case 5:
          guardado_para_menus(true);
          break;
    }
}

void menu_de_calefaccion_manual(){
    switch (Fleg)
    {
      case 2:
        if (min_temp>temperatura_actual) temperatura_a_calentar=min_temp;
        else  temperatura_a_calentar = temperatura_actual;
        Fleg=3;
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
            Fleg=4;
            lcd.setCursor(17,0); lcd.print("   ");
          }
        if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Estadoequipo=menu1;
            Fleg=1;
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
              Fleg=5;
              tiempo_submenues=mili_segundos;
              lcd.clear();
            }
          if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=3; lcd.clear();}
          break;

        case 5:
          guardado_para_menus(true);
          break;
    }
}

void menu_de_auto_por_hora()
{
  switch (Fleg)
  {
    case 2:
      Fleg=3;
      sumador_hora=0;
      sumador_minuto=0;
      Yposo=0;
      hora_to_modify=hora;
      minuto_to_modify=minutos;
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("Seleccionar:"); lcd.print(ActualStructa+1); lcd.print(" slot");
      lcd.setCursor(0,1);
      lcd.print("1:");lcd.print(String_de_hora(CharToUINT(1,save[0].hour),CharToUINT(2,save[0].hour)));
      lcd.setCursor(0,2);
      lcd.print("2:");lcd.print(String_de_hora(CharToUINT(1,save[1].hour),CharToUINT(2,save[1].hour)));
      lcd.setCursor(0,3);
      lcd.print("3:");lcd.print(String_de_hora(CharToUINT(1,save[2].hour),CharToUINT(2,save[2].hour)));

      ActualStructa = ReturnToCero(Yposo,3);
      if((PIND & (1<<PD2)) == 0){
          while((PIND & (1<<PD2)) == 0){}
          Yposo=ReturnToCero(Yposo+1,3);
        }
      if((PIND & (1<<PD3)) == 0){
          while((PIND & (1<<PD3)) == 0){}
          Yposo=ReturnToCero(Yposo-1,3);
      }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          save[ActualStructa].temp=min_temp;
          save[ActualStructa].level=min_nivel;
          Fleg=4;
        }
      if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Estadoequipo=menu1;
            Fleg=1;
            funcionActual=posicion_inicial;
            lcd.clear();
            save[ActualStructa].temp = eep.read(3*(ActualStructa+1));
            save[ActualStructa].level = eep.read(2*(ActualStructa+1));
          }
      break; 

    case 4:
      lcd.setCursor(0,0);
      lcd.print("Temp. max:");
      if(use_farenheit == false) {
        lcd.print(save[ActualStructa].temp); 
        if (save[ActualStructa].temp<100){lcd.print((char)223); lcd.print("C    ");}
        else{lcd.print((char)223);lcd.setCursor(20,0); lcd.print("C");}
      }
      if(use_farenheit == true) {
        lcd.print(((9*save[ActualStructa].temp)/5)+32);
        if (save[ActualStructa].temp<100){lcd.print((char)223); lcd.print("F   ");}
        else{lcd.print((char)223); lcd.print("F");}
      }

      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if (save[ActualStructa].temp>maxi_cast)save[ActualStructa].temp=maxi_cast;
      if (save[ActualStructa].temp<min_temp)save[ActualStructa].temp=min_temp;
      if((PIND & (1<<PD2)) == 0 && save[ActualStructa].temp < maxi_cast){
          while((PIND & (1<<PD2)) == 0){}
          save[ActualStructa].temp += sumador_temperatura;
        }
      if((PIND & (1<<PD3)) == 0 && save[ActualStructa].temp> min_temp){
          while((PIND & (1<<PD3)) == 0){}
          save[ActualStructa].temp -= sumador_temperatura;
        }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Fleg=5;
          lcd.clear();
        }
      if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=3; lcd.clear();}
        break;

      case 5:
       lcd.setCursor(0,0);
      lcd.print("Nivel max:"); 
      lcd.print(save[ActualStructa].level);
      if (save[ActualStructa].level<100)lcd.print("% ");
      else lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("Sumar 5 con 1 ");
      lcd.setCursor(0,2);
      lcd.print("Restar 5 con 2");
      lcd.setCursor(0,3);
      lcd.print("Confirmar con 3");

      if (save[ActualStructa].level>max_nivel)save[ActualStructa].level=max_nivel;
      if (save[ActualStructa].level<min_nivel)save[ActualStructa].level=min_nivel;
      if((PIND & (1<<PD2)) == 0 && save[ActualStructa].level<max_nivel){
          while((PIND & (1<<PD2)) == 0){}
          save[ActualStructa].level += sumador_nivel;
        }
      if((PIND & (1<<PD3)) == 0 && save[ActualStructa].level>min_nivel){
          while((PIND & (1<<PD3)) == 0){}
          save[ActualStructa].level -= sumador_nivel;
        }
      if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          Fleg=6;
          sumador_hora=0;
          sumador_minuto=0;
          lcd.clear();
        }
      if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=4; lcd.clear();}
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
        if((PIND & (1<<PD2)) == 0){while((PIND & (1<<PD2)) == 0){}sumador_hora++;}
        if((PIND & (1<<PD3)) == 0){while((PIND & (1<<PD3)) == 0){}sumador_hora--;}
        if((PIND & (1<<PD4)) == 0){while((PIND & (1<<PD4)) == 0){}Fleg=7;lcd.clear();}
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=5; lcd.clear();}
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


        minuto_to_modify = ReturnToCero(minutos+sumador_minuto,minuto_max);
        if((PIND & (1<<PD2)) == 0){while((PIND & (1<<PD2)) == 0){}sumador_minuto++;}
        if((PIND & (1<<PD3)) == 0){while((PIND & (1<<PD3)) == 0){}sumador_minuto--;}
        if((PIND & (1<<PD4)) == 0){while((PIND & (1<<PD4)) == 0){}Fleg=8;lcd.clear();}
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=6; lcd.clear();}
        break;

      case 8:
        lcd.setCursor(0,0);
        lcd.print("A las:");lcd.print(String_de_hora(hora_to_modify,minuto_to_modify));
        lcd.setCursor(0,1);
        lcd.print("Calentar:");
        if(use_farenheit == false) {lcd.print(save[ActualStructa].temp); lcd.print((char)223); lcd.print("C");}
        if(use_farenheit == true) {lcd.print(((9*save[ActualStructa].temp)/5)+32);lcd.print((char)223); lcd.print("F  ");}
        lcd.setCursor(0,2);
        lcd.print("Llenar:"); lcd.print(save[ActualStructa].level);lcd.print("%");
        lcd.setCursor(0,3);
        lcd.print("     Confirmar?    ");

        if((PIND & (1<<PD4)) == 0){
          while((PIND & (1<<PD4)) == 0){}
          tiempo_submenues=mili_segundos;//corregir porque no hace la espera //Dale Teo, una buena noticia dame
          lcd.clear();
          Fleg=9;
          save[ActualStructa].hour= StringToChar(1,String_de_hora(hora_to_modify,minuto_to_modify));
        }
        if((PIND & (1<<PD5)) == 0){while((PIND & (1<<PD5)) == 0){} Fleg=7; lcd.clear();}
        break;

      case 9:
        eep.write((ActualStructa*3)+1, save[ActualStructa].hour);
        eep.write((ActualStructa*3)+2, save[ActualStructa].level);
        eep.write((ActualStructa*3)+3, save[ActualStructa].temp);
        guardado_para_menus(true);
      break;
    }
  }

void menu_de_llenado_auto()
{
  switch (Fleg)
  {
    case 2:
      Valorinicial=eep.read(12);
      Valorfinal=eep.read(13);
      eep.write(12,min_nivel);
      Fleg=3;
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
        Fleg=4;
        eep.write(13,eep.read(12)+sumador_nivel);
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        eep.write(12,Valorinicial);
        eep.write(13,Valorfinal);
        Estadoequipo=menu1;
        Fleg=1;
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
        Fleg=5;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Fleg=3;
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
        Fleg=6;
        tiempo_submenues=mili_segundos;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Fleg=4;
        lcd.clear();
      }
      break;

    case 6:
      guardado_para_menus(true);
      break; 
  }
}

void menu_de_calefaccion_auto(){
  switch (Fleg)
  {
    case 2:
      Valorinicial=eep.read(10);
      Valorfinal=eep.read(11);
      eep.write(10,min_temp);
      Yposo=0;
      Fleg=3;
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
        Fleg=4;
        eep.write(11,eep.read(10)+sumador_temperatura);
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        eep.write(10,Valorinicial);
        eep.write(11,Valorfinal);
        Estadoequipo=menu1;
        Fleg=1;
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
        Fleg=5;
        lcd.clear();
      }
      if((PIND & (1<<PD5)) == 0){
        while((PIND & (1<<PD5)) == 0){}
        Fleg=3;
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
            Fleg=6;
            tiempo_submenues=mili_segundos;
            lcd.clear();
          }
          if((PIND & (1<<PD5)) == 0){
            while((PIND & (1<<PD5)) == 0){}
            Fleg=4;
            lcd.clear();
          }
          break;

      case 6:
        guardado_para_menus(true);
        break; 
  }
}

void menu_modificar_hora_rtc()
  {
    switch (Fleg)
    {
      case 4:
        sumador_hora=0;
        sumador_minuto=0;
        hora_to_modify=hora;
        minuto_to_modify=minutos;
        Fleg=5;
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
          Fleg=6;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Estadoequipo=menu2;
          Fleg=3;
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
          Fleg=7;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Fleg=5;
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
          Fleg=8;
          DateTime now = rtc.now();
          rtc.adjust(DateTime(now.year(),now.month(),now.day(),hora_to_modify,minuto_to_modify,now.second()));
          tiempo_submenues=mili_segundos;
          lcd.clear();
        }
        if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Fleg=6;
          lcd.clear();
        }
        break;

      case 8:
        guardado_para_menus(false);
        break; 
    }
  }

void menu_activar_bomba(){
switch (Fleg)
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
          Fleg=5;
        }
      if((PIND & (1<<PD2)) == 0 ){
          while((PIND & (1<<PD2)) == 0){}
          Activar_bomba = true;
          Fleg=5;
        }
      if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Estadoequipo=menu2;
          Fleg=3;
          funcionActual=posicion_inicial;
          lcd.clear();
        }
    break;

    case 5:
      lcd.clear();
      tiempo_submenues=mili_segundos;
      Fleg=6;
    break;

    case 6:
      guardado_para_menus(false);
    break;
  }
  
}     

void menu_farenheit_celsius()
{
  switch (Fleg)
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
          Fleg=5;
        }
      if((PIND & (1<<PD2)) == 0 ){
          while((PIND & (1<<PD2)) == 0){}
          use_farenheit = true;
          Fleg=5;
        }
      if((PIND & (1<<PD5)) == 0){
          while((PIND & (1<<PD5)) == 0){}
          Estadoequipo=menu2;
          Fleg=3;
          funcionActual=posicion_inicial;
          lcd.clear();
        }
    break;

    case 5:
      lcd.clear();
      tiempo_submenues=mili_segundos;
      Fleg=6;
    break;

    case 6:
      guardado_para_menus(false);
    break;
  }
  
}

void menu_seteo_wifi(){  

  switch (Fleg)
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
        Fleg=5;
      }
    if((PIND & (1<<PD5)) == 0 ){
        while((PIND & (1<<PD5)) == 0){}
        Estadoequipo=menu2;
        Fleg=3;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
    break;
  case 5:
    for (Auxiliar0=0;Auxiliar0<=18;Auxiliar0++){ 
    WIFISSID[Auxiliar0]='\0';
    WIFIPASS[Auxiliar0]='\0';}
    Yposo=0;
    Actualchar=0;
    Fleg=6;
    break;
  case 6:
    lcd.setCursor(0,0);
    lcd.print("Nombre Wifi:");
    lcd.setCursor(0,1);
    for (Auxiliar0 = 0; Auxiliar0<=19; Auxiliar0++){
    if (WIFISSID[Auxiliar0]=='\0' )lcd.print(" ");
    if (WIFISSID[Auxiliar0]!='\0' )lcd.print(WIFISSID[Auxiliar0]);
    }
    Actualchar=ReturnToCero(Actualchar, 40);
    Yposo=ReturnToCero(Yposo,20);
    WIFISSID[Yposo]=Character_Return(Actualchar, mayusculas);
 
    Actualchar=map(analogRead(A2), 0, 1023, 0, 39);
    if((PIND & (1<<PD2)) == 0 )
      {
        while((PIND & (1<<PD2)) == 0){}
        mayusculas=!mayusculas;
      }
    if((PIND & (1<<PD3)) == 0 && Yposo <= 19)
      {
        while((PIND & (1<<PD3)) == 0){}
        Yposo++;
        Actualchar=0;
      }
    if((PIND & (1<<PD4)) == 0 )
      {
        while((PIND & (1<<PD4)) == 0){}
        WIFISSID[Yposo]='\0';
        lcd.clear();
        Fleg=7;
        Yposo=0;
        Actualchar=0;
      }
    if((PIND & (1<<PD5)) == 0 )
      {
        while((PIND & (1<<PD5)) == 0){}
        WIFISSID[Yposo]='\0';
        lcd.clear();
        Fleg=4;
        Yposo=0;
        Actualchar=0;
      }
    break;

    case 7:
    lcd.setCursor(0,0);
    lcd.print("Pass Wifi:");
    lcd.setCursor(0,1);
    lcd.print(WIFIPASS);
    Actualchar=ReturnToCero(Actualchar, 40);
    Yposo=ReturnToCero(Yposo,20);
    WIFIPASS[Yposo]=Character_Return(Actualchar, mayusculas);
    Actualchar=map(analogRead(A2), 0, 1023, 0, 39);
    if((PIND & (1<<PD2)) == 0 )
      {
        while((PIND & (1<<PD2)) == 0){}
        mayusculas=!mayusculas;
      }
    if((PIND & (1<<PD3)) == 0 )
      {
        while((PIND & (1<<PD3)) == 0){}
        Yposo ++;
        Actualchar=0;
      }
    if((PIND & (1<<PD4)) == 0 )
      {
        while((PIND & (1<<PD4)) == 0){}
        WIFIPASS[Yposo]='\0';
        lcd.clear();
        tiempo_submenues=mili_segundos;
        Fleg=8;
        Yposo=0;
      }
    if((PIND & (1<<PD5)) == 0 )
      {
        while((PIND & (1<<PD5)) == 0){}
        WIFISSID[Yposo]='\0';
        lcd.clear();
        Fleg=4;
        Yposo=0;
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