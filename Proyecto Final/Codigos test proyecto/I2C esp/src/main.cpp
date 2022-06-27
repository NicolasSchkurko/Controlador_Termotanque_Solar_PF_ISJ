#include <Arduino.h>
char conversionSave(char,const char*);

struct guardado{ char pos; char hour; char level; char temp;};

void setup() {
conversionSave(1,"12:45");
}

void loop() {
  // put your main code here, to run repeatedly:
}

char conversionSave(char func,const char* str){
String data=str;
if (func==1){
  int i=0;
  for(int i=0;i<sizeof(data);i++){
    Serial.println(str[i]);
 }
}
}