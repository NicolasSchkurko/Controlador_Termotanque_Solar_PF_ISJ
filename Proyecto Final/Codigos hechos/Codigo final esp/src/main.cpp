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
String TextoHora = "12:30";
String TextoErrorGuardado = "     ";
String TextoErrorNivel = "    ";
String TextoErrorTemp = "    ";
String TextoEstado;
char Contraseña[22] = "k";
char SSID[22] = "k";
char TextoTemp[5] = "60";
char TextoNivel[4] = "70";
Guardado guardado[3];
bool EnviarIP;
bool DatosObtenidos;
char Estado;
char Datos[22];
char Letra;
uint8_t LargoDatos;
int8_t  TempActual = 0;
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
      TextoHora=TextoEstado;
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } });

  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {   
  String TextoEstado;
  //Toma datos del slider Temp
    if (request->hasParam("temp")) 
    {
      TextoEstado = request->getParam("temp")->value();
      TextoEstado.toCharArray(TextoTemp,5);
      TemperaturaCalentar = atoi(TextoTemp);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    }
    if (request->hasParam("tempmax")) 
    {
      TextoEstado = request->getParam("tempmax")->value();
      TextoEstado.toCharArray(TextoTemp,5);
      TemperaturaMaxima = atoi(TextoTemp);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    }
    if (request->hasParam("tempmin")) 
    {
      TextoEstado = request->getParam("tempmin")->value();
      TextoEstado.toCharArray(TextoTemp,5);
      TemperaturaMinima = atoi(TextoTemp);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    }
    //Toma datos del slider nivel
    if (request->hasParam("nivel")) 
    {
      TextoEstado = request->getParam("nivel")->value();
       TextoEstado.toCharArray(TextoNivel,4);
      NivelLlenar = atoi(TextoNivel);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } 
    if (request->hasParam("nivelmax")) 
    {
      TextoEstado = request->getParam("nivelmax")->value();
       TextoEstado.toCharArray(TextoNivel,4);
      NivelMaximo = atoi(TextoNivel);
      request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb);
    } 
    if (request->hasParam("nivelmin")) 
    {
      TextoEstado = request->getParam("nivelmin")->value();
       TextoEstado.toCharArray(TextoNivel,4);
      NivelMinimo = atoi(TextoNivel);
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
      TextoErrorTemp="";
    }
    else{
      TextoErrorTemp="La Temperatura minima debe ser menor a la maxima";
    }
    request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });

  // Setea el llenado de manera automatica
  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (NivelMaximo>NivelMinimo){
      NivelMaximoGuardado = NivelMaximo;
      NivelMinimoGuardado = NivelMinimo;
      Enviar_Serial(5);
      TextoErrorTemp="";
    }
    else TextoErrorTemp="El nivel maximo debe ser mayor al minimo";
    request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });
  // Redirige a pagina de automatico por tiempo
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  server.on("/AUTOSET", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send(LittleFS, "/SetValue.html", String(), false, ImprimirEnWeb); });
  // Guarda en el primer slot de guardado por tiempo AUTOSET
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (Convercionhora(1, TextoHora) == guardado[1].Hora || Convercionhora(1, TextoHora) == guardado[2].Hora){
      TextoErrorGuardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    TextoErrorGuardado="";
    guardado[0].Hora=Convercionhora(1, TextoHora);
    guardado[0].Nivel=NivelLlenar;
    guardado[0].Temp=TemperaturaCalentar;
    SlotGuardado=0;
    Enviar_Serial(3);
    }
  request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  // Guarda en el segundo slot de guardado por tiempo
  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    if (Convercionhora(1, TextoHora) == guardado[0].Hora || Convercionhora(1, TextoHora) == guardado[2].Hora){
      TextoErrorGuardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    TextoErrorGuardado="";
    guardado[1].Hora=Convercionhora(1, TextoHora);
    guardado[1].Nivel=NivelLlenar;
    guardado[1].Temp=TemperaturaCalentar;
    SlotGuardado=1;
    Enviar_Serial(3);
    }
  request->send(LittleFS, "/setTimer.html", String(), false, ImprimirEnWeb); });
  // Guarda en el tercer slot de guardado por tiempo
  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request)
            {
 
    if (Convercionhora(1, TextoHora) == guardado[0].Hora || Convercionhora(1, TextoHora) == guardado[1].Hora){
      TextoErrorGuardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    TextoErrorGuardado="";
    guardado[2].Hora=Convercionhora(1, TextoHora);
    guardado[2].Nivel=NivelLlenar;
    guardado[2].Temp=TemperaturaCalentar;
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
  { request->send_P(200, "text/plain", DesConvercionhora(1, guardado[0].Hora).c_str()); }); 
  server.on("/H2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", DesConvercionhora(1, guardado[1].Hora).c_str()); }); 
  server.on("/H3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", DesConvercionhora(1, guardado[2].Hora).c_str()); }); 
  server.on("/L1", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[0].Nivel).c_str()); }); 
  server.on("/L2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[1].Nivel).c_str()); }); 
  server.on("/L3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[2].Nivel).c_str()); }); 
  server.on("/T1", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[0].Temp).c_str()); }); 
  server.on("/T2", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[1].Temp).c_str()); }); 
  server.on("/T3", HTTP_GET, [](AsyncWebServerRequest *request)
  { request->send_P(200, "text/plain", String(guardado[2].Temp).c_str()); }); 
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
    return String(TextoHora);
  // entrega strings de error
  if (var == "ERRORguardado")
    return TextoErrorGuardado;
  if (var == "ERRORLVL")
    return TextoErrorNivel;
  if (var == "ERRORTEMP")
    return TextoErrorTemp;
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
    return DesConvercionhora(1, guardado[0].Hora);
  if (var == "L1")
    return String(guardado[0].Nivel);
  if (var == "T1")
    return String(guardado[0].Temp);
  // entrega datos de horas seteadas (segundo slot)
  if (var == "H2")
    return DesConvercionhora(1, guardado[1].Hora);
  if (var == "L2")
    return String(guardado[1].Nivel);
  if (var == "T2")
    return String(guardado[1].Temp);
  // entrega datos de horas seteadas (tercer slot)
  if (var == "H3")
    return DesConvercionhora(1, guardado[2].Hora);
  if (var == "L3")
    return String(guardado[2].Nivel);
  if (var == "T3")
    return String(guardado[2].Temp);
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
  char datosEnviarSerial[22];
  switch (WhatSend)
  {
  case 1:
    sprintf(datosEnviarSerial, "W_%cI", TemperaturaCalentar + 34);
    Serial.println(datosEnviarSerial);
    break;
  case 2:
    sprintf(datosEnviarSerial, "F_%cI", NivelLlenar + 34);
    Serial.println(datosEnviarSerial);
    break;
  case 3:
    sprintf(datosEnviarSerial, "K_%c%c%c%c", guardado[SlotGuardado].Hora + 34, guardado[SlotGuardado].Temp + 34, guardado[SlotGuardado].Nivel + 34, SlotGuardado + 49);
    Serial.println(datosEnviarSerial);
    break;
  case 4:
    sprintf(datosEnviarSerial, "H_%c%c", TemperaturaMinimaGuardado + 34, TemperaturaMaximaGuardado + 34);
    Serial.println(datosEnviarSerial);
    break;
  case 5:
    sprintf(datosEnviarSerial, "C_%c%c", NivelMinimoGuardado + 34, NivelMaximoGuardado + 34);
    Serial.println(datosEnviarSerial);
    break;
  case 6:
    sprintf(datosEnviarSerial, "I_%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    Serial.println(datosEnviarSerial);
    break;
  }
}

