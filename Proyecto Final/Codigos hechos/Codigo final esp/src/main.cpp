#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

String processor(const String &var);
String desconvercionhora(uint8_t, uint8_t);
uint8_t convercionhora(uint8_t, String);
void Serial_Read_NODEMCU();
void Serial_Send_NODEMCU(uint8_t);

//█████████████████████████████████████████████████████████████████████████████████

String ssid = "WifiChuco";
String password = "AloAmbAr!";

//█████████████████████████████████████████████████████████████████████████████████
bool EnviarIP;
struct save_data
{
  uint8_t hour;
  uint8_t level;
  uint8_t temp;
};
save_data save[3];

String TVal = "60";
String LVal = "70";
String HVal = "12:30";
String errorS = "     ";
String errorLVL = "    ";
String errorTEMP = "    ";

String IP;
String InputString;
char Individualdata[22];
char input;
char message;
char IParray[18];
uint8_t seriallength;
uint8_t i;

char Output_message[60];

int8_t TEMP_VAL = 0;
uint8_t LVL_VAL = 0;
uint8_t HOUR_VAL = 0;
uint8_t MINUTE_VAL = 0;
uint8_t Struct = 0;
uint8_t Temp_Max = 0;
uint8_t Temp_Min = 0;
uint8_t Level_Max = 0;
uint8_t Level_Min = 0;
uint8_t Actual_temp = 0;
uint8_t Actual_level = 0;

bool CHARGING_STATE = false;
bool HEATING_STATE = false;
uint8_t AUTOTEMP_STATE = false;
uint8_t AUTOLVL_STATE = false;

bool ConvertString = true;
bool ComunicationError = false;

//█████████████████████████████████████████████████████████████████████████████████
// Asigna el webserver al puerto 80 de la red wifi
AsyncWebServer server(80);

//█████████████████████████████████████████████████████████████████████████████████

void setup()
{

  Serial.begin(9600);
  LittleFS.begin();
  // Connect to Wi-Fi
  // Print ESP32 Local IP Address

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, processor); });

  server.on("/desing.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/desing.css", "text/css"); });
  // Toma datos del slider y los guarda en una variable

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {  
  if (request->hasParam("hour")) 
    {
      String inputMessage = request->getParam("hour")->value();
      HVal = inputMessage;
      request->send(LittleFS, "/index.html", String(), false, processor);
    } });

  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request)
            {   
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
    } });

  // detectaa cuando se redirige (producto de que se presiona un boton) en alguna pagina y realiza algo
  // Activa el calentamiento de manera manual
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    HEATING_STATE= !HEATING_STATE; 
    Serial_Send_NODEMCU(1);
    request->send(LittleFS, "/index.html", String(), false, processor); });
  // Activa el llenado de agua manual
  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    CHARGING_STATE= !CHARGING_STATE;
    Serial_Send_NODEMCU(2); 
    request->send(LittleFS, "/index.html", String(), false, processor); });

  // Setea el calentamiento de manera automatica
  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AUTOTEMP_STATE++; 
    if (AUTOTEMP_STATE==1 && TEMP_VAL<90)Temp_Min=TEMP_VAL;
    if (AUTOTEMP_STATE==1 &&TEMP_VAL>=90) errorTEMP="La temperatura minima debe ser menor a 90";

    if (AUTOTEMP_STATE==2 && TEMP_VAL>Temp_Min)Temp_Max=TEMP_VAL;
    if (AUTOTEMP_STATE==2 && TEMP_VAL<Temp_Min)errorTEMP="La temperatura maxima debe ser mayor a la minima";

    if(AUTOTEMP_STATE>2){
      AUTOTEMP_STATE=0; 
      Serial_Send_NODEMCU(4);
    }

    request->send(LittleFS, "/index.html", String(), false, processor); });

  // Setea el llenado de manera automatica
  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AUTOLVL_STATE++; 
    if (AUTOLVL_STATE==1 && TEMP_VAL<100)Level_Min=LVL_VAL;
    if (AUTOLVL_STATE==1 && TEMP_VAL>=100)errorLVL="El nivel minimo debe ser menor a 100";

    if (AUTOLVL_STATE==2 && LVL_VAL>Level_Min)Level_Max=LVL_VAL;
    if (AUTOLVL_STATE==2 && LVL_VAL<=Level_Min)errorLVL="El nivel maximo debe ser mayor al minimo";

    if(AUTOLVL_STATE>2){
      AUTOLVL_STATE=0; 
      Serial_Send_NODEMCU(5);
    }

    request->send(LittleFS, "/index.html", String(), false, processor); });
  // Redirige a pagina de automatico por tiempo
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/config.html", String(), false, processor); });
  // Guarda en el primer slot de guardado por tiempo
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request)
            {

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
    request->send(LittleFS, "/config.html", String(), false, processor); });
  // Guarda en el segundo slot de guardado por tiempo
  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    
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
    request->send(LittleFS, "/config.html", String(), false, processor); });
  // Guarda en el tercer slot de guardado por tiempo
  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request)
            {
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
    request->send(LittleFS, "/config.html", String(), false, processor); });
  // Redirige a pagina principal (index)
  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, processor); });
  server.begin();
}

