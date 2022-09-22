
// Libs
#include <Arduino.h>
#include <AT24CX.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "RTClib.h"
#include "Declaraciones.h"

//████████████████████████████████████████████████████████████████████
// Defines
// Control de temp
// Entradas y salida
AT24C32 eep;
RTC_DS1307 rtc;
DateTime now;
OneWire sensor_t(onewire);
DallasTemperature Sensor_temp(&sensor_t);
LiquidCrystal_I2C lcd(0x27, 20, 4); // LiquidCrystal_I2C lcd(0x27,20,4);

typedef enum
{
  posicion_inicial,
  llenado_manual,
  calefaccion_manual,
  funcion_menu_de_auto_por_hora,
  llenado_auto,
  calefaccion_auto,
  funcion_de_menu_modificar_hora_rtc,
  funcion_farenheit_celsius,
  funcion_activar_bomba,
  funcion_de_menu_seteo_wifi
} Seleccionar_Funciones;
typedef enum
{
  estado_standby,
  estado_inicial,
  menu1,
  menu2,
  funciones
} estadoMEF;

byte hora[8] = {
    B01110,
    B10101,
    B10101,
    B10111,
    B10001,
    B10001,
    B01110,
    B00000};
byte wifi[8] = {
    B00000,
    B00000,
    B11110,
    B00001,
    B11001,
    B00101,
    B10101,
    B00000};
byte barra_derecha[8] = {
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001};
byte barra_izquierda[8] = {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000};
byte barra_arriba[8] = {
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};
byte barra_abajo[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111};
Seleccionar_Funciones funcionActual = posicion_inicial;
estadoMEF Estadoequipo = estado_inicial;

char nombre_wifi_setear[20];
char password_wifi_setear[20];
char imprimir_lcd[20];

uint16_t tiempo_de_standby;
uint16_t mili_segundos = 0;
uint16_t tiempo_sensores;
uint8_t Actualchar = 0;
uint8_t Vaux1, Vaux2;
uint8_t Flag = 0;
uint8_t Posicion_actual; // es el "sumador" del encoder
uint8_t nivel_actual;
uint8_t temperatura_a_calentar;
uint8_t nivel_a_llenar;
int8_t temperatura_actual; // temp actual
uint8_t hora_to_modify, minuto_to_modify;
uint8_t PulsadorEncoder;
bool mayusculas = false;
bool llenar;
bool calentar;
bool esp_working = false;

//█████████████████████████████████████████████████████████████████████████████████

void setup()
{
  // Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2 | 0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  // pulsadores pra manejar los menusggg
  DDRD &= B00001111; // 0 input, 1 output
  DDRB &= B11111110;

  PORTD |= B11110000; // setea pull up o pull down
  PORTB |= B00000001; // 1 pull up 0 pull down
  // pines encoder
  attachInterrupt(PressedButton(2), doEncodeA, CHANGE);
  attachInterrupt(PressedButton(3), doEncodeB, CHANGE);
  // inicia sensor temp
  Wire.begin();
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);
  rtc.begin(); // inicializacion del rtc
  // inicializacion del serial arduino-esp
  Serial.begin(9600);
  Serial.setTimeout(200);

  lcd.init(); // Iniciacion del LCD
  lcd.createChar(0, hora);
  lcd.createChar(3, wifi);
  lcd.createChar(4, barra_abajo);
  lcd.createChar(5, barra_derecha);
  lcd.createChar(6, barra_izquierda);
  lcd.createChar(7, barra_arriba);
  // Sensr de temperatura
  eep.readChars(14, password_wifi_setear, 20);
  eep.readChars(34, nombre_wifi_setear, 20);

  while (Vaux1 <= 7)
  {
    if (Vaux1 == 0)
    {
      Serial_Send_UNO(4, 0);
      Vaux1++;
    }
    if (Vaux1 == 1)
    {
      Serial_Send_UNO(3, 0);
      Vaux1++;
    }
    if (Vaux1 == 2)
    {
      Serial_Send_UNO(5, 0);
      Vaux1++;
    }
    if (Vaux1 == 3)
    {
      Serial_Send_UNO(6, 0);
      Vaux1++;
    }
    if (Vaux1 == 4)
    {
      Serial_Send_UNO(2, 1);
      Vaux1++;
    }
    if (Vaux1 == 5)
    {
      Serial_Send_UNO(2, 2);
      Vaux1++;
    }
    if (Vaux1 == 6)
    {
      Serial_Send_UNO(2, 3);
      Vaux1++;
    }
    if (Vaux1 == 7)
    {
      Serial_Send_UNO(1, 0);
      Vaux1++;
    }
  }
  tiempo_de_standby = mili_segundos;
  Vaux1 = Posicion_actual;
  Estadoequipo = estado_inicial;
}

void loop()
{
  Actualizar_entradas();
  Actualizar_salidas();

  if (Serial.available() > 0)
    Serial_Read_UNO(); // si recibe un dato del serial lo lee

  switch (Estadoequipo)
  {
  case estado_standby:
    standby(); // sin backlight
    lcd.noBacklight();
    break;
  case estado_inicial:
    lcd.backlight();
    standby(); // con backlight
    break;
  case menu1:
    menu_basico();
    break;
  case menu2:
    menu_avanzado();
    break;
  case funciones:
    switch (funcionActual)
    {
    case llenado_manual:
      nivel_a_llenar = menu_de_llenado_manual();
      break;
    case calefaccion_manual:
      temperatura_a_calentar = menu_de_calefaccion_manual();
      break;
    case funcion_menu_de_auto_por_hora:
      menu_de_auto_por_hora(now.hour(), now.minute());
      break;
    case llenado_auto:
      menu_de_llenado_auto();
      break;
    case calefaccion_auto:
      menu_de_calefaccion_auto();
      break;
    case funcion_de_menu_modificar_hora_rtc:
      menu_modificar_hora_rtc(now.hour(), now.minute());
      break;
    case funcion_farenheit_celsius:
      menu_farenheit_celsius(); // Menu avanzado
      break;
    case funcion_activar_bomba:
      menu_activar_bomba(); // Menu avanzado
      break;
    case funcion_de_menu_seteo_wifi:
      menu_seteo_wifi(); // Menu avanzado
      break;
    }
    break;
  }
}

ISR(TIMER2_OVF_vect)
{
  mili_segundos++;
}

void doEncodeA()
{
  if (PressedButton(2) == PressedButton(3))
    Posicion_actual++;
  else
    Posicion_actual--;
}

void doEncodeB()
{
  if (PressedButton(2) != PressedButton(3))
    Posicion_actual++;
  else
    Posicion_actual--;
}

//███████████████████████████████████████CONTROL DE ENTRADAS/SALIDAS██████████████████████████████████████████

void Actualizar_entradas()
{
  if (mili_segundos >= tiempo_sensores + tiempo_para_temperatura)
  {
    if (Estadoequipo == estado_standby || Estadoequipo == estado_inicial)
    {
      Sensor_temp.requestTemperatures();
      temperatura_actual = Sensor_temp.getTempCByIndex(0);
      tiempo_sensores = mili_segundos;
      Serial_Send_UNO(1, 0);
    }
  }
  if (analogRead(nivel_del_tanque) < 100)
    nivel_actual = 0;
  if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)
    nivel_actual = 25;
  if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)
    nivel_actual = 50;
  if (analogRead(nivel_del_tanque) >= 512 && analogRead(nivel_del_tanque) < 768)
    nivel_actual = 75;
  if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)
    nivel_actual = 100;
  // Actualiza el rtc
  now = rtc.now();
}

