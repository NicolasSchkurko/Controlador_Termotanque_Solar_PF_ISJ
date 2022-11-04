#include <Arduino.h>
#define F_CPU 16000000UL
#define SENSOR_NIVEL A2
#define TIEMPO_LECTURA_TEMP 3000
#define TIEMPO_LECTURA_NIVEL 3000
#define COLUMNAS_MAXIMAS_M1 7
#define COLUMNAS_MAXIMAS_M2 5
#define SENSOR_TEMP A1 // pin del SENSOR_TEMP
#define SUMADOR_TEMP 5
#define TEMP_MINIMO 40
#define SUMADOR_NIVEL 25
#define NIVEL_MINIMO 25
#define HORA_MAX 24
#define MINUTO_MAX 60
#define TIEMPO_ESPERA_MENUES 8000 
#define TIEMPO_ESPERA_FUNCIONES 15000
#define TIEMPO_ESPERA_GUARDADO 3000

void    Menu_Avanzado();
void    Menu_Basico();
void    Standby();

void    Enviar_Serial(uint8_t, uint8_t);
void    Leer_Serial();

void    Barra_de_carga(uint8_t, uint8_t, int16_t);
int16_t Celcius_O_Farenheit(int8_t,uint8_t);
void    Imprimir_Hora(char[20], uint8_t, uint8_t);
uint8_t Guardado_a_hora(uint8_t, uint8_t);
void    Imprimir_LCD(char[20], uint8_t, uint8_t);
uint8_t Volver_a_Cero(uint8_t,uint8_t);
char    Retorno_Caracter(uint8_t, bool);
int8_t  Hora_a_guardado(char[20]);
void    Pin_Salida(uint8_t, bool);
bool    Pin_Entrada(uint8_t);

void    Actualizar_entradas();
void    Actualizar_salidas();
void    EncoderPinA();
void    EncoderPinB();

void    menu_modificar_hora_rtc(uint8_t,uint8_t);
void    menu_de_auto_por_hora(uint8_t, uint8_t);
uint8_t menu_de_calefaccion_manual();
uint8_t menu_de_llenado_manual();
void    menu_de_calefaccion_auto();
void    guardado_para_menus(bool);
void    menu_farenheit_celsius();
void    menu_de_llenado_auto();
void    menu_activar_bomba();
void    menu_seteo_wifi();




