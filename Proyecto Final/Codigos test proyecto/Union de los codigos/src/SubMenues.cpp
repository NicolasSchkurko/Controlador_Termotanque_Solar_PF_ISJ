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

extern char LCDMessage[20];
extern uint16_t mili_segundos;
extern AT24C32 eep;
extern RTC_DS1307 rtc;
extern uint16_t tiempo_de_standby;

uint8_t Flag=0; 
char Actualchar=0;
bool mayusculas=false;
int8_t Aux;
uint8_t hora_to_modify, minuto_to_modify;

uint8_t Nivel_a_setear;
uint8_t Temp_a_setear;
uint8_t menu_de_llenado_manual(uint8_t nivel_actual, uint8_t Sumador_encoder){
    switch (Flag)
    {
      case 0:
        if(nivel_actual>min_nivel)Nivel_a_setear=nivel_actual;
        if(nivel_actual<=min_nivel)Nivel_a_setear=min_nivel;
        lcd.clear();
        encoder_value(0,1);
        Flag=1;
        break;
      case 1:

        memcpy(LCDMessage, "Nivel a llenar:", 16);                          PrintLCD (LCDMessage,0,0);
        sprintf(LCDMessage,"%d%c",Nivel_a_setear,'%');                      PrintLCD (LCDMessage,16,0);
        memcpy(LCDMessage, "Sumar 25 con 1", 15);                           PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 25 con 2", 16);                          PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                          PrintLCD (LCDMessage,0,3);

        encoder_value(4,4);
        if((Sumador_encoder+1)*sumador_nivel!=Nivel_a_setear){
          Nivel_a_setear=(Sumador_encoder+1)*sumador_nivel;
        }
        if (PressedButton(1))encoder_value(1,2); // suma 1 a Ypos
        if (PressedButton(2))encoder_value(1,3); // resta 1 a Ypos
        if (PressedButton(3)){Flag=2; lcd.clear();}
        if (PressedButton(4)){Estadoequipo=menu1; funcionActual=posicion_inicial; lcd.clear();}
        break; 

      case 2:

        memcpy(LCDMessage, "Llenar hasta:", 14);                      PrintLCD (LCDMessage,0,0);
        sprintf(LCDMessage, "%d%c",Nivel_a_setear,'%');               PrintLCD (LCDMessage,14,0);
        memcpy(LCDMessage, "Confirmar?", 11);                         PrintLCD (LCDMessage,5,3);

        if(PressedButton(3)){lcd.clear(); Flag=3;}
        if(PressedButton(4)){ Flag=1; lcd.clear();}
        break;

      case 3:
        guardado_para_menus(true);
        return Nivel_a_setear;
        break;
    }
}

uint8_t menu_de_calefaccion_manual(int8_t valor_temperatura_actual,bool Unidad_medida, uint8_t Sumador_encoder){
    switch (Flag)
    {
      case 0:
        if (min_temp>valor_temperatura_actual) Temp_a_setear=min_temp;
        else  Temp_a_setear = valor_temperatura_actual;
        lcd.clear();
        encoder_value(0,1);
        Flag=1;
        encoder_value(0,1);
        break;

      case 1:

        memcpy(LCDMessage, "Calentar a", 11);             PrintLCD (LCDMessage,0,0);
        memcpy(LCDMessage, "Sumar 5 con 1", 14);          PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "Restar 5 con 2", 15);         PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);        PrintLCD (LCDMessage,0,3);

        if(Unidad_medida == false)sprintf(LCDMessage, "%d%cC",Temp_a_setear,(char)223);
        if(Unidad_medida == true)sprintf(LCDMessage, "%d%cF",((9*Temp_a_setear)/5)+32,(char)223);
        PrintLCD (LCDMessage,12,0);

        encoder_value(9,4);
        if((Sumador_encoder+8)*sumador_temperatura!=Temp_a_setear){
          Temp_a_setear=(Sumador_encoder+8)*sumador_temperatura;
        }

        if (PressedButton(1))encoder_value(1,2); // suma 1 a Ypos
        if (PressedButton(2))encoder_value(1,3); // resta 1 a Ypos
        if(PressedButton(3)){ Flag=2; lcd.clear();}
        if(PressedButton(4)){ Estadoequipo=menu1; Flag=0; funcionActual=posicion_inicial; lcd.clear();}
        break; 

      case 2:
        memcpy(LCDMessage, "Calentar a", 11);                                     PrintLCD (LCDMessage,0,0);
        memcpy(LCDMessage, "Confirmar?", 11);                                     PrintLCD (LCDMessage,5,3);

        if(Unidad_medida == false)sprintf(LCDMessage, "%d%cC",Temp_a_setear,(char)223);
        if(Unidad_medida == true)sprintf(LCDMessage, "%d%cF",((9*Temp_a_setear)/5)+32,(char)223);
        PrintLCD (LCDMessage,12,0);

        if(PressedButton(3)){Flag=3; lcd.clear();}
        if(PressedButton(4)){Flag=1; lcd.clear();}
        break;

      case 3:
        guardado_para_menus(true);
        return Temp_a_setear;
        break;
    }
}