//█████████████████████████████████████████████████████████████████████████████████

void loop()
{
  if (Serial.available() > 0)
    Serial_Read_NODEMCU();

  if (WiFi.status() == WL_CONNECTED && EnviarIP == true)
  {
    Serial_Send_NODEMCU(6);
    EnviarIP = false;
  }
}

//█████████████████████████████████████████████████████████████████████████████████
String processor(const String &var)
{
  // Se encarga de buscar ciertas variables declaradas con %(nombre)% dentro del html y remplazarlos por strings
  // entrega numeros de temperatura actual a la pagina
  if (var == "TVAL")
    return TVal;
  if (var == "LVAL")
    return LVal;
  if (var == "HVAL")
    return HVal;
  // entrega strings de error
  if (var == "ERRORSAVE")
    return errorS;
  if (var == "ERRORLVL")
    return errorLVL;
  if (var == "ERRORTEMP")
    return errorTEMP;
  // entrega numeros de temperatura y nivel maxima y minima
  if (var == "LMAX")
    return String(Level_Max);
  if (var == "LMIN")
    return String(Level_Min);
  if (var == "TMAX")
    return String(Temp_Max);
  if (var == "TMIN")
    return String(Temp_Min);
  // entrega datos de horas seteadas (primer slot)
  if (var == "H1")
    return String(save[0].hour);
  if (var == "L1")
    return String(save[0].level);
  if (var == "T1")
    return String(save[0].temp);
  // entrega datos de horas seteadas (segundo slot)
  if (var == "H2")
    return String(save[1].hour);
  if (var == "L2")
    return String(save[1].level);
  if (var == "T2")
    return String(save[1].temp);
  // entrega datos de horas seteadas (tercer slot)
  if (var == "H3")
    return String(save[2].hour);
  if (var == "L3")
    return String(save[2].level);
  if (var == "T3")
    return String(save[2].temp);
  // Devuelve un texto (Activar calentamiento)
  if (var == "BTNT")
  {
    if (HEATING_STATE)
      return "Calentamiento encendido";
    else
      return "Calentamiento apagado";
  }

  // Devuelve un texto (Activar llenado)
  if (var == "BTNL")
  {
    if (CHARGING_STATE)
      return "llenado encendido";
    else
      return "llenado apagado";
  }

  // Devuelve un texto (Activar calentamiento automatico)
  if (var == "STTA")
  {
    if (AUTOTEMP_STATE == 0)
      return "Setear calentamiento automatico";
    if (AUTOTEMP_STATE == 1)
      return "Setear temperatura minima";
    if (AUTOTEMP_STATE == 2)
      return "Setear temperatura a calentar";
    if (AUTOTEMP_STATE == 3)
      return "Confirmar seteo";
  }

  // Devuelve un texto (Activar llenado automatico)
  if (var == "STLA")
  {
    if (AUTOLVL_STATE == 0)
      return "Setear llenado automatico";
    if (AUTOLVL_STATE == 1)
      return "Setear llenado minima";
    if (AUTOLVL_STATE == 2)
      return "Setear llenado a calentar";
    if (AUTOLVL_STATE == 3)
      return "Confirmar seteo";
  }

  return String();
}

void Serial_Send_NODEMCU(uint8_t WhatSend)
{

  switch (WhatSend)
  {
  case 1:
    if (HEATING_STATE == false)
      message = 'O';
    if (HEATING_STATE == true)
      message = 'I';
    sprintf(Output_message, "W_%c%c", TEMP_VAL + 33, message);
    Serial.println(Output_message);
    break;
  case 2:
    if (CHARGING_STATE == false)
      message = 'O';
    if (CHARGING_STATE == true)
      message = 'I';
    sprintf(Output_message, "F_%c%c", LVL_VAL + 33, message);
    Serial.println(Output_message);
    break;
  case 3:
    sprintf(Output_message, "S_%c%c%c%c", save[Struct].hour + 33, save[Struct].temp + 33, save[Struct].level + 33, Struct + 48);
    Serial.println(Output_message);
    break;
  case 4:
    sprintf(Output_message, "H_%c%c", Temp_Min + 33, Temp_Max + 33);
    Serial.println(Output_message);
    break;
  case 5:
    sprintf(Output_message, "C_%c%c", Level_Min + 33, Level_Max + 33);
    Serial.println(Output_message);
    break;
  case 6:
    IP = WiFi.localIP().toString();
    seriallength = IP.length();
    IP.toCharArray(IParray,18);
    sprintf(Output_message, "I_%s", IParray);
    Serial.println(Output_message);
    break;
  case 7:
    Serial.println("E_E");
  }
}

