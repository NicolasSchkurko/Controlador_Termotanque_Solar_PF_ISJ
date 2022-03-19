 #include <LiquidCrystal.h>
LiquidCrystal lcd (13, 12, 11, 10, 9, 8);
const int pulsador=3;
const int led=1;
int i=0;
void STOP();

void setup() {
lcd.begin(16,2);
pinMode (pulsador,INPUT);
pinMode (led,OUTPUT);
attachInterrupt(digitalPinToInterrupt(3),STOP,LOW);
}

void loop() {
  if(i==0)
 {
 lcd.setCursor(0,0);
 lcd.print("estoy en");
 lcd.setCursor(0,1);
 lcd.print("loop");
 digitalWrite(led,LOW);
 }
 if(i==1)
 {
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("estoy en");
 lcd.setCursor(0,1);
 lcd.print("interrupt");
 digitalWrite(led,HIGH);
 i=0;
 delay(1000);
 lcd.clear();
 }

}
void STOP(){
i=1;
}
