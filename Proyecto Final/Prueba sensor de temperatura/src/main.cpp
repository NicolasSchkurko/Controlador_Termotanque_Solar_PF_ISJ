#include <Arduino.h>
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DallasTemperature.h>

LiquidCrystal lcd(12,11,10,9,8,7);

const int sensor = 4;

OneWire onewire(sensor);

DallasTemperature sensor_temp(&onewire);

void setup() 
{
  lcd.begin(16,2);
  lcd.print("Prueba temp");
  sensor_temp.begin();
}

void loop() 
{
  lcd.clear();
  sensor_temp.requestTemperatures();
  lcd.print(sensor_temp.requestTemperaturesByIndex(0));
}