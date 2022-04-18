#include <Arduino.h>
#define TIEMPO_CAMBIO_ESTADO 1000

typedef enum{cincuentams,cienms}ESTADOMEF;

ESTADOMEF tipo_intermitente= cincuentams;
volatile unsigned int ciclos_timer2=0;
volatile unsigned int tiempo_estado_led=0;
bool estado_led = false;
float Verificacion_tipo_ciclo=0;


const int pulsador1 = 2;
const int pulsador2 = 3;
const int led = 5;

void setup() {
pinMode(led,OUTPUT);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
SREG = (SREG & 0b01111111); //Desabilitar interrupciones
TIMSK2 = TIMSK2|0b00000001; //Habilita la interrupcion por desbordamiento
TCCR2B = 0b00000011; //Configura preescala para que FT2 sea de 7812.5Hz
SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones //Desabilitar interrupciones
Serial.begin(9600);
}
void loop() {
  switch (tipo_intermitente)
  {
  case cienms:
      tiempo_estado_led=500;
      Verificacion_tipo_ciclo=(tiempo_estado_led%TIEMPO_CAMBIO_ESTADO)*100;
      if (ciclos_timer2>=TIEMPO_CAMBIO_ESTADO) //ex 26
      {
        tipo_intermitente = cincuentams;
        ciclos_timer2 = 0;
      }
    break;
  
  case cincuentams:
      tiempo_estado_led=50;
      Verificacion_tipo_ciclo=(tiempo_estado_led%TIEMPO_CAMBIO_ESTADO)*100;

      if (ciclos_timer2>=TIEMPO_CAMBIO_ESTADO) //ex 26
      {
        tipo_intermitente = cienms;
        ciclos_timer2 = 0;
      }
    break;
  }
}

ISR(TIMER2_OVF_vect){
ciclos_timer2++;
if(Verificacion_tipo_ciclo==(tiempo_estado_led%ciclos_timer2)*100){
  digitalWrite(led, estado_led);
  estado_led = !estado_led;
}
}