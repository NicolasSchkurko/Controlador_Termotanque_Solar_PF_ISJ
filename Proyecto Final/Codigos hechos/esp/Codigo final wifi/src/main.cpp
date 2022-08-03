#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

String processor(const String& var);
String desconvercionhora(uint8_t,uint8_t);
uint8_t convercionhora(uint8_t, String);
void Serial_Read_NODEMCU();
void Serial_Send_NODEMCU(uint8_t);

//█████████████████████████████████████████████████████████████████████████████████

String ssid = "Jere";
String password = "chucotest";

//█████████████████████████████████████████████████████████████████████████████████
bool EnviarIP;
struct save_data{ uint8_t hour; uint8_t level; uint8_t temp;};
save_data save[3]; 
String TVal = "60";
String LVal = "70";
String HVal = "12:30";
String errorS= "     ";
String errorLVL= "    ";
String errorTEMP= "    ";

uint8_t TEMP_VAL=0;
uint8_t LVL_VAL=0;
uint8_t HOUR_VAL=0;
uint8_t MINUTE_VAL=0;
uint8_t Struct=0;
uint8_t Temp_Max=0;
uint8_t Temp_Min=0;
uint8_t Level_Max=0;
uint8_t Level_Min=0;
uint8_t Actual_temp=0;
uint8_t Actual_level=0;

bool CHARGING_STATE=0;
bool HEATING_STATE=0;
uint8_t AUTOTEMP_STATE=0;
uint8_t AUTOLVL_STATE=0;

uint8_t ActualIndividualDataPos=0;
bool ConvertString=true;
bool ComunicationError=false;

//█████████████████████████████████████████████████████████████████████████████████
// Asigna el webserver al puerto 80 de la red wifi
AsyncWebServer server(80);

//█████████████████████████████████████████████████████████████████████████████████

void setup(){

  Serial.begin(9600);
  LittleFS.begin();
 
  // Connect to Wi-Fi
  // Print ESP32 Local IP Address
  WiFi.begin(ssid, password);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send(LittleFS, "/index.html", String(), false, processor);});

  server.on("/desing.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(LittleFS, "/desing.css", "text/css");});
  //Toma datos del slider y los guarda en una variable

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {  
  if (request->hasParam("hour")) 
    {
      String inputMessage = request->getParam("hour")->value();
      HVal = inputMessage;
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
  //Activa el calentamiento de manera manual
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request){
    HEATING_STATE= !HEATING_STATE; 
    Serial_Send_NODEMCU(1);
    request->send(LittleFS, "/index.html", String(), false, processor);
  }); 
  //Activa el llenado de agua manual
  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request){
    CHARGING_STATE= !CHARGING_STATE;
    Serial_Send_NODEMCU(2); 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  //Setea el calentamiento de manera automatica
  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request){
    AUTOTEMP_STATE++; 
    if (AUTOTEMP_STATE==1)Temp_Min=TEMP_VAL;
    if (AUTOTEMP_STATE==2)Temp_Max=TEMP_VAL;
    if (AUTOTEMP_STATE==3)Serial_Send_NODEMCU(4);
    if(AUTOTEMP_STATE>3)AUTOTEMP_STATE=0; 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  //Setea el llenado de manera automatica
  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request){
    AUTOLVL_STATE++; 
    if (AUTOLVL_STATE==1)Level_Min=TEMP_VAL;
    if (AUTOLVL_STATE==2)Level_Max=TEMP_VAL;
    if (AUTOLVL_STATE==3)Serial_Send_NODEMCU(5);
    if(AUTOLVL_STATE>3)AUTOLVL_STATE=0; 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  //Redirige a pagina de automatico por tiempo
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(LittleFS, "/config.html", String(), false, processor);
  });
  //Guarda en el primer slot de guardado por tiempo
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request){

    if (convercionhora(1, HVal) == save[1].hour || convercionhora(1, HVal) == save[2].hour){
      errorS = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[0].hour=convercionhora(1, HVal);
    save[0].level=convercionhora(2, LVal);
    save[0].temp=convercionhora(2, TVal);
    Struct=0;
    Serial_Send_NODEMCU(3);
    }
    request->send(LittleFS, "/config.html", String(), false, processor);
  });
  //Guarda en el segundo slot de guardado por tiempo
  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request){
    
    if (convercionhora(1, HVal) == save[0].hour || convercionhora(1, HVal) == save[2].hour){
      errorS = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[1].hour=convercionhora(1, HVal);
    save[1].level=convercionhora(2, LVal);
    save[1].temp=convercionhora(2, TVal);
    Struct=1;
    Serial_Send_NODEMCU(3);
    }
    request->send(LittleFS, "/config.html", String(), false, processor);
  });
  //Guarda en el tercer slot de guardado por tiempo
  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request){
    if (convercionhora(1, HVal) == save[0].hour || convercionhora(1, HVal) == save[1].hour){
      errorS = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[2].hour=convercionhora(1, HVal);
    save[2].level=convercionhora(2, LVal);
    save[2].temp=convercionhora(2, TVal);
    Struct=2;
    Serial_Send_NODEMCU(3);
    }
    request->send(LittleFS, "/config.html", String(), false, processor);
  });
  //Redirige a pagina principal (index)
  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  server.begin();
}

