const int pulsador1=2;
const int pulsador2=3;
const int pulsador3=4;
const int pulsador4=5;
const int pulsador5=6;
const int led=9;
int val=0;
float intensidad=0;

void setup() {
pinMode (pulsador1, INPUT);
pinMode (pulsador2, INPUT);
pinMode (pulsador3, INPUT);
pinMode (pulsador4, INPUT);
pinMode (pulsador5, INPUT);
pinMode (led,OUTPUT);
}

void loop() {
if(digitalRead(pulsador1)==HIGH and digitalRead(pulsador2)==LOW and digitalRead(pulsador3)==LOW and digitalRead(pulsador4)==LOW and digitalRead(pulsador5)==LOW){
  val=0;
}
if(digitalRead(pulsador2)==HIGH and digitalRead(pulsador1)==LOW and digitalRead(pulsador3)==LOW and digitalRead(pulsador4)==LOW and digitalRead(pulsador5)==LOW){
  val=25;
}
if(digitalRead(pulsador3)==HIGH and digitalRead(pulsador1)==LOW and digitalRead(pulsador2)==LOW and digitalRead(pulsador4)==LOW and digitalRead(pulsador5)==LOW){
  val=50;
}
if(digitalRead(pulsador4)==HIGH and digitalRead(pulsador1)==LOW and digitalRead(pulsador2)==LOW and digitalRead(pulsador3)==LOW and digitalRead(pulsador5)==LOW){
  val=75;
}
if(digitalRead(pulsador5)==HIGH and digitalRead(pulsador1)==LOW and digitalRead(pulsador2)==LOW and digitalRead(pulsador3)==LOW and digitalRead(pulsador4)==LOW){
  val=100;
}
intensidad= (val/100)*255;
digitalWrite(led,intensidad);
}
