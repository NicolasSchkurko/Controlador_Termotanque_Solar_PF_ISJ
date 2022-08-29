#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,20,4);

void nivel_auto();
void nivel_manual();
void cambio_de_variable_a_nivel();
void sensar_nivel();

const int nivel_del_tanque = A0;
const int electrovalvula = 10;
int nivel = 50;

typedef enum{carga_automatica, carga_manual} carga; 
carga tipo_de_carga = carga_manual;

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_actual;
niveles nivel_seteado;

void setup() {
  pinMode(nivel_del_tanque, INPUT);
  pinMode(electrovalvula, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  sensar_nivel();
  nivel_auto();
  Serial.print("Nivel:");
  Serial.println(nivel_actual);
}

void nivel_auto (){
  if (nivel_actual <= tanque_al_25)    digitalWrite(electrovalvula, HIGH);
  if (nivel_actual == tanque_al_100)    digitalWrite(electrovalvula, LOW);
}

void sensar_nivel(){
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
}