void Actualizar_salidas()
{
  char Array_hora[6];

  if (temperatura_actual < temperatura_a_calentar)
    calentar = true;

  else if (temperatura_actual <= eep.read(10))            //Temp minima
    temperatura_a_calentar = eep.read(11);               //Temp maxima

  else if (temperatura_actual >= temperatura_a_calentar)
  {
    calentar = false;
    temperatura_a_calentar = 0;
  }

  if (nivel_actual < nivel_a_llenar)
    llenar = true;

  else if (nivel_actual <= eep.read(12))      //Nivel minimo
    nivel_a_llenar = eep.read(13);            //Nivel maximo

  else if (nivel_actual >= nivel_a_llenar)
  {
    llenar = false;
    nivel_a_llenar = 0;
  }

  if (eep.read(57) == 254)    //eep57 es la activacion de la bomba, si esta en 254 (valor maximo) equivale a un booleano en true
    PrintOutput(12, llenar); // bomba

  PrintOutput(11, llenar);   // electrovalvula
  PrintOutput(10, calentar); // resistencia

  Printhora(Array_hora, now.hour(), now.minute()); 

  for (uint8_t i = 0; i < 3; i++)
  {
    if (Hora_a_guardado(Array_hora) == eep.read(i * 3 + 1)) // compara hora con las guardadas y activa segun hora, multiplica por 3 para agrupar entre los tres "perfiles".
    {
      nivel_a_llenar = eep.read(i * 3 + 2);
      temperatura_a_calentar = eep.read(i * 3 + 3);
    }
  }
}

//███████████████████████████████████████████MENUES PRINCIPALES██████████████████████████████████████████
void standby()
{
  lcd.setCursor(5, 3); // imprime flechita o igual temperatura

  if (calentar)
    lcd.write(char(94));

  else
    lcd.write(char(61));

  lcd.setCursor(7, 3); // imprime C o F dependiendo de lo seteado
  lcd.print(char(CelciusOrFarenheit(temperatura_actual, 2)));

  lcd.setCursor(12, 3); // imprime bomba o bomba titilando con una cruz

  if (eep.read(57) == 254)
    lcd.print(char(244));

  else
  {
    if (now.second() % 2 == 1)
      lcd.write(char(244));
    else
      lcd.write('x');
  }

  lcd.setCursor(14, 3); // imprime flechita o igual llenado

  if (llenar)
    lcd.write(char(94));

  else
    lcd.write(char(61));

  lcd.setCursor(9, 3); // imprime logo wifi
  lcd.print(char(3));

  lcd.setCursor(10, 3); // imprime cruz o algo

  if (esp_working)
    lcd.print(char(175));

  else
    lcd.write('x');

  sprintf(imprimir_lcd, "%d%c", CelciusOrFarenheit(temperatura_actual, 1), char(223)); // temp
  PrintLCD(imprimir_lcd, 0, 0);

  sprintf(imprimir_lcd, "%d%c", nivel_actual, '%'); // nivel

  for (Vaux2 = 0; imprimir_lcd[Vaux2] != '\0'; Vaux2++)
  {
  }

  PrintLCD(imprimir_lcd, 20 - Vaux2, 0);

  if (4 - Vaux2 == 2)
    PrintLCD("  ", 16, 0);

  if (4 - Vaux2 == 1)
    PrintLCD(" ", 16, 0);

  lcd.setCursor(7, 1); // imprime relojito
  lcd.print(char(0));

  Printhora(imprimir_lcd, now.hour(), now.minute());
  PrintLCD(imprimir_lcd, 8, 1);

  PrintHorizontalBar(1, 1, temperatura_actual);
  PrintHorizontalBar(16, 1, nivel_actual);

  if (PressedButton(4) || PressedButton(5) || PressedButton(6) || PressedButton(7) || PressedButton(42))
    Posicion_actual += 1;

  if (Vaux1 != Posicion_actual)
  {
    switch (Estadoequipo)
    {
    case estado_standby:
      Estadoequipo = estado_inicial;
      break;
    case estado_inicial:
      Estadoequipo = menu1;
      lcd.clear();
      break;
    default:
      Estadoequipo = estado_standby;
      break;
    }
    Vaux1 = Posicion_actual;
    tiempo_de_standby = mili_segundos;
  }

  if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu && Estadoequipo == estado_inicial)
  {
    Estadoequipo = estado_standby;
    tiempo_de_standby = mili_segundos;
  }
}

void menu_basico()
{
  const char *menuprincipal[maxY_menu1] = {
      "Carga manual",
      "Calentado manual",
      "Seteo por hora",
      "Carga auto",
      "Calentado auto",
      "Menu avanzado",
      "Volver"};
  switch (Flag)
  {
  case 0:
    tiempo_de_standby = mili_segundos;
    lcd.clear();
    Vaux1 = 0;
    Posicion_actual = 0;
    Flag = 1;
    break;
  case 1:
    sprintf(imprimir_lcd, ">%s", menuprincipal[ReturnToCero(Vaux1, maxY_menu1)]);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%s", menuprincipal[ReturnToCero(Vaux1 + 1, maxY_menu1)]);
    PrintLCD(imprimir_lcd, 1, 1);
    sprintf(imprimir_lcd, "%s", menuprincipal[ReturnToCero(Vaux1 + 2, maxY_menu1)]);
    PrintLCD(imprimir_lcd, 1, 2);
    sprintf(imprimir_lcd, "%s", menuprincipal[ReturnToCero(Vaux1 + 3, maxY_menu1)]);
    PrintLCD(imprimir_lcd, 1, 3);

    if (Posicion_actual / 2 != Vaux1) // cada 2 pulsos del encoder baja/sube una columna del menu
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual / 2;
    }

    if (PressedButton(4))
      Posicion_actual += 2; // baja una columna

    if (PressedButton(5))
      Posicion_actual -= 2;
    ; // sube una columna

    if (PulsadorEncoder != 200 && mili_segundos >= tiempo_de_standby + 250)
      PulsadorEncoder = 200; // pulsador encoder == 200 mili segundos

    if (PressedButton(42) && mili_segundos >= tiempo_de_standby + PulsadorEncoder)
    { // va sumando +2 mientras acelera la diferencia en el que se suma 2
      Posicion_actual += 2;
      if (PulsadorEncoder == 200)
        PulsadorEncoder -= 20;
      if (PulsadorEncoder == 180)
        PulsadorEncoder -= 30;
      if (PulsadorEncoder == 150)
        PulsadorEncoder -= 50;
      if (PulsadorEncoder <= 100 && PulsadorEncoder >= 10)
        PulsadorEncoder -= 10;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, maxY_menu1 * 2);

    if (PressedButton(6))
    {
      tiempo_de_standby = mili_segundos;
      Flag = Vaux1 + 2;
      lcd.clear();
    }

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual;
      Estadoequipo = estado_inicial;
    }
    break;
  case 2:
    Flag = 0;
    Estadoequipo = funciones;
    funcionActual = llenado_manual;
    break;
  case 3:
    Flag = 0;
    Estadoequipo = funciones;
    funcionActual = calefaccion_manual;
    break;
  case 4:
    Flag = 0;
    Estadoequipo = funciones;
    funcionActual = funcion_menu_de_auto_por_hora;
    break;
  case 5:
    Flag = 0;
    Estadoequipo = funciones;
    funcionActual = llenado_auto;
    break;
  case 6:
    Flag = 0;
    Estadoequipo = funciones;
    funcionActual = calefaccion_auto;
    break;
  case 7:
    Flag = 0;
    Estadoequipo = menu2;
    break;
  case 8:
    Flag = 0;
    Vaux1 = Posicion_actual;
    tiempo_de_standby = mili_segundos;
    Estadoequipo = estado_inicial;
    break;
  }
}