//█████████████████████████████████████████████████████████████████████████████████ 

void loop(){
  if (Serial.available()>0){Serial_Read_NODEMCU();}
  if (WiFi.status() == WL_CONNECTED && EnviarIP==true){Serial.println(WiFi.localIP()); EnviarIP=false;}
}


//█████████████████████████████████████████████████████████████████████████████████
String processor(const String& var){
// Se encarga de buscar ciertas variables declaradas con %(nombre)% dentro del html y remplazarlos por strings
//entrega numeros de temperatura actual a la pagina
if(var == "TVAL")return TVal;
if(var == "LVAL")return LVal;
if(var == "HVAL")return HVal;
//entrega strings de error
if(var == "ERRORSAVE")return errorS;
if(var == "ERRORLVL")return errorLVL;
if(var == "ERRORTEMP")return errorTEMP;
//entrega numeros de temperatura y nivel maxima y minima
if(var == "LMAX")return String(Level_Max);
if(var == "LMIN")return String(Level_Min);
if(var == "TMAX")return String(Temp_Max);
if(var == "TMIN")return String(Temp_Min);
//entrega datos de horas seteadas (primer slot)
if(var == "H1")return String(save[0].hour);
if(var == "L1")return String(save[0].level);
if(var == "T1")return String(save[0].temp);
//entrega datos de horas seteadas (segundo slot)
if(var == "H2")return String(save[1].hour);
if(var == "L2")return String(save[1].level);
if(var == "T2")return String(save[1].temp);
//entrega datos de horas seteadas (tercer slot)
if(var == "H3")return String(save[2].hour);
if(var == "L3")return String(save[2].level);
if(var == "T3")return String(save[2].temp);
//Devuelve un texto (Activar calentamiento)
if(var == "BTNT"){
if(HEATING_STATE==1)return "Calentamiento encendido"; 
else return "Calentamiento apagado";
}

//Devuelve un texto (Activar llenado)
if(var == "BTNL"){
if(CHARGING_STATE==1)return "llenado encendido"; 
else return "llenado apagado";
}

//Devuelve un texto (Activar calentamiento automatico)
if(var == "STTA"){
  if(AUTOTEMP_STATE==0)return "Setear calentamiento automatico";
  if(AUTOTEMP_STATE==1)return "Setear temperatura minima";
  if(AUTOTEMP_STATE==2)return "Setear temperatura a calentar";
  if(AUTOTEMP_STATE==3)return "Confirmar seteo";
}

//Devuelve un texto (Activar llenado automatico)
if(var == "STLA"){
  if(AUTOLVL_STATE==0)return "Setear llenado automatico";
  if(AUTOLVL_STATE==1)return "Setear llenado minima";
  if(AUTOLVL_STATE==2)return "Setear llenado a calentar";
  if(AUTOLVL_STATE==3)return "Confirmar seteo";
}

return String();
}

void Serial_Send_NODEMCU(uint8_t WhatSend)
  {
    String message;
    switch (WhatSend){
      case 1:
        if (HEATING_STATE == true)message = "ON";
        if (HEATING_STATE == false)message = "OFF";
        Serial.println("H_"+TVal+":"+message);
        break;
      case 2:
        if (CHARGING_STATE == true)message = "ON";
        if (CHARGING_STATE == false)message = "OFF";
        Serial.println("C_"+LVal+":"+message);
        break;
      case 3:
    
        Serial.println("K_"+String(save[Struct].hour)+":"+desconvercionhora(2,save[Struct].temp)+":"+desconvercionhora(3,save[Struct].level)+":"+String(Struct));
        break;
      case 4:
        Serial.println("J_"+String(Temp_Min)+":"+String(Temp_Max));
        break;
      case 5:
        Serial.println("V_"+String(Level_Min)+":"+String(Level_Max));
        break;
      case 6:
        Serial.println("E_ERROR:");// Si no entiende un mensaje envia error
        break;
      case 7:
        Serial.println("O_OK:");// Si entiende el mensaje manda ok
        break;
    }
    
  }

