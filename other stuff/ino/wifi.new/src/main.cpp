#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

// Replace with your network credentials
const char* ssid = "Proyectos1";
const char* password = "proy.Tec";

const int output = 2;
char TEMP_VAL=0;
char TEMP_STATE=0;
char LVL_VAL=0;
char LVL_STATE=0;
String sliderTEMPValue = "60";
String sliderLVLValue = "70";

const char* PARAM_INPUT = "values";
const char* PARAM_INPUT2 = "value";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>SLB_x02</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    </head><body>
    <div class="slidertemp"><input type="range" id="temp" name="temp" min="40"  class="slidertemp" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TEMPVALUE%" oninput="this.nextElementSibling.value = this.value">
    <p><span id="TEMPSliderValue">%TEMPVALUE%</span></p>
    <div class="caja">
      <p><button class="button2" onclick=location.href="/TEMP=ON"> Encender</button></p> 
      <p><button class="button" onclick=location.href="/TEMP=OFF" >Apagar</button></p> 
      <p class="titulo">Estado: %ESTADO_TEMP%</p>
  </div>
    <script>
    function updateSliderTEMP(element) {
        var sliderTEMPValue = document.getElementById("temp").value;
        document.getElementById("TEMPSliderValue").innerHTML = sliderTEMPValue;
        console.log(sliderTEMPValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?values="+sliderTEMPValue, true);
        xhr.send();
      }
    </script>
    <div class="sliderlvl">
        <input type="range" id="lvl" name="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVLVALUE%"  oninput="this.nextElementSibling.value = this.value">
        <p><span id="LVLSliderValue">%LVLVALUE%</span></p>
        <div class="caja">
          <p><button class="button2" onclick=location.href="/LVL=ON"> Encender</button></p> 
          <p><button class="button" onclick=location.href="/LVL=OFF" >Apagar</button></p> 
          <p class="titulo">Estado: %ESTADO_LVL%</p>
      </div>
        <script>
    function updateSliderLVL(element) {
        var sliderLVLValue = document.getElementById("lvl").value;
        document.getElementById("LVLSliderValue").innerHTML = sliderLVLValue;
        console.log(sliderLVLValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?value="+sliderLVLValue, true);
        xhr.send();
      }
    </script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  
  if (var == "LVLVALUE"){
    return sliderLVLValue;
  }
  if (var == "TEMPVALUE"){
    return sliderTEMPValue;
  }
  if(var == "ESTADO_TEMP"){
      if(TEMP_STATE==1)return "calentamiento encendido";
      else return "calentamiento apagado";
    }
  if(var == "ESTADO_LVL"){
      if(LVL_STATE==1)return "llenado encendido";
      else return "llenado apagado";
    }
    
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderTEMPValue = inputMessage;
      TEMP_VAL = sliderTEMPValue.toInt();
      request->send_P(200, "text/html", index_html, processor);
    }
    if (request->hasParam(PARAM_INPUT2)) {
      inputMessage = request->getParam(PARAM_INPUT2)->value();
      sliderLVLValue = inputMessage;
      LVL_VAL = sliderLVLValue.toInt();
      request->send_P(200, "text/html", index_html, processor);

    }
  });
  server.on("/TEMP=ON", HTTP_GET, [](AsyncWebServerRequest *request){TEMP_STATE=1; request->send_P(10, "text/html", index_html, processor);}); 
  
  server.on("/TEMP=OFF", HTTP_GET, [](AsyncWebServerRequest *request){TEMP_STATE=0; request->send_P(10, "text/html", index_html, processor);});

  server.on("/LVL=ON", HTTP_GET, [](AsyncWebServerRequest *request){LVL_STATE=1; request->send_P(10, "text/html", index_html, processor);});

  server.on("/LVL=OFF", HTTP_GET, [](AsyncWebServerRequest *request){LVL_STATE=0; request->send_P(10, "text/html", index_html, processor);});
  // Start server
  server.begin();
}
  
void loop() {
  int print;
  print= TEMP_VAL;
  Serial.println(print);
  print= TEMP_STATE;
  Serial.println(print);
  print= LVL_VAL;
  Serial.println(print);
  print= LVL_STATE;
  Serial.println(print);
  delay(5000);
}