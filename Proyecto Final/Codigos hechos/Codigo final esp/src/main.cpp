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
char Texto_Hora[6] = "12:30";
String Texto_Error_Guardado = "     ";
String Texto_Error_Nivel = "    ";
String Texto_Error_Temp = "    ";
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
uint8_t TemperaturaMinima = 0;
uint8_t NivelCalentar = 0;
uint8_t NivelMinimo = 0;
bool Llenando = false;
bool Calentando = false;
bool LoopFunca;
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
  WiFi.begin(String(SSID), String(Password));

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
              request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
              sprintf(Texto_Temp,"%c",TempActual);
              sprintf(Texto_Nivel,"%c",NivelActual); 
                  Leer_Serial();
                  });

  server.on("/desing.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/desing.css", "text/css"); 
            LoopFunca=true;     Leer_Serial();});
  // Toma datos del slider y los guarda en una variable

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {  
  if (request->hasParam("Hora")) 
    {
      LoopFunca=true;
      String LetraEstado = request->getParam("Hora")->value();
      LetraEstado.toCharArray(Texto_Hora,6);
          Leer_Serial();
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } });

  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request)
            {   
              LoopFunca=true;
                  Leer_Serial();
    String LetraEstado;
    //Toma datos del slider Temp
    if (request->hasParam("Temp")) 
    {
      LetraEstado = request->getParam("Temp")->value();
      LetraEstado.toCharArray(Texto_Temp,5);
          Leer_Serial();
      TempActual = atoi(Texto_Temp);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    }
    //Toma datos del slider nivel
    if (request->hasParam("nivel")) 
    {
      LetraEstado = request->getParam("nivel")->value();
       LetraEstado.toCharArray(Texto_Nivel,4);
           Leer_Serial();
      NivelActual = atoi(Texto_Nivel);
      request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb);
    } });

  // detectaa cuando se redirige (producto de que se presiona un boton) en alguna pagina y realiza algo
  // Activa el calentamiento de manera manual
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
    Calentando= !Calentando; 
    Enviar_Serial(1);
    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });
  // Activa el llenado de agua manual
  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
    Llenando= !Llenando;
    Enviar_Serial(2); 
    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });

  // Setea el calentamiento de manera automatica
  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
    CalentadoAuto++; 
    if (CalentadoAuto==1 && TempActual<90)TemperaturaMinima=TempActual;
    if (CalentadoAuto==1 &&TempActual>=90) Texto_Error_Temp="La Temperatura minima debe ser menor a 90";

    if (CalentadoAuto==2 && TempActual>TemperaturaMinima)TemperaturaCalentar=TempActual;
    if (CalentadoAuto==2 && TempActual<TemperaturaMinima)Texto_Error_Temp="La Temperatura maxima debe ser mayor a la minima";

    if(CalentadoAuto>2){
      CalentadoAuto=0; 
      Enviar_Serial(4);
    }
    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });

  // Setea el llenado de manera automatica
  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
    LLenadoAuto++; 
    if (LLenadoAuto==1 && TempActual<100)NivelMinimo=NivelActual;
    if (LLenadoAuto==1 && TempActual>=100)Texto_Error_Nivel="El nivel minimo debe ser menor a 100";

    if (LLenadoAuto==2 && NivelActual>NivelMinimo)NivelCalentar=NivelActual;
    if (LLenadoAuto==2 && NivelActual<=NivelMinimo)Texto_Error_Nivel="El nivel maximo debe ser mayor al minimo";

    if(LLenadoAuto>2){
      LLenadoAuto=0; 
      Enviar_Serial(5);
    }

    request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });
  // Redirige a pagina de automatico por tiempo
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request)
            { LoopFunca=true;
              request->send(LittleFS, "/config.html", String(), false, ImprimirEnWeb); });
  // Guarda en el primer slot de guardado por tiempo
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request)
            {
LoopFunca=true;
    if (Convercionhora(1, Texto_Hora) == save[1].Hora || Convercionhora(1, Texto_Hora) == save[2].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[0].Hora=Convercionhora(1, Texto_Hora);
    save[0].Nivel=Convercionhora(2, Texto_Nivel);
    save[0].Temp=Convercionhora(2, Texto_Temp);
    SlotGuardado=0;
    Enviar_Serial(3);
    }
    request->send(LittleFS, "/config.html", String(), false, ImprimirEnWeb); });
  // Guarda en el segundo slot de guardado por tiempo
  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    LoopFunca=true;
    if (Convercionhora(1, Texto_Hora) == save[0].Hora || Convercionhora(1, Texto_Hora) == save[2].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[1].Hora=Convercionhora(1, Texto_Hora);
    save[1].Nivel=Convercionhora(2, Texto_Nivel);
    save[1].Temp=Convercionhora(2, Texto_Temp);
    SlotGuardado=1;
    Enviar_Serial(3);
    }
    request->send(LittleFS, "/config.html", String(), false, ImprimirEnWeb); });
  // Guarda en el tercer slot de guardado por tiempo
  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              LoopFunca=true;
    if (Convercionhora(1, Texto_Hora) == save[0].Hora || Convercionhora(1, Texto_Hora) == save[1].Hora){
      Texto_Error_Guardado = "No podes guardar dos variables en la misma hora";
    }
    else{
    save[2].Hora=Convercionhora(1, Texto_Hora);
    save[2].Nivel=Convercionhora(2, Texto_Nivel);
    save[2].Temp=Convercionhora(2, Texto_Temp);
    SlotGuardado=2;
    Enviar_Serial(3);
    }
    request->send(LittleFS, "/config.html", String(), false, ImprimirEnWeb); });
  // Redirige a pagina principal (index)
  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, ImprimirEnWeb); });
    server.begin();
}

