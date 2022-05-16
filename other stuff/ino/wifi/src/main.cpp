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
 
 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset='utf-8'>");
  client.println(<meta name="viewport" content="width=device-width, user-scalable=no">);
  client.println(<meta http-equiv='X-UA-Compatible' content='IE=edge'>);
  client.println(<title>SLB_x02</title>);
  client.println(<meta name='viewport' content='width=device-width, initial-scale=1'>);
  client.println(<link rel='stylesheet' type='text/css' media='screen' href='main.css'>);
  client.println(<script src='main.js'></script>);
  client.println(<style>);
  client.println(      body{);
  client.println(          font-family: monospace;);
  client.println(          background-color: rgb(23, 6, 18););
  client.println(      });
  client.println(      h1{);
  client.println(          color: aliceblue;);
  client.println(          text-align: center;);
  client.println(          margin: 90px auto 20px;;   );
  client.println(          font-size: 20px;);
  client.println(      });
  client.println(      .BarraTemp{);
  client.println(          height: 200px;);
  client.println(          width: 200px;);
  client.println(          background-color: yellow;);
  client.println(          margin: auto;);
  client.println(          border-radius: 50%;);
  client.println(          overflow: hidden;);
  client.println(      });
  client.println(       .cont{);
 client.println(           height: 100px;);
 client.println(           width: 200px;);
 client.println(       });
 client.println(       .centroTemp{);
 client.println(           height: 180px;);
 client.println(           width: 200px;);
 client.println(           background-color: rgb(23, 6, 18););
 client.println(           margin: auto;);
 client.println(           border-radius: 50%;);
 client.println(           position: relative;);
 client.println(           margin: -172px auto 50px;);
client.println(        });
 client.println(       .slider{);
 client.println(           width: 300px;);
 client.println(           margin: auto;);
 client.println(           position: relative;);
 client.println(       });
 client.println(       #temp{);
 client.println(       height: 19px;);
 client.println(       -webkit-appearance: none;);
 client.println(       width: 100%;);
 client.println(       margin:0 auto;);
  client.println(      });
 client.println(       #temp::-webkit-slider-runnable-track {);
 client.println(       width: 500px;);
 client.println(       height: 20px;);
 client.println(       cursor: pointer;);
 client.println(       animation: 0.2s;);
 client.println(       box-shadow: 0px 0px 0px #50555C;);
 client.println(       background: #C70039;);
 client.println(       border-radius: 0px;);
 client.println(       border: 2px solid #000000;);
 client.println(       text-align: center;);
 client.println(       });
 client.println(       #temp::-webkit-slider-thumb {);
 client.println(       border: 2px solid #000000;);
 client.println(       height: 20px;);
 client.println(       width: 40px;);
 client.println(       border-radius: 0px;);
 client.println(       background: yellow;);
 client.println(       cursor: pointer;);
 client.println(       -webkit-appearance: none;);
 client.println(       margin-top: -2px;);
 client.println(       });
 client.println(       #TEMPNUMB{);
 client.println(           color: aliceblue;);
 client.println(           text-align: center;);
 client.println(           font-size: 20px;);
 client.println(           margin:auto;);
 client.println(           position: relative;);
 client.println(       });
 client.println(       .switchtemp input {);
 client.println(       position: absolute;);
 client.println(       opacity: 0;);
 client.println(       });
 client.println(       .switchtemp {);
 client.println(       margin: auto;);
 client.println(       position: relative;);
 client.println(        right: -260px;);
 client.println(        margin: -100px auto 10px;);
 client.println(        display: inline-block;);
 client.println(        font-size: 20px; /* 1 */);
 client.println(        height: 1em;);
 client.println(        width: 2em;);
 client.println(        background: #ff0000;);
 client.println(        border-radius: 1em;);
 client.println(        });
 client.println(        .switchtemp div {);
 client.println(        height: 1em;);
 client.println(        width: 1em;);
 client.println(        border-radius: 1em;);
 client.println(        background: rgb(229, 255, 0););
 client.println(        -webkit-transition: all 300ms;);
 client.println(        -moz-transition: all 300ms;);
 client.println(        transition: all 300ms;);
 client.println(        });
 client.println(        .switchtemp input:checked + div {);
 client.println(        -webkit-transform: translate3d(100%, 0, 0););
 client.println(        -moz-transform: translate3d(100%, 0, 0););
 client.println(        transform: translate3d(100%, 0, 0););
 client.println(        });
 client.println(        #lvl{);
 client.println(        height: 19px;);
 client.println(        -webkit-appearance: none;);
 client.println(        width: 100%;);
 client.println(        margin:0 auto;);
 client.println(        });
 client.println(        #lvl::-webkit-slider-runnable-track {);
 client.println(        width: 500px;);
 client.println(        height: 20px;);
 client.println(        cursor: pointer;);
 client.println(        animation: 0.2s;);
 client.println(        box-shadow: 0px 0px 0px #50555C;);
 client.println(        background: #C70039;);
 client.println(        border-radius: 0px;);
 client.println(        border: 2px solid #000000;);
 client.println(        text-align: center;);
 client.println(        });
 client.println(        #lvl::-webkit-slider-thumb {);
 client.println(        border: 2px solid #000000;);
 client.println(        height: 20px;);
 client.println(        width: 40px;);
 client.println(        border-radius: 0px;);
 client.println(        background: yellow;);
 client.println(        cursor: pointer;);
 client.println(        -webkit-appearance: none;);
 client.println(        margin-top: -2px;);
 client.println(        });
 client.println(        #TEMPLVL{);
 client.println(            color: aliceblue;);
 client.println(            text-align: center;);
 client.println(            font-size: 20px;);
 client.println(            margin:auto;);
 client.println(            position: relative;);
 client.println(        });
 client.println(        .switchLVL input {);
 client.println(        position: absolute;);
 client.println(        opacity: 0;);
 client.println(        });
 client.println(        .switchLVL {);
 client.println(        margin: auto;);
 client.println(        position: relative;);
 client.println(        right: -260px;);
 client.println(        margin: -100px auto 10px;);
 client.println(        display: inline-block;);
 client.println(        font-size: 20px; /* 1 */);
 client.println(        height: 1em;);
 client.println(        width: 2em;);
 client.println(        background: #ff0000;);
 client.println(        border-radius: 1em;);
 client.println(        });
 client.println(        .switchLVL div {);
 client.println(        height: 1em;);
 client.println(        width: 1em;);
 client.println(        border-radius: 1em;);
 client.println(        background: rgb(229, 255, 0););
 client.println(        -webkit-transition: all 300ms;);
 client.println(        -moz-transition: all 300ms;);
 client.println(        transition: all 300ms;);
 client.println(        });
 client.println(        .switchLVL input:checked + div {);
 client.println(        -webkit-transform: translate3d(100%, 0, 0););
 client.println(        -moz-transform: translate3d(100%, 0, 0););
 client.println(        transform: translate3d(100%, 0, 0););
 client.println(        });
 client.println(    </style>);
 client.println(    <link rel="stylesheet" type="text/css" href="desing.css");
client.println( </head>);
client.println( <body>);
 client.println(    <h1>Mediciones en tiempo real</h1>);
 client.println(    <h1>Temperatura</h1>);
 client.println(    <div class="BarraTemp" style="transform: rotate(35deg);">);
 client.println(        <div class="cont" style=" background-color: rgb(255, 195, 0)"></div>);
 client.println(        <div class="cont" style=" background-color: rgb(199, 0, 57)"></div>);
 client.println(    </div>);
 client.println(    <div class="centroTemp"></div>);
client.println(     <h1 style="font-size: 35px; position: relative; margin: -180px auto 20px;">50°c</h1>);
 client.println(    <div class="slider">);
 client.println(        <input type="range" id="temp" name="temp" min="0" max="100" step="5" oninput="this.nextElementSibling.value = this.value">);
 client.println(        <output id="TEMPNUMB">50</output>);
 client.println(        <div class="container">);
 client.println(            <label class="switchtemp"><input type="checkbox" />    <div></div>);
 client.println(            </label>);
 client.println(          </div>);
 client.println(    </div> );
 client.println(    <h1>Nivel de agua</h1>);
 client.println(    <div class="BarraTemp" style="transform: rotate(35deg);">);
 client.println(        <div class="cont" style=" background-color: rgb(255, 195, 0)"></div>);
 client.println(        <div class="cont" style=" background-color: rgb(199, 0, 57)"></div>);
 client.println(    </div>);
 client.println(    <div class="centroTemp"></div>);
 client.println(    <h1 style="font-size: 35px; position: relative; margin: -180px auto 20px;">50%</h1>);
 client.println(    <div class="slider">);
 client.println(        <input type="range" id="lvl" name="lvl" min="0" max="100" step="5" oninput="this.nextElementSibling.value = this.value">);
 client.println(        <output id="TEMPLVL">50</output>);
 client.println(        <div class="container">);
 client.println(            <label class="switchLVL"><input type="checkbox" />    <div></div>);
 client.println(            </label>);
 client.println(          </div>);
 client.println(    </div>);
client.println( </body>);
client.println( </html>);
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

