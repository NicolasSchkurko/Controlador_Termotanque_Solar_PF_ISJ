#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

const char* ssid = "ssid";
const char* password = "contraseña";

IPAddress ip(192,168,0,33);     
IPAddress gateway(192,168,0,1);   
IPAddress subnet(255,255,255,0);

const int Rele_Pin = 2;

String Estado_Pin;

AsyncWebServer server(80);

String processor(const String& var)
{
  if(var == "ESTADO_RELE")
    {
     if(digitalRead(Rele_Pin)==1)
     {
      Estado_Pin= "Encendido";
     }
     else
     {
      Estado_Pin = "Apagado";
     }
    return Estado_Pin;
    }
  
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(Rele_Pin, OUTPUT);

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/estilos.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/estilos.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/RELE=ON", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(Rele_Pin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/RELE=OFF", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(Rele_Pin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.begin();
}
void loop() {

}
