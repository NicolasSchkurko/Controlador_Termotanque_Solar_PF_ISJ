#include <Arduino.h>
#include <ESP8266WiFi.h>

 
 const char* ssid = "WifiChuco";
const char* password = "AloAmbAr!";
  // Declaración de la direcciones MAC,IP,GATEWAY y SUBNET.
  
  // Creamos un servidor Web con el puerto 80 que es el puerto HTTP por defecto
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
    WiFiClient client = server.available(); // Creamos un cliente Web
    // Cuando detecte un cliente a través de una petición HTTP
    if (client)
    {
      Serial.println(); // Salto de línea
      Serial.println("Nuevo cliente");
      Serial.println();
      boolean currentLineIsBlank = true; // Una petición HTTP acaba con una línea en blanco
      String cadena=""; // Creamos una cadena de caracteres vacía
      
      while (client.connected())
      {
        if (client.available())
        {
          char c = client.read();// Leemos la petición HTTP carácter por carácter
          Serial.write(c);// Visualizamos la petición HTTP por el Monitor Serial
          cadena.concat(c);// Unimos el String 'cadena' con la petición HTTP (c).
          // De esta manera convertimos la petición HTTP a un String
   
           // Ya que hemos convertido la petición HTTP a una cadena de caracteres, ahora podremos buscar partes del texto.
           int posicion_1=cadena.indexOf("CALENTAR_AGUA="); // Guardamos la posición de la instancia "LED_1=" a la variable 'posicion_1'
           int posicion_2=cadena.indexOf("CARGAR_AGUA="); // Guardamos la posición de la instancia "LED_1=" a la variable 'posicion_1'
   
            if(cadena.substring(posicion_1)=="CALENTAR_AGUA=ON") // Si en la posición 'posicion_1' hay "LED_1=ON"
            {
              digitalWrite(CALENTAR_AGUA,HIGH);
              Estado_1="ON";
            }
            if(cadena.substring(posicion_1)=="CALENTAR_AGUA=OFF") // Si en la posición 'posicion_1' hay "LED_1=OFF"
            {
              digitalWrite(CALENTAR_AGUA,LOW);
              Estado_1="OFF";
            }


            if(cadena.substring(posicion_2)=="CARGAR_AGUA=ON") 
            {
              digitalWrite(CARGAR_AGUA,HIGH);
              Estado_2="ON";
            }
            if(cadena.substring(posicion_2)=="CARGAR_AGUA=OFF") 
            {
              digitalWrite(CARGAR_AGUA,LOW);
              Estado_2="OFF";
            }
   
          // Cuando reciba una línea en blanco, quiere decir que la petición HTTP ha acabado y el servidor Web está listo
          // para enviar una respuesta
          if (c == '\n' && currentLineIsBlank)
          {
              // Enviamos al cliente una respuesta HTTP
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/html");
              
              client.println("Connection: close");  // the connection will be closed after completion of the response
              client.println("Refresh: 5");  // refresh the page automatically every 5 sec
              client.println();
              client.println("<!DOCTYPE HTML>");
      
              // Página web en formato HTML
              client.println("<html>");
              client.println("<head>");
              client.println("</head>");
              client.println("<body>");
              client.println("<br/><br/>");
              client.println("<h1 align='center'>Control LED - Servidor Web</h1>");
              // Creamos los botones.
              // Para enviar parámetros a través de HTML se utiliza el método URL encode.
              // Los parámetros se envían a través del símbolo '?'
              //--------------------------------------------------
              client.println("<div style='text-align:center;'>");
              client.println("<label><b>LED_1 &nbsp;&nbsp;&nbsp;</b></label>");
              client.println("<button onClick=location.href='./?CALENTAR_AGUA=ON\' style='margin:auto;background-color: #84B1FF;color: snow;padding: 10px;border: 1px solid #3F7CFF;width:85px;'>");
              client.println("ON");
              client.println("</button>");
              client.println("<button onClick=location.href='./?CALENTAR_AGUA=OFF\' style='margin:auto;background-color: #84B1FF;color: snow;padding: 10px;border: 1px solid #3F7CFF;width:85px;'>");
              client.println("OFF");
              client.println("</button>");
              client.println("<br/><br/>");
              client.println("<b>Estado_1 - ");
              client.print(Estado_1);       
              client.println("</div>");
              client.println("</b><br/>");
              //--------------------------------------------------     
              client.println("<div style='text-align:center;'>");
              client.println("<label><b>LED_2 &nbsp;&nbsp;&nbsp;</b></label>");
              client.println("<button onClick=location.href='./?CARGAR_AGUA=ON\' style='margin:auto;background-color: #84B1FF;color: snow;padding: 10px;border: 1px solid #3F7CFF;width:85px;'>");
              client.println("ON");
              client.println("</button>");
              client.println("<button onClick=location.href='./?CARGAR_AGUA=OFF\' style='margin:auto;background-color: #84B1FF;color: snow;padding: 10px;border: 1px solid #3F7CFF;width:85px;'>");
              client.println("OFF");
              client.println("</button>");
              client.println("<br/><br/>");
              client.println("<b>Estado_2 - ");
              client.print(Estado_2);       
              client.println("</div>");
              client.println("</b><br/>");
              //--------------------------------------------------   
              client.println("<div style='text-align:center;'>");
              client.println("<label><b>LECTURAS ANALOGAS</b></label>");
              client.println("<br />");
              client.println("<br />");
               // output the value of each analog input pin
              for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
                int sensorReading = analogRead(analogChannel);
                client.print("El Valor de la ecntrada Analoga ");
                client.print(analogChannel);
                client.print(" es ");
                client.print(sensorReading);
                client.println("<br />");
              }
              client.println("</div>");
              client.println("</b><br/>");
              //-------------------------------------------------- 
              
              client.println("</b></body>");
              client.println("</html>");
              break;
          }
          if (c == '\n')
          {
            currentLineIsBlank = true;
          }
          else if (c != '\r')
          {
            currentLineIsBlank = false;
          }
        }
      }
      // Dar tiempo al navegador para recibir los datos
    }
  }


  
