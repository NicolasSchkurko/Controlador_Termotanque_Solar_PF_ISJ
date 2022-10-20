#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

String DesConvercionhora(uint8_t, uint8_t);
uint8_t Convercionhora(uint8_t, String);
String ImprimirEnWeb(const String &var);
void Enviar_Serial(uint8_t);
void Leer_Serial();

struct Guardado
{
  uint8_t Hora;
  uint8_t Nivel;
  uint8_t Temp;
};

char Password[22] = "k";
char SSID[22] = "k";
bool EnviarIP;
Guardado save[3];
char Texto_Temp[5] = "60";
char Texto_Nivel[4] = "70";
String Texto_Hora = "12:30";
String Texto_Error_Guardado = "     ";
String Texto_Error_Nivel = "    ";
String Texto_Error_Temp = "    ";
String TextoEstado;
char Estado;
char Datos[22];
char Letra;
uint8_t i;
uint8_t LargoDatos;
bool DatosObtenidos;
int8_t TempActual = 0;
uint8_t NivelActual = 0;
uint8_t SlotGuardado = 0;
uint8_t TemperaturaCalentar = 0;
uint8_t TemperaturaMaxima = 0;
uint8_t TemperaturaMinima = 0;
uint8_t TemperaturaMaximaGuardado = 0;
uint8_t TemperaturaMinimaGuardado = 0;
uint8_t NivelLlenar = 0;
uint8_t NivelMaximo = 0;
uint8_t NivelMinimo = 0;
uint8_t NivelMaximoGuardado = 0;
uint8_t NivelMinimoGuardado = 0;

uint8_t CalentadoAuto = 0;
uint8_t LLenadoAuto = 0;
//█████████████████████████████████████████████████████████████████████████████████
// Asigna el webserver al puerto 80 de la red wifi
AsyncWebServer server(80);

//█████████████████████████████████████████████████████████████████████████████████

