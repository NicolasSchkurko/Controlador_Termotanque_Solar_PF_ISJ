#include "SubMenues.h"
#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include "menu.h"
#include "FuncionesSoporte.h"
#include <AT24CX.h>
#include "RTClib.h"
#include "comunicacion.h"



typedef enum{posicion_inicial, llenado_manual, calefaccion_manual, funcion_menu_de_auto_por_hora, llenado_auto, calefaccion_auto, funcion_de_menu_modificar_hora_rtc,funcion_farenheit_celsius, funcion_activar_bomba, funcion_de_menu_seteo_wifi} Seleccionar_Funciones;  
typedef enum{estado_standby,estado_inicial,menu1,menu2,funciones} estadoMEF; 
  
extern Seleccionar_Funciones funcionActual;
extern estadoMEF Estadoequipo;
extern LiquidCrystal_I2C lcd;//LiquidCrystal_I2C lcd(0x27,20,4);

extern int8_t temperatura_a_calentar; 
extern int8_t nivel_a_llenar; 
extern char WIFISSID [20];
extern char WIFIPASS [20];
extern char LCDMessage[20];
extern bool use_farenheit;
extern uint16_t  mili_segundos;
extern int8_t temperatura_actual;
extern uint8_t nivel_actual;
extern uint8_t hora,minutos;
extern bool Activar_bomba;
extern AT24C32 eep;
extern RTC_DS1307 rtc;
extern uint8_t encoder0Pos;
extern uint16_t tiempo_de_standby;

uint8_t Flag=0; 
uint8_t hora_to_modify, minuto_to_modify;
uint8_t sumador_hora, sumador_minuto;
uint8_t Valorinicial,Valorfinal;
char Actualchar=0;
bool mayusculas=false;
int8_t Aux;
uint8_t ActualSlot=0; 
uint8_t Set_level;
uint8_t Set_temp;  


void menu_de_llenado_manual(){
    switch (Flag)
    {
      case 0:
        nivel_a_llenar=min_nivel;
        lcd.clear();
        Flag=1;
        break;
      case 1:
        sprintf(LCDMessage,"Nivel a llenar: %d%c",nivel_a_llenar,'%');     PrintLCD (LCDMessage,0,0);
        memcpy(LCDMessage, "Sumar 25 con 1", 15);                           PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 25 con 2", 16);                          PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                          PrintLCD (LCDMessage,0,3);

        if (nivel_a_llenar>max_nivel)nivel_a_llenar=max_nivel;
        if (nivel_a_llenar<min_nivel)nivel_a_llenar=min_nivel;

        encoder0Pos=ReturnToCero(encoder0Pos,4);
        if((encoder0Pos+1)*sumador_nivel!=nivel_a_llenar){
          nivel_a_llenar=(encoder0Pos+1)*sumador_nivel;
        }

        if (PressedButton(1))encoder0Pos--; // suma 1 a Ypos
        if (PressedButton(2))encoder0Pos++; // resta 1 a Ypos
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
              tiempo_de_standby=mili_segundos;
              lcd.clear();
            }
          break;

        case 3:
          guardado_para_menus(true);
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
        memcpy(LCDMessage, "Calentar a", 11);          PrintLCD (LCDMessage,0,0);
        if(use_farenheit == false)sprintf(LCDMessage, "%d%cC",temperatura_a_calentar,(char)223);
        if(use_farenheit == true)sprintf(LCDMessage, "%d%cF",((9*temperatura_a_calentar)/5)+32,(char)223);
        PrintLCD (LCDMessage,12,0);
        memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

        if (temperatura_a_calentar>maxi_cast)temperatura_a_calentar=maxi_cast;
        if (temperatura_a_calentar<min_temp)temperatura_a_calentar=min_temp;

        encoder0Pos=ReturnToCero(encoder0Pos,9);
        if((encoder0Pos+8)*sumador_temperatura!=nivel_a_llenar){
          temperatura_a_calentar=(encoder0Pos+8)*sumador_temperatura;
        }

        if (PressedButton(1))encoder0Pos--; // suma 1 a Ypos
        if (PressedButton(2))encoder0Pos++; // resta 1 a Ypos
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
          memcpy(LCDMessage, "Calentar a", 11);                                     PrintLCD (LCDMessage,0,0);
          if(use_farenheit == false)sprintf(LCDMessage, "%d%cC",temperatura_a_calentar,(char)223);
          if(use_farenheit == true)sprintf(LCDMessage, "%d%cF",((9*temperatura_a_calentar)/5)+32,(char)223);
          PrintLCD (LCDMessage,12,0);
          memcpy(LCDMessage, "Confirmar?", 11);                                     PrintLCD (LCDMessage,5,3);
          if(PressedButton(3)){Flag=3; lcd.clear(); tiempo_de_standby=mili_segundos;}
          if(PressedButton(4)){Flag=1; lcd.clear();}
          break;

        case 3:
          guardado_para_menus(true);
          break;
    }
}

