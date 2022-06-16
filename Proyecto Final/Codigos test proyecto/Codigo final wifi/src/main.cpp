#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
String processor(const String& var);

const char settimer_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>Set timer</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    </head><body>
    <p class="titl">Hour:<span id="HSV">%HVAL%</span>:<span id="MSV">%MVAL%</span></p>
    <p class="titl">Temperature: <span id="TSV">%TVAL%</span>°C</p>
    <p class="titl">Water level: <span id="LSV">%LVALUE%</span>℅</p>
    <input type="range" id="temp"  min="40"  class="slider" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TVAL%" oninput="this.nextElementSibling.value = this.value">
  <script>
    function updateSliderTEMP(element) {
        var TVal = document.getElementById("temp").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?temp="+TVal, true);
        xhr.send();
      }
    </script>
    <input type="range" id="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVAL%"  oninput="this.nextElementSibling.value = this.value">
    <script>
    function updateSliderLVL(element) {
        var LVal = document.getElementById("lvl").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?nivel="+LVal, true);
        xhr.send();
      }
    </script>
    <input type="range" id="hour"  min="0" max="23" step="1" onchange="updateSliderHOUR(this)" class="slider" value="%HVAL%"  oninput="this.nextElementSibling.value = this.value">
    <script>
    function updateSliderHOUR(element) {
        var HVal = document.getElementById("hour").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?hour="+HVal, true);
        xhr.send();
      }
    </script>
    <input type="range" id="minute" min="0" max="45" step="15" onchange="updateSliderMINUTE(this)" class="slider" value="%MVAL%"  oninput="this.nextElementSibling.value = this.value">
    <script>
    function updateSliderMINUTE(element) {
        var MVal = document.getElementById("minute").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?minute="+MVal, true);
        xhr.send();
      }
    </script>
    <p><button class="save1" onclick=location.href="/SAVE1" >Save in slot 1</button></p> 
    <p><button class="save2" onclick=location.href="/SAVE2" >Save in slot 2</button></p> 
    <p><button class="save3" onclick=location.href="/SAVE3" >Save in slot 3</button></p> 
    <p><button class="return" onclick=location.href="/RETURN" >RETURN TO MAIN PAGE</button></p> 
</body>
</html>
  )rawliteral";

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'> <meta name="viewport" content="width=device-width, user-scalable=no"><title>SLB_x02</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    </head><body>
    <input type="range" id="temp"  min="40"  class="slider" max="80" step="5" onchange="updateSliderTEMP(this)" value="%TVAL%" oninput="this.nextElementSibling.value = this.value">
    <p><span id="TSV">%TVAL%</span></p>
    <p><button class="button" onclick=location.href="/STATEMP">%BTNT%</button></p> 
    <div id="auto"><h5> temp start min: %TMIN% Automatic temp max: %TMAX%  </h5></div>
    <p><button class="Set" onclick=location.href="/SETEMP" >%STTA%</button></p> 
    <script>
    function updateSliderTEMP(element){
        var TVal = document.getElementById("temp").value;
        console.log(TVal);
        document.getElementById("TSV").innerHTML = TVal;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?temp="+TVal, true);
        xhr.send();}
    </script>
    <input type="range" id="lvl" min="40" max="100" step="10" onchange="updateSliderLVL(this)" class="slider" value="%LVAL%"  oninput="this.nextElementSibling.value = this.value">
    <p><span id="LSV">%LVAL%</span></p>
    <p><button class="button" onclick=location.href="/STALVL">%BTNL%</button></p> 
    <div id="auto"><h5> Automatic level start min: %LMIN% Automatic level max: %LMAX%  </h5></div>
    <p><button class="SetLVL" onclick=location.href="/SETLVL">%STLA%</button></p> 
    <script>
    function updateSliderLVL(element) {
        var LVal = document.getElementById("lvl").value;
        console.log(LVal);
        document.getElementById("LSV").innerHTML = LVal;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?lvl="+LVal, true);
        xhr.send();
      }
    </script>
<div id="Set"><h3>Start by timer 1</h3><p>Hour: %H1% Water level: %L1% Water temp:%T1%       
<h3>Start by timer 2</h3><p>Hour: %H2% Water level: %L2% Water temp:%T2%       
<h3>Start by timer 3</h3><p>Hour: %H3% Water level: %L3% Water temp:%T3%</div>        
  <p><button class="config" onclick=location.href="/TIMERSET" >Set timer </button></p>  
</body>
</html>
)rawliteral";
struct save{char hora;char temp;char lvl;};