void menu_avanzado()
{
  const char *menuavanzado[maxY_menu2] = {
      "Setear hora",
      "Cambiar unidad",
      "Habilitar la bomba",
      "conexion wifi",
      "volver"};
  switch (Flag)
  {
  case 0:
    tiempo_de_standby = mili_segundos;
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;
  case 1:
    sprintf(imprimir_lcd, ">%s", menuavanzado[ReturnToCero(Vaux1, maxY_menu2)]);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, " %s", menuavanzado[ReturnToCero(Vaux1 + 1, maxY_menu2)]);
    PrintLCD(imprimir_lcd, 0, 1);
    sprintf(imprimir_lcd, " %s", menuavanzado[ReturnToCero(Vaux1 + 2, maxY_menu2)]);
    PrintLCD(imprimir_lcd, 0, 2);
    sprintf(imprimir_lcd, " %s", menuavanzado[ReturnToCero(Vaux1 + 3, maxY_menu2)]);
    PrintLCD(imprimir_lcd, 0, 3);

    if (Posicion_actual / 2 != Vaux1)
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual / 2;
    }

    if (PressedButton(4))
      Posicion_actual += 2; // suma 1 a Vaux1
    if (PressedButton(5))
      Posicion_actual -= 2; // resta 1 a Vaux1

    if (PulsadorEncoder != 200 && mili_segundos >= tiempo_de_standby + 250)
      PulsadorEncoder = 200;

    if (PressedButton(42) && mili_segundos >= tiempo_de_standby + PulsadorEncoder)
    {
      Posicion_actual += 2;
      if (PulsadorEncoder == 200)
        PulsadorEncoder -= 20;
      if (PulsadorEncoder == 180)
        PulsadorEncoder -= 30;
      if (PulsadorEncoder == 150)
        PulsadorEncoder -= 50;
      if (PulsadorEncoder <= 100 && PulsadorEncoder >= 10)
        PulsadorEncoder -= 10;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, maxY_menu2 * 2);

    if (PressedButton(6))
    {
      Flag = Vaux1 + 2;
      tiempo_de_standby = mili_segundos;
      lcd.clear();
    }

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
    {
      tiempo_de_standby = mili_segundos;
      Vaux1 = Posicion_actual;
      lcd.clear();
      Estadoequipo = estado_inicial;
    }
    break;
  case 2:
    Estadoequipo = funciones;
    funcionActual = funcion_de_menu_modificar_hora_rtc;
    Flag = 0;
    break;
  case 3:
    Estadoequipo = funciones;
    funcionActual = funcion_farenheit_celsius;
    Flag = 0;
    break;
  case 4:
    Estadoequipo = funciones;
    funcionActual = funcion_activar_bomba;
    Flag = 0;
    break;
  case 5:
    Estadoequipo = funciones;
    funcionActual = funcion_de_menu_seteo_wifi;
    Flag = 0;
    break;
  case 6:
    Flag = 0;
    Posicion_actual = 0;
    Estadoequipo = menu1;
    break;
  }
}

//███████████████████████████████████████████MENUES FUNCIONALES██████████████████████████████████████████

uint8_t menu_de_llenado_manual()
{
  switch (Flag)
  /*Vaux1=Nivel a setear*/
  {
  case 0:
    if (min_nivel > nivel_actual)
      Vaux1 = min_temp;
    else
      Vaux1 = nivel_actual;
    Vaux1 = min_nivel;
    lcd.clear();
    Posicion_actual = 0;
    Flag = 1;
    break;
  case 1:
    memcpy(imprimir_lcd, "llenar hasta", 14);
    PrintLCD(imprimir_lcd, 2, 0);
    sprintf(imprimir_lcd, "%d%c ", Vaux1, '%');
    PrintLCD(imprimir_lcd, 15, 0);
    memcpy(imprimir_lcd, "1 +25", 7);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-25 2", 7);
    PrintLCD(imprimir_lcd, 15, 2);
    memcpy(imprimir_lcd, "3 seguir", 10);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;

    if (PressedButton(5))
      Posicion_actual -= 1;

    if ((Posicion_actual + 1) * sumador_nivel != Vaux1)
    {
      Vaux1 = (Posicion_actual + 1) * sumador_nivel; // actualiza el nivel a setear
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 2;
      lcd.clear();
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(7) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 4);
    break;

  case 2:

    memcpy(imprimir_lcd, "Llenar hasta:", 14);
    PrintLCD(imprimir_lcd, 1, 0);
    sprintf(imprimir_lcd, "%d%c", Vaux1, '%');
    PrintLCD(imprimir_lcd, 15, 0);
    memcpy(imprimir_lcd, "3 Guardar", 10);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      lcd.clear();
      Flag = 3;
    }

    if (PressedButton(7))
    {
      Flag = 1;
      Posicion_actual = 0;
      tiempo_de_standby = mili_segundos;
      lcd.clear();
    }
    break;

  case 3:
    tiempo_de_standby = mili_segundos;
    guardado_para_menus(true);
    return Vaux1;
    break;
  }
}

uint8_t menu_de_calefaccion_manual()
{
  /*Vaux2=temp a setear*/
  switch (Flag)
  {
  case 0:
    if (min_temp > temperatura_actual)
      Vaux2 = min_temp;
    else
      Vaux2 = (temperatura_actual / 5) * 5;
    lcd.clear();
    Posicion_actual = 0;
    Flag = 1;
    break;

  case 1:
    Posicion_actual = ReturnToCero(Posicion_actual, 9);

    memcpy(imprimir_lcd, "Calentar a", 11);
    PrintLCD(imprimir_lcd, 2, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2, 1), (char)223, CelciusOrFarenheit(Vaux2, 2));
    PrintLCD(imprimir_lcd, 13, 0);
    sprintf(imprimir_lcd, "1 +%d", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 0, 2);
    sprintf(imprimir_lcd, "-%d 2", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if ((Posicion_actual + 8) * sumador_temperatura != Vaux2)
    {
      Vaux2 = (Posicion_actual + 8) * sumador_temperatura; // actualiza el nivel a setear
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 2;
      Posicion_actual = 0;
      lcd.clear();
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(7) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }
    break;

  case 2:
    memcpy(imprimir_lcd, "Calentar a", 11);
    PrintLCD(imprimir_lcd, 2, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2, 1), (char)223, CelciusOrFarenheit(Vaux2, 2));
    PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "3 Guardar", 10);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 3;
      lcd.clear();
    }

    if (PressedButton(7))
    {
      Flag = 1;
      Posicion_actual = 0;
      tiempo_de_standby = 0;
      lcd.clear();
    }
    break;

  case 3:
    tiempo_de_standby = mili_segundos;
    guardado_para_menus(true);
    return Vaux2;
    break;
  }
}

