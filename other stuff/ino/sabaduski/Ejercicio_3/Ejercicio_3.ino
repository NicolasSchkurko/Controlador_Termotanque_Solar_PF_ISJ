const int pulsador1=1;
const int pulsador2=2;
const int led=3;
void setup() {
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
pinMode(led,OUTPUT);
}
void loop() {
if(digitalRead(pulsador1)==LOW&&digitalRead(pulsador2)==LOW){
  digitalWrite(led,LOW);
}
else{
  digitalWrite(led,HIGH);
}
}