void setup()
{

  Serial.begin(9600);
  LittleFS.begin();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });

  server.on("/desing.css", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/desing.css", "text/css"); });
  // Toma datos del slider y los guarda en una variable

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {  
  if (request->hasParam("hora")) 
    {
      TextoEstado = request->getParam("hora")->value();
      Texto_Hora=TextoEstado;
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } });

  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {   
  String TextoEstado;
  //Toma datos del slider Temp
    if (request->hasParam("temp")) 
    {
      TextoEstado = request->getParam("temp")->value();
      TextoEstado.toCharArray(Texto_Temp,5);
      TemperaturaCalentar = atoi(Texto_Temp);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    }
    if (request->hasParam("tempmax")) 
    {
      TextoEstado = request->getParam("tempmax")->value();
      TextoEstado.toCharArray(Texto_Temp,5);
      TemperaturaMaxima = atoi(Texto_Temp);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    }
    if (request->hasParam("tempmin")) 
    {
      TextoEstado = request->getParam("tempmin")->value();
      TextoEstado.toCharArray(Texto_Temp,5);
      TemperaturaMinima = atoi(Texto_Temp);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    }
    //Toma datos del slider nivel
    if (request->hasParam("nivel")) 
    {
      TextoEstado = request->getParam("nivel")->value();
       TextoEstado.toCharArray(Texto_Nivel,4);
      NivelLlenar = atoi(Texto_Nivel);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } 
    if (request->hasParam("nivelmax")) 
    {
      TextoEstado = request->getParam("nivelmax")->value();
       TextoEstado.toCharArray(Texto_Nivel,4);
      NivelMaximo = atoi(Texto_Nivel);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    } 
    if (request->hasParam("nivelmin")) 
    {
      TextoEstado = request->getParam("nivelmin")->value();
       TextoEstado.toCharArray(Texto_Nivel,4);
      NivelMinimo = atoi(Texto_Nivel);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
  } });

  // detectaa cuando se redirige (producto de que se presiona un boton) en alguna pagina y realiza algo
  // Activa el calentamiento de manera manual
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Enviar_Serial(1);
    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });
  // Activa el llenado de agua manual
  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Enviar_Serial(2); 
    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });

  // Setea el calentamiento de manera automatica
  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (TemperaturaMaxima>TemperaturaMinima){
      TemperaturaMaximaGuardado = TemperaturaMaxima;
      TemperaturaMinimaGuardado = TemperaturaMinima;
      Enviar_Serial(4);
      Texto_Error_Temp="";
    }
    else{
      Texto_Error_Temp="La Temperatura minima debe ser menor a la maxima";
    }
    request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });

  // Setea el llenado de manera automatica
  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (NivelMaximo>NivelMinimo){
      NivelMaximoGuardado = NivelMaximo;
      NivelMinimoGuardado = NivelMinimo;
      Enviar_Serial(5);
      Texto_Error_Temp="";
    }
    else Texto_Error_Temp="El nivel maximo debe ser mayor al minimo";
    request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });
  // Redirige a pagina de automatico por tiempo
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  server.on("/AUTOSET", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });
  // Guarda en el primer slot de guardado por tiempo AUTOSET
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (Convercionhora(1, Texto_Hora) == save[1].Hora || Convercionhora(1, Texto_Hora) == save[2].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    Texto_Error_Guardado="";
    save[0].Hora=Convercionhora(1, Texto_Hora);
    save[0].Nivel=NivelLlenar;
    save[0].Temp=TemperaturaCalentar;
    SlotGuardado=0;
    Enviar_Serial(3);
    }
  request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  // Guarda en el segundo slot de guardado por tiempo
  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    if (Convercionhora(1, Texto_Hora) == save[0].Hora || Convercionhora(1, Texto_Hora) == save[2].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    Texto_Error_Guardado="";
    save[1].Hora=Convercionhora(1, Texto_Hora);
    save[1].Nivel=NivelLlenar;
    save[1].Temp=TemperaturaCalentar;
    SlotGuardado=1;
    Enviar_Serial(3);
    }
  request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  // Guarda en el tercer slot de guardado por tiempo
  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request)
            {
 
    if (Convercionhora(1, Texto_Hora) == save[0].Hora || Convercionhora(1, Texto_Hora) == save[1].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    Texto_Error_Guardado="";
    save[2].Hora=Convercionhora(1, Texto_Hora);
    save[2].Nivel=NivelLlenar;
    save[2].Temp=TemperaturaCalentar;
    SlotGuardado=2;
    Enviar_Serial(3);
    }
    request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  // Redirige a pagina principal (index)
  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });
  server.on("/TSVs", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(TempActual).c_str()); });
  server.on("/LSVs", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(NivelActual).c_str()); }); 
  server.on("/STSV", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(TemperaturaMaximaGuardado).c_str()); }); 
  server.on("/-STSV", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(TemperaturaMinimaGuardado).c_str()); }); 
  server.on("/SLSV", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(NivelMaximoGuardado).c_str()); }); 
  server.on("/-SLSV", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(NivelMinimoGuardado).c_str()); }); 
  server.on("/H1", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", DesConvercionhora(1, save[0].Hora).c_str()); }); 
  server.on("/H2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", DesConvercionhora(1, save[1].Hora).c_str()); }); 
  server.on("/H3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", DesConvercionhora(1, save[2].Hora).c_str()); }); 
  server.on("/L1", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[0].Nivel).c_str()); }); 
  server.on("/L2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[1].Nivel).c_str()); }); 
  server.on("/L3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[2].Nivel).c_str()); }); 
  server.on("/T1", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[0].Temp).c_str()); }); 
  server.on("/T2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[1].Temp).c_str()); }); 
  server.on("/T3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(save[2].Temp).c_str()); }); 
  server.begin();
}

//█████████████████████████████████████████████████████████████████████████████████

void loop()
{
  if (Serial.available() > 0)
    Leer_Serial();

  if (WiFi.status() == WL_CONNECTED && EnviarIP == true)
  {
    Enviar_Serial(6);
    EnviarIP = false;
  }
  delay(1);
}

//█████████████████████████████████████████████████████████████████████████████████
String ImprimirEnWeb(const String &var)
{
  // Se encarga de buscar ciertas variables declaradas con %(nombre)% dentro del html y remplazarlos por strings
  // entrega numeros de Temperatura actual a la pagina
  if (var == "TVAL")
    return String(TempActual);
  if (var == "LVAL")
    return String(NivelActual);
  if (var == "HVAL")
    return String(Texto_Hora);
  // entrega strings de error
  if (var == "ERRORSAVE")
    return Texto_Error_Guardado;
  if (var == "ERRORLVL")
    return Texto_Error_Nivel;
  if (var == "ERRORTEMP")
    return Texto_Error_Temp;
  // entrega numeros de Temperatura y nivel maxima y minima
  if (var == "LMAX")
    return String(NivelMaximo);
  if (var == "LMIN")
    return String(NivelMinimo);
  if (var == "TMAX")
    return String(TemperaturaCalentar);
  if (var == "TMIN")
    return String(TemperaturaMinima);
  // entrega datos de horas seteadas (primer slot)
  if (var == "H1")
    return DesConvercionhora(1, save[0].Hora);
  if (var == "L1")
    return String(save[0].Nivel);
  if (var == "T1")
    return String(save[0].Temp);
  // entrega datos de horas seteadas (segundo slot)
  if (var == "H2")
    return DesConvercionhora(1, save[1].Hora);
  if (var == "L2")
    return String(save[1].Nivel);
  if (var == "T2")
    return String(save[1].Temp);
  // entrega datos de horas seteadas (tercer slot)
  if (var == "H3")
    return DesConvercionhora(1, save[2].Hora);
  if (var == "L3")
    return String(save[2].Nivel);
  if (var == "T3")
    return String(save[2].Temp);
  // Devuelve un texto (Activar calentamiento)
  if (var == "BTNT")
      return "Encender calentamiento manual";

  // Devuelve un texto (Activar llenado)
  if (var == "BTNL")
      return "Encender calentamiento manual";

  // Devuelve un texto (Activar calentamiento automatico)
  if (var == "STTA")
      return "Setear calentamiento automatico";

  // Devuelve un texto (Activar llenado automatico)
  if (var == "STLA")
      return "Setear llenado automatico";

  return String();
}

void Enviar_Serial(uint8_t WhatSend)
{
  char DatosEnviarSerial[22];
  switch (WhatSend)
  {
  case 1:
    sprintf(DatosEnviarSerial, "W_%cI", TemperaturaCalentar + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 2:
    sprintf(DatosEnviarSerial, "F_%cI", NivelLlenar + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 3:
    sprintf(DatosEnviarSerial, "K_%c%c%c%c", save[SlotGuardado].Hora + 34, save[SlotGuardado].Temp + 34, save[SlotGuardado].Nivel + 34, SlotGuardado + 49);
    Serial.println(DatosEnviarSerial);
    break;
  case 4:
    sprintf(DatosEnviarSerial, "H_%c%c", TemperaturaMinimaGuardado + 34, TemperaturaMaximaGuardado + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 5:
    sprintf(DatosEnviarSerial, "C_%c%c", NivelMinimoGuardado + 34, NivelMaximoGuardado + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 6:
    sprintf(DatosEnviarSerial, "I_%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    Serial.println(DatosEnviarSerial);
    break;
  }
}

void Leer_Serial()
{
  String Serial_Input;
  uint8_t StringLength;
  String Individualdata[4];
  char input;
  bool datosObtenidos;

  Serial_Input = Serial.readString();   // iguala el string del serial a un string imput
  StringLength = Serial_Input.length(); // saca el largo del string
  input = Serial_Input.charAt(0);       // toma el char de comando (el primer char usualmente una letra)
  memset(Datos, 0, 22);
  for (uint8_t CharPos = 0; CharPos <= 22; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
    if (CharPos < StringLength - 2)
      Datos[CharPos] = Serial_Input.charAt(CharPos + 2); 
    if (CharPos == StringLength - 2)
      datosObtenidos = true; // activa el comando final (flag)
    if (CharPos > StringLength - 2)
      Datos[CharPos] = 0;
    
  }

  if (datosObtenidos == true)
  {
    /* */
    switch (input) // dependiendo del char de comando
    {
    case 'N':
      memcpy(SSID, Datos, 22);
      datosObtenidos = false;
      Serial.print(SSID);
      break;
    case 'P':
      memcpy(Password, Datos, 22);
      Serial.print(Password);
      WiFi.begin(String(SSID), String(Password));
      EnviarIP = true;
      datosObtenidos = false;
      break;
    case 'U':
      TempActual = Datos[0] - 128;
      NivelActual = Datos[1] - 34;
      if(Datos[4]!=Datos[7] && Datos[4]!=Datos[10]){
        save[0].Hora = int(Datos[2] - 34);
        save[0].Nivel = int(Datos[3] - 34);
        save[0].Temp = int(Datos[4] - 34);
        save[1].Hora = int(Datos[5] - 34);
        save[1].Nivel = int(Datos[6] - 34);
        save[1].Temp = int(Datos[7] - 34);
        save[2].Hora = int(Datos[8] - 34);
        save[2].Nivel = int(Datos[9] - 34);
        save[2].Temp = int(Datos[10] - 34);
      }
      NivelMaximoGuardado = Datos[12] - 34;
      NivelMinimoGuardado = Datos[11] - 34;
      TemperaturaMaximaGuardado = Datos[14] - 34;
      TemperaturaMinimaGuardado = Datos[13] - 34;
      datosObtenidos = false;
      break;

    case 'H':
      if(Datos[1] - 34<Datos[0] - 34)
        break;
      else
      {
      TemperaturaMaximaGuardado = Datos[1] - 34;
      TemperaturaMinimaGuardado = Datos[0] - 34;
      datosObtenidos = false;
      break;
      }
    }
  }
}

String DesConvercionhora(uint8_t function, uint8_t save)
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
    if(var1_deconvert<10)
      returned = "0" + String(var1_deconvert) + ':';
    else
      returned = String(var1_deconvert) + ':';
    if(var2_deconvert<10)
      returned = returned + "0" + String(var2_deconvert);
    else
    returned = returned + String(var2_deconvert);
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

uint8_t Convercionhora(uint8_t function, String toconvert)
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