void menu_de_auto_por_hora(uint8_t hora_actual, uint8_t minutos_actual)
{
  switch (Flag)
  /*  Vaux1=Nivel a setear
      Vaux2=temp a setear
      Actualchar = Struct;
      */
  {
  case 0:
    hora_to_modify = hora_actual;
    minuto_to_modify = minutos_actual;
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    // Save a int convierte el char de guardado en 1 la hora y en 2 el minuto guardado en la eeprom, por ende esto imprime la hora y los minutos

    memcpy(imprimir_lcd, "Seleccionar", 13);
    PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "1:", 4);
    PrintLCD(imprimir_lcd, 1, 1);
    memcpy(imprimir_lcd, "2:", 4);
    PrintLCD(imprimir_lcd, 1, 2);
    memcpy(imprimir_lcd, "3:", 4);
    PrintLCD(imprimir_lcd, 1, 3);
    Printhora(imprimir_lcd, Guardado_a_hora(1, eep.read(1)), Guardado_a_hora(2, eep.read(1)));
    PrintLCD(imprimir_lcd, 3, 1);
    Printhora(imprimir_lcd, Guardado_a_hora(1, eep.read(4)), Guardado_a_hora(2, eep.read(4)));
    PrintLCD(imprimir_lcd, 3, 3);
    Printhora(imprimir_lcd, Guardado_a_hora(1, eep.read(8)), Guardado_a_hora(2, eep.read(8)));
    PrintLCD(imprimir_lcd, 3, 2);
    memcpy(imprimir_lcd, ">", 2);
    PrintLCD(imprimir_lcd, 0, Actualchar + 1);

    if (Vaux1 != Actualchar)
    {
      memcpy(imprimir_lcd, " ", 2);
      PrintLCD(imprimir_lcd, 0, Vaux1 + 1);
      Vaux1 = Actualchar;
      tiempo_de_standby = mili_segundos;
    }

    Actualchar = Posicion_actual / 4;

    if (PressedButton(4))
      Posicion_actual += 4;
    if (PressedButton(5))
      Posicion_actual -= 4;
    if (PressedButton(42))
      Posicion_actual += 1;

    if (PressedButton(6))
    {
      Vaux2 = min_temp;
      Vaux1 = min_nivel;
      Posicion_actual = 0;
      lcd.clear();
      Flag = 2;
    }

    if (PressedButton(7) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    Posicion_actual = ReturnToCero(Posicion_actual, 12);
    break;

  case 2:
    memcpy(imprimir_lcd, "Calentar a", 11);
    PrintLCD(imprimir_lcd, 2, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2, 1), (char)223, CelciusOrFarenheit(Vaux2, 2));
    PrintLCD(imprimir_lcd, 13, 0);
    sprintf(imprimir_lcd, "1 +%d", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 0, 2);
    sprintf(imprimir_lcd, "-%d 2", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if ((Posicion_actual + 8) * sumador_temperatura != Vaux2)
    {
      Vaux2 = (Posicion_actual + 8) * sumador_temperatura;
    }

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (PressedButton(6))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 9);

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(imprimir_lcd, "Cargar a", 10);
    PrintLCD(imprimir_lcd, 4, 0);
    sprintf(imprimir_lcd, "%d%c ", Vaux1, '%');
    PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "1 +25", 20);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-25 2", 20);
    PrintLCD(imprimir_lcd, 15, 2);
    memcpy(imprimir_lcd, "3 Salir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if ((Posicion_actual + 1) * sumador_nivel != Vaux1)
    {
      Vaux1 = (Posicion_actual + 1) * sumador_nivel;
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (PressedButton(6))
    {
      Flag = 4;
      Posicion_actual = 0;
      lcd.clear();
      Posicion_actual = 0;
    }
    if (PressedButton(7))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }
    Posicion_actual = ReturnToCero(Posicion_actual, 4);

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 4:
    memcpy(imprimir_lcd, "Setear hora", 13);
    PrintLCD(imprimir_lcd, 1, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "1 +1", 5);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-1 2", 5);
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 Seguir", 10);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (hora_to_modify != ReturnToCero((hora_actual + Posicion_actual), hora_max))
    {
      hora_to_modify = ReturnToCero((hora_actual + Posicion_actual), hora_max);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (PressedButton(6))
    {
      Flag = 5;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, hora_max);

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 5:
    memcpy(imprimir_lcd, "Setear min", 12);
    PrintLCD(imprimir_lcd, 2, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "1 +1", 5);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-1 2", 5);
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 Seguir", 10);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (minuto_to_modify != ReturnToCero((minutos_actual + Posicion_actual), minuto_max))
    {
      minuto_to_modify = ReturnToCero((minutos_actual + Posicion_actual), minuto_max);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (PressedButton(6))
    {
      Flag = 6;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 4;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, minuto_max);

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 6:
    memcpy(imprimir_lcd, "A las", 7);
    PrintLCD(imprimir_lcd, 4, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 11, 0);
    memcpy(imprimir_lcd, "Calentar", 10);
    PrintLCD(imprimir_lcd, 3, 1);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2, 1), (char)223, CelciusOrFarenheit(Vaux2, 2));
    PrintLCD(imprimir_lcd, 13, 1);
    sprintf(imprimir_lcd, "Llenar %d%c", Vaux1, '%');
    PrintLCD(imprimir_lcd, 5, 2);
    memcpy(imprimir_lcd, "3 Guardar", 11);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      lcd.clear();
      Flag = 7;
    }

    if (PressedButton(7))
    {
      Flag = 5;
      lcd.clear();
      Posicion_actual = 0;
      tiempo_de_standby = mili_segundos;
    }
    break;

  case 7:
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    eep.write((Actualchar * 3) + 1, Hora_a_guardado(imprimir_lcd));

    eep.write((Actualchar * 3) + 2, Vaux1);
    eep.write((Actualchar * 3) + 3, Vaux2);

    Serial_Send_UNO(2, Actualchar);

    tiempo_de_standby = mili_segundos;
    guardado_para_menus(true);
    break;
  }
}

