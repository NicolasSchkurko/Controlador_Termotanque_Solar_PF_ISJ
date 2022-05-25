#include <Arduino.h>
#include "ESP8266WiFi.h"//aquí incluimos la libreria para comunicación WiFi del ESP8266
#include <SPI.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Proyectos1";
const char* password = "proy.Tec";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,50);
String readString; 
WiFiServer server(80);
void SET_WIFI(); 
void RUN_WIFI();

int nivel = 0; // LED Rojo
int ledPin1 = D2; // LED Verde
int ledPin2 = D3; // LED Azul


void setup() {
  Serial.begin(9600);
  delay(10);
 
   // Inicia LED rojo apagado

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
 const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Brightness Control Web Server</title>
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #38c0ff  ;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background:#01070a; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #01070a; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>ESP32 Brightness Control Web Server</h2>
  <p><span id="textslider_value">%SLIDERVALUE%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="255" value="%SLIDERVALUE%" step="1" class="slider"></p>
<script>
function updateSliderPWM(element) {
  var slider_value = document.getElementById("pwmSlider").value;
  document.getElementById("textslider_value").innerHTML = slider_value;
  console.log(slider_value);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+slider_value, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";
    Serial.println("Client disonnected");
    Serial.println("");
}

