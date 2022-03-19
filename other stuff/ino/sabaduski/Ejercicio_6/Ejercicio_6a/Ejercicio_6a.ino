#include <LiquidCrystal.h>
int up=2;
int down=1;
int val=0;
LiquidCrystal lcd (12,11,10,9,8,7);
void setup() {
  lcd.begin(16,2);
  pinMode(up, INPUT);
  pinMode(down, INPUT);
}

void loop() {
  lcd.setCursor(0,1);
  lcd.print(val);
  if(digitalRead(up)==HIGH){
    val++;
    delay(250);
  }
    if(digitalRead(down)==HIGH){
    val=val-1;
    delay(250);
  }
  if(val>=100){
    lcd.clear();
    val=99;
  }
   if(val<=-1){
    val=0;
  }
   if(val==9){
   lcd.clear();
   }
  }

  
 
