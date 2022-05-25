#include <Arduino.h>
#include "ESP8266WiFi.h"//aquí incluimos la libreria para comunicación WiFi del ESP8266
#include <SPI.h> 
const char* ssid = "WifiChuco";
const char* password = "AloAmbAr!";
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
 
  // Coincide con la solicitud
 
  int value = HIGH;
  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(ledPin, HIGH);
    value =HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(ledPin,LOW);
    value = LOW;
  }

  int value1 = HIGH;
  if (request.indexOf("/LED1=ON") != -1)  {
    digitalWrite(ledPin1, HIGH);
    value1 = HIGH;
  }
  if (request.indexOf("/LED1=OFF") != -1)  {
    digitalWrite(ledPin1, LOW);
    value1 = LOW;
  }

  int value2 = HIGH;
  if (request.indexOf("/LED2=ON") != -1)  {
    digitalWrite(ledPin2, HIGH);
    value2 = HIGH;
  }
  if (request.indexOf("/LED2=OFF") != -1)  {
    digitalWrite(ledPin2, LOW);
    value2 = LOW;
  }
 
 // Establecer ledPin de acuerdo a la solicitud
 // devuelve la respuesta
 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Led pin es ahora: ");
 
  if(value == HIGH) {
    client.print("Off");
  } else {
    client.print("On");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>LED ROJO ON </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>LED ROJO OFF </button></a><br />");  
  client.println("</html>"); 
 
  // Establecer ledPin1 de acuerdo a la solicitud
  // devuelve la respuesta  
   
  client.print("Led pin 1 es ahora: ");
 
  if(value1 == HIGH) {
    client.print("Off");
  } else {
    client.print("On");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED1=ON\"\"><button>LED VERDE ON </button></a>");
  client.println("<a href=\"/LED1=OFF\"\"><button>LED VERDE OFF </button></a><br />");  
  client.println("</html>");
  
 
  // Establecer ledPin2 de acuerdo a la solicitud
  // devuelve la respuesta
  
  client.print("Led pin 2 es ahora: ");
 
  if(value2 == HIGH) {
    client.print("Off");
  } else {
    client.print("On");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED2=ON\"\"><button>LED AZUL ON </button></a>");
  client.println("<a href=\"/LED2=OFF\"\"><button>LED AZUL OFF </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