uint8_t ActualSlot; 
void menu_de_auto_por_hora(uint8_t hora_actual, uint8_t minutos_actual,bool Unidad_medida,uint8_t Sumador_encoder)
{
  switch (Flag)
  {
    case 0:
      hora_to_modify=hora_actual;
      minuto_to_modify=minutos_actual;
      lcd.clear();
      encoder_value(0,1);
      Flag=1;
      break;

    case 1:
      memcpy(LCDMessage, "Seleccionar:", 13);                                     PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "%d slot",ActualSlot+1);                                PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "1:", 20);                                               PrintLCD (LCDMessage,0,1);
      Printhora(LCDMessage,CharToUINT(1,eep.read(1)),CharToUINT(2,eep.read(1)));  PrintLCD (LCDMessage,3,1);
      memcpy(LCDMessage, "2:", 20);                                               PrintLCD (LCDMessage,0,2);
      Printhora(LCDMessage,CharToUINT(1,eep.read(4)),CharToUINT(2,eep.read(4)));  PrintLCD (LCDMessage,3,2);
      memcpy(LCDMessage, "3:", 20);                                               PrintLCD (LCDMessage,0,3);
      Printhora(LCDMessage,CharToUINT(1,eep.read(8)),CharToUINT(2,eep.read(8)));  PrintLCD (LCDMessage,3,3);

      ActualSlot = Sumador_encoder/4;
      encoder_value(12,4);

      if (PressedButton(1))encoder_value(4,2); // suma 1 a Ypos
      if (PressedButton(2))encoder_value(4,3); // resta 1 a Ypos
      if(PressedButton(3)){Temp_a_setear=min_temp; Nivel_a_setear=min_nivel; lcd.clear(); Flag=2;encoder_value(0,1);}
      if(PressedButton(4)){Estadoequipo=menu1; Flag=0; funcionActual=posicion_inicial; lcd.clear();encoder_value(0,1);}
      break; 

    case 2:        
      memcpy(LCDMessage, "Temp. max:", 11);          PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);

      if(Unidad_medida == false)sprintf(LCDMessage, "%d%cC",Temp_a_setear,(char)223);
      if(Unidad_medida == true)sprintf(LCDMessage, "%d%cF",((9*Temp_a_setear)/5)+32,(char)223);
      PrintLCD (LCDMessage,12,0);

      encoder_value(9,4);
      if((Sumador_encoder+8)*sumador_temperatura!=Temp_a_setear){
        Temp_a_setear=(Sumador_encoder+8)*sumador_temperatura;
      }

      if (PressedButton(1))encoder_value(1,2);
      if (PressedButton(2))encoder_value(1,3);
      if(PressedButton(3)){Flag=3;  lcd.clear(); encoder_value(0,1);}
      if(PressedButton(4)){Flag=1;  lcd.clear(); encoder_value(0,1);}
      break;

    case 3:
      memcpy(LCDMessage, "Nivel max", 20);          PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "%d%c",Nivel_a_setear,'%');       PrintLCD (LCDMessage,10,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);

      encoder_value(4,4);
      if((Sumador_encoder+1)*sumador_nivel!=Nivel_a_setear){
        Nivel_a_setear=(Sumador_encoder+1)*sumador_nivel;
      }

      if (PressedButton(1))encoder_value(1,2);
      if (PressedButton(2))encoder_value(1,3);
      if(PressedButton(3)){Flag=4;Sumador_encoder=0;lcd.clear();encoder_value(0,1);}
      if(PressedButton(4)){Flag=2; lcd.clear();encoder_value(0,1);}
      break;

    case 4:
      memcpy(LCDMessage, "Setear hora:", 20);                                    PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 20);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 20);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                                 PrintLCD (LCDMessage,0,3);

      encoder_value(hora_max,4);
      hora_to_modify = ReturnToCero((hora_actual+Sumador_encoder),hora_max);

      if (PressedButton(1))encoder_value(1,2);
      if (PressedButton(2))encoder_value(1,3);
      if(PressedButton(3)){Flag=5; lcd.clear(); Sumador_encoder=0;encoder_value(0,1);}
      if(PressedButton(4)){Flag=3; lcd.clear(); Sumador_encoder=0;encoder_value(0,1);}
      break;

    case 5:
      memcpy(LCDMessage, "Setear min:", 20);                                     PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,13,0);
      memcpy(LCDMessage, "aumentar con 1", 20);                                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "disminuir con 2", 20);                                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                                 PrintLCD (LCDMessage,0,3);

      encoder_value(minuto_max,4);
      minuto_to_modify = ReturnToCero((minutos_actual+Sumador_encoder),minuto_max);

      if (PressedButton(1))encoder_value(1,2); // suma 1 a Ypos
      if (PressedButton(2))encoder_value(1,3);// resta 1 a Ypos
      if(PressedButton(3)){Flag=6; lcd.clear();encoder_value(0,1);}
      if(PressedButton(4)){Flag=4; lcd.clear();encoder_value(0,1);}
      break;

    case 6:
      memcpy(LCDMessage, "A las:",20);                                          PrintLCD (LCDMessage,0,0);
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                    PrintLCD (LCDMessage,7,0);
      memcpy(LCDMessage, "Calentar:",20);                                      PrintLCD (LCDMessage,0,1);

      sprintf(LCDMessage, "Llenar: %d%c",Nivel_a_setear,'%');                        PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar?",20);                                      PrintLCD (LCDMessage,5,3);

      if(Unidad_medida == false) {sprintf(LCDMessage, "%d%cC",Temp_a_setear,(char)223);}
      if(Unidad_medida == true) {sprintf(LCDMessage, "%d%cF",((9*Temp_a_setear)/5)+32,(char)223);}
      PrintLCD (LCDMessage,11,1);

      if(PressedButton(3)){lcd.clear(); Flag=7;encoder_value(0,1);}
      if(PressedButton(4)){Flag=5; lcd.clear();encoder_value(0,1);}
      break;

    case 7:
      Printhora(LCDMessage,hora_to_modify,minuto_to_modify); 
      eep.write((ActualSlot*3)+1,ArrayToChar(1,LCDMessage));
      eep.write((ActualSlot*3)+2, Nivel_a_setear);
      eep.write((ActualSlot*3)+3, Temp_a_setear);

      ActualSlot=0;
      guardado_para_menus(true);
      break;
  }
}

