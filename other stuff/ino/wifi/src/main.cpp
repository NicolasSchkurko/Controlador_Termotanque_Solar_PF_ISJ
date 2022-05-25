#include <Arduino.h>
#include "ESP8266WiFi.h"//aquí incluimos la libreria para comunicación WiFi del ESP8266
#include <SPI.h> 
const char* ssid = "Proyectos1";
const char* password = "proy.Tec";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,50);
String readString; 
WiFiServer server(80);
void SET_WIFI(); 
void RUN_WIFI();

int ledPin = D1; // LED Rojo
int ledPin1 = D2; // LED Verde
int ledPin2 = D3; // LED Azul


void setup() {
  Serial.begin(9600);
  delay(10);
 
  pinMode(ledPin, OUTPUT);   // Inicia LED rojo apagado
  digitalWrite(ledPin, LOW);

  pinMode(ledPin1, OUTPUT);    // Inicia LED verde apagado
  digitalWrite(ledPin1, LOW);

  pinMode(ledPin2, OUTPUT);   // Inicia LED azul apagado
  digitalWrite(ledPin2, LOW);
 
  SET_WIFI();
}
 
void SET_WIFI() {
 // Conectarse a la red WiFi
  Serial.println();
  Serial.println();
  Serial.print("Conectando ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
 
  // Iniciar el servidor
  server.begin();
  Serial.println("Server iniciado");
 
  // Imprimir la dirección IP
  Serial.print("URL ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  RUN_WIFI();
}
  
void RUN_WIFI(){
 // Comprueba si cliente se ha conectado
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
// Espera hasta que el cliente envíe algunos datos.
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Lee la primera línea requerida
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset='utf-8'><meta name=\"viewport\" content=\"width=device-width, user-scalable=no\"><meta http-equiv='X-UA-Compatible' content='IE=edge'><title>SLB_x02</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'><script src='main.js'></script><style>");
  client.println("body{font-family: monospace;background-color: rgb(23, 6, 18);}h1{color: aliceblue;text-align: center;margin: 90px auto 20px;;   font-size: 20px;}.BarraTemp{height: 200px;width: 200px;background-color: yellow;margin: auto;border-radius: 50%;overflow: hidden;}.cont{height: 100px;width: 200px;}.centroTemp{height: 180px;width: 200px;background-color: rgb(23, 6, 18);margin: auto;border-radius: 50%;position: relative;margin: -172px auto 50px;}.slider{width: 300px;margin: auto;position: relative;}#temp{height: 19px;-webkit-appearance: none;width: 100%;margin:0 auto;}#temp::-webkit-slider-runnable-track {width: 500px;height: 20px;cursor: pointer;animation: 0.2s;box-shadow: 0px 0px 0px #50555C;background: #C70039;border-radius: 0px;border: 2px solid #000000;text-align: center;}#temp::-webkit-slider-thumb {border: 2px solid #000000;height: 20px;width: 40px;border-radius: 0px;background: yellow;cursor: pointer;-webkit-appearance: none;margin-top: -2px;}#TEMPNUMB{color: aliceblue;text-align: center;font-size: 20px;margin:auto;position: relati}.switchtemp input {position: absolute;opacity: 0; }.switchtemp {margin: auto;position: relative;right: -260px;margin: -100px auto 10px;display: inline-block;font-size: 20px; /* 1 */height: 1em;width: 2em; background: #ff0000;border-radius: 1em;} .switchtemp div {height: 1em;width: 1em;border-radius: 1em;background: rgb(229, 255, 0);-webkit-transition: all 300ms;-moz-transition: all 300ms;transition: all 300ms;}.switchtemp input:checked + div {-webkit-transform: translate3d(100%, 0, 0);-moz-transform: translate3d(100%, 0, 0);transform: translate3d(100%, 0, 0);} #lvl{height: 19px;-webkit-appearance: none;width: 100%;margin:0 auto;} #lvl::-webkit-slider-runnable-track {width: 500px;height: 20px;cursor: pointer;animation: 0.2s;box-shadow: 0px 0px 0px #50555C;background: #C70039;border-radius: 0px;border: 2px solid #000000;text-align: center;}#lvl::-webkit-slider-thumb {border: 2px solid #000000;height: 20px;width: 40px;border-radius: 0px;background: yellow;cursor: pointer;-webkit-appearance: none;margin-top: -2px;}#TEMPLVL{color: aliceblue;text-align: center;font-size: 20px;margin:auto;position: relative;}.switchLVL input {position: absolute;opacity: 0;}.switchLVL {margin: auto;position: relative;right: -260px;margin: -100px auto 10px;display: inline-block;font-size: 20px; /* 1 */height: 1em;width: 2em;background: #ff0000;border-radius: 1em;}.switchLVL div {height: 1em;width: 1em;border-radius: 1em;background: rgb(229, 255, 0);-webkit-transition: all 300ms;-moz-transition: all 300ms;transition: all 300ms;}.switchLVL input:checked + div {-webkit-transform: translate3d(100%, 0, 0);-moz-transform: translate3d(100%, 0, 0);transform: translate3d(100%, 0, 0);}</style><link rel=\"stylesheet\" type=\"text/css\" href=\"desing.css\">");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Mediciones en tiempo real</h1>");
  client.println("<h1>Temperatura</h1>");
  client.println("<div class=\"BarraTemp\" style=\"transform: rotate(35deg);\">");
  client.println("<div class=\"cont\" style=\" background-color: rgb(255, 195, 0)\"></div>");
  client.println("<div class=\"cont\"style=\" background-color: rgb(199, 0, 57)\"></div>");
  client.println("</div>");
  client.println("<div class=\"centroTemp\"></div>");
  client.println("<h1 style=\"font-size: 35px; position: relative; margin: -180px auto 20px;\">50°c</h1>");
  client.println("<div class=\"slider\">");
  client.println("<input type=\"range\" id=\"temp\" name=\"temp\" min=\"0\" max=\"100\" step=\"5\" oninput=\"this.nextElementSibling.value = this.value\">");
  client.println("<output id=\"TEMPNUMB\">50</output>");
  client.println("<div class=\"container\">");
  client.println("<label class=\"switchtemp\"><input type=\"checkbox\"/>    <div></div>");
  client.println("</label>");
  client.println("</div>");
  client.println("</div>");
  client.println("<h1>Nivel de agua</h1>");
  client.println("<div class=\"BarraTemp\"style=\"transform: rotate(35deg);\">");
  client.println("<div class=\"cont\" style=\" background-color: rgb(255, 195, 0)\"></div>");
  client.println("<div class=\"cont\"style=\" background-color: rgb(199, 0, 57)\"></div></div>");
  client.println("<div class=\"centroTemp\"></div>");
  client.println("<h1 style=\"font-size: 35px; position: relative; margin: -180px auto 20px;\">50%</h1>");
  client.println("<div class=\"slider\">");
  client.println("<input type=\"range\" id=\"lvl\" name=\"lvl\"min=\"0\" max=\"100\" step=\"5\" oninput=\"this.nextElementSibling.value = this.value\">");
  client.println("<output id=\"TEMPLVL\">50</output>");
  client.println("<div class=\"container\">");
  client.println("<label class=\"switchLVL\"><input type=\"checkbox\" />    <div></div>");
  client.println("</label></div></div></body></html>");
    delay(1);
    Serial.println("Client disonnected");
    Serial.println("");
}