void Serial_Read_NODEMCU()
{
  InputString = Serial.readString();
  seriallength = InputString.length();

  for (i = 0; i <= seriallength; i++)
  {
    if (i == 0)
      input = InputString.charAt(0);
    if (i >= 2 && i < seriallength)
      Individualdata[i - 2] = InputString.charAt(i);
  }

  switch (input) // dependiendo del char de comando
  {
  case 'K':
    Struct = Individualdata[3] - 48;
    if (Struct <= 2 && Struct >= 0)
    {
      save[Struct].hour = Individualdata[0] - 33;
      save[Struct].level = Individualdata[1] - 33;
      save[Struct].temp = Individualdata[2] - 33;
    }
    break;
  case 'H':
    Temp_Max = Individualdata[1] - 33;
    Temp_Min = Individualdata[0] - 33;
    break;
  case 'C':
    Level_Max = Individualdata[1] - 33;
    Level_Min = Individualdata[0] - 33;
    break;
  case 'U':
    TEMP_VAL = Individualdata[0] - 128;
    LVL_VAL = Individualdata[1] - 33;
    if (Individualdata[2] == 'I')
      HEATING_STATE = true;
    else
      HEATING_STATE = false;
    if (Individualdata[3] == 'I')
      CHARGING_STATE = true;
    else
      CHARGING_STATE = false;
    break;

  case 'P':
    password = "";
    for (i = 0; i <= seriallength - 3; i++)
      password += Individualdata[i];
    EnviarIP = true;
    WiFi.begin(ssid, password);
    break;
  case 'N':
    ssid = "";
    for (i = 0; i <= seriallength - 3; i++)
      ssid += Individualdata[i];
    break;
  }
}

String desconvercionhora(uint8_t function, uint8_t save)
{
  uint8_t var1_deconvert = 0; // solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t var2_deconvert = 0;
  uint8_t resto_deconvert = 0;
  String returned;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ ??
  switch (function)
  {
  case 1:
    resto_deconvert = (save) % 4;
    var1_deconvert = (save - resto_deconvert) / 4;
    var2_deconvert = resto_deconvert * 15;
    returned = String(var1_deconvert) + ':' + String(var2_deconvert);
    return returned;
    break;
  case 2:
    returned = String(save);
    return returned;
    break;
  case 3:
    returned = String(save);
    return returned;
    break;
  default:
    return "ERROR";
    break;
  }
}

uint8_t convercionhora(uint8_t function, String toconvert)
{
  uint8_t var1_convert;  // solo una variable (_convert  nos evita modificar variables globales como bldos)
  uint8_t var2_convert;  // solo una variable
  uint8_t resto_convert; // guarda el resto del calculo de tiempo
  switch (function)
  {
  case 1:
    var1_convert = (toconvert.charAt(0) - '0') * 10; // toma el valor del primer digito del string y lo convierte en int (numero de base 10)
    var1_convert += (toconvert.charAt(1) - '0');     // toma el valor del segundo digito
    var1_convert = var1_convert * 4;                 // multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

    var2_convert = (toconvert.charAt(3) - '0') * 10; // lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
    var2_convert += (toconvert.charAt(4) - '0');     // lo mismo que en el var 1 pero con minutos
    resto_convert = var2_convert % 15;               // saca el resto (ejemplo 7/5 resto 2)
    if (resto_convert < 8)
      var2_convert = var2_convert - resto_convert; // utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
    else
      var2_convert = var2_convert + 15 - resto_convert; // utiliza el resto para redondear arriba
    var2_convert = var2_convert / 15;                   // convierte los minutos en la proporcion del char (1 entero = 15 minutos)

    var1_convert += var2_convert; // suma horas y minutos
    return var1_convert;
    break;
  case 2:
    var1_convert = toconvert.toInt(); // hace magia y despues de tirar magia convierte el string en un int (fua ta dificil la conversion de ete)
    return var1_convert;
    break;
  default:
    var2_convert = toconvert.toInt(); // mismo sistema
    return var2_convert;
    break;
  }
}