void Leer_Serial()
{
  String entradaSerial;
  uint8_t largoMensaje;
  char comando;

  entradaSerial = Serial.readString();   // iguala el string del serial a un string imput
  largoMensaje = entradaSerial.length(); // saca el largo del string
  comando = entradaSerial.charAt(0);       // toma el char de comando (el primer char usualmente una letra)
  memset(Datos, 0, 22);
  for (uint8_t CharPos = 0; CharPos <= 22; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
    if (CharPos < largoMensaje - 2)
      Datos[CharPos] = entradaSerial.charAt(CharPos + 2); 
    if (CharPos == largoMensaje - 2)
      DatosObtenidos = true; // activa el comando final (flag)
    if (CharPos > largoMensaje - 2)
      Datos[CharPos] = 0;
    
  }

  if (DatosObtenidos == true)
  {
    /* */
    switch (comando) // dependiendo del char de comando
    {
    case 'N':
      memcpy(SSID, Datos, 22);
      DatosObtenidos = false;
      break;
    case 'P':
      memcpy(Contraseña, Datos, 22);
      WiFi.begin(String(SSID), String(Contraseña));
      EnviarIP = true;
      DatosObtenidos = false;
      break;
    case 'U':
      TempActual = Datos[0] - 128;
      NivelActual = Datos[1] - 34;
      if(Datos[4]!=Datos[7] && Datos[4]!=Datos[10]){
        guardado[0].Hora = int(Datos[2] - 34);
        guardado[0].Nivel = int(Datos[3] - 34);
        guardado[0].Temp = int(Datos[4] - 34);
        guardado[1].Hora = int(Datos[5] - 34);
        guardado[1].Nivel = int(Datos[6] - 34);
        guardado[1].Temp = int(Datos[7] - 34);
        guardado[2].Hora = int(Datos[8] - 34);
        guardado[2].Nivel = int(Datos[9] - 34);
        guardado[2].Temp = int(Datos[10] - 34);
      }
      NivelMaximoGuardado = Datos[12] - 34;
      NivelMinimoGuardado = Datos[11] - 34;
      TemperaturaMaximaGuardado = Datos[14] - 34;
      TemperaturaMinimaGuardado = Datos[13] - 34;
      DatosObtenidos = false;
      break;

    case 'H':
      if(Datos[1] - 34<Datos[0] - 34)
        break;
      else
      {
      TemperaturaMaximaGuardado = Datos[1] - 34;
      TemperaturaMinimaGuardado = Datos[0] - 34;
      DatosObtenidos = false;
      break;
      }
    }
  }
}

