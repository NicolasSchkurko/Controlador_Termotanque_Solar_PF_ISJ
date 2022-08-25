#include <Arduino.h>

#define sumador_temperatura 5 
#define maxi_cast 80           
#define min_temp 40    
#define sumador_nivel 25
#define min_nivel 25   
#define max_nivel 100 
#define hora_max 24
#define minuto_max 60

void menu_de_llenado_manual(uint8_t,uint8_t);
void menu_de_calefaccion_manual(int8_t,uint8_t,bool,uint8_t);
void menu_de_auto_por_hora(uint8_t,uint8_t,bool,uint8_t);
void menu_de_llenado_auto(uint8_t);
void menu_de_calefaccion_auto(bool,uint8_t);

void menu_modificar_hora_rtc(uint8_t, uint8_t, uint8_t);
void menu_activar_bomba(bool,uint8_t);
void menu_farenheit_celsius(bool,uint8_t);
void menu_seteo_wifi();

void guardado_para_menus(bool);