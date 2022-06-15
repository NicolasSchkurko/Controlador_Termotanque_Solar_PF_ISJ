#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,10,9,8,7);
int Analog0 = 0;
float tension = 0;
void setup() 
{
  lcd.begin(16,2);  
}

void loop() 
{
  Analog0 = analogRead(A0);
  tension = Analog0*5.0/1023;
  lcd.home();
  lcd.print("Analog: ");
  lcd.setCursor(8,0);
  lcd.print(Analog0);
  lcd.setCursor(0,1);
  lcd.print("Tension: ");
  lcd.setCursor(9,1);
  lcd.print(tension);
  if (Analog0==993){
  lcd.setCursor(8,0);
  lcd.print("      ");
  }
    if (Analog0==92){
  lcd.setCursor(8,0);
  lcd.print("      ");
  }
    if (Analog0==9){
  lcd.setCursor(8,0);
  lcd.print("      ");
  }
}