void menu_de_auto_por_hora()
{
switch (Flag)
{
  case 0:
    sumador_hora=0;
    sumador_minuto=0;
    Aux=0;
    hora_to_modify=hora;
    minuto_to_modify=minutos;
    lcd.clear();
    Flag=1;
    break;
  case 1:
    sprintf(LCDMessage, "Seleccionar:%d slot",ActualSlot+1);                    PrintLCD (LCDMessage,0,0);
    memcpy(LCDMessage, "1:", 20);                                               PrintLCD (LCDMessage,0,1);
    Printhora(LCDMessage,CharToUINT(1,eep.read(1)),CharToUINT(2,eep.read(1)));  PrintLCD (LCDMessage,3,1);
    memcpy(LCDMessage, "2:", 20);                                               PrintLCD (LCDMessage,0,2);
    Printhora(LCDMessage,CharToUINT(1,eep.read(4)),CharToUINT(2,eep.read(4)));  PrintLCD (LCDMessage,3,2);
    memcpy(LCDMessage, "3:", 20);                                               PrintLCD (LCDMessage,0,3);
    Printhora(LCDMessage,CharToUINT(1,eep.read(8)),CharToUINT(2,eep.read(8)));  PrintLCD (LCDMessage,3,3);

    ActualSlot = encoder0Pos/4;
    encoder0Pos=ReturnToCero(encoder0Pos,12);

    if (PressedButton(1))encoder0Pos-=4; // suma 1 a Ypos
    if (PressedButton(2))encoder0Pos+=4; // resta 1 a Ypos
    if(PressedButton(3)){
      Set_temp=min_temp;
      Set_level=min_nivel;
      lcd.clear();
      Flag=2;
    }
    if(PressedButton(4)){
      Estadoequipo=menu1;
      Flag=0;
      funcionActual=posicion_inicial;
      lcd.clear();
    }
    break; 

  case 2:        
    memcpy(LCDMessage, "Temp. max:", 20);          PrintLCD (LCDMessage,0,0);
    if(use_farenheit == false)sprintf(LCDMessage, "%d%cC",Set_temp,(char)223);
    if(use_farenheit == true)sprintf(LCDMessage, "%d%cF",((9*Set_temp)/5)+32,(char)223);
    PrintLCD (LCDMessage,12,0);
    memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);
    
    if (Set_temp>maxi_cast)Set_temp=maxi_cast;
    if (Set_temp<min_temp)Set_temp=min_temp;

    encoder0Pos=ReturnToCero(encoder0Pos,9);
    if((encoder0Pos+8)*sumador_temperatura!=nivel_a_llenar){
      Set_temp=(encoder0Pos+8)*sumador_temperatura;
    }

    if (PressedButton(1))encoder0Pos--; // suma 1 a Ypos
    if (PressedButton(2))encoder0Pos++; // resta 1 a Ypos
    if(PressedButton(3)){Flag=3;  lcd.clear();}
    if(PressedButton(4)){Flag=1;  lcd.clear();}
    break;

    case 3:
      sprintf(LCDMessage, "Nivel max:%d%c",Set_level,'%');       PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);

      if (Set_level>max_nivel)Set_level=max_nivel;
      if (Set_level<min_nivel)Set_level=min_nivel;

      encoder0Pos=ReturnToCero(encoder0Pos,4);
      if((encoder0Pos+1)*sumador_nivel!=nivel_a_llenar){
        Set_level=(encoder0Pos+1)*sumador_nivel;
      }

      if(PressedButton(1))Set_level = Set_level+sumador_nivel;
      if(PressedButton(2))Set_level = Set_level-sumador_nivel;
      if(PressedButton(3)){
          Flag=4;
          sumador_hora=0;
          sumador_minuto=0;
          lcd.clear();
      }
      if(PressedButton(4)){Flag=2; lcd.clear();}
      break;

    case 4:
      memcpy(LCDMessage, "Setear hora:", 20);                                    PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 20);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 20);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                                 PrintLCD (LCDMessage,0,3);

      hora_to_modify = ReturnToCero((hora+encoder0Pos),hora_max);
      encoder0Pos=ReturnToCero(encoder0Pos,hora_max);

      if (PressedButton(1))encoder0Pos--; // suma 1 a Ypos
      if (PressedButton(2))encoder0Pos++; // resta 1 a Ypos
      if(PressedButton(3)){Flag=5; lcd.clear();}
      if(PressedButton(4)){Flag=3; lcd.clear();}
      break;

    case 5:
      memcpy(LCDMessage, "Setear min:", 20);                                  PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 20);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 20);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                                 PrintLCD (LCDMessage,0,3);

      minuto_to_modify = ReturnToCero((minutos+encoder0Pos),minuto_max);
      encoder0Pos=ReturnToCero(encoder0Pos,minuto_max);

      if (PressedButton(1))encoder0Pos--; // suma 1 a Ypos
      if (PressedButton(2))encoder0Pos++; // resta 1 a Ypos
      if(PressedButton(3)){Flag=6; lcd.clear();}
      if(PressedButton(4)){Flag=4; lcd.clear();}
      break;

    case 6:
      memcpy(LCDMessage, "A las:",20);                                          PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                    PrintLCD (LCDMessage,7,0);
      memcpy(LCDMessage, "Calentar:",20);                                      PrintLCD (LCDMessage,0,1);

      if(use_farenheit == false) {sprintf(LCDMessage, "%d%cC",Set_temp,(char)223);}
      if(use_farenheit == true) {sprintf(LCDMessage, "%d%cF",((9*Set_temp)/5)+32,(char)223);}
      PrintLCD (LCDMessage,11,1);

      sprintf(LCDMessage, "Llenar: %d%c",Set_level,'%');                        PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar?",20);                                     PrintLCD (LCDMessage,5,3);


      if(PressedButton(3)){lcd.clear(); Flag=7; tiempo_de_standby=mili_segundos;}
      if(PressedButton(4)){Flag=5; lcd.clear();}
      break;

    case 7:
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify); 
      eep.write((ActualSlot*3)+1,ArrayToChar(1,LCDMessage));
      eep.write((ActualSlot*3)+2, Set_level);
      eep.write((ActualSlot*3)+3, Set_temp);
      guardado_para_menus(true);
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
      lcd.clear();
      Flag=1;
      break;

    case 1:
      sprintf(LCDMessage, "Nivel min:%d%c",eep.read(12),'%');   PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                PrintLCD (LCDMessage,0,3);

      if (eep.read(12)>max_nivel-sumador_nivel)eep.write(12,max_nivel-sumador_nivel);
      if (eep.read(12)<min_nivel)eep.write(12,min_nivel);


      if(PressedButton(1)==true)eep.write(12,eep.read(12)+sumador_nivel);
      if(PressedButton(2)==true)eep.write(12,eep.read(12)-sumador_nivel);
      if(PressedButton(3)==true){
        Flag=2;
        eep.write(13,eep.read(12)+sumador_nivel);
        lcd.clear();
      }
      if(PressedButton(4)==true){
        eep.write(12,Valorinicial);
        eep.write(13,Valorfinal);
        Estadoequipo=menu1;
        Flag=0;
        funcionActual=posicion_inicial;
        lcd.clear();
      }
      break;
      
    case 2:
      sprintf(LCDMessage, "Nivel max: %d%c",eep.read(13),'%');  PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                PrintLCD (LCDMessage,0,3);

      if (eep.read(13)>max_nivel)eep.write(13,max_nivel);
      if (eep.read(13)<eep.read(12)+sumador_nivel)eep.write(13,eep.read(12)+sumador_nivel);

      if(PressedButton(1)==true)eep.write(13,eep.read(13)+sumador_nivel); 
      if(PressedButton(2)==true)eep.write(13,eep.read(13)-sumador_nivel); 
      if(PressedButton(3)){Flag=3;  lcd.clear();}
      if(PressedButton(4)){Flag=1;  lcd.clear();}
      break;

    case 3: 
      sprintf(LCDMessage, "Al llegar a %d%c",eep.read(12),'%');   PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "LLenar hasta %d%c",eep.read(13),'%');  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Confirmar?", 20);                       PrintLCD (LCDMessage,5,3);

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
      lcd.clear();
      Flag=1;
      break;
    case 1:
      memcpy(LCDMessage, "Temp. min:", 20);             PrintLCD (LCDMessage,0,0);

      if(use_farenheit == false)sprintf(LCDMessage, "%d%cC",eep.read(10),(char)223);
      if(use_farenheit == true)sprintf(LCDMessage, "%d%cF",((9*eep.read(10))/5)+32,(char)223);
      PrintLCD (LCDMessage,12,0);

      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);


      if (eep.read(10)>maxi_cast-sumador_temperatura)eep.write(10,maxi_cast-sumador_temperatura);
      if (eep.read(10)<min_temp)eep.write(10,min_temp);

      if(PressedButton(1)==true)eep.write(10,eep.read(10)+sumador_temperatura);
      if(PressedButton(2)==true)eep.write(10,eep.read(10)-sumador_temperatura);
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
      memcpy(LCDMessage, "Temp. max:", 20);             PrintLCD (LCDMessage,0,0);

      if(use_farenheit == false)sprintf(LCDMessage, "%d%cC",eep.read(11),(char)223);
      if(use_farenheit == true)sprintf(LCDMessage, "%d%cF",((9*eep.read(11))/5)+32,(char)223);
      PrintLCD (LCDMessage,12,0);

      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);
  
      if (eep.read(11)>maxi_cast)eep.write(11,maxi_cast);
      if (eep.read(11)<eep.read(10)+sumador_temperatura)eep.write(11,eep.read(10)+sumador_temperatura);

      if(PressedButton(1)==true)eep.write(11,eep.read(11)+sumador_temperatura);
      if(PressedButton(2)==true)eep.write(11,eep.read(11)-sumador_temperatura);
      if(PressedButton(3)){Flag=3;  lcd.clear();}
      if(PressedButton(4)){Flag=1;  lcd.clear();}
      break;

    case 3:
      memcpy(LCDMessage, "A los:", 20);                                  PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Calentar a:", 20);                             PrintLCD (LCDMessage,0,1);

      if(use_farenheit == false){
        sprintf(LCDMessage,"%d%cC",eep.read(10),(char)223);             PrintLCD (LCDMessage,7,0);
        sprintf(LCDMessage,"%d%cC",eep.read(11),(char)223);             PrintLCD (LCDMessage,12,1);
      }

      if(use_farenheit == true){
        sprintf(LCDMessage, "%d%cF",((9*eep.read(10))/5)+32,(char)223);  PrintLCD (LCDMessage,8,0);
        sprintf(LCDMessage, "%d%cF",((9*eep.read(11))/5)+32,(char)223);  PrintLCD (LCDMessage,13,1);
      }
      
      memcpy(LCDMessage, "Confirmar con 3", 20);                         PrintLCD (LCDMessage,0,3);

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
      memcpy(LCDMessage, "Unidad actual", 14);                                     PrintLCD (LCDMessage,0,1);

      if(use_farenheit)sprintf(LCDMessage, "%cF",(char)223);          
      if(!use_farenheit)sprintf(LCDMessage,"%cC",(char)223);                       
      PrintLCD (LCDMessage,15,1);

      sprintf(LCDMessage, "1:%cC",(char)223);           PrintLCD (LCDMessage,3,3);
      sprintf(LCDMessage, "2:%cF",(char)223);           PrintLCD (LCDMessage,13,3);

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
    memcpy(LCDMessage, "SSID:", 6);                                PrintLCD (LCDMessage,0,0);
    memcpy(LCDMessage, "PASS:", 6);                                PrintLCD (LCDMessage,0,1);
    sprintf(LCDMessage, "%s",WIFISSID);                            PrintLCD (LCDMessage,6,0);
    sprintf(LCDMessage, "%s",WIFIPASS);                            PrintLCD (LCDMessage,6,1);
    memcpy(LCDMessage, "modificar?", 11);                          PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                 PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                 PrintLCD (LCDMessage,11,3);

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
    Serial_Send_UNO(6,0);
    guardado_para_menus(false);
    Flag=0;
    break;
  }

}

void guardado_para_menus(bool Menu){
  memcpy(LCDMessage, "Guardando...", 13);                                   PrintLCD (LCDMessage,4,0);
  if(mili_segundos>=tiempo_de_standby+tiempo_de_espera_menu){
  if(Menu == true){
      Estadoequipo=menu1;
  }
  if(Menu == false){
    Estadoequipo=menu2;  
  }
  funcionActual=posicion_inicial;
  Flag=0;
  lcd.clear();
  tiempo_de_standby=mili_segundos;
  }
}