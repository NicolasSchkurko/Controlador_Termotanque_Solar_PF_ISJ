#include <LiquidCrystal.h>
const int b5=5;
const int b4=4;
const int b3=3;
const int b2=2;
const int b1=1;
int val1=0;
int val2=0;
int val3=0;
int val4=0;
int aux=0;
int retardo=0;
int errores=0;
unsigned long t1;
unsigned long t2;
LiquidCrystal lcd (12,11,10,9,8,7);

void setup() {
  lcd.begin(16,2);
  pinMode(b1, INPUT);
  pinMode(b2, INPUT);
  pinMode(b3, INPUT);
  pinMode(b4, INPUT);
  pinMode(b5, INPUT);
}

void loop() {
  

    if(digitalRead(b5)==HIGH and val1==4 and val2==2 and val3==3 and val4==4 and aux==1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("ganaste");
          delay(5000);
    }
        if(digitalRead(b5)==HIGH and val1!=4 and val2!=2 and val3!=3 and val4!=4 and aux==1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("ERROR");
          errores++;
          lcd.setCursor(0,1);
          lcd.print(errores);
          retardo= errores*700;
          t1=millis();
          aux=0;
    }
      while (t2<=retardo and aux==0){
        t2=millis()-t1;
      }
      if(t2>=retardo-1){
      lcd.setCursor(0,1);
      lcd.print("                   ");
      aux=0;
    }
    
    if(digitalRead(b5)==LOW ){
      lcd.setCursor(0,0);
    lcd.print("ingrese 4234");
    if(digitalRead(b4)==HIGH){
      if (val4 >= 9)val4=0;
     val4++;
     
     lcd.setCursor(4,1);
     lcd.print(val4);
     delay(500);
    }
    if(digitalRead(b3)==HIGH){
      if (val3 >= 9)val3=0;
     val3++;
     
     lcd.setCursor(3,1);
     lcd.print(val3);
     delay(500);
    }
    if(digitalRead(b2)==HIGH){
      if (val2 >= 9)val2=0;
     val2++;
     
     lcd.setCursor(2,1);
     lcd.print(val2);
     delay(500);
    }
    if(digitalRead(b1)==HIGH){
     if (val1>=9)val1=0;
     val1++;
      
     lcd.setCursor(1,1);
     lcd.print(val1);
     delay(500);
    }
    aux=1;
    }

  }


  
 
