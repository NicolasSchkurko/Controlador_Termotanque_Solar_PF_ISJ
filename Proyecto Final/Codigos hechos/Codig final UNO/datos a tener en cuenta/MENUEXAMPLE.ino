/**
 * Autor: Misael Saenz Flores
 * Esta obra está bajo una Licencia Creative Commons 
 * Atribución-NoComercial-SinDerivadas 4.0 Internacional.
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

const byte NumMenuP = 5;
const byte NumMaxSubM = 4;
const byte NumBttn = 5;


byte Buttn[NumBttn] = {
  2, //UP
  3, //RIGHT
  4, //DOWN
  5, //INT
  6 //LEFT
};

String MenuP[NumMenuP] = {
  "Item 1",
  "Item 2",
  "Item 3",
  "Item 4",
  "Item 5"
};

String SubMenus[NumMenuP][NumMaxSubM] = {
  {"Sub item 1.1","Sub item 1.2","Sub item 1.3","Sub item 1.4"},
  {"Sub item 2.1","Sub item 2.2","Sub item 2.3","Sub item 2.4"},
  {"Sub item 3.1","Sub item 3.2","Sub item 3.3","Sub item 3.4"},
  {"Sub item 4.1","Sub item 4.2","Sub item 4.3","Sub item 4.4"},
  {"Sub item 5.1","Sub item 5.2","Sub item 5.3","Sub item 5.4"}
};

int tecla = -1;
int teclaAnt = -1;
boolean cursorAct = false;
unsigned long tiempo;
int x = -1;
int y = 0;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NumBttn; i++) {
    pinMode(Buttn[i], INPUT);
  }
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("*Menu LCD 16X02*");
  lcd.setCursor(3,1);
  lcd.print("Arduino");
  delay(500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" Menu Principal");
  lcd.setCursor(0,1);
  tiempo = millis();
}

void loop() {
  if(millis() - tiempo > 5000){
    lcd.noBlink();
    cursorAct = false;
  }

  tecla = get_Key(Buttn);
  
  if(tecla != teclaAnt){
  
    delay(50);
    tecla = get_Key(Buttn);
    if(tecla != teclaAnt){
      tiempo = millis();
      teclaAnt = tecla;
      if(tecla >= 0){
        lcd.blink();
      }
      
      if(tecla == 2){ //DOWN
        x++;
        y = 0;
        if(x > NumMenuP - 1) x = NumMenuP - 1;
      }

      if(tecla == 0){ //UP
        x--;
        if( x < 0) x = 0;
        y = 0;
        Serial.println(x);
      }

      if(tecla == 1){ //RIGTH
        y++;
        if( y > NumMaxSubM - 1) y = NumMaxSubM - 1;
      }

      if(tecla == 4){ //LEFT
        y--;
        if(y < 0) y = 0;
      }

      if(tecla == 3){

      }

      lcd.clear();
      lcd.print(MenuP[x]);
      lcd.setCursor(0,1); 
      lcd.print(SubMenus[x][y]);
    }
  }
  delay(50);
}

int get_Key(byte args[]) {
  int k;
  if (digitalRead(args[0]) == HIGH) {
    k = 0;
  } else if (digitalRead(args[1]) == HIGH) {
    k = 1;
  } else if (digitalRead(args[2]) == HIGH) {
    k = 2;
  } else if (digitalRead(args[3]) == HIGH) {
    k = 3;
  } else if (digitalRead(args[4]) == HIGH) {
    k = 4;
  } else {
    k = -1;
  }
  return k;
}