uint8_t Valor_guardado_inicial,Valor_guardado_final;

void menu_de_llenado_auto(uint8_t Sumador_encoder)
{
  switch (Flag)
  {
    case 0:
      Valor_guardado_inicial=eep.read(12);
      Valor_guardado_final=eep.read(13);
      eep.write(12,min_nivel);
      encoder_value(0,1);
      lcd.clear();
      Flag=1;
      break;

    case 1:
      memcpy(LCDMessage, "Nivel min:", 20);                  PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "%d%c",eep.read(12),'%');             PrintLCD (LCDMessage,11,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                PrintLCD (LCDMessage,0,3);

      encoder_value(3,4);
      if((Sumador_encoder+1)*sumador_nivel!=eep.read(12)){
        eep.write(12,(Sumador_encoder+1)*sumador_nivel);
      }

      if(PressedButton(1)==true)encoder_value(1,2);
      if(PressedButton(2)==true)encoder_value(1,3);
      if(PressedButton(3)==true){Flag=2;eep.write(13,eep.read(12)+sumador_nivel);lcd.clear(); encoder_value(0,1);}
      if(PressedButton(4)==true){eep.write(12,Valor_guardado_inicial); eep.write(13,Valor_guardado_final); Estadoequipo=menu1; Flag=0; funcionActual=posicion_inicial; lcd.clear(); encoder_value(0,1);}
      break;
      
    case 2:
      memcpy(LCDMessage, "Nivel max:", 20);                     PrintLCD (LCDMessage,0,1);
      sprintf(LCDMessage, "%d%c",eep.read(13),'%');             PrintLCD (LCDMessage,11,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);                  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);                 PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);                PrintLCD (LCDMessage,0,3);

      encoder_value(eep.read(12)/sumador_nivel,4);
      if((Sumador_encoder+1)*sumador_nivel!=eep.read(13)){
        eep.write(12,(Sumador_encoder+1)*sumador_nivel);
      }

      if(PressedButton(1)==true)encoder_value(1,2);
      if(PressedButton(2)==true)encoder_value(1,3);
      if(PressedButton(3)){Flag=3;  lcd.clear(); encoder_value(0,1);}
      if(PressedButton(4)){Flag=1;  lcd.clear(); encoder_value(0,1);}
      break;

    case 3: 
      sprintf(LCDMessage, "Al llegar a %d%c",eep.read(12),'%');   PrintLCD (LCDMessage,0,0);
      sprintf(LCDMessage, "LLenar hasta %d%c",eep.read(13),'%');  PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Confirmar?", 20);                       PrintLCD (LCDMessage,5,3);

      if(PressedButton(3)){Flag=4;lcd.clear(); encoder_value(0,1);}
      if(PressedButton(4)){Flag=2;lcd.clear(); encoder_value(0,1);}
      break;

    case 4:
      guardado_para_menus(true);
      Flag=0;
      break; 
  }
}