//█████████████████████████████████████████████████████████████████████████████████

void loop()
{
   if (Serial.available()>0) Leer_Serial();

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
  if (var == "Texto_Temp")
    return Texto_Temp;
  if (var == "Texto_Nivel")
    return Texto_Nivel;
  if (var == "Texto_Hora")
    return Texto_Hora;
  // entrega strings de error
  if (var == "Texto_Error_GuardadoAVE")
    return Texto_Error_Guardado;
  if (var == "Texto_Error_Nivel")
    return Texto_Error_Nivel;
  if (var == "Texto_Error_Temp")
    return Texto_Error_Temp;
  // entrega numeros de Temperatura y nivel maxima y minima
  if (var == "LMAX")
    return String(NivelCalentar);
  if (var == "LMIN")
    return String(NivelMinimo);
  if (var == "TMAX")
    return String(TemperaturaCalentar);
  if (var == "TMIN")
    return String(TemperaturaMinima);
  // entrega datos de horas seteadas (primer slot)
  if (var == "H1")
    return String(save[0].Hora);
  if (var == "L1")
    return String(save[0].Nivel);
  if (var == "T1")
    return String(save[0].Temp);
  // entrega datos de horas seteadas (segundo slot)
  if (var == "H2")
    return String(save[1].Hora);
  if (var == "L2")
    return String(save[1].Nivel);
  if (var == "T2")
    return String(save[1].Temp);
  // entrega datos de horas seteadas (tercer slot)
  if (var == "H3")
    return String(save[2].Hora);
  if (var == "L3")
    return String(save[2].Nivel);
  if (var == "T3")
    return String(save[2].Temp);
  // Devuelve un texto (Activar calentamiento)
  if (var == "BTNT")
  {
    if (Calentando)
      return "Calentamiento encendido";
    else
      return "Calentamiento apagado";
  }

  // Devuelve un texto (Activar llenado)
  if (var == "BTNL")
  {
    if (Llenando)
      return "llenado encendido";
    else
      return "llenado apagado";
  }

  // Devuelve un texto (Activar calentamiento automatico)
  if (var == "STTA")
  {
    if (CalentadoAuto == 0)
      return "Setear calentamiento automatico";
    if (CalentadoAuto == 1)
      return "Setear Temperatura minima";
    if (CalentadoAuto == 2)
      return "Setear Temperatura a calentar";
    if (CalentadoAuto == 3)
      return "Confirmar seteo";
  }

  // Devuelve un texto (Activar llenado automatico)
  if (var == "STLA")
  {
    if (LLenadoAuto == 0)
      return "Setear llenado automatico";
    if (LLenadoAuto == 1)
      return "Setear llenado minima";
    if (LLenadoAuto == 2)
      return "Setear llenado a calentar";
    if (LLenadoAuto == 3)
      return "Confirmar seteo";
  }

  return String();
}

void Enviar_Serial(uint8_t WhatSend)
{
  char DatosEnviarSerial[22];
  switch (WhatSend)
  {
  case 1:
    if (Calentando == false)
      Estado = 'O';
    if (Calentando == true)
      Estado = 'I';
    sprintf(DatosEnviarSerial, "W_%c%c", TempActual + 34, Estado);
    Serial.println(DatosEnviarSerial);
    break;
  case 2:
    if (Llenando == false)
      Estado = 'O';
    if (Llenando == true)
      Estado = 'I';
    sprintf(DatosEnviarSerial, "F_%c%c", NivelActual + 34, Estado);
    Serial.println(DatosEnviarSerial);
    break;
  case 3:
    sprintf(DatosEnviarSerial, "K_%c%c%c%c", save[SlotGuardado].Hora + 34, save[SlotGuardado].Temp + 34, save[SlotGuardado].Nivel + 34, SlotGuardado + 48);
    Serial.println(DatosEnviarSerial);
    break;
  case 4:
    sprintf(DatosEnviarSerial, "H_%c%c", TemperaturaMinima + 34, TemperaturaCalentar + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 5:
    sprintf(DatosEnviarSerial, "C_%c%c", NivelMinimo + 34, NivelCalentar + 34);
    Serial.println(DatosEnviarSerial);
    break;
  case 6:
    sprintf(DatosEnviarSerial, "I_%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    Serial.println(DatosEnviarSerial);
    break;
  case 7:
    Serial.println("E_E");
  }
}

void Leer_Serial()
{
  String Serial_Input;
  uint8_t StringLength;
  String Individualdata[4];
  char input;
  bool datosObtenidos;

  Serial_Input=Serial.readString();// iguala el string del serial a un string imput
  StringLength= Serial_Input.length();// saca el largo del string
  input=Serial_Input.charAt(0); // toma el char de comando (el primer char usualmente una letra)
  memset(Datos, 0, 22);
  for (uint8_t CharPos = 0; CharPos <= StringLength-2; CharPos++) // comeinza desde la posicion 2 del char (tras el _) y toma todos los datos
  {
      if(CharPos<StringLength-2)Datos[CharPos]=Serial_Input.charAt(CharPos+2); //si hay : divide los datos
      if(CharPos==StringLength-2)datosObtenidos=true;// activa el comando final (flag)
  } 

  if (datosObtenidos==true){
    
    switch (input)//dependiendo del char de comando
    {
    case 'N':
      Serial.println(Datos);
      break;
    case 'P':
            datosObtenidos=false;
    break;
    case 'U':
          datosObtenidos=false;
      break;
    case 'J':
          datosObtenidos=false;
      break;
    case 'V':
            datosObtenidos=false;
      break;
    case '?':
            // RESET INO
            datosObtenidos=false;
        break;
    default:
      break;
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