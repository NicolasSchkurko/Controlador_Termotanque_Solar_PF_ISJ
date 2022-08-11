#include <Arduino.h>

String String_de_hora(uint8_t,uint8_t);
uint8_t CharToUINT(uint8_t,uint8_t);
uint8_t StringToChar(uint8_t,const String);
uint8_t ReturnToCero(int8_t , uint8_t);
char Character_Return(uint8_t , bool);
bool PressedButton (uint8_t);
void PrintLCD (char[20], uint8_t, uint8_t);
void Printhora (uint8_t, uint8_t);