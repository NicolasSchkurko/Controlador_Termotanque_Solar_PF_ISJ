#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

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
        xhr.open("GET", "/slider?temp="+sliderTEMPValue, true);
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
        xhr.open("GET", "/slider?nivel="+sliderLVLValue, true);
        xhr.send();
      }
    </script>
    <div id="seteos_hora">
      <div class="caja-automatizacion">
          <div id="set-1">
            <h3>Automatic start 1</h3>
            <p>Hour: %HOUR1% 
              Water level: %WATERLEVEL1% 
              Water temp:%WATERTEMP1%
          </div>        
          <div id="set-2">
            <h3>Automatic start 2</h3>
            <p>Hour: %HOUR2TEST% 
              Water level: %WATERLEVEL2% 
              Water temp:%WATERTEMP2%
          </div>        
          <div id="set-1">
            <h3>Automatic start 3</h3>
            <p>Hour: %HOUR3% 
              Water level: %WATERLEVEL3% 
              Water temp:%WATERTEMP3%
          </div>        
      </div>
    </div>
  <p><button class="config" onclick=location.href="/CONFIGURATION" >Auto-start configuration</button></p> 
</body>
</html>
)rawliteral";

void desconvercionhora(char hora,char tempylvl);
void convercionhora(int horas, int minuto,int nivel, int temp);

struct save{char hora;char tempylvl;};
struct save UNO{.hora=93,.tempylvl=105 };
struct save DOS{.hora=73,.tempylvl=025 };
struct save TRES{.hora=255,.tempylvl=255 };
// Replace with your network credentials
const char* ssid = "Proyectos1";
const char* password = "proy.Tec";

int minuto=0;
int horas=0;
int temp=0;
int nivel=0;

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
// Replaces placeholder with button section in your web page
String processor(const String& var){
  

if(UNO.hora!=255 && UNO.tempylvl!=255)
{
desconvercionhora(UNO.hora,UNO.tempylvl);
String output= (String) horas;
String output2= (String) minuto;
String output3= (String) temp;
String output4= (String) nivel;
  if(var == "HOUR1")return output+":"+output2;
  if(var == "WATERLEVEL1")return output3;
  if(var == "WATERTEMP1")return output4;
}
  if(var == "LVLVALUE")return sliderLVLValue;
  if(var == "TEMPVALUE")return sliderTEMPValue;
  if(var == "HOURVALUE")return sliderHOURValue;
  if(var == "MINUTEVALUE")return sliderMINUTEValue;
  if(var == "ESTADO_TEMP"){if(TEMP_STATE==1)return "calentamiento encendido"; else return "calentamiento apagado";}
  if(var == "ESTADO_LVL"){if(LVL_STATE==1)return "llenado encendido";else return "llenado apagado";}
    
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
  server.on("/TEMP=ON", HTTP_GET, [](AsyncWebServerRequest *request){TEMP_STATE=1; request->send_P(10, "text/html", index_html, processor);}); 
  
  server.on("/TEMP=OFF", HTTP_GET, [](AsyncWebServerRequest *request){TEMP_STATE=0; request->send_P(10, "text/html", index_html, processor);});

  server.on("/LVL=ON", HTTP_GET, [](AsyncWebServerRequest *request){LVL_STATE=1; request->send_P(10, "text/html", index_html, processor);});

  server.on("/LVL=OFF", HTTP_GET, [](AsyncWebServerRequest *request){LVL_STATE=0; request->send_P(200, "text/html", index_html, processor);});

  server.on("/CONFIGURATION", HTTP_GET, [](AsyncWebServerRequest *request){ auto_start_config=1; request->send_P(200, "text/html", sethour_html, processor);});
  // Start server
  server.begin();
}
  
void loop() {
  if(auto_start_config==1){
    convercionhora(HOUR_VAL,MINUTE_VAL,LVL_VAL,TEMP_VAL );
    UNO.hora=hora;
    UNO.tempylvl=tempylvl;
  }
    if(auto_start_config==2){
    convercionhora(HOUR_VAL,MINUTE_VAL,LVL_VAL,TEMP_VAL );
    DOS.hora=hora;
    DOS.tempylvl=tempylvl;
  }
    if(auto_start_config==3){
    convercionhora(HOUR_VAL,MINUTE_VAL,LVL_VAL,TEMP_VAL );
    TRES.hora=hora;
    TRES.tempylvl=tempylvl;
  }
    if(auto_start_config==0){
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
  
}

void desconvercionhora(char hora,char tempylvl)
{
int aux=0;

minuto=(hora*15)-aux;
if (minuto==60)
  {
    aux=60;
    horas++;
  }
int aux2=0;
int digito=0;
int digito_cal=0;

switch (tempylvl)
{
  case 255:
    temp=0;
    nivel=0;
    minuto=0;
    horas=0;
    break;

  default:
    while(tempylvl!=0)
    {
      digito=tempylvl%10;
      tempylvl=tempylvl/10;
      aux2++;
      if(aux2==3)aux2=0;
    }
    if(aux2>=0 && aux2<=1)
    {
      switch (aux2)
      {
        case 0:
        digito_cal=digito*10;
          break;
        case 1:
        digito_cal=digito+digito_cal;
          break;
      }
      digito_cal=digito_cal/2;
      nivel=50+(digito_cal*5);
    }
    if(aux2==2)temp=40+(digito*5);
      break;
}
}
void convercionhora(int horas, int minuto,int nivel, int temp)
{
char hora=0;
char tempylvl=0;
int nivel_temp=0;
int temp_temp=0;

hora=(minuto/15)+(horas/15);
nivel_temp=((nivel-50)/5)*20;
temp_temp=(temp-40)/5;
tempylvl=nivel_temp+temp_temp;
}