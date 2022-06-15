const int pulsador=1;
const int led=10;
const int pulsador2=2;
const int led2=9;
void setup() {
pinMode(pulsador,INPUT);
pinMode(led,OUTPUT);
pinMode(pulsador2,INPUT);
pinMode(led2,OUTPUT);
}
void loop(){
if (digitalRead(pulsador)==HIGH){
  digitalWrite(led,HIGH);
}
if (digitalRead(pulsador2)==HIGH){
  digitalWrite(led2,HIGH);
}
if (digitalRead(pulsador)==LOW){
  digitalWrite(led,LOW);
}
if (digitalRead(pulsador2)==LOW){
  digitalWrite(led2,LOW);
}
}
