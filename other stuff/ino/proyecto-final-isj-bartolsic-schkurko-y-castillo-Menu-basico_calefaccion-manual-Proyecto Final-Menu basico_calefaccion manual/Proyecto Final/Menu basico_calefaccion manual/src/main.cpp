#include <Arduino.h>
//#include <LiquidCrystal.h>
#include

const int pulsador6 = 0; //pulsador de retorno
const int pulsador5 = 1;
const int pulsador1 = 2;
const int pulsador2 = 3;
const int pulsador3 = 4;
const int pulsador4 = 5;
int i = 0;
bool jaja = true;

//int temperatura[10];

int final_temp = 0;

LiquidCrystal_I2C lcd(0x20,20,4);

//LiquidCrystal lcd(12,11,10,9,8,7);
void setup() 
{
  //lcd.begin(16,2);
  pinMode(pulsador1, INPUT_PULLUP);
  pinMode(pulsador2, INPUT_PULLUP);
  pinMode(pulsador3, INPUT_PULLUP);
  pinMode(pulsador4, INPUT_PULLUP);
  pinMode(pulsador5, INPUT_PULLUP);
  pinMode(pulsador6, INPUT_PULLUP);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  /*temperatura[0] = 0;
  temperatura[1] = 1;
  temperatura[2] = 2;
  temperatura[3] = 3;
  temperatura[4] = 4;
  temperatura[5] = 5;
  temperatura[6] = 6;
  temperatura[7] = 7;
  temperatura[8] = 8;
  temperatura[9] = 9;*/
}

void loop() 
{
  //while(1)
  //{
    lcd.print(final_temp);
    if(digitalRead(pulsador1) == LOW)
    {
      while(digitalRead(pulsador1) == LOW){}
      i++;
      final_temp = i*10;
    }
    if(digitalRead(pulsador2) == LOW)
    {
      while(digitalRead(pulsador2) == LOW){}
      lcd.setCursor(1,0);
      final_temp = final_temp + i;
    }
    if(digitalRead(pulsador5) == LOW)
    {
      while(digitalRead(pulsador5) == LOW){}
      lcd.print("Temp: ");
      lcd.setCursor(6,0);
      lcd.print(final_temp);
    }
  //}
}