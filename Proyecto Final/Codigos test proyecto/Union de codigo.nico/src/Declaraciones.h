#include <Arduino.h>
#define encoder0PinA 2
#define encoder0PinB 3
#define nivel_del_tanque A3
#define onewire 9 // pin del onewire
#define tiempo_para_temperatura 5000
#define tiempo_para_nivel 3000
#define maxY_menu1 7
#define maxY_menu2 5
#define tiempo_de_parpadeo 700
#define onewire 9 // pin del onewire
#define tiempo_para_temperatura 5000
#define tiempo_para_nivel 3000
#define sumador_temperatura 5
#define maxi_cast 80
#define min_temp 40
#define sumador_nivel 25
#define min_nivel 25
#define max_nivel 100
#define hora_max 24
#define minuto_max 60
#define tiempo_de_espera_menu 3000 

void standby(bool);
void menu_basico();
void menu_avanzado();
void guardado_para_menus(bool);
void Serial_Read_UNO();
void Serial_Send_UNO(uint8_t, uint8_t);
String String_de_hora(uint8_t, uint8_t);
uint8_t CharToUINT(uint8_t, uint8_t);
int8_t ArrayToChar(uint8_t, char[20]);
uint8_t ReturnToCero(int8_t, uint8_t);
char Character_Return(uint8_t, bool);
bool PressedButton(uint8_t);
void PrintLCD(char[20], uint8_t, uint8_t);
void Printhora(char[20], uint8_t, uint8_t);
void PrintOutput(uint8_t, bool);
void encoder_value(uint8_t, uint8_t);
uint8_t menu_de_llenado_manual();
uint8_t menu_de_calefaccion_manual( bool);
void menu_de_auto_por_hora(uint8_t, uint8_t, bool);
void menu_de_llenado_auto();
void menu_de_calefaccion_auto(bool);
void menu_modificar_hora_rtc();
void menu_activar_bomba(bool);
void menu_farenheit_celsius(bool);
void menu_seteo_wifi();
void guardado_para_menus(bool);
void Controltemp();
void Controllvl();
void ControlPorHora();
void Actualizar_entradas();
void Sum_Encoder();
void doEncodeA();
void doEncodeB();