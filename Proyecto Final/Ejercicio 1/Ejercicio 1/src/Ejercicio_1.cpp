#include <Arduino.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>

const int led1 = 0;
const int led2 = 1;
const int led3 = 2;
const int led4 = 3;
const int led5 = 4;
const int pulsador = 5;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
float voltaje = 0;
int caso = 0;
int contador_ms = 0;
void interrupcion();

void setup()
{
    pinMode(pulsador, INPUT);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(led4, OUTPUT);
    pinMode(led5, OUTPUT);
    lcd.begin(16, 2);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(interrupcion);
}

void loop()
{
    voltaje = analogRead(A0)*5.0/1023;
    lcd.setCursor(0,0);
    lcd.print("Voltaje: ");
    lcd.setCursor(9,0);
    lcd.print(voltaje);
    
    if(voltaje >= 1 && voltaje < 2) caso = 1;
    if(voltaje >= 2 && voltaje < 3) caso = 2;
    if(voltaje >= 3 && voltaje < 4) caso = 3;
    if(voltaje >= 4 && voltaje < 5) caso = 4;
    if(voltaje == 5) caso = 5;
    
    switch (caso)
    {
        case 1:
            digitalWrite(led1, HIGH);
            break;
        
        case 2:
            digitalWrite(led2, HIGH);
            break;
        case 3:
            digitalWrite(led3, HIGH);
            break;
        case 4:
            digitalWrite(led4, HIGH);
            break;
        case 5:
            digitalWrite(led5, HIGH);
            break;
    }
    if(digitalRead(pulsador) == HIGH)
    {
        digitalWrite(led1, HIGH);   
        if(contador_ms >= 500 && contador_ms < 1000) 
        {
            digitalWrite(led2, HIGH); 
        }
        if(contador_ms >= 1000 && contador_ms < 1500)
        {
            digitalWrite(led3, HIGH);
        } 
        if(contador_ms >= 1500 && contador_ms < 2000)
        {
            digitalWrite(led4, HIGH); 
        } 
        if(contador_ms >= 2000 && contador_ms < 2500)
        {
            digitalWrite(led5, HIGH); 
        } 
        if(contador_ms >= 2500 && contador_ms < 3000)
        {
            digitalWrite(led1, LOW);
            digitalWrite(led2, LOW);
            digitalWrite(led3, LOW);
            digitalWrite(led4, LOW);
            digitalWrite(led5, LOW);
        }
    }    
}
void interrupcion()
{
    contador_ms++;
    if(contador_ms > 3000) contador_ms = 0;
}