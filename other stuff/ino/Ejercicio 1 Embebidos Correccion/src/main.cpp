#include <Arduino.h>

volatile unsigned int cuenta = 0;
bool ESTADO = false;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int led = 5;
int info_estado = 0;
volatile unsigned int tiempo=0;

void setup() {
pinMode(led,OUTPUT);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
SREG = (SREG & 0b01111111); //Desabilitar interrupciones
TIMSK2 = TIMSK2|0b00000001; //Habilita la interrupcion por desbordamiento
TCCR2B = 0b00000111; //Configura preescala para que FT2 sea de 7812.5Hz
SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones //Desabilitar interrupciones
Serial.begin(9600);
}
void loop() {
  if(digitalRead(pulsador1) == HIGH)
{
  info_estado = 1;
  tiempo=15;
}

if(digitalRead(pulsador2) == HIGH)
{
  info_estado = 2;
  tiempo=8;
}

}
ISR(TIMER2_OVF_vect){
cuenta++;
if(cuenta>tiempo){
  digitalWrite(led, ESTADO);
  ESTADO = !ESTADO;
  cuenta = 0;
}
}