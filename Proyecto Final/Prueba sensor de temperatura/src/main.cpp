#include <Arduino.h>
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>

LiquidCrystal lcd(12,11,10,9,8,7);

const int sensor = 4;

OneWire onewire(sensor);

DallasTemperature sensor_temp(&onewire);

void setup() 
{
  lcd.begin(16,2);
  sensor_temp.begin();
}

void loop() 
{
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.setCursor(6,0);
  sensor_temp.requestTemperatures();
  lcd.print(sensor_temp.getTempCByIndex(0));
  lcd.setCursor(11,0);
  lcd.print(" C");
}