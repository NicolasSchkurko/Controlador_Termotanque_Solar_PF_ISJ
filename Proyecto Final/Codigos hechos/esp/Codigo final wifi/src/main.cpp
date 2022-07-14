#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

void Serial_Send_UNO(uint8_t);
String processor(const String& var);
//█████████████████████████████████████████████████████████████████████████████████

struct save{uint8_t hora;uint8_t temp;uint8_t lvl;};
String ssid = "Jere";
String password = "chucotest";
//█████████████████████████████████████████████████████████████████████████████████

String TVal = "60";
String LVal = "70";
String HVal = "12:30";
uint8_t TEMP_VAL=0;
uint8_t LVL_VAL=0;
uint8_t HOUR_VAL=0;
uint8_t MINUTE_VAL=0;
bool CHARGING_STATE=0;
bool HEATING_STATE=0;
uint8_t AUTOTEMP_STATE=0;
uint8_t AUTOLVL_STATE=0;

uint8_t InitComunication=false;
uint8_t ComunicationError=false;

//█████████████████████████████████████████████████████████████████████████████████
// Asigna el webserver al puerto 80 de la red wifi
AsyncWebServer server(80);

//█████████████████████████████████████████████████████████████████████████████████

void setup(){

  Serial.begin(9600);
  WiFi.begin(ssid, password);
  LittleFS.begin();
 
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    InitComunication=true;
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send(LittleFS, "/index.html", String(), false, processor);});

  server.on("/desing.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(LittleFS, "/desing.css", "text/css");});
  //Toma datos del slider y los guarda en una variable

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {  
  if (request->hasParam("hour")) 
    {
      String inputMessage = request->getParam("hour")->value();
      HVal = inputMessage;
      Serial.println(HVal);
      request->send(LittleFS, "/index.html", String(), false, processor);
    }
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {   
    String inputMessage;
    //Toma datos del slider temp
    if (request->hasParam("temp")) 
    {
      inputMessage = request->getParam("temp")->value();
      TVal= inputMessage;
      TEMP_VAL = TVal.toInt();
      request->send(LittleFS, "/index.html", String(), false, processor);
    }
    //Toma datos del slider nivel
    if (request->hasParam("nivel")) 
    {
      inputMessage = request->getParam("nivel")->value();
      LVal = inputMessage;
      LVL_VAL = LVal.toInt();
      request->send(LittleFS, "/index.html", String(), false, processor);
    }
  });
  
  //detectaa cuando se redirige (producto de que se presiona un boton) en alguna pagina y realiza algo
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request){
    HEATING_STATE= !HEATING_STATE; 
    Serial_Send_UNO(65);
    request->send(LittleFS, "/index.html", String(), false, processor);
  }); 

  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request){
    CHARGING_STATE= !CHARGING_STATE;
    Serial_Send_UNO(65); 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request){
    AUTOTEMP_STATE++; 
    Serial_Send_UNO(65);
    if(AUTOTEMP_STATE>3)AUTOTEMP_STATE=0; 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request){
    AUTOLVL_STATE++; 
    Serial_Send_UNO(65);
    if(AUTOLVL_STATE>3)AUTOLVL_STATE=0; 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request){ 
    Serial_Send_UNO(65);
    request->send(LittleFS, "/config.html", String(), false, processor);
  });


  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial_Send_UNO(65);
    request->send(LittleFS, "/config.html", String(), false, processor);
  });

  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial_Send_UNO(65);
    request->send(LittleFS, "/config.html", String(), false, processor);
  });

  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial_Send_UNO(65);
    request->send(LittleFS, "/config.html", String(), false, processor);
  });

  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request){ 
    Serial_Send_UNO(65);
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  server.begin();
}

//█████████████████████████████████████████████████████████████████████████████████ 

void loop() 
{

}

void Serial_Send_UNO(uint8_t WhatSend)
  {
    uint8_t MessagePoss;
    if (InitComunication==true)MessagePoss=0;
    switch (WhatSend){
      case 1:
        if (ComunicationError==false && InitComunication==true && MessagePoss<=5)//&&Send_time>=1000) BUSCAR INTERRUPCIONES EN ESP
        {
            switch (MessagePoss)
            {
              case 0:
                Serial.println("S_""SSID"":""Pass");
                 MessagePoss++;
                 //Send_time=0;
                break;
              case 1:
                Serial.println("K_HORA:TEMP:LVL:0");
                MessagePoss++;
                //Send_time=0;
                break;
              case 2:
                Serial.println("K_HORA:TEMP:LVL:1");
                MessagePoss++;
                //Send_time=0;
                break;
              case 3:
                Serial.println("K_HORA:TEMP:LVL:2");
                MessagePoss++;
                //Send_time=0;
                break;
              case 4:
                Serial.println("J_255:TEMPMIN:TEMPMAX:3");
                MessagePoss++;
                //Send_time=0;
                break;
              case 5:
                Serial.println("V_255:TEMPMIN:TEMPMAX:3");
                InitComunication=false;
                MessagePoss=0;
                //Send_time=0;
                break;
            // delay de 1 seg
            } //Send_time =0;
        }
        break;
      case 2:
        if (ComunicationError==false && InitComunication==false)
          {
              Serial.println("U_TEMP:LVL:HORA:STATE");
          }
      case 3:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("K_HORA:TEMP:LVL:STRUCTPOS");
          }
      case 4:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("J_255:TEMPMIN:TEMPMAX:3");
          }
      case 5:
        if (ComunicationError==false && InitComunication==false)
          {
            Serial.println("V_255:LVLMIN:LVLMAX:4");
          }
    }
    
  }
//█████████████████████████████████████████████████████████████████████████████████
// Se encarga de buscar ciertas variables declaradas con %(nombre)% dentro del html y remplazarlos por strings
String processor(const String& var){

//Devuelve numeros
if(var == "TVAL")return TVal;


if(var == "LVAL")return LVal;
if(var == "HVAL")return HVal;

//Devuelve un texto
if(var == "BTNT"){
if(HEATING_STATE==1)return "llenado encendido"; 
else return "llenado apagado";
}

if(var == "BTNL"){
if(CHARGING_STATE==1)return "llenado encendido"; 
else return "llenado apagado";
}

if(var == "STTA"){
  if(AUTOTEMP_STATE==0)return "Setear calentamiento automatico";
  if(AUTOTEMP_STATE==1)return "Setear temperatura minima";
  if(AUTOTEMP_STATE==2)return "Setear temperatura a calentar";
  if(AUTOTEMP_STATE==3)return "Confirmar seteo";
}

if(var == "STLA"){
  if(AUTOLVL_STATE==0)return "Setear llenado automatico";
  if(AUTOLVL_STATE==1)return "Setear llenado minima";
  if(AUTOLVL_STATE==2)return "Setear llenado a calentar";
  if(AUTOLVL_STATE==3)return "Confirmar seteo";
}

return String();
}

