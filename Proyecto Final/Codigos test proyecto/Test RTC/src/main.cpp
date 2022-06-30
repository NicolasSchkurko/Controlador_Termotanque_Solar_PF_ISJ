/**
   GeekFactory - "INNOVATING TOGETHER"
   Distribucion de materiales para el desarrollo e innovacion tecnologica
   www.geekfactory.mx
   Ejemplo para arduino de escritura de fecha y hora al DS1307 en el
   modulo TinyRTC. Este ejemplo pone a tiempo el reloj con los datos
   provistos en la funcion setup.
*/
#include <Wire.h>
#include <Arduino.h>
// Declaracion de las variables para almacenar informacion de tiempo
uint8_t second, minute, hour, wday, day, month, year;
bool write_ds1307();
uint8_t bin2bcd(uint8_t bin);
void print_time();

/**
   Inicio del sketch: Este codigo se ejecuta al conectar el arduino
*/
void setup() {
  // NOTA: Estas sentencias se requieren para alimentar directamente el
  // chip RTC desde los pines A3 Y A2 (colocar directamente el modulo sobre
  // la tarjeta arduino, sin la necesidad de cablear en Arduino UNO).
  // Si no se quiere hacer esto, se pueden eliminar o comentar sin problemas
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  // Configurar la comunicacion a 9600 baudios
  Serial.begin(9600);
  // Preparar la librería Wire (I2C)
  Wire.begin();
  // Imprimir encabezado a la terminal
  Serial.println("----------------------------------------------------");
  Serial.println(" EJEMPLO ESCRITURA DE DS1307 EN TINYRTC CON ARDUINO ");
  Serial.println("            https://www.geekfactory.mx               ");
  Serial.println("----------------------------------------------------");
  // Inicializar las variables con el tiempo a cargar en el modulo RTC
  // ¡¡¡ COLOCA AQUI LA FECHA Y HORA QUE QUIERAS GRABAR EN EL RTC !!!
  second = 00;
  minute = 14;
  hour = 15;
  wday = 5;
  day = 30;
  month = 6;
  year = 22;
}
/**
   Ciclo principàl del sketch: Escribir informacion de fecha y hora una sola vez
*/
void loop() {
  // Escribir hora y fecha al RTC
  write_ds1307();
  // Imprimimos la fecha y hora
  Serial.println("Poner a tiempo el DS1307 en Tiny RTC:");
  print_time();
  // Ciclo infinito, no hacemos nada mas
  for (;;);
}
/**
   Esta funcion establece la cominicación con el DS1307 y escribe los registros
   de fecha y hora. Utiliza la información provista en la función setup que debes
   modificar con la fecha y hora actuales
*/
bool write_ds1307()
{
  // Iniciar el intercambio de información con el DS1307 (0x68)
  Wire.beginTransmission(0x68);
  // Escribir la dirección del registro segundero
  Wire.write(0x00);
  // Escribir valores en los registros, nos aseguramos que el bit clock halt
  // en el registro del segundero este desactivado (esto hace que el reloj funcione)
  Wire.write(bin2bcd(second & 0x7F)); // <--- Esto hace que el reloj comience a trabajar
  Wire.write(bin2bcd(minute));
  Wire.write(bin2bcd(hour));
  Wire.write(bin2bcd(wday));
  Wire.write(bin2bcd(day));
  Wire.write(bin2bcd(month));
  Wire.write(bin2bcd(year));
  // Terminamos la escritura y verificamos si el DS1307 respondio
  // Si la escritura se llevo a cabo el metodo endTransmission retorna 0
  if (Wire.endTransmission() != 0)
    return false;
  // Retornar verdadero si se escribio con exito
  return true;
}
/**
   Convierte un numero binario a BCD
*/
uint8_t bin2bcd(uint8_t bin)
{
  return (bin / 10 * 16) + (bin % 10);
}
/**
   Imprime la fecha y hora al monitor serial de arduino
*/
void print_time()
{
  Serial.print("Fecha: ");
  Serial.print(day);
  Serial.print('/');
  Serial.print(month);
  Serial.print('/');
  Serial.print(year);
  Serial.print("  Hora: ");
  Serial.print(hour);
  Serial.print(':');
  Serial.print(minute);
  Serial.print(':');
  Serial.print(second);
  Serial.println();
}