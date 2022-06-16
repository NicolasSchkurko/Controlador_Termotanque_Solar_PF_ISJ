#include <Arduino.h>

void nivel_manual();
void nivel_auto();
void cambio_de_variable_a_nivel();
void sensar_nivel();

const int nivel_del_tanque = A0;\
const int electrovalvula = 10;
int valor_seteado = 50;

typedef enum{carga_automatica, carga_manual} carga; 
carga tipo_de_carga = carga_automatica;

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_actual;
niveles nivel_seteado;

void setup() {
  pinMode(nivel_del_tanque, INPUT);
}

void loop() {
  sensar_nivel();
  switch (tipo_de_carga)
  {
  case carga_automatica:
    nivel_auto();
    break;
  case carga_manual:
    nivel_manual();
    break;
  }
}

void nivel_auto (){
  cambio_de_variable_a_nivel();
  if (nivel_actual <= tanque_al_25)    digitalWrite(electrovalvula, HIGH);
  if (nivel_actual == tanque_al_100)    digitalWrite(electrovalvula, LOW);
}

void nivel_manual(){
  cambio_de_variable_a_nivel();
  if (nivel_seteado != nivel_actual)    digitalWrite(electrovalvula, HIGH);
  if (nivel_seteado == nivel_actual )
  {
    digitalWrite(electrovalvula, LOW);
    nivel_seteado = tanque_vacio;
  }
}

void cambio_de_variable_a_nivel(){
  if(valor_seteado == 0)    nivel_seteado = tanque_vacio;
  if(valor_seteado == 25)    nivel_seteado = tanque_al_25;
  if(valor_seteado == 50)    nivel_seteado = tanque_al_50;
  if(valor_seteado == 75)    nivel_seteado = tanque_al_75;
  if(valor_seteado == 100)    nivel_seteado = tanque_al_100;
}

void sensar_nivel(){
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
}