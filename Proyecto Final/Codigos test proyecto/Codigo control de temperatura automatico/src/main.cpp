#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// este es un menu de vefiracion donde al confirmanr
// se debe enviar la infomacion guardada para que se ponga la resistencia a calentar el agua

void control_de_temp_auto(int temperatura_inicial,int temperatura_final);

const int resistencia = 8;
int temperatura_inicial = 40;
int temperatura_final = 40;
int min_temp_ini = 40;
int maxima_temp_fin = 80;
bool confirmar = false;

LiquidCrystal_I2C lcd(0x27,16,2);

const int onewire = 6;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t); 

void setup() {
  pinMode(resistencia, OUTPUT);
}

void loop() {
  control_de_temp_auto(temperatura_inicial,temperatura_final);
}


void control_de_temp_auto(int temp_a_alcanzar,int temp_minima){
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