const char* ssid = "WifiChuco";
const char* password = "AloAmbAr!";

String TVal = "60";
String LVal = "70";
String HVal = "12";
String MVal = "30";

char TEMP_VAL=0;
char LVL_VAL=0;
char HOUR_VAL=0;
char MINUTE_VAL=0;

bool CHARGING_STATE=0;
bool HEATING_STATE=0;

char AUTOTEMP_STATE=0;
char AUTOLVL_STATE=0;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void setup(){
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connectarado");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Setea la pagina main 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, "text/html", index_html, processor);});

  //Toma datos del slider y los guarda en una variable
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) { 
    
    String inputMessage;
    if (request->hasParam("temp")) {
      inputMessage = request->getParam("temp")->value();
      TVal= inputMessage;
      TEMP_VAL = TVal.toInt();
      request->send_P(260, "text/html", index_html, processor);
    }

    if (request->hasParam("nivel")) {
      inputMessage = request->getParam("nivel")->value();
      LVal = inputMessage;
      LVL_VAL = LVal.toInt();
      request->send_P(260, "text/html", index_html, processor);
    }
    if (request->hasParam("hour")) {
      inputMessage = request->getParam("hour")->value();
      HVal = inputMessage;
      HOUR_VAL = HVal.toInt();
      request->send_P(260, "text/html", index_html, processor);
    }
    if (request->hasParam("minute")) {
      inputMessage = request->getParam("minute")->value();
      MVal = inputMessage;
      MINUTE_VAL = MVal.toInt();
      request->send_P(260, "text/html", index_html, processor);
    }
  });

  //detecta cuando se redirige (producto de que se presiona un boton) en la mainpage y realiza algo
  server.on("/STATEMP", HTTP_GET, [](AsyncWebServerRequest *request){HEATING_STATE= !HEATING_STATE;  request->send_P(500, "text/html", index_html, processor);}); 

  server.on("/STALVL", HTTP_GET, [](AsyncWebServerRequest *request){CHARGING_STATE= !CHARGING_STATE; request->send_P(500, "text/html", index_html, processor);});

  server.on("/SETEMP", HTTP_GET, [](AsyncWebServerRequest *request){AUTOTEMP_STATE++; if(AUTOTEMP_STATE>3)AUTOTEMP_STATE=0;  request->send_P(500, "text/html", index_html, processor);});

  server.on("/SETLVL", HTTP_GET, [](AsyncWebServerRequest *request){AUTOLVL_STATE++; if(AUTOLVL_STATE>3)AUTOLVL_STATE=0; request->send_P(500, "text/html", index_html, processor);});

  server.on("/TIMERSET", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(500, "text/html", settimer_html, processor);});

  //detecta cuando se  presiona un boton en la set timer by page y realiza algo
  server.on("/S1", HTTP_GET, [](AsyncWebServerRequest *request){
    ; request->send_P(500, "text/html", settimer_html, processor);
    });

  server.on("/S2", HTTP_GET, [](AsyncWebServerRequest *request){
    ; request->send_P(500, "text/html", settimer_html, processor);
    });

  server.on("/S3", HTTP_GET, [](AsyncWebServerRequest *request){
    ; request->send_P(500, "text/html", settimer_html, processor);
    });

  server.on("/RETURN", HTTP_GET, [](AsyncWebServerRequest *request){; request->send_P(500, "text/html", index_html, processor);});

  // Start server
  server.begin();
}
  
void loop() {}
  
  

String processor(const String& var){

if(var == "TVAL")return TVal;
if(var == "LVAL")return LVal;
if(var == "HVAL")return HVal;
if(var == "MVAL")return MVal;

if(var == "BTNT"){
if(HEATING_STATE==1)return "llenado encendido"; 
else return "llenado apagado";
}

if(var == "BTNL"){
if(CHARGING_STATE==1)return "llenado encendido"; 
else return "llenado apagado";
}

if(var == "STTA"){
  if(AUTOTEMP_STATE==0)return "Setear calentamiento automatico";
  if(AUTOTEMP_STATE==1)return "Setear temperatura minima";
  if(AUTOTEMP_STATE==2)return "Setear temperatura a calentar";
  if(AUTOTEMP_STATE==3)return "Confirmar seteo";
}

if(var == "STLA"){
  if(AUTOLVL_STATE==0)return "Setear llenado automatico";
  if(AUTOLVL_STATE==1)return "Setear llenado minima";
  if(AUTOLVL_STATE==2)return "Setear llenado a calentar";
  if(AUTOLVL_STATE==3)return "Confirmar seteo";
}

return String();
}