void menu_de_llenado_auto()
{
  switch (Flag)
  {
  case 0:
    Vaux1 = eep.read(12); // nivel minimo
    Vaux2 = eep.read(13); // nivel maximo
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(imprimir_lcd, "Nivel min", 11);
    PrintLCD(imprimir_lcd, 3, 0);
    sprintf(imprimir_lcd, "%d%c ", eep.read(12), '%');
    PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "1 +25", 6);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-25 2", 6);
    PrintLCD(imprimir_lcd, 15, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if ((Posicion_actual)*sumador_nivel != eep.read(12))
    {
      eep.write(12, (Posicion_actual)*sumador_nivel);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 2;
      eep.write(13, eep.read(12) + sumador_nivel);
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7) == true || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      eep.write(12, Vaux1);
      eep.write(13, Vaux2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 4);

    break;

  case 2:
    memcpy(imprimir_lcd, "Nivel max", 11);
    PrintLCD(imprimir_lcd, 3, 0);
    sprintf(imprimir_lcd, "%d%c ", eep.read(13), '%');
    PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "1 +25", 6);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-25 2", 6);
    PrintLCD(imprimir_lcd, 15, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5) == true)
      Posicion_actual -= 1;

    if ((Posicion_actual + 1 + (eep.read(12) / sumador_nivel)) * sumador_nivel != eep.read(13))
    {
      eep.write(13, ((Posicion_actual + 1 + (eep.read(12) / sumador_nivel)) * sumador_nivel));
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 4 - (eep.read(12) / sumador_nivel));

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(12, Vaux1);
      eep.write(13, Vaux2);
      lcd.clear();
    }
    break;

  case 3:
    sprintf(imprimir_lcd, "A el %d%c", eep.read(12), '%');
    PrintLCD(imprimir_lcd, 6, 0);
    sprintf(imprimir_lcd, "Llenar a %d%c", eep.read(13), '%');
    PrintLCD(imprimir_lcd, 4, 1);
    memcpy(imprimir_lcd, "3 Guardar", 11);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 4;
      lcd.clear();
    }

    if (PressedButton(7))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }
    break;

  case 4:
    Serial_Send_UNO(5, 0);
    guardado_para_menus(true);
    break;
  }
}

void menu_de_calefaccion_auto()
{
  switch (Flag)
  {
  case 0:
    Vaux1 = eep.read(10);
    Vaux2 = eep.read(11);
    eep.write(10, min_temp);
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(imprimir_lcd, "Temp. min", 11);
    PrintLCD(imprimir_lcd, 3, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(10), 1), (char)223, CelciusOrFarenheit(eep.read(10), 2));
    PrintLCD(imprimir_lcd, 13, 0);
    sprintf(imprimir_lcd, "1 +%d", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 0, 1);
    sprintf(imprimir_lcd, "-%d 2", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 16, 1);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if ((Posicion_actual + 8) * sumador_temperatura != eep.read(10))
    {
      eep.write(10, (Posicion_actual + 8) * sumador_temperatura);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 2;
      eep.write(11, eep.read(10) + sumador_temperatura);
      Posicion_actual = 0;
    }

    if (PressedButton(7) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      eep.write(10, Vaux1);
      eep.write(11, Vaux2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 8);
    break;

  case 2:
    memcpy(imprimir_lcd, "Temp. max", 11);
    PrintLCD(imprimir_lcd, 3, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(11), 1), (char)223, CelciusOrFarenheit(eep.read(11), 2));
    PrintLCD(imprimir_lcd, 13, 0);
    sprintf(imprimir_lcd, "1 +%d", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 0, 1);
    sprintf(imprimir_lcd, "-%d 2", CelciusOrFarenheit(5, 3));
    PrintLCD(imprimir_lcd, 16, 1);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if ((Posicion_actual + 1 + (eep.read(10) / sumador_temperatura)) * sumador_temperatura != eep.read(11))
    {
      eep.write(11, (Posicion_actual + 1 + (eep.read(10) / sumador_temperatura)) * sumador_temperatura);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 16 - (eep.read(10) / sumador_temperatura));

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(10, Vaux1);
      eep.write(11, Vaux2);
      lcd.clear();
    }
    break;

  case 3:
    sprintf(imprimir_lcd, "A los %d%c%c ", CelciusOrFarenheit(eep.read(10), 1), (char)223, CelciusOrFarenheit(0, 2));
    PrintLCD(imprimir_lcd, 5, 0);
    sprintf(imprimir_lcd, "Calentar %d%c%c ", CelciusOrFarenheit(eep.read(11), 1), (char)223, CelciusOrFarenheit(eep.read(11), 2));
    PrintLCD(imprimir_lcd, 3, 1);
    memcpy(imprimir_lcd, "3 Guardar", 11);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 4;
      lcd.clear();
    }

    if (PressedButton(7))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
      tiempo_de_standby = mili_segundos;
    }
    break;

  case 4:
    tiempo_de_standby = mili_segundos;
    Serial_Send_UNO(6, 0);
    guardado_para_menus(true);
    break;
  }
}

void menu_modificar_hora_rtc(uint8_t hora, uint8_t minutos)
{
  switch (Flag)
  {
  case 0:
    hora_to_modify = hora;
    minuto_to_modify = minutos;
    Flag = 1;
    lcd.clear();
    Posicion_actual = 0;
    break;
  case 1:
    memcpy(imprimir_lcd, "Cambiar hora", 14);
    PrintLCD(imprimir_lcd, 1, 0);
    Printhora(imprimir_lcd, hora_to_modify, minutos);
    PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "1 +1", 5);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-1 2", 5);
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (hora_to_modify != ReturnToCero(hora + Posicion_actual, hora_max))
    {
      hora_to_modify = ReturnToCero(hora + Posicion_actual, hora_max);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (PressedButton(6))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, hora_max);

    break;

  case 2:
    memcpy(imprimir_lcd, "Cambiar min", 12);
    PrintLCD(imprimir_lcd, 1, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "1 +1", 5);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "-1 2", 5);
    PrintLCD(imprimir_lcd, 16, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(4) || PressedButton(42))
      Posicion_actual += 1;
    if (PressedButton(5))
      Posicion_actual -= 1;

    if (minuto_to_modify != ReturnToCero(minutos + Posicion_actual, minuto_max))
    {
      minuto_to_modify = ReturnToCero(minutos + Posicion_actual, minuto_max);
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6))
    {
      Flag = 3;
      lcd.clear();
      tiempo_de_standby = mili_segundos;
      Posicion_actual = 0;
    }
    if (PressedButton(7))
    {
      Flag = 1;
      Posicion_actual = 0;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, minuto_max);

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(imprimir_lcd, "Hora", 5);
    PrintLCD(imprimir_lcd, 5, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 10, 0);
    memcpy(imprimir_lcd, "3 Guardar", 11);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 10);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 4;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(7))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }
    break;

  case 4:
    now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hora_to_modify, minuto_to_modify, now.second()));
    tiempo_de_standby = mili_segundos;
    guardado_para_menus(false);
    break;
  }
}

