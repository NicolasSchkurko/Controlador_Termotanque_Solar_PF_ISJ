#include <Arduino.h>

int control_de_nivel_por_menu_manual(int nivel_a_alcanzar);

typedef enum{tanque_vacio,tanque_al_25, tanque_al_50, tanque_al_75, tanque_al_100} niveles; 
niveles nivel;//misma MEF que para la deteccion de nivel


void setup() { }

void loop() {}

int control_de_temp_por_menu_manual(niveles nivel_para_men_manual;){
  int nivel_actual = 0;
  if( nivel_para_men_manual < nivel_a_alcanzar)
  {
    nivel_actual = Sensor_temp.getTempCByIndex(0);
    digitalWrite(, HIGH);
  }
  if(nivel_para_men_manual >= nivel_a_alcanzar)digitalWrite(, LOW);
}
  