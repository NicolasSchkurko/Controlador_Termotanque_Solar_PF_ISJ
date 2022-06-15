#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#define  onewire 6
float temperatura = 0;
float verificacion = 0;
int cuenta = 0;
bool time_out = false;
LiquidCrystal_I2C lcd(0x27,20,4);

OneWire sensor_t(onewire);

DallasTemperature Sensor_temp(&sensor_t); 


void setup() 
{
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  Serial.begin(9600);
  Sensor_temp.begin();
  lcd.init();
  lcd.backlight();
  Serial.println();
}

void loop() 
{
  lcd.setCursor(0,0);
  lcd.print(cuenta);
  if (time_out)
  {
    Sensor_temp.requestTemperatures();
    temperatura = Sensor_temp.getTempCByIndex(0);
    if (temperatura !=  -127.00){
      Serial.println("temperatura:");
      Serial.println(temperatura);
      lcd.setCursor(6,2);
      lcd.print(temperatura);
      Serial.println();
    }
    time_out=false;
  }
}

ISR(TIMER2_OVF_vect){
  cuenta++;
  if (cuenta > 201)
  {
    cuenta = 0;
    time_out=true;
  }
}