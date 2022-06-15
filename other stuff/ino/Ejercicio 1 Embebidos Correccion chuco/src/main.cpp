#include <Arduino.h>

#define TIEMPO_VELOCIDAD_1 500 //tiempo en ms
#define TIEMPO_VELOCIDAD_2 50 //tiempo en ms

typedef enum{velocidaduno,velocidaddos}ESTADOMEF;

void Parpadeo();
void Calc_tiempo();

ESTADOMEF info_estado = velocidaduno;
volatile unsigned int cuenta = 0;
volatile unsigned int tiempo=0;
bool ESTADO = false;
char tiempo1=0;
char tiempo2=0;

const int pulsador1 = 2;
const int pulsador2 = 3;
const int led = 5;

void setup() {
Calc_tiempo();
pinMode(led,OUTPUT);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
SREG = (SREG & 0b01111111); //Desabilitar interrupciones
TIMSK2 = TIMSK2|0b00000001; //Habilita la interrupcion por desbordamiento
TCCR2B = 0b00000111; //Configura preescala para que FT2 sea de 7812.5Hz
SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones //Desabilitar interrupciones
}

void loop() {
Parpadeo(); 
  if(digitalRead(pulsador1) == HIGH)
{
  info_estado = velocidaduno;
}

if(digitalRead(pulsador2) == HIGH)
{
  info_estado = velocidaddos;
}
}

void Parpadeo(){
switch (info_estado) //cambia velocidades de parpadeo 
    {
    case velocidaddos:
        tiempo=tiempo1;
      break;
  
    case velocidaduno:
       tiempo=tiempo2;
     break;
    }; 

if(cuenta>tiempo) //enciende/apaga LED segun tiempo establecido
{
  digitalWrite(led, ESTADO);
  ESTADO = !ESTADO;
  cuenta = 0;
}
}

void Calc_tiempo(){
tiempo1=TIEMPO_VELOCIDAD_1/37,7;
tiempo2=TIEMPO_VELOCIDAD_2/37,7;
}

ISR(TIMER2_OVF_vect){
cuenta++;
}