void menu_de_calefaccion_auto(bool Unidad_medida,uint8_t Sumador_encoder){
  switch (Flag)
  {
    case 0:
      Valor_guardado_inicial=eep.read(10);
      Valor_guardado_inicial=eep.read(11);
      eep.write(10,min_temp);
      encoder_value(0,1);
      lcd.clear();
      Flag=1;
      break;

    case 1:
      memcpy(LCDMessage, "Temp. min:", 20);             PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);

      if(Unidad_medida == false)sprintf(LCDMessage, "%d%cC",eep.read(10),(char)223);
      if(Unidad_medida == true)sprintf(LCDMessage, "%d%cF",((9*eep.read(10))/5)+32,(char)223);
      PrintLCD (LCDMessage,12,0);

      encoder_value(3,4);
      if((Sumador_encoder+1)*sumador_temperatura!=eep.read(12)){
        eep.write(12,(Sumador_encoder+1)*sumador_temperatura);
      }

      if(PressedButton(1)==true)encoder_value(1,2);
      if(PressedButton(2)==true)encoder_value(1,3);
      if(PressedButton(3)){Flag=2;  eep.write(11,eep.read(10)+sumador_temperatura);encoder_value(0,1);}
      if(PressedButton(4)){eep.write(10,Valor_guardado_inicial);eep.write(11,Valor_guardado_inicial);Estadoequipo=menu1;Flag=0;funcionActual=posicion_inicial;lcd.clear();encoder_value(0,1);}
      break; 

    case 2:
      memcpy(LCDMessage, "Temp. max:", 20);             PrintLCD (LCDMessage,0,0);

      if(Unidad_medida == false)sprintf(LCDMessage, "%d%cC",eep.read(11),(char)223);
      if(Unidad_medida == true)sprintf(LCDMessage, "%d%cF",((9*eep.read(11))/5)+32,(char)223);
      PrintLCD (LCDMessage,12,0);

      memcpy(LCDMessage, "Sumar 5 con 1", 20);          PrintLCD (LCDMessage,0,1);
      memcpy(LCDMessage, "Restar 5 con 2", 20);         PrintLCD (LCDMessage,0,2);
      memcpy(LCDMessage, "Confirmar con 3", 20);        PrintLCD (LCDMessage,0,3);
  
      encoder_value(eep.read(10)/sumador_temperatura,4);
      if((Sumador_encoder+1)*sumador_nivel!=eep.read(11)){
        eep.write(11,(Sumador_encoder+1)*sumador_temperatura);
      }

      if(PressedButton(1))encoder_value(1,2);
      if(PressedButton(2))encoder_value(1,3);
      if(PressedButton(3)){Flag=3;  lcd.clear();encoder_value(0,1);}
      if(PressedButton(4)){Flag=1;  lcd.clear();encoder_value(0,1);}
      break;

    case 3:
      memcpy(LCDMessage, "A los:", 20);                                  PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Calentar a:", 20);                             PrintLCD (LCDMessage,0,1);

      if(Unidad_medida == false){
        sprintf(LCDMessage,"%d%cC",eep.read(10),(char)223);             PrintLCD (LCDMessage,7,0);
        sprintf(LCDMessage,"%d%cC",eep.read(11),(char)223);             PrintLCD (LCDMessage,12,1);
      }

      if(Unidad_medida == true){
        sprintf(LCDMessage, "%d%cF",((9*eep.read(10))/5)+32,(char)223);  PrintLCD (LCDMessage,8,0);
        sprintf(LCDMessage, "%d%cF",((9*eep.read(11))/5)+32,(char)223);  PrintLCD (LCDMessage,13,1);
      }
      
      memcpy(LCDMessage, "Confirmar con 3", 20);                         PrintLCD (LCDMessage,0,3);

      if(PressedButton(3)){Flag=4;  lcd.clear();encoder_value(0,1);}
      if(PressedButton(4)){Flag=2;  lcd.clear();encoder_value(0,1);}
        break;

    case 4:
      guardado_para_menus(true);
      Flag=0;
      break; 
  }
}