void Serial_Read_NODEMCU(){
  String Serial_Input;
  uint8_t StringLength;
  String Individualdata[4];
  char input;

  Serial_Input=Serial.readString();// iguala el string del serial a un string imput
  StringLength= Serial_Input.length();// saca el largo del string
  Serial.println(Serial_Input);// Solo de verificacion (eliminar en el final)
  input=Serial_Input.charAt(0); // toma el char de comando (el primer char usualmente una letra)

  for (uint8_t CharPos = 2; CharPos <= StringLength; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
      if(Serial_Input.charAt(CharPos)==':')ActualIndividualDataPos++; //si hay : divide los datos
      if(Serial_Input.charAt(CharPos)!=':' && Serial_Input.charAt(CharPos)!='_' && Serial_Input.charAt(CharPos)!='/')// si no es nungun caracter especial:
        {
          if(Serial_Input.charAt(CharPos-1)==':' || Serial_Input.charAt(CharPos-1)=='_')Individualdata[ActualIndividualDataPos]=Serial_Input.charAt(CharPos);//si es el primer digito lo iguala
          else Individualdata[ActualIndividualDataPos]+=Serial_Input.charAt(CharPos);//si es el segundo en adelante lo suma
        }
      if(CharPos==StringLength)ConvertString=true;// activa el comando final (flag)
  } 

  if (ConvertString==true){
    switch (input)//dependiendo del char de comando
    {
    case 'K':
          Struct=Individualdata[3].toInt();
          if (Struct<=2 && Struct>=0){
            save[Struct].hour=Individualdata[0].toInt();
            save[Struct].level=Individualdata[1].toInt();
            save[Struct].temp=Individualdata[2].toInt();
            ActualIndividualDataPos=0;
            ConvertString=false;
            Serial_Send_NODEMCU(7);
          }

      break;
    case 'S':
            ssid=Individualdata[0];
            password=Individualdata[1];
            ConvertString=false;
            Serial_Send_NODEMCU(7);
            EnviarIP=true;
      
    break;
    case 'U':
          HVal=Individualdata[0];
          LVal=Individualdata[1];
          TVal=Individualdata[2];
          ActualIndividualDataPos=0;
          ConvertString=false;
      break;
    case 'J':
          Temp_Min=Individualdata[0].toInt();// tempin
          Temp_Max=Individualdata[1].toInt();// tempmax
          ActualIndividualDataPos=0;
          ConvertString=false;
          Serial_Send_NODEMCU(7);
      break;
    case 'V':
            Level_Min=Individualdata[0].toInt();// lvlmin
            Level_Max=Individualdata[1].toInt();// lvlmax
            ActualIndividualDataPos=0;
            ConvertString=false;
            Serial_Send_NODEMCU(7);
      break;
    case '?':
            // RESET INO setup();
            ActualIndividualDataPos=0;
            ConvertString=false;
        break;
    }
  }
  else Serial.println("E_ERROR");
}

String desconvercionhora(uint8_t function,uint8_t save)
{
  uint8_t var1_deconvert=0;//solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t var2_deconvert=0;
  uint8_t resto_deconvert=0;
  String returned;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ ??
  switch (function)
    {
      case 1:
        resto_deconvert= (save) % 4;
        var1_deconvert= (save-resto_deconvert)/4;
        var2_deconvert=resto_deconvert*15;
        returned= String(var1_deconvert)+':'+String(var2_deconvert);
        return returned;
      break;
      case 2:
        returned= String(save);
        return returned;
      break;
      case 3:
        returned= String(save);
        return returned;
      break;
      default:
        return "ERROR";
        break;
    }
}

uint8_t convercionhora(uint8_t function, String toconvert)
{
  uint8_t var1_convert; // solo una variable (_convert  nos evita modificar variables globales como bldos)
  uint8_t var2_convert; // solo una variable
  uint8_t resto_convert; //guarda el resto del calculo de tiempo
  switch (function)
  {
    case 1:
      var1_convert=(toconvert.charAt(0)- '0')*10;// toma el valor del primer digito del string y lo convierte en int (numero de base 10)
      var1_convert+=(toconvert.charAt(1)-'0');// toma el valor del segundo digito
      var1_convert=var1_convert*4;//multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

      var2_convert=(toconvert.charAt(3)- '0')*10;//lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
      var2_convert+=(toconvert.charAt(4)-'0');//lo mismo que en el var 1 pero con minutos
      resto_convert=var2_convert%15; //saca el resto (ejemplo 7/5 resto 2)
      if(resto_convert<8) var2_convert= var2_convert-resto_convert; //utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
      else var2_convert=var2_convert+15-resto_convert;// utiliza el resto para redondear arriba
      var2_convert=var2_convert/15;// convierte los minutos en la proporcion del char (1 entero = 15 minutos)

      var1_convert+=var2_convert;// suma horas y minutos
      return var1_convert;
      break;
    case 2:
      var1_convert= toconvert.toInt();// hace magia y despues de tirar magia convierte el string en un int (fua ta dificil la conversion de ete)
      return var1_convert;
      break;
    default:
      var2_convert= toconvert.toInt();// mismo sistema
      return var2_convert;
      break;
  }
}