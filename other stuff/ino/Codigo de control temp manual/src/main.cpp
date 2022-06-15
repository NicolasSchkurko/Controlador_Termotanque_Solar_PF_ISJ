#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

int control_de_temp_por_menu_manual(int temp_a_alcanzar);

const int resistencia = 8;

const int onewire = 6;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 

void setup() {
  Sensor_temp.begin();
  pinMode(resistencia, OUTPUT);
}

void loop() {}

int control_de_temp_por_menu_manual(int temp_a_alcanzar){
  int umbral_de_temperatura = 5;
  int temperatura_actual = 0;
  if( temperatura_actual < temp_a_alcanzar + umbral_de_temperatura)
  {
    Sensor_temp.requestTemperatures();
    temperatura_actual = Sensor_temp.getTempCByIndex(0);
    digitalWrite(resistencia, HIGH);
  }
  if(temperatura_actual >= temp_a_alcanzar + umbral_de_temperatura)digitalWrite(resistencia, LOW);
}
  