String DesConvercionhora(uint8_t funcion, uint8_t guardado)
{
  uint8_t primerNumero = 0; // solo una variable (_deconvert  nos evita modificar variables globales como bldos)
  uint8_t segundoNumero = 0;
  uint8_t resto = 0;
  String mensajeRetorno;
  // todo el dia con la mielcita jere ¯\_(ツ)_/¯ ??
  switch (funcion)
  {
  case 1:
    resto = (guardado) % 4;
    primerNumero = (guardado - resto) / 4;
    segundoNumero = resto * 15;
    if(primerNumero<10)
      mensajeRetorno = "0" + String(primerNumero) + ':';
    else
      mensajeRetorno = String(primerNumero) + ':';
    if(segundoNumero<10)
      mensajeRetorno = mensajeRetorno + "0" + String(segundoNumero);
    else
    mensajeRetorno = mensajeRetorno + String(segundoNumero);
    return mensajeRetorno;
    break;
  case 2:
    mensajeRetorno = String(guardado);
    return mensajeRetorno;
    break;
  case 3:
    mensajeRetorno = String(guardado);
    return mensajeRetorno;
    break;
  default:
    return "ERROR";
    break;
  }
}

uint8_t Convercionhora(uint8_t funcion, String mensajeOriginal)
{
  uint8_t primerNumero;  // solo una variable (_convert  nos evita modificar variables globales como bldos)
  uint8_t segundoNumero;  // solo una variable
  uint8_t resto; // guarda el resto del calculo de tiempo
  switch (funcion)
  {
  case 1:
    primerNumero = (mensajeOriginal.charAt(0) - '0') * 10; // toma el valor del primer digito del string y lo convierte en int (numero de base 10)
    primerNumero += (mensajeOriginal.charAt(1) - '0');     // toma el valor del segundo digito
    primerNumero = primerNumero * 4;                 // multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

    segundoNumero = (mensajeOriginal.charAt(3) - '0') * 10; // lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
    segundoNumero += (mensajeOriginal.charAt(4) - '0');     // lo mismo que en el var 1 pero con minutos
    resto = segundoNumero % 15;               // saca el resto (ejemplo 7/5 resto 2)
    if (resto < 8)
      segundoNumero = segundoNumero - resto; // utiliza el resto para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
    else
      segundoNumero = segundoNumero + 15 - resto; // utiliza el resto para redondear arriba
    segundoNumero = segundoNumero / 15;                   // convierte los minutos en la proporcion del char (1 entero = 15 minutos)

    primerNumero += segundoNumero; // suma horas y minutos
    return primerNumero;
    break;
  case 2:
    primerNumero = mensajeOriginal.toInt(); // hace magia y despues de tirar magia convierte el string en un int (fua ta dificil la conversion de ete)
    return primerNumero;
    break;
  default:
    segundoNumero = mensajeOriginal.toInt(); // mismo sistema
    return segundoNumero;
    break;
  }
}