void menu_activar_bomba()
{
  switch (Flag)
  {
  case 0:
    memcpy(imprimir_lcd, "Activar bomba", 14);
    PrintLCD(imprimir_lcd, 3, 0);
    if (eep.read(57) == 254)
    {
      memcpy(imprimir_lcd, "Bomba  activada", 16);
      PrintLCD(imprimir_lcd, 2, 1);
    }
    if (eep.read(57) == 1)
    {
      memcpy(imprimir_lcd, "Bomba desactivada", 19);
      PrintLCD(imprimir_lcd, 1, 1);
    }
    memcpy(imprimir_lcd, "1:Si", 5);
    PrintLCD(imprimir_lcd, 3, 3);
    memcpy(imprimir_lcd, "2:No", 5);
    PrintLCD(imprimir_lcd, 11, 3);

    if (PressedButton(4))
      Posicion_actual = 10;
    if (PressedButton(5))
      Posicion_actual = 5;

    if (Posicion_actual > 8)
      eep.write(57, 254);
    if (Posicion_actual <= 8)
      eep.write(57, 1);

    if (Posicion_actual != Vaux1)
    {
      lcd.clear();
      Vaux1 = Posicion_actual;
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6) || PressedButton(7) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 1;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 16);
    break;
  case 1:
    tiempo_de_standby = mili_segundos;
    guardado_para_menus(false);
    break;
  }
}

void menu_farenheit_celsius()
{
  switch (Flag)
  {
  case 0:
    memcpy(imprimir_lcd, "Cambiar unidad", 15);
    PrintLCD(imprimir_lcd, 3, 0);
    memcpy(imprimir_lcd, "Unidad actual", 14);
    PrintLCD(imprimir_lcd, 2, 1);
    sprintf(imprimir_lcd, "%c%c", (char)223, CelciusOrFarenheit(0, 2));
    PrintLCD(imprimir_lcd, 16, 1);

    sprintf(imprimir_lcd, "1:%cC", (char)223);
    PrintLCD(imprimir_lcd, 3, 3);
    sprintf(imprimir_lcd, "%cF:2", (char)223);
    PrintLCD(imprimir_lcd, 13, 3);

    if (Posicion_actual > 8)
      eep.write(56, 254);
    if (Posicion_actual <= 8)
      eep.write(56, 1);

    if (PressedButton(4))
      Posicion_actual += 8;
    if (PressedButton(5))
      Posicion_actual -= 8;

    if (Posicion_actual != Vaux1)
    {
      Vaux1 = Posicion_actual;
      tiempo_de_standby = mili_segundos;
    }

    if (PressedButton(6) || PressedButton(7) || PressedButton(42) || mili_segundos >= tiempo_de_standby + tiempo_de_espera_submenues)
    {
      Flag = 1;
      lcd.clear();
    }

    Posicion_actual = ReturnToCero(Posicion_actual, 16);
    break;

  case 1:
    Posicion_actual = 0;
    guardado_para_menus(false);
    break;
  }
}

void menu_seteo_wifi()
{
  switch (Flag)
  {
  case 0:
    sprintf(imprimir_lcd, "%s", nombre_wifi_setear);
    PrintLCD(imprimir_lcd, 6, 0);
    sprintf(imprimir_lcd, "%s", password_wifi_setear);
    PrintLCD(imprimir_lcd, 6, 1);
    sprintf(imprimir_lcd, "%d:%d:%d:%d   ", eep.read(58), eep.read(59), eep.read(60), eep.read(61));
    PrintLCD(imprimir_lcd, 4, 2);
    memcpy(imprimir_lcd, "SSID:", 6);
    PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "PASS:", 6);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "IP:", 4);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "3 Seguir", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Volver 4", 9);
    PrintLCD(imprimir_lcd, 12, 3);

    if (PressedButton(6))
    {
      lcd.clear();
      Flag = 1;
      Posicion_actual = 0;
    }
    if (PressedButton(7))
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

    break;
  case 1:

    for (Vaux1 = 0; Vaux1 < 19; Vaux1++)
    {
      nombre_wifi_setear[Vaux1] = '\0';
      password_wifi_setear[Vaux1] = '\0';
    }
    Actualchar = 0;
    Posicion_actual = 0;
    Vaux2 = 0;
    Flag = 2;
    break;
  case 2:
    memcpy(imprimir_lcd, "Nombre Wifi:", 13);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%s", nombre_wifi_setear);
    PrintLCD(imprimir_lcd, 0, 1);

    memcpy(imprimir_lcd, "1 Mayus.", 10);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Sig. 2", 8);
    PrintLCD(imprimir_lcd, 14, 2);
    memcpy(imprimir_lcd, "3 Pass.", 9);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Del. 4", 10);
    PrintLCD(imprimir_lcd, 14, 3);

    Actualchar = Posicion_actual / 2;
    Posicion_actual = ReturnToCero(Posicion_actual, 80);
    nombre_wifi_setear[Vaux2] = Character_Return(Actualchar, mayusculas);
    if (PressedButton(42))
      Posicion_actual++;
    if (PressedButton(4))
      mayusculas = !mayusculas;
    if (PressedButton(5) && Vaux2 <= 19)
    {
      Vaux2++;
      Actualchar = 0;
    }
    if (PressedButton(6) && Vaux2 >= 6)
    {
      nombre_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 3;
      Vaux2 = 0;
      Actualchar = 0;
    }
    if (PressedButton(7) && Vaux2 > 0)
    {
      Actualchar = 0;
      nombre_wifi_setear[Vaux2] = '\0';
      Vaux2--;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(imprimir_lcd, "Pass Wifi:", 11);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%s", password_wifi_setear);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "1 Mayus.", 10);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Sig. 2", 8);
    PrintLCD(imprimir_lcd, 14, 2);
    memcpy(imprimir_lcd, "3 Guardar", 11);
    PrintLCD(imprimir_lcd, 0, 3);
    memcpy(imprimir_lcd, "Del. 4", 10);
    PrintLCD(imprimir_lcd, 14, 3);

    Actualchar = Posicion_actual / 2;
    Posicion_actual = ReturnToCero(Posicion_actual, 80);

    password_wifi_setear[Vaux2] = Character_Return(Actualchar, mayusculas);

    if (PressedButton(42))
      Posicion_actual++;
    if (PressedButton(4))
      mayusculas = !mayusculas;
    if (PressedButton(5) && Vaux2 <= 19)
    {
      Vaux2++;
      Actualchar = 0;
    }
    if (PressedButton(6) && Vaux2 >= 6)
    {
      password_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 4;
    }
    if (PressedButton(7) && Vaux2 > 0)
    {
      Actualchar = 0;
      password_wifi_setear[Vaux2] = '\0';
      Vaux2--;
      lcd.clear();
    }
    break;
  case 4:
    for (Vaux1 = 0; Vaux1 < 20; Vaux1++)
    {
      eep.writeChars(14, password_wifi_setear, 20);
      eep.writeChars(34, nombre_wifi_setear, 20);
    }
    Serial_Send_UNO(3, 0);
    Serial_Send_UNO(4, 0);
    tiempo_de_standby = mili_segundos;
    Posicion_actual = 0;
    guardado_para_menus(false);
    break;
  }
}

void guardado_para_menus(bool Menu)
{

  while (mili_segundos <= tiempo_de_standby + tiempo_de_espera_guardado)
  {
    if (mili_segundos - tiempo_de_standby <= 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("-------------------");
      lcd.setCursor(4, 1);
      lcd.print(" Guardando ");
      lcd.setCursor(4, 2);
      lcd.print(" Guardando ");
      lcd.setCursor(0, 3);
      lcd.print("-------------------");
    }
    else
    {
      if (mili_segundos % 125 == 0)
      {
        lcd.scrollDisplayLeft();
      }
    }
  }
  if (mili_segundos > tiempo_de_standby + tiempo_de_espera_guardado)
  {

    if (Menu == true)
      Estadoequipo = menu1;

    if (Menu == false)
      Estadoequipo = menu2;

    Flag = 0;
    lcd.noAutoscroll();
    lcd.clear();
    tiempo_de_standby = mili_segundos;
    funcionActual = posicion_inicial;
  }
}

