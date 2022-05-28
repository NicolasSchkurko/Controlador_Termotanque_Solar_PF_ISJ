#include <Arduino.h>
#include <ESP8266WiFi.h>
const char* ssid = "WifiChuco";
const char* password = "AloAmbAr!";
WiFiServer server(80);
 
  int CALENTAR_AGUA = 7; // Pin digital para el LED_1
  String Estado_1 = "OFF"; // Estado_1 del LED_1 inicialmente "OFF"c

  int CARGAR_AGUA = 6; // Pin digital para el LED_1
  String Estado_2 = "OFF"; // Estado_1 del LED_1 inicialmente "OFF"c

  
  void setup()
  {
     Serial.begin(9600);
  

  // Connect to your Wi-Fi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

   
  void loop()
  {

        }
      }
      // Dar tiempo al navegador para recibir los datos
    }
  }


  
