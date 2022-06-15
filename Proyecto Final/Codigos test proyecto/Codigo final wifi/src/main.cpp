#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

const char auto_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>Automatization</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
</head><body>
  <p class="titulo">Temperature: <span id="TEMPSliderValue">%TEMPVALUE%</span>°C</p>
    <div class="slidertemp"><input type="range" id="temp" name="temp" min="40"  class="slidertemp" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TEMPVALUE%" oninput="this.nextElementSibling.value = this.value"> </div>
    <script>
    function updateSliderTEMP(element) {
        var sliderTEMPValue = document.getElementById("temp").value;
        document.getElementById("TEMPSliderValue").innerHTML = sliderTEMPValue;
        console.log(sliderTEMPValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?temp="+sliderTEMPValue, true);
        xhr.send();
      }
    </script>
    <p><button class="Settemp" onclick=location.href="/SETEMP" >%SETTEMP%</button></p> 
    <div id="set">
      <h5> temp start min: %TEMPMIN% Automatic temp max: %TEMPMAX%  </h5>
    </div>
    <p class="titulo">Water level: <span id="LVLSliderValue">%LVLVALUE%</span>℅</p>
    <div class="sliderlvl"><input type="range" id="lvl" name="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVLVALUE%"  oninput="this.nextElementSibling.value = this.value"></div>
        <script>
    function updateSliderLVL(element) {
        var sliderLVLValue = document.getElementById("lvl").value;
        document.getElementById("LVLSliderValue").innerHTML = sliderLVLValue;
        console.log(sliderLVLValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?nivel="+sliderLVLValue, true);
        xhr.send();
      }
    </script>
    <p><button class="SetLVL" onclick=location.href="/SETLVL" >%SETLVL%</button></p> 
    <div id="set">
      <h5>Automatic level start min: %LVLMIN% Automatic level max: %LVLMAX% </h5>   
    </div>
    <p><button class="return" onclick=location.href="/RETURN" >RETURN TO MAIN PAGE</button></p> 
</body>
</html>
  )rawliteral";

const char sethour_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>Configuration</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    </head><body>
        <p><span id="HOURSliderValue">%HOURVALUE%</span>:<span id="MINUTESliderValue">%MINUTEVALUE%</span></p>
        <p class="titulo">Temperature: <span id="TEMPSliderValue">%TEMPVALUE%</span>°C</p>
        <p class="titulo">Water level: <span id="LVLSliderValue">%LVLVALUE%</span>℅</p>
    <div class="slidertemp"><input type="range" id="temp" name="temp" min="40"  class="slidertemp" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TEMPVALUE%" oninput="this.nextElementSibling.value = this.value">
  </div>
    <script>
    function updateSliderTEMP(element) {
        var sliderTEMPValue = document.getElementById("temp").value;
        document.getElementById("TEMPSliderValue").innerHTML = sliderTEMPValue;
        console.log(sliderTEMPValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?temp="+sliderTEMPValue, true);
        xhr.send();
      }
    </script>
    <div class="sliderlvl">
        <input type="range" id="lvl" name="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVLVALUE%"  oninput="this.nextElementSibling.value = this.value">

      </div>
        <script>
    function updateSliderLVL(element) {
        var sliderLVLValue = document.getElementById("lvl").value;
        document.getElementById("LVLSliderValue").innerHTML = sliderLVLValue;
        console.log(sliderLVLValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?nivel="+sliderLVLValue, true);
        xhr.send();
      }
    </script>
    <div class="sliderhour">
        <input type="range" id="hour" name="hour" min="0" max="23" step="1" onchange="updateSliderHOUR(this)" class="slider" value="%HOURVALUE%"  oninput="this.nextElementSibling.value = this.value">
      </div>
        <script>
    function updateSliderHOUR(element) {
        var sliderHOURValue = document.getElementById("hour").value;
        document.getElementById("HOURSliderValue").innerHTML = sliderHOURValue;
        console.log(sliderHOURValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?hour="+sliderHOURValue, true);
        xhr.send();
      }
    </script>
        <div class="sliderminute">
        <input type="range" id="minute" name="minute" min="0" max="45" step="15" onchange="updateSliderMINUTE(this)" class="slider" value="%MINUTEVALUE%"  oninput="this.nextElementSibling.value = this.value">
      </div>
        <script>
    function updateSliderMINUTE(element) {
        var sliderMINUTEValue = document.getElementById("minute").value;
        document.getElementById("MINUTESliderValue").innerHTML = sliderMINUTEValue;
        console.log(sliderMINUTEValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?minute="+sliderMINUTEValue, true);
        xhr.send();
      }
    </script>
    <body>
    <p><button class="save1" onclick=location.href="/SAVE1" >Save in slot 1</button></p> 
    <p><button class="save2" onclick=location.href="/SAVE2" >Save in slot 2</button></p> 
    <p><button class="save3" onclick=location.href="/SAVE3" >Save in slot 3</button></p> 
    </body>
    <p><button class="return" onclick=location.href="/RETURN" >RETURN TO MAIN PAGE</button></p> 
</body>
</html>
  )rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>SLB_x02</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    </head><body>
    <div class="slidertemp"><input type="range" id="temp" name="temp" min="40"  class="slidertemp" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TEMPVALUE%" oninput="this.nextElementSibling.value = this.value">
    <p><span id="TEMPSliderValue">%TEMPVALUE%</span></p>
    <div class="caja">
      <p><button class="button2" onclick=location.href="/TEMP=ON"> %BUTTON_TEMP%</button></p> 
      <p class="titulo">Estado: %ESTADO_TEMP%</p>
  </div>
    <script>
    function updateSliderTEMP(element) {
        var sliderTEMPValue = document.getElementById("temp").value;
        document.getElementById("TEMPSliderValue").innerHTML = sliderTEMPValue;
        console.log(sliderTEMPValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?temp="+sliderTEMPValue, true);
        xhr.send();
      }
    </script>
    <div class="sliderlvl">
        <input type="range" id="lvl" name="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVLVALUE%"  oninput="this.nextElementSibling.value = this.value">
        <p><span id="LVLSliderValue">%LVLVALUE%</span></p>
        <div class="caja">
          <p><button class="button" onclick=location.href="/LVL=ON">%BUTTON_LVL%</button></p> 
          <p class="titulo">Estado: %ESTADO_LVL%</p>
      </div>
        <script>
    function updateSliderLVL(element) {
        var sliderLVLValue = document.getElementById("lvl").value;
        document.getElementById("LVLSliderValue").innerHTML = sliderLVLValue;
        console.log(sliderLVLValue);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?nivel="+sliderLVLValue, true);
        xhr.send();
      }
    </script>
    <div id="seteos_hora">
      <div class="caja-automatizacion">
          <div id="set-1">
            <h3>Start by timer 1</h3>
            <p>Hour: %HOUR1% 
              Water level: %WATERLEVEL1% 
              Water temp:%WATERTEMP1%
          </div>        
          <div id="set-2">
            <h3>Start by timer 2</h3>
            <p>Hour: %HOUR2% 
              Water level: %WATERLEVEL2% 
              Water temp:%WATERTEMP2%
          </div>        
          <div id="set-1">
            <h3>Start by timer 3</h3>
            <p>Hour: %HOUR3% 
              Water level: %WATERLEVEL3% 
              Water temp:%WATERTEMP3%
          </div>        
      </div>
    </div>
  <p><button class="config" onclick=location.href="/TIMERSET" >Set timer </button></p> 
  <p><button class="config" onclick=location.href="/AUTOMATIZATION" >Set automatization </button></p> 
</body>
</html>
)rawliteral";



struct save{char hora;char tempylvl;};
struct save UNO{.hora=93,.tempylvl=105 };
struct save DOS{.hora=73,.tempylvl=25 };
struct save TRES{.hora=255,.tempylvl=255 };
int minuto=0;
int horas=0;
int temp=0;
int nivel=0;
char save=1;
char hora=0;
char tempylvl=0;
int nivel_temp=0;
int temp_temp=0;
char TEMP_VAL=0;
boolean TEMP_STATE=0;
char LVL_VAL=0;
boolean LVL_STATE=0;
char HOUR_VAL=0;
char MINUTE_VAL=0;
char auto_start_config=0;

const char* ssid = "Proyectos1";
const char* password = "proy.Tec";
String sliderTEMPValue = "60";
String sliderLVLValue = "70";
String sliderHOURValue = "12";
String sliderMINUTEValue = "30";
const char* PARAM_INPUT_TEMP = "temp";
const char* PARAM_INPUT_NIVEL = "nivel";
const char* PARAM_INPUT_HOUR = "hour";
const char* PARAM_INPUT_MINUTE = "minute";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
String processor(const String& var){
  if(UNO.hora!=255 && UNO.tempylvl!=255)
{
horas=10;
minuto=10;
temp=10;
nivel=10;
String output= (String) horas;
String output2= (String) minuto;
String output3= (String) temp;
String output4= (String) nivel;
  if(var == "HOUR1")return output+":"+output2;
  if(var == "WATERLEVEL1")return output3;
  if(var == "WATERTEMP1")return output4;
}
  if(var == "HOUR2")return "23:22";
  if(var == "WATERLEVEL2")return "22";
  if(var == "WATERTEMP2")return "22";
  if(var == "HOUR3")return "22:22";
  if(var == "WATERLEVEL3")return "22";
  if(var == "WATERTEMP3")return "22";
  if(var == "LVLVALUE")return sliderLVLValue;
  if(var == "TEMPVALUE")return sliderTEMPValue;
  if(var == "HOURVALUE")return sliderHOURValue;
  if(var == "MINUTEVALUE")return sliderMINUTEValue;
  if(var == "BUTTON_TEMP"){if(TEMP_STATE==1)return "TURN OFF"; else return "TURN ON";}
  if(var == "ESTADO_TEMP"){if(TEMP_STATE==1)return "calentamiento encendido"; else return "calentamiento apagado";}
  if(var == "ESTADO_LVL"){if(LVL_STATE==1)return "llenado encendido";else return "llenado apagado";}
  if(var == "BUTTON_LVL"){if(LVL_STATE==1)return "TURN OFF";else return "TURN ON";}   
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
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

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT_TEMP)) {
      inputMessage = request->getParam(PARAM_INPUT_TEMP)->value();
      sliderTEMPValue = inputMessage;
      TEMP_VAL = sliderTEMPValue.toInt();
      request->send_P(200, "text/html", index_html, processor);
    }
    if (request->hasParam(PARAM_INPUT_NIVEL)) {
      inputMessage = request->getParam(PARAM_INPUT_NIVEL)->value();
      sliderLVLValue = inputMessage;
      LVL_VAL = sliderLVLValue.toInt();
      request->send_P(200, "text/html", index_html, processor);
    }
    if (request->hasParam(PARAM_INPUT_HOUR)) {
      inputMessage = request->getParam(PARAM_INPUT_HOUR)->value();
      sliderHOURValue = inputMessage;
      HOUR_VAL = sliderHOURValue.toInt();
      request->send_P(200, "text/html", index_html, processor);
    }
    if (request->hasParam(PARAM_INPUT_MINUTE)) {
      inputMessage = request->getParam(PARAM_INPUT_MINUTE)->value();
      sliderMINUTEValue = inputMessage;
      MINUTE_VAL = sliderMINUTEValue.toInt();
      request->send_P(200, "text/html", index_html, processor);
    }
  });
  server.on("/TEMP=ON", HTTP_GET, [](AsyncWebServerRequest *request){if(TEMP_STATE==0)TEMP_STATE=1; else TEMP_STATE=0; request->send_P(10, "text/html", index_html, processor);}); 
  server.on("/LVL=ON", HTTP_GET, [](AsyncWebServerRequest *request){if(LVL_STATE==0)LVL_STATE=1; else LVL_STATE=0; request->send_P(10, "text/html", index_html, processor);});
  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request){ auto_start_config=1; request->send_P(200, "text/html", sethour_html, processor);});
  server.on("/SAVE1", HTTP_GET, [](AsyncWebServerRequest *request){ save=1; request->send_P(2000, "text/html", sethour_html, processor);});
  server.on("/SAVE2", HTTP_GET, [](AsyncWebServerRequest *request){ save=2; request->send_P(2000, "text/html", sethour_html, processor);});
  server.on("/SAVE3", HTTP_GET, [](AsyncWebServerRequest *request){ save=3; request->send_P(2, "text/html", sethour_html, processor);});
  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request){  request->send_P(2, "text/html", index_html, processor);});
    server.on("/AUTOMATIZATION", HTTP_GET, [](AsyncWebServerRequest *request){  request->send_P(2, "text/html", auto_html, processor);});
  // Start server
  server.begin();
}
  
void loop() {
  if(auto_start_config==1){
  int print;
  print= TEMP_VAL;
  Serial.println(print);
  print= TEMP_STATE;
  Serial.println(print);
  print= LVL_VAL;
  Serial.println(print);
  print= LVL_STATE;
  Serial.println(print);
  auto_start_config=0;
  }
  
}

