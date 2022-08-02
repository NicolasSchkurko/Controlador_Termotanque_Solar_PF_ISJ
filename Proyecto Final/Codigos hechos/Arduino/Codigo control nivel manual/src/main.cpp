#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,20,4);

void nivel_manual();
void nivel_auto();
void cambio_de_numero_a_maquina_de_estado();
void sensar_nivel();

const int nivel_del_tanque = A0;
const int electrovalvula = 10;
int nivel = 75;
bool escribir = false;

typedef enum{carga_automatica, carga_manual} carga; 
carga tipo_de_carga = carga_manual;

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel_actual;
niveles nivel_seteado;

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(nivel_del_tanque, INPUT);
  pinMode(electrovalvula, OUTPUT);
}

void loop() {
  sensar_nivel();
  nivel_manual();
  if (escribir == false)
  {
    lcd.setCursor(0,0);
    lcd.print("Llenando tanque");
  }

  if (escribir == true)
  {
    lcd.setCursor(0,0);
    lcd.print("Tanque lleno         ");
  }
  
}

void nivel_manual(){
  cambio_de_numero_a_maquina_de_estado();
  if (nivel_seteado > nivel_actual){
    digitalWrite(electrovalvula, HIGH); 
    escribir = false;
  }
  
  if (nivel_seteado <= nivel_actual)
  {
    escribir = true;
    digitalWrite(electrovalvula, LOW);
    nivel_seteado = tanque_vacio;
  }
}

void cambio_de_numero_a_maquina_de_estado(){
  if(nivel == 0)    nivel_seteado = tanque_vacio;
  if(nivel == 25)    nivel_seteado = tanque_al_25;
  if(nivel == 50)    nivel_seteado = tanque_al_50;
  if(nivel == 75)    nivel_seteado = tanque_al_75;
  if(nivel == 100)    nivel_seteado = tanque_al_100;
}

void sensar_nivel(){
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)    nivel_actual = tanque_al_25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)    nivel_actual = tanque_al_50;
    if (analogRead(nivel_del_tanque) >=512  && analogRead(nivel_del_tanque) < 768)    nivel_actual = tanque_al_75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)    nivel_actual = tanque_al_100;
    if (analogRead(nivel_del_tanque) < 100) nivel_actual = tanque_vacio;  
}