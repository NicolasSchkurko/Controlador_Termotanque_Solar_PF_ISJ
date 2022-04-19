// Nicolas Schkurko


#include <Arduino.h>
#define TIEMPO_CAMBIO_ESTADO 1000 //tiempo establecido para el cambio de velocidad

typedef enum{cincuentams,cienms}ESTADOMEF;

void Seleccion_velocidad_parpadeo(); // decide la velocidad de parpadeo a ejecutar

ESTADOMEF tipo_parpadeo = cincuentams;
volatile unsigned int ciclos_timer2=0; //contador ciclos totales utilizados
volatile unsigned int cambio_estado=0;  //contador ciclos solo del "parpadeo" LED
volatile unsigned int tiempo_estado_led=0; //Tiempo encedido/apagado del LED
bool estado_led = false;
bool velocidad_parpadeo = false;

const int led = 5;

void setup() {
pinMode(led,OUTPUT);
SREG = (SREG & 0b01111111); //Desabilitar interrupciones
TIMSK2 = TIMSK2|0b00000001; //Habilita la interrupcion por desbordamiento
TCCR2B = 0b00000011; //Configura preescala para que FT2 sea de 7812.5Hz
SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones //Desabilitar interrupciones
}

void loop() {
  Seleccion_velocidad_parpadeo(); 
    switch (tipo_parpadeo) // establece tiempo parpadeo
  {
  case cienms:
      tiempo_estado_led=100;
    break;
  
  case cincuentams:
      tiempo_estado_led=50;
    break;
  }
}


void Seleccion_velocidad_parpadeo(){
if (ciclos_timer2>TIEMPO_CAMBIO_ESTADO) //compara el timer con el tiempo establecido (1s) y ejecuta el codigo
  {
  switch (tipo_parpadeo) //cambia velocidades de parpadeo segun el tiempo establecido
    {
    case cienms:
        tipo_parpadeo=cincuentams;
      break;
  
    case cincuentams:
       tipo_parpadeo=cienms;
     break;
    }
  ciclos_timer2 = 0; // reincia el contador para el siguiente cambio
  }

}


ISR(TIMER2_OVF_vect){
ciclos_timer2++;
cambio_estado++;
if(cambio_estado>tiempo_estado_led) //prende y apaga el led
  {
  digitalWrite(led, estado_led);
  estado_led = !estado_led;
  cambio_estado = 0;
  }
}
