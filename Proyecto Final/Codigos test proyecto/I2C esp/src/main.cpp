#include <Arduino.h>
char conversionSave(char,const char*);

struct guardado{ char pos; char hour; char level; char temp;};

void setup() {
  Serial.begin(9600);
conversionSave(1,"12:45");
}

void loop() {
conversionSave(1,"12:45");
}

char conversionSave(char func,const char* str){
String data=str;
char pablo=023;
if (func==1){
  int i=0;
  for(int i=0;i<sizeof(data);i++){
    Serial.println(str[i]);//imprime cada uno de los caracteres
 }
}
return pablo;
}