#include <Arduino.h>
#include <AT24CX.h>

AT24C32 eep;

//Siguiente posicion libre            (61)

byte Vacio_abajo_derecha[8] = {
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B11111
};
byte lleno_abajo_derecha[8] = {
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B00001,
  B11111
};

byte vacio_medio_derecha[8] = {
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001
};
byte lleno_medio_derecha[8] = {
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101
};

byte vacio_arriba_derecha[8] = {
  B11111,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001
};
byte lleno_arriba_derecha[8] = {
  B11111,
  B00001,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101,
  B11101
};

byte Vacio_abajo_medio[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};
byte lleno_abajo_medio[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111
};

byte Vacio_medio_medio[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte lleno_medio_medio[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte vacio_arriba_medio[8] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte lleno_arriba_medio[8] = {
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte vacio_abajo_izquierda[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111
};
byte lleno_abajo_izquierda[8] = {
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10000,
  B11111
};

byte vacio_medio_izquierda[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};
byte lleno_medio_izquierda[8] = {
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111
};

byte vacio_arriba_izquierda[8] = {
  B11111,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};
byte lleno_arriba_izquierda[8] = {
  B11111,
  B10000,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111,
  B10111
};

void setup() {
eep.write(61,Vacio_abajo_derecha,8);      // custom 5
eep.write(69,lleno_abajo_derecha,8);      // custom 6

eep.write(77,vacio_medio_derecha,8);      // custom 7
eep.write(85,lleno_medio_derecha,8);      // custom 8

eep.write(93,vacio_arriba_derecha,8);     // custom 9
eep.write(101,lleno_arriba_derecha,8);    // custom 10

eep.write(109,Vacio_abajo_derecha,8);     // custom 11
eep.write(117,lleno_abajo_medio,8);       // custom 12

eep.write(125,Vacio_medio_medio,8);       // custom 13
eep.write(133,lleno_medio_medio,8);       // custom 14

eep.write(141,vacio_arriba_medio,8);      // custom 15
eep.write(149,lleno_arriba_medio,8);      // custom 16

eep.write(157,vacio_abajo_izquierda,8);   // custom 17
eep.write(165,lleno_abajo_izquierda,8);   // custom 18

eep.write(173,vacio_medio_izquierda,8);   // custom 19
eep.write(181,lleno_medio_izquierda,8);   // custom 20

eep.write(189,vacio_arriba_izquierda,8);  // custom 21
eep.write(197,lleno_arriba_izquierda,8);  // custom 22

}

void loop() {

}

//convierte