//██████████████████████████████████████████FUNCIONES DE SOPORTE██████████████████████████████████████████
void PrintHorizontalBar(uint8_t column, uint8_t row, int16_t value)
{
  uint8_t Estado_barra;

  if (value <= 12)
    Estado_barra = 0;
  if (value >= 13 && value <= 39)
    Estado_barra = 1;
  if (value >= 40 && value <= 79)
    Estado_barra = 2;
  if (value >= 80)
    Estado_barra = 3;

  switch (Estado_barra)
  {

  case 0:
    // imprime todo lo que esta arriba
    lcd.setCursor(0 + column, row);
    lcd.write(char(6)); // CARACTER DE LA IZQUIERDA 6 = barrita izq |
    lcd.setCursor(1 + column, row);
    lcd.write(char(7)); // CARACTER MEDIO 6 = barrita arriba -
    lcd.setCursor(2 + column, row);
    lcd.write(char(5)); // CARACTER DERECHA = barrita der |
    // imprime todo lo que esta en el medio
    lcd.setCursor(0 + column, 1 + row);
    lcd.write(char(6));
    lcd.setCursor(1 + column, 1 + row);
    lcd.write(' ');
    lcd.setCursor(2 + column, 1 + row);
    lcd.write(char(5));
    // imprime todo lo que esta abajo
    lcd.setCursor(0 + column, 2 + row);
    lcd.write(char(6));
    lcd.setCursor(1 + column, 2 + row);
    lcd.write(char(4));
    lcd.setCursor(2 + column, 2 + row);
    lcd.write(char(5));
    break;
  case 1:
    lcd.setCursor(0 + column, row);
    lcd.write(char(6));
    lcd.setCursor(1 + column, row);
    lcd.write(char(7));
    lcd.setCursor(2 + column, row);
    lcd.write(char(5));

    lcd.setCursor(0 + column, 1 + row);
    lcd.write(char(6));
    lcd.setCursor(1 + column, 1 + row);
    lcd.write(' ');
    lcd.setCursor(2 + column, 1 + row);
    lcd.write(char(5));

    lcd.setCursor(0 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, 2 + row);
    lcd.write(char(255));
    break;
  case 2:
    lcd.setCursor(0 + column, row);
    lcd.write(char(6));
    lcd.setCursor(1 + column, row);
    lcd.write(char(7));
    lcd.setCursor(2 + column, row);
    lcd.write(char(5));

    lcd.setCursor(0 + column, 1 + row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, 1 + row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, 1 + row);
    lcd.write(char(255));

    lcd.setCursor(0 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, 2 + row);
    lcd.write(char(255));
    break;
  case 3:
    lcd.setCursor(0 + column, row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, row);
    lcd.write(char(255));

    lcd.setCursor(0 + column, 1 + row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, 1 + row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, 1 + row);
    lcd.write(char(255));

    lcd.setCursor(0 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(1 + column, 2 + row);
    lcd.write(char(255));
    lcd.setCursor(2 + column, 2 + row);
    lcd.write(char(255));
    break;
  }
}

int16_t CelciusOrFarenheit(int8_t value, uint8_t function)
{
  if (function == 1)
  {
    if (eep.read(56) == 254)
      return ((9 * value) / 5) + 32;
    if (eep.read(56) == 1)
      return value;
  }
  if (function == 2)
  {
    if (eep.read(56) == 254)
      return 'F';
    if (eep.read(56) == 1)
      return 'C';
  }
  if (function == 3)
  {
    if (eep.read(56) == 254)
      return ((9 * value) / 5);
    if (eep.read(56) == 1)
      return value;
  }
}

uint8_t ReturnToCero(int8_t actualpos, uint8_t maxpos)
{
  if (actualpos >= maxpos)
  {
    return actualpos % maxpos;
  }
  if (actualpos < 0)
  {
    return maxpos - actualpos;
  }
  if (actualpos >= 0 && actualpos < maxpos)
    return actualpos;
  return (0);
}

uint8_t Guardado_a_hora(uint8_t function, uint8_t save)
{
  uint8_t resto;
  uint8_t retorno;
  switch (function)
  {
  case 1:
    resto = save % 4;
    return (save - resto) / 4;
    break;
  case 2:
    resto = save % 4;
    return resto * 15;
    break;
  default:
    return (0);
    break;
  }
}

int8_t Hora_a_guardado(char buffer[20]) //// ya arregle lo de colver
{
  uint8_t hora;
  uint8_t minuto;
  uint8_t resto;
  uint8_t NumeroFinal; // solo una variable (_convert  nos evita modificar variables globales como bldos)

  hora = (buffer[0] - '0') * 10; // toma el valor del primer digito del string y lo convierte en int (numero de base 10) 18 :32
  hora += (buffer[1] - '0');     // toma el valor del segundo digito
  hora = hora * 4;               // multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

  minuto = (buffer[3] - '0') * 10; // lo mismo que en el var 1 pero con minutos (10:[puntero aca]0) 30
  minuto += (buffer[4] - '0');     // lo mismo que en el var 1 pero con minutos

  resto = minuto % 15; // saca el Vaux4 (ejemplo 7/5 Vaux4 2)
  if (resto < 8)
    minuto = minuto - resto; // utiliza el Vaux4 para redondear abajo (Esto se da pq en el propio diseño del sistema decidimos guardar todas las horas en un char)
  else
    minuto = minuto + 15 - resto; // utiliza el Vaux4 para redondear arriba
  minuto = minuto / 15;           // convierte los minutos en la proporcion del char (1 entero = 15 minutos)

  NumeroFinal = hora + minuto; // suma horas y minutos
  if (NumeroFinal >= 96)
    NumeroFinal = 0;
  return NumeroFinal;
}

void Printhora(char buffer[20], uint8_t hora_entrada, uint8_t minuto_entrada)
{
  uint8_t slot;
  for (slot = 0; slot < 20; slot++)
    buffer[slot] = '\0';
  slot = hora_entrada / 10;
  buffer[0] = slot + '0'; // te convierte el numero a caracter de su mismo valor
  hora_entrada = hora_entrada - (slot * 10);
  buffer[1] = hora_entrada + '0'; // te convierte el numero a caracter de su mismo valor
  buffer[2] = ':';
  slot = minuto_entrada / 10;
  buffer[3] = slot + '0'; // te convierte el numero a caracter de su mismo valor
  minuto_entrada = minuto_entrada - (slot * 10);
  buffer[4] = minuto_entrada + '0'; // te convierte el numero a caracter de su mismo valor
}

char Character_Return(uint8_t Character_pos, bool mayus)
{
  switch (mayus)
  {
  case true:
    if (Character_pos >= 0 && Character_pos <= 25)
      return Character_pos + 65;
    if (Character_pos >= 26 && Character_pos <= 29)
      return Character_pos + 9;
    if (Character_pos == 30)
      return 47;
    if (Character_pos == 31)
      return 40;
    if (Character_pos == 32)
      return 41;
    if (Character_pos == 33)
      return 61;
    if (Character_pos == 34)
      return 59;
    if (Character_pos == 35)
      return 58;
    if (Character_pos == 36)
      return 94;
    if (Character_pos == 37)
      return 63;
    if (Character_pos == 38)
      return 95;
    if (Character_pos == 39)
      return 27;
    if (Character_pos > 39)
      return 0;
    break;

  case false:
    if (Character_pos >= 0 && Character_pos <= 25)
      return Character_pos + 97;
    if (Character_pos >= 26 && Character_pos <= 35)
      return Character_pos + 22;
    if (Character_pos == 36)
      return 33;
    if (Character_pos == 37)
      return 45;
    if (Character_pos == 38)
      return 64;
    if (Character_pos == 39)
      return 0;
    if (Character_pos > 39)
      return 0;
    break;
  }
  return (0);
}

bool PressedButton(uint8_t Wich_Button)
{
  switch (Wich_Button)
  {
  case 2:
    if ((PIND & (1 << PD2)) == 0)
      return true;
    else
      return false;
    break;

  case 3:
    if ((PIND & (1 << PD3)) == 0)
      return true;
    else
      return false;
    break;

  case 4:
    if ((PIND & (1 << PD4)) == 0)
    {
      while ((PIND & (1 << PD4)) == 0)
      {
      }
      return true;
    }
    else
      return false;
    break;

  case 5:
    if ((PIND & (1 << PD5)) == 0)
    {
      while ((PIND & (1 << PD5)) == 0)
      {
      }
      return true;
    }
    else
      return false;
    break;

  case 6:
    if ((PIND & (1 << PD6)) == 0)
    {
      while ((PIND & (1 << PD6)) == 0)
      {
      }
      return true;
    }
    else
      return false;
    break;

  case 7:
    if ((PIND & (1 << PD7)) == 0)
    {
      while ((PIND & (1 << PD7)) == 0)
      {
      }
      return true;
    }
    else
      return false;
    break;

  case 42:
    if ((PINB & (1 << PB0)) == 0)
      return true;
    else
      return false;
    break;

  default:
    return false;
    break;
  }
}

void PrintOutput(uint8_t nat_pin, bool state)
{
  switch (nat_pin)
  {
  case 10:
    if (state)
      PORTB |= B00000100;
    if (!state)
      PORTB &= B11111011;
    break;

  case 11:
    if (state)
      PORTB |= B00001000;
    if (!state)
      PORTB &= B11110111;
    break;

  case 12:
    if (state)
      PORTB |= B00010000;
    if (!state)
      PORTB &= B11101111;
    break;
  }
}

void PrintLCD(char buffer[20], uint8_t Column, uint8_t Row)
{
  lcd.setCursor(Column, Row);
  lcd.print(buffer);
}

//████████████████████████████████████████████COMUNICACIONES██████████████████████████████████████████

void Serial_Read_UNO()
{
  bool Take_Comunication_Data;
  uint8_t Individualdata[4];
  uint8_t seriallength;
  uint8_t saveslot;
  uint8_t ActualIndividualDataPos;

  seriallength = Serial.available();

  for (uint8_t i = 0; i <= seriallength; i++)
  {
    if (i == 0)
    {
      Actualchar = Serial.read();
      ActualIndividualDataPos = 0;
    }

    if (i == 1)
      Serial.read();

    if (i >= 2 && i < seriallength)
    {                                                          // si no es nungun caracter especial:
      Individualdata[ActualIndividualDataPos] = Serial.read(); // copia al individual
      ActualIndividualDataPos++;
    }

    if (i == seriallength)
      Take_Comunication_Data = true;
  }

  if (Take_Comunication_Data == true)
  {
    switch (Actualchar)
    {         // dependiendo del char de comando
    case 'W': // calentamiento manual
      temperatura_a_calentar = Individualdata[0] - 33;

      if (Individualdata[1] == 'O')
        calentar = false;

      if (Individualdata[1] == 'I')
        calentar = true;

      Take_Comunication_Data = false;
      break;

    case 'F': // llenmado manual
      nivel_a_llenar = Individualdata[0] - 33;

      if (Individualdata[1] == 'O') // prendido
        llenar = false;

      if (Individualdata[1] == 'I') // apagado
        llenar = true;

      Take_Comunication_Data = false;
      break;

    case 'S': // H por hora
      saveslot = Individualdata[0] - 48;
      if (saveslot <= 2 && saveslot >= 0)
      {
        eep.write((saveslot * 3) + 1, Individualdata[1] - 33); // hora
        eep.write((saveslot * 3) + 2, Individualdata[2] - 33); // nivel
        eep.write((saveslot * 3) + 3, Individualdata[3] - 33); // temp
        Take_Comunication_Data = false;
      }
      break;

    case 'H': // temp auto
      eep.write(10, Individualdata[0] - 33);
      eep.write(11, Individualdata[1] - 33);
      Take_Comunication_Data = false;
      break;
    case 'C': // llenado auto
      eep.write(12, Individualdata[0] - 33);
      eep.write(13, Individualdata[1] - 33);
      Take_Comunication_Data = false;
      break;
    case 'I': // Ip
      eep.write(58, Individualdata[0] - 33);
      eep.write(59, Individualdata[1] - 33);
      eep.write(60, Individualdata[2] - 33);
      eep.write(61, Individualdata[3] - 33);
      esp_working = true;
      if (Estadoequipo == funciones && funcionActual == funcion_de_menu_seteo_wifi)
        lcd.clear();
      Take_Comunication_Data = false;
      break;

    case 'E':
      Serial_Send_UNO(6, 0);
      Take_Comunication_Data = false;
      break;
    }
    ActualIndividualDataPos = 0;
  }
}

void Serial_Send_UNO(uint8_t WhatSend, uint8_t What_slot)
{
  char OutputMessage[22];
  char calentando, llenando;

  if (calentar)
    calentando = 'I';
  else
    calentando = 'O';

  if (llenar)
    llenando = 'I';
  else
    calentando = 'O';

  switch (WhatSend)
  {

  case 1:
    sprintf(OutputMessage, "U_%c%c%c%c", 128 + temperatura_actual, 33 + nivel_actual, calentando, llenando);
    break;
  case 2:
    sprintf(OutputMessage, "K_%c%c%c%c", 33 + eep.read((What_slot * 3) + 1), 33 + eep.read((What_slot * 3) + 2), 33 + eep.read((What_slot * 3) + 3), What_slot + 48);
    break;
  case 3:
    sprintf(OutputMessage, "N_%s", nombre_wifi_setear);
    break;
  case 4:
    sprintf(OutputMessage, "P_%s", password_wifi_setear);
    break;
  case 5:
    sprintf(OutputMessage, "C_%c%c", 33 + eep.read(12), 33 + eep.read(13));
    break;
  case 6:
    sprintf(OutputMessage, "H_%c%c", 33 + eep.read(10), 33 + eep.read(11));
  default:
    break;
  }
  Serial.println(OutputMessage);
}