void menu_modificar_hora_rtc(uint8_t hora_actual, uint8_t minutos_actual, uint8_t Sumador_encoder)
  {
    switch (Flag)
    {
      case 0:
        hora_to_modify=hora_actual;
        minuto_to_modify=minutos_actual;
        Flag=1;
        lcd.clear();
        encoder_value(0,1);
        break;
      case 1:
        memcpy(LCDMessage, "Setear hora:", 15);                                    PrintLCD (LCDMessage,0,0);
        Printhora(LCDMessage,hora_to_modify,minutos_actual);                       PrintLCD (LCDMessage,13,0);
        memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

        hora_to_modify = ReturnToCero(hora_actual+Sumador_encoder,hora_max);
        encoder_value(hora_max,4);

        if(PressedButton(1))encoder_value(1,2);
        if(PressedButton(2))encoder_value(1,3);
        if(PressedButton(3)){Flag=2;  lcd.clear(); encoder_value(0,1);}
        if(PressedButton(4)){Estadoequipo=menu2; Flag=0; funcionActual=posicion_inicial; lcd.clear(); encoder_value(0,1);}
        break;          

      case 2:
        memcpy(LCDMessage, "Setear min:", 12);                                     PrintLCD (LCDMessage,0,0);
        Printhora(LCDMessage,hora_to_modify,minuto_to_modify);                     PrintLCD (LCDMessage,12,0);
        memcpy(LCDMessage, "aumentar con 1", 15);                                  PrintLCD (LCDMessage,0,1);
        memcpy(LCDMessage, "disminuir con 2", 16);                                 PrintLCD (LCDMessage,0,2);
        memcpy(LCDMessage, "Confirmar con 3", 16);                                 PrintLCD (LCDMessage,0,3);

        minuto_to_modify = ReturnToCero(minutos_actual+Sumador_encoder,minuto_max);
        encoder_value(minuto_max,4);

        if(PressedButton(1))encoder_value(1,2);
        if(PressedButton(2))encoder_value(1,3);
        if(PressedButton(3)){Flag=1;  lcd.clear(); encoder_value(0,1);}
        if(PressedButton(4)){Flag=3;  lcd.clear(); encoder_value(0,1);}
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
          encoder_value(0,1);
        }
        if(PressedButton(4)){Flag=2;  lcd.clear(); encoder_value(0,1);}
        break;

      case 4:
        guardado_para_menus(false);
        Flag=0;
        break; 
    }
  }

