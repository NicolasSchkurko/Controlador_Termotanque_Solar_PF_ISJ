#include <LiquidCrystal.h>

LiquidCrystal lcd(13,12,11,10,9,8);
const int pulsador1=1;
const int pulsador2=2;
const int pote=A5;
const int led1=3;
const int led2=4;
unsigned long tiempo=0;//toma el tiempo gral para utilizar millis()
unsigned long tiempoclave=0;//toma el tiempo inicial que se dispone para colocar la clave
int contrareloj=0;
int valorpote=0; //imprime el numero equivalente del pote (0-9)
int state; //toma el estado del programa y evita errores
int stateclave;//toma el estado del ingreso de la clave
int numeros[3]={0,0,0};//guarda los numeros de la clave en un vector



void setup() {
lcd.begin(16,2);
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
pinMode(pote,INPUT);
pinMode(led1,OUTPUT);
pinMode(led2,OUTPUT);
tiempo=millis();
state=0;
}

void loop() {
  
  if(state==0){
    lcd.setCursor(0,0);
    lcd.print("pulse inico");
    digitalWrite(led2,LOW);
    digitalWrite(led1,LOW);
  }
  
  if(digitalRead(pulsador1)==HIGH && state==0){
    state=1;
    lcd.setCursor(0,0);
    lcd.print("ingrese codigo");
    stateclave=0;
    tiempoclave=millis();
  }
  
if(state==1){
  valorpote=map(analogRead(pote),0,1023,0,9);
  lcd.setCursor(0,1);
  lcd.print(valorpote);
  lcd.setCursor(1,1);
  lcd.print(" T:");
  contrareloj=(millis()-tiempoclave)/1000;
  if(contrareloj>=15)digitalWrite(led2,HIGH);
  lcd.setCursor(4,1);
  lcd.print(contrareloj);
  
  if(millis()-tiempoclave>=20000){
    state=3;
  }

  if(digitalRead(pulsador2)==HIGH && stateclave==0){
    
    if(millis()-tiempo>=1000){
    numeros[0]=valorpote;
    lcd.setCursor(13,1);
    lcd.print(numeros[0]);
    stateclave=1;
    tiempo=millis();
    }
  }
  
  if(digitalRead(pulsador2)==HIGH && stateclave==1){
    
    if(millis()-tiempo>=1000){
    numeros[1]=valorpote;
    lcd.setCursor(14,1);
    lcd.print(numeros[1]);
    stateclave=2;
    tiempo=millis();
    }
  }
  
  if(digitalRead(pulsador2)==HIGH && stateclave==2){
    
    if(millis()-tiempo>=1000){
    stateclave=3;
    numeros[2]=valorpote;
    lcd.setCursor(15,1);
    lcd.print(numeros[2]);
    tiempo=millis();
    }
  }
}
if(digitalRead(pulsador1)==HIGH && stateclave==3){
  lcd.clear();
  state=2;
  stateclave=4;
  }

if(state==2){
  
  if(numeros[0]==3 && numeros[1]==5 && numeros[2]==8){
    lcd.setCursor(0,0);
    lcd.print("caja abierta");
    digitalWrite(led1,HIGH);
  }
  
else{
    lcd.setCursor(0,0);
    lcd.print("INCORRECTO");
    lcd.setCursor(0,1);
    lcd.print("puls2:reset");
    digitalWrite(led2,HIGH);
    
    if(digitalRead(pulsador2)==HIGH && stateclave==4){
      state=3;
    }
  }
}

if(state==3){
  lcd.clear();
  state=0;
  }
}
