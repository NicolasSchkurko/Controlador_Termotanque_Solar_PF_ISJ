const int pulsador1=1;
const int pulsador2=2;
const int pulsador3=3;
const int led=4;

void setup() {
pinMode(pulsador1,INPUT);
pinMode(pulsador2,INPUT);
pinMode(pulsador3,INPUT);
pinMode(led,OUTPUT);
}

void loop() {
if (digitalRead(pulsador1)==HIGH && digitalRead(pulsador2)==HIGH && digitalRead(pulsador3)==HIGH){
  digitalWrite(led,HIGH);
}

}