void menu_activar_bomba(bool Estado_bomba, uint8_t Sumador_encoder){
switch (Flag)
  {
    case 0:
      memcpy(LCDMessage, "Activar bomba", 14);                                    PrintLCD (LCDMessage,0,0);
      if(Estado_bomba){memcpy(LCDMessage, "Bomba Activada", 15);                 PrintLCD (LCDMessage,0,1);}
      if(!Estado_bomba){memcpy(LCDMessage, "Bomba Desactivada", 18);             PrintLCD (LCDMessage,0,1);}
      memcpy(LCDMessage, "1:Si", 5);                                              PrintLCD (LCDMessage,3,3);
      memcpy(LCDMessage, "2:No", 5);                                              PrintLCD (LCDMessage,11,3);



      if (Sumador_encoder>8)Estado_bomba=true;
      if (Sumador_encoder<=8)Estado_bomba=false;
      encoder_value(16,4);

      if (PressedButton(1))encoder_value(8,2); // suma 1 a Ypos
      if (PressedButton(2))encoder_value(8,3); // resta 1 a Ypos
      if(PressedButton(3)){Flag=1;  lcd.clear();}
    break;
    case 1:
      guardado_para_menus(false);
    break;
  }
  
}     

void menu_farenheit_celsius(bool Unidad_medida, uint8_t Sumador_encoder)
{
  switch (Flag)
  {
    case 0:
      memcpy(LCDMessage, "Cambiar unidad", 15);                                    PrintLCD (LCDMessage,0,0);
      memcpy(LCDMessage, "Unidad actual", 14);                                     PrintLCD (LCDMessage,0,1);

      if(Unidad_medida)sprintf(LCDMessage, "%cF",(char)223);          
      if(!Unidad_medida)sprintf(LCDMessage,"%cC",(char)223);                       
      PrintLCD (LCDMessage,15,1);

      sprintf(LCDMessage, "1:%cC",(char)223);           PrintLCD (LCDMessage,3,3);
      sprintf(LCDMessage, "2:%cF",(char)223);           PrintLCD (LCDMessage,13,3);

      if (Sumador_encoder>8)Unidad_medida=true;
      if (Sumador_encoder<=8)Unidad_medida=false;
      encoder_value(16,4);

      if (PressedButton(1))encoder_value(8,2); // suma 1 a Ypos
      if (PressedButton(2))encoder_value(8,3); // resta 1 a Ypos
      if(PressedButton(3)){Flag=1;lcd.clear();}
    break;

    case 1:
      guardado_para_menus(false);
    break;
  }
}
char nombre_wifi_setear [20];
char password_wifi_setear[20];
void menu_seteo_wifi(char nombre_wifi [20],char password_wifi [20],uint8_t Sumador_encoder){  

  switch (Flag)
  {
  case 0:
    memcpy(LCDMessage, "SSID:", 6);                                PrintLCD (LCDMessage,0,0);
    memcpy(LCDMessage, "PASS:", 6);                                PrintLCD (LCDMessage,0,1);
    sprintf(LCDMessage, "%s",nombre_wifi);                         PrintLCD (LCDMessage,6,0);
    sprintf(LCDMessage, "%s",password_wifi);                       PrintLCD (LCDMessage,6,1);
    memcpy(LCDMessage, "modificar?", 11);                          PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                 PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                 PrintLCD (LCDMessage,11,3);

    if(PressedButton(3)){lcd.clear();Flag=1;encoder_value(0,1);}
    if(PressedButton(4)){Estadoequipo=menu2;Flag=0;funcionActual=posicion_inicial;lcd.clear();}

    break;
  case 1:
    
    for (uint8_t Auxiliar0=0;Auxiliar0<=19;Auxiliar0++){ 
    nombre_wifi_setear[Auxiliar0]='\0';
    password_wifi_setear[Auxiliar0]='\0';
    }
    Aux=0;
    Actualchar=0;
    Flag=2;
    encoder_value(0,1);
    break;
  case 2:
    memcpy(LCDMessage, "Nombre Wifi:", 13);                                   PrintLCD (LCDMessage,0,0);
    sprintf(LCDMessage, "%s",nombre_wifi_setear);                                    PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "modificar?", 11);                                     PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                            PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                            PrintLCD (LCDMessage,11,3);

    Actualchar=Sumador_encoder/2;
    encoder_value(80,4);
    Aux=ReturnToCero(Aux,20);

    nombre_wifi_setear[Aux]=Character_Return(Actualchar, mayusculas);

    if(PressedButton(1))mayusculas=!mayusculas;
    if(PressedButton(2) == true && Aux <= 19){Aux++; Actualchar=0;}
    if(PressedButton(3)){
        nombre_wifi_setear[Aux]='\0';
        lcd.clear();
        Flag=3;
        Aux=0;
        Actualchar=0;
      }
    if(PressedButton(4)){
        nombre_wifi_setear[Aux]='\0';
        lcd.clear();
        Flag=0;
        Aux=0;
        Actualchar=0;
      }
    break;

  case 3:
    memcpy(LCDMessage, "Pass Wifi:", 11);                                     PrintLCD (LCDMessage,0,0);
    sprintf(LCDMessage, "%s",password_wifi_setear);                                       PrintLCD (LCDMessage,0,1);
    memcpy(LCDMessage, "modificar?", 11);                                     PrintLCD (LCDMessage,0,2);
    memcpy(LCDMessage, "3:Si", 5);                                            PrintLCD (LCDMessage,3,3);
    memcpy(LCDMessage, "4:No", 5);                                            PrintLCD (LCDMessage,11,3);

    Actualchar=Sumador_encoder/2;
    encoder_value(80,4);
    Aux=ReturnToCero(Aux,20);
    
    password_wifi_setear[Aux]=Character_Return(Actualchar, mayusculas);

    if(PressedButton(1))mayusculas=!mayusculas;
    if(PressedButton(2)){Aux ++;  Actualchar=0;}
    if(PressedButton(3)){
        password_wifi_setear[Aux]='\0';
        lcd.clear();
        Flag=4;
      }
    if(PressedButton(4)){
        nombre_wifi_setear[Aux]='\0';
        lcd.clear();
        Flag=2;
        Aux=0;
        Actualchar=0;
      }
    break;
  case 4:
    for (uint8_t i = 0; i < 19; i++){
    eep.write(14 + i, password_wifi_setear[i]);
    eep.write(34 + i, nombre_wifi_setear[i]);
    }
    Serial_Send_UNO(6,0);
    guardado_para_menus(false);
    break;
  }

}

void guardado_para_menus(bool Menu){
  memcpy(LCDMessage, "Guardando...", 13);                                   PrintLCD (LCDMessage,4,0);
  if(mili_segundos>=tiempo_de_standby+tiempo_de_espera_menu){
  if(Menu == true){
    Estadoequipo=menu1;
    Flag=0;
  }
  if(Menu == false){
    Estadoequipo=menu2;  
    Flag=0;
  }
  funcionActual=posicion_inicial;
  lcd.clear();
  tiempo_de_standby=mili_segundos;
  }
}