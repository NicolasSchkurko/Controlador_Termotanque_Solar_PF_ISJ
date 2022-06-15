const int pulsadorA=1;
const int pulsadorD=0;
const int D=5;
const int C=4;
const int B=3;
const int A=2;
int contador=0;
void setup() {
pinMode(pulsadorA,INPUT);
pinMode(pulsadorD,INPUT);
pinMode(A,OUTPUT);
pinMode(B,OUTPUT);
pinMode(C,OUTPUT);
pinMode(D,OUTPUT);
}

void loop() {
if(digitalRead(pulsadorA)==HIGH && contador<9 ){
  contador++;
  delay(550);
}
if(digitalRead(pulsadorD)==HIGH && contador>0){
  contador--;
  delay(550);
}
if(contador==0){
  digitalWrite(A,LOW);
  digitalWrite(B,LOW);
  digitalWrite(C,LOW);
  digitalWrite(D,LOW);
}
if(contador==1){
  digitalWrite(A,LOW);
  digitalWrite(B,LOW);
  digitalWrite(C,LOW);
  digitalWrite(D,HIGH);
}
if(contador==2){
  digitalWrite(A,LOW);
  digitalWrite(B,LOW);
  digitalWrite(C,HIGH);
  digitalWrite(D,LOW);
}
if(contador==3){
  digitalWrite(A,LOW);
  digitalWrite(B,LOW);
  digitalWrite(C,HIGH);
  digitalWrite(D,HIGH);
}
if(contador==4){
  digitalWrite(A,LOW);
  digitalWrite(B,HIGH);
  digitalWrite(C,LOW);
  digitalWrite(D,LOW);
}
if(contador==5){
  digitalWrite(A,LOW);
  digitalWrite(B,HIGH);
  digitalWrite(C,LOW);
  digitalWrite(D,HIGH);
}
if(contador==6){
  digitalWrite(A,LOW);
  digitalWrite(B,HIGH);
  digitalWrite(C,HIGH);
  digitalWrite(D,LOW);
}
if(contador==7){
  digitalWrite(A,LOW);
  digitalWrite(B,HIGH);
  digitalWrite(C,HIGH);
  digitalWrite(D,HIGH);
}
if(contador==8){
  digitalWrite(A,HIGH);
  digitalWrite(B,LOW);
  digitalWrite(C,LOW);
  digitalWrite(D,LOW);
}
if(contador==9){
  digitalWrite(A,HIGH);
  digitalWrite(B,LOW);
  digitalWrite(C,LOW);
  digitalWrite(D,HIGH);
}
}
