
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

//█████████████████████████████████ Declaraciones libs ███████████████████████████████████
OneWire sensor_t(SENSOR_TEMP);
DallasTemperature Sensor_temp(&sensor_t);
LiquidCrystal_I2C lcd(0x27, 20, 4); 
RTC_DS1307 rtc;
DateTime now;
AT24C32 eep;

//██████████████████████████████████ Declaraciones mef ██████████████████████████████████
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
  estado_Standby,
  estado_inicial,
  menu1,
  menu2,
  funciones
} estadoMEF;
Seleccionar_Funciones funcionActual = posicion_inicial;
estadoMEF Estadoequipo = estado_inicial;

//██████████████████████████████████ Declaraciones Vars ██████████████████████████████████
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

char ImprimirLCD[20];
char ContraWifi[20];
char NombreWifi[20];

bool InternetDisponible = false;
uint8_t TiempoPulsadorEncoder;
uint8_t LetraSeleccionada = 0;
uint8_t TemperaturaACalentar;
uint16_t TiempoDeStandby;
int8_t TemperaturaActual; // temp actual
bool Mayusculas = false;
uint8_t PosicionActual; // es el "sumador" del encoder
uint16_t MiliSegundos;
uint8_t MinutoASetear;
uint8_t NivelALLenar;
uint8_t Vaux1, Vaux2; // por cambiar
uint8_t HoraASetear;
uint8_t NivelActual;
uint8_t Flag = 0;
bool Calentar;
bool LLenar;

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
  attachInterrupt(Pin_Entrada(2), EncoderPinA, CHANGE);
  attachInterrupt(Pin_Entrada(3), EncoderPinB, CHANGE);
  // inicia sensor temp
  Wire.begin();
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  TemperaturaActual = Sensor_temp.getTempCByIndex(0);
  rtc.begin(); 
  Serial.begin(9600);
  lcd.init(); // Iniciacion del LCD

  lcd.createChar(0, hora);
  lcd.createChar(3, wifi);
  lcd.createChar(4, barra_abajo);
  lcd.createChar(5, barra_derecha);
  lcd.createChar(6, barra_izquierda);
  lcd.createChar(7, barra_arriba);
  eep.readChars(14, ContraWifi, 20);
  eep.readChars(34, NombreWifi, 20);

  MiliSegundos=0;
  while (Vaux1 <= 7) // manda set-up de todos los datos actuales al esp (con retardo pq sino no puede leer todos los datos juntos)
  {
    if (Vaux1 == 0 && MiliSegundos>250)
    {
      Enviar_Serial(3, 0);
      Vaux1++;
    }
    if (Vaux1 == 1 && MiliSegundos>500)
    {
      Enviar_Serial(1, 0);
      Vaux1++;
    }
    if (Vaux1 == 2 && MiliSegundos>750)
    {
      Enviar_Serial(5, 0);
      Vaux1++;
    }
    if (Vaux1 == 3 && MiliSegundos>1000)
    {
      Enviar_Serial(6, 0);
      Vaux1++;
    }
    if (Vaux1 == 4 && MiliSegundos>1250)
    {
      Enviar_Serial(2, 1);
      Vaux1++;
    }
    if (Vaux1 == 5 && MiliSegundos>1500)
    {
      Enviar_Serial(2, 2);
      Vaux1++;
    }
    if (Vaux1 == 6 && MiliSegundos>1750)
    {
      Enviar_Serial(2, 3);
      Vaux1++;
    }
    if (Vaux1 == 7 && MiliSegundos>2000)
    {
      Enviar_Serial(4, 0);
      Vaux1++;
    }
  }

  TiempoDeStandby = MiliSegundos;
  Vaux1 = PosicionActual;
  Estadoequipo = estado_inicial;
}

void loop()
{
  Actualizar_entradas();
  Actualizar_salidas();
  if (Serial.available() > 0)
    Leer_Serial(); // si recibe un dato del serial lo lee

  switch (Estadoequipo)
  {
  case estado_Standby:
    Standby(); // sin backlight
    lcd.noBacklight();
    break;
  case estado_inicial:
    lcd.backlight();
    Standby(); // con backlight
    break;
  case menu1:
    Menu_Basico();
    break;
  case menu2:
    Menu_Avanzado();
    break;
  case funciones:
    switch (funcionActual)
    {
    case llenado_manual:
      NivelALLenar = menu_de_llenado_manual();
      break;
    case calefaccion_manual:
      TemperaturaACalentar = menu_de_calefaccion_manual();
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

//███████████████████████████████████████CONTROL DE ENTRADAS/SALIDAS██████████████████████████████████████████

void Actualizar_entradas()
{
uint16_t tiempoSensores;

  if (MiliSegundos >= tiempoSensores + TIEMPO_LECTURA_TEMP)
  {
    if (Estadoequipo == estado_Standby || Estadoequipo == estado_inicial)
    {
      Sensor_temp.requestTemperatures();
      TemperaturaActual = Sensor_temp.getTempCByIndex(0);
      tiempoSensores = MiliSegundos;
      if(InternetDisponible)
        Enviar_Serial(1, 0);
    }
  }

  if (analogRead(SENSOR_NIVEL) < 100)
    NivelActual = 0;
  if (analogRead(SENSOR_NIVEL) >= 100 && analogRead(SENSOR_NIVEL) < 256)
    NivelActual = 25;
  if (analogRead(SENSOR_NIVEL) >= 256 && analogRead(SENSOR_NIVEL) < 512)
    NivelActual = 50;
  if (analogRead(SENSOR_NIVEL) >= 512 && analogRead(SENSOR_NIVEL) < 768)
    NivelActual = 75;
  if (analogRead(SENSOR_NIVEL) >= 768 && analogRead(SENSOR_NIVEL) <= 1024)
    NivelActual = 100;
  
  now = rtc.now();// Actualiza el rtc
}

void Actualizar_salidas()
{
  char textoHora[6];

  if (TemperaturaActual < TemperaturaACalentar)
    Calentar = true;
  else if (TemperaturaActual <= eep.read(10))            //Temp minima
    TemperaturaACalentar = eep.read(11);               //Temp maxima
  else if (TemperaturaActual >= TemperaturaACalentar)
  {
    Calentar = false;
    TemperaturaACalentar = 0;
  }

  if (NivelActual < NivelALLenar)
    LLenar = true;
  else if (NivelActual <= eep.read(12))      //Nivel minimo
    NivelALLenar = eep.read(13);            //Nivel maximo
  else if (NivelActual >= NivelALLenar)
  {
    LLenar = false;
    NivelALLenar = 0;
  }

  if (eep.read(57) == 254)    //eep57 es la activacion de la bomba, si esta en 254 (valor maximo) equivale a un booleano en true
    Pin_Salida(12, LLenar); // bomba

  Pin_Salida(11, LLenar);   // electrovalvula
  Pin_Salida(10, Calentar); // resistencia

  Imprimir_Hora(textoHora, now.hour(), now.minute()); 
  for (uint8_t i = 0; i < 3; i++)
  {
    if (Hora_a_guardado(textoHora) == eep.read(i * 3 + 1)) // compara hora con las guardadas y activa segun hora, multiplica por 3 para agrupar entre los tres "perfiles".
    {
      NivelALLenar = eep.read(i * 3 + 2);
      TemperaturaACalentar = eep.read(i * 3 + 3);
    }
  }
}

ISR(TIMER2_OVF_vect)
{
  MiliSegundos++;
}

void EncoderPinA()
{
  if (Pin_Entrada(2) == Pin_Entrada(3))
    PosicionActual++;
  else
    PosicionActual--;
}

void EncoderPinB()
{
  if (Pin_Entrada(2) != Pin_Entrada(3))
    PosicionActual++;
  else
    PosicionActual--;
}

//███████████████████████████████████████████MENUES PRINCIPALES██████████████████████████████████████████
void Standby()
{
  lcd.setCursor(5, 3); // imprime carga o espera temperatura
  if (Calentar)
    lcd.write(char(94));
  else
    lcd.write(char(61));


  lcd.setCursor(7, 3); // imprime C o F dependiendo de lo seteado
  lcd.print(char(Celcius_O_Farenheit(TemperaturaActual, 2)));

  lcd.setCursor(12, 3); // imprime bomba o bomba desactivada titilando con una cruz
  if (eep.read(57) == 254)
    lcd.print(char(244));
  else
  {
    if (now.second() % 2 == 1)
      lcd.write(char(244));
    else
      lcd.write('x');
  }

  lcd.setCursor(14, 3); // imprime carga o espera llenado
  if (LLenar)
    lcd.write(char(94));
  else
    lcd.write(char(61));

  lcd.setCursor(9, 3); // imprime logo wifi
  lcd.print(char(3));

  lcd.setCursor(10, 3); // imprime cruz o carita sonriente si hay o no wifi (cambiar iconos)
  if (InternetDisponible)
    lcd.print(char(175));
  else
    lcd.write('x');


  sprintf(ImprimirLCD, "%d%c", Celcius_O_Farenheit(TemperaturaActual, 1), char(223)); // imprime temp
  Imprimir_LCD(ImprimirLCD, 0, 0);

  sprintf(ImprimirLCD, "%d%c", NivelActual, '%'); // inserta nivel al array
  for (Vaux2 = 0; ImprimirLCD[Vaux2] != '\0'; Vaux2++) {} //Detecta espacios en blanco para imprimirlo a la derecha 
  Imprimir_LCD(ImprimirLCD, 20 - Vaux2, 0); //imprime nivel a la derecha

  if (4 - Vaux2 == 2)
    Imprimir_LCD("  ", 16, 0);// Borra caracter para no dejarlo en el lcd
  if (4 - Vaux2 == 1)
    Imprimir_LCD(" ", 17, 0);// Borra caracter para no dejarlo en el lcd

  lcd.setCursor(7, 1); // imprime relojito
  lcd.print(char(0));

  Imprimir_Hora(ImprimirLCD, now.hour(), now.minute()); // imprime hora
  Imprimir_LCD(ImprimirLCD, 8, 1);

  Barra_de_carga(1, 1, TemperaturaActual); // imprime barra segun la temperatura
  Barra_de_carga(16, 1, NivelActual); // imprime barra segun el nivel

  if (Pin_Entrada(4) || Pin_Entrada(5) || Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42))
    PosicionActual += 1;

  if (Vaux1 != PosicionActual)
  {
    switch (Estadoequipo)
    {
    case estado_Standby:
      Estadoequipo = estado_inicial;
      break;
    case estado_inicial:
      Estadoequipo = menu1;
      lcd.clear();
      break;
    default:
      Estadoequipo = estado_Standby;
      break;
    }
    Vaux1 = PosicionActual;
    TiempoDeStandby = MiliSegundos;
  }

  if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_MENUES && Estadoequipo == estado_inicial)
  {
    Estadoequipo = estado_Standby;
    TiempoDeStandby = MiliSegundos;
  }
}

void Menu_Basico()
{
  const char *Menuprincipal[COLUMNAS_MAXIMAS_M1] = {
      "Carga manual",
      "Calentado manual",
      "Seteo por hora",
      "Carga auto",
      "Calentado auto",
      "Menu avanzado",
      "Volver"};

  switch (Flag)
  {
  case 0: // inicializa variables
    TiempoDeStandby = MiliSegundos;
    lcd.clear();
    Vaux1 = 0;
    PosicionActual = 0;
    Flag = 1;
    break;
  case 1:
    sprintf(ImprimirLCD, ">%s", Menuprincipal[Volver_a_Cero(Vaux1, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(Vaux1 + 1, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 1);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(Vaux1 + 2, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 2);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(Vaux1 + 3, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 3);

    if (PosicionActual / 2 != Vaux1) // cada 2 pulsos del encoder baja/sube una columna del menu
    {
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
      Vaux1 = PosicionActual / 2;
    }

    if (Pin_Entrada(4))
      PosicionActual += 2; // baja una columna

    if (Pin_Entrada(5))
      PosicionActual -= 2; // sube una columna

    if (TiempoPulsadorEncoder != 200 && MiliSegundos >= TiempoDeStandby + 250)
      TiempoPulsadorEncoder = 200; // pulsador encoder == 200 mili segundos

    if (Pin_Entrada(42) && MiliSegundos >= TiempoDeStandby + TiempoPulsadorEncoder)
    { // va sumando +2 mientras acelera la diferencia en el que se suma 2
      PosicionActual += 2;
      if (TiempoPulsadorEncoder == 200)
        TiempoPulsadorEncoder -= 20;
      if (TiempoPulsadorEncoder == 180)
        TiempoPulsadorEncoder -= 30;
      if (TiempoPulsadorEncoder == 150)
        TiempoPulsadorEncoder -= 50;
      if (TiempoPulsadorEncoder <= 100 && TiempoPulsadorEncoder >= 10)
        TiempoPulsadorEncoder -= 10;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, COLUMNAS_MAXIMAS_M1 * 2);

    if (Pin_Entrada(6))
    {
      TiempoDeStandby = MiliSegundos;
      Flag = Vaux1 + 2;
      lcd.clear();
    }

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_MENUES)
    {
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
      Vaux1 = PosicionActual;
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
    Vaux1 = PosicionActual;
    TiempoDeStandby = MiliSegundos;
    Estadoequipo = estado_inicial;
    break;
  }
}

void Menu_Avanzado()
{
  const char *Menuavanzado[COLUMNAS_MAXIMAS_M2] = {
      "Setear hora",
      "Cambiar unidad",
      "Habilitar la bomba",
      "conexion wifi",
      "volver"};

  switch (Flag)
  {
  case 0:
    TiempoDeStandby = MiliSegundos;
    PosicionActual = 0;
    lcd.clear();
    Flag = 1;
    break;
  case 1:
    sprintf(ImprimirLCD, ">%s", Menuavanzado[Volver_a_Cero(Vaux1, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(Vaux1 + 1, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 1);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(Vaux1 + 2, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(Vaux1 + 3, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 3);

    if (PosicionActual / 2 != Vaux1)
    {
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
      Vaux1 = PosicionActual / 2;
    }

    if (Pin_Entrada(4))
      PosicionActual += 2; // suma 1 a Vaux1
    if (Pin_Entrada(5))
      PosicionActual -= 2; // resta 1 a Vaux1

    if (TiempoPulsadorEncoder != 200 && MiliSegundos >= TiempoDeStandby + 250)
      TiempoPulsadorEncoder = 200;

    if (Pin_Entrada(42) && MiliSegundos >= TiempoDeStandby + TiempoPulsadorEncoder)
    {
      PosicionActual += 2;
      if (TiempoPulsadorEncoder == 200)
        TiempoPulsadorEncoder -= 20;
      if (TiempoPulsadorEncoder == 180)
        TiempoPulsadorEncoder -= 30;
      if (TiempoPulsadorEncoder == 150)
        TiempoPulsadorEncoder -= 50;
      if (TiempoPulsadorEncoder <= 100 && TiempoPulsadorEncoder >= 10)
        TiempoPulsadorEncoder -= 10;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, COLUMNAS_MAXIMAS_M2 * 2);

    if (Pin_Entrada(6))
    {
      Flag = Vaux1 + 2;
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
    }

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_MENUES)
    {
      TiempoDeStandby = MiliSegundos;
      Vaux1 = PosicionActual;
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
    PosicionActual = 0;
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
    if (NIVEL_MINIMO > NivelActual)
      Vaux1 = TEMP_MINIMO;
    else
      Vaux1 = NivelActual;
    Vaux1 = NIVEL_MINIMO;
    lcd.clear();
    PosicionActual = 0;
    Flag = 1;
    break;
  case 1:
    memcpy(ImprimirLCD, "LLenar hasta", 14);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c ", Vaux1, '%');
    Imprimir_LCD(ImprimirLCD, 15, 0);
    memcpy(ImprimirLCD, "1 +25", 7);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-25 2", 7);
    Imprimir_LCD(ImprimirLCD, 15, 2);
    memcpy(ImprimirLCD, "3 seguir", 10);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;

    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if ((PosicionActual + 1) * SUMADOR_NIVEL != Vaux1)
    {
      Vaux1 = (PosicionActual + 1) * SUMADOR_NIVEL; // actualiza el nivel a setear
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      lcd.clear();
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 4);
    break;

  case 2:

    memcpy(ImprimirLCD, "LLenar hasta:", 14);
    Imprimir_LCD(ImprimirLCD, 1, 0);
    sprintf(ImprimirLCD, "%d%c", Vaux1, '%');
    Imprimir_LCD(ImprimirLCD, 15, 0);
    memcpy(ImprimirLCD, "3 Guardar", 10);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      lcd.clear();
      Flag = 3;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      PosicionActual = 0;
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
    }
    break;

  case 3:
    TiempoDeStandby = MiliSegundos;
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
    if (TEMP_MINIMO > TemperaturaActual)
      Vaux2 = TEMP_MINIMO;
    else
      Vaux2 = (TemperaturaActual / 5) * 5;
    lcd.clear();
    PosicionActual = 0;
    Flag = 1;
    break;

  case 1:
    PosicionActual = Volver_a_Cero(PosicionActual, 9);

    memcpy(ImprimirLCD, "Calentar a", 11);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(Vaux2, 1), (char)223, Celcius_O_Farenheit(Vaux2, 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    sprintf(ImprimirLCD, "1 +%d", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 0, 2);
    sprintf(ImprimirLCD, "-%d 2", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if ((PosicionActual + 8) * SUMADOR_TEMP != Vaux2)
    {
      Vaux2 = (PosicionActual + 8) * SUMADOR_TEMP; // actualiza el nivel a setear
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      PosicionActual = 0;
      lcd.clear();
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }
    break;

  case 2:
    memcpy(ImprimirLCD, "Calentar a", 11);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(Vaux2, 1), (char)223, Celcius_O_Farenheit(Vaux2, 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    memcpy(ImprimirLCD, "3 Guardar", 10);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 3;
      lcd.clear();
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      PosicionActual = 0;
      TiempoDeStandby = 0;
      lcd.clear();
    }
    break;

  case 3:
    TiempoDeStandby = MiliSegundos;
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
      LetraSeleccionada = Struct;
      */
  {
  case 0:
    HoraASetear = hora_actual;
    MinutoASetear = minutos_actual;
    PosicionActual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    // Save a int convierte el char de guardado en 1 la hora y en 2 el minuto guardado en la eeprom, por ende esto imprime la hora y los minutos

    memcpy(ImprimirLCD, "Seleccionar", 13);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    memcpy(ImprimirLCD, "1:", 4);
    Imprimir_LCD(ImprimirLCD, 1, 1);
    memcpy(ImprimirLCD, "2:", 4);
    Imprimir_LCD(ImprimirLCD, 1, 2);
    memcpy(ImprimirLCD, "3:", 4);
    Imprimir_LCD(ImprimirLCD, 1, 3);
    Imprimir_Hora(ImprimirLCD, Guardado_a_hora(1, eep.read(1)), Guardado_a_hora(2, eep.read(1)));
    Imprimir_LCD(ImprimirLCD, 3, 1);
    Imprimir_Hora(ImprimirLCD, Guardado_a_hora(1, eep.read(4)), Guardado_a_hora(2, eep.read(4)));
    Imprimir_LCD(ImprimirLCD, 3, 3);
    Imprimir_Hora(ImprimirLCD, Guardado_a_hora(1, eep.read(8)), Guardado_a_hora(2, eep.read(8)));
    Imprimir_LCD(ImprimirLCD, 3, 2);
    memcpy(ImprimirLCD, ">", 2);
    Imprimir_LCD(ImprimirLCD, 0, LetraSeleccionada + 1);

    if (Vaux1 != LetraSeleccionada)
    {
      memcpy(ImprimirLCD, " ", 2);
      Imprimir_LCD(ImprimirLCD, 0, Vaux1 + 1);
      Vaux1 = LetraSeleccionada;
      TiempoDeStandby = MiliSegundos;
    }

    LetraSeleccionada = PosicionActual / 4;

    if (Pin_Entrada(4))
      PosicionActual += 4;
    if (Pin_Entrada(5))
      PosicionActual -= 4;
    if (Pin_Entrada(42))
      PosicionActual += 1;

    if (Pin_Entrada(6))
    {
      Vaux2 = TEMP_MINIMO;
      Vaux1 = NIVEL_MINIMO;
      PosicionActual = 0;
      lcd.clear();
      Flag = 2;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    PosicionActual = Volver_a_Cero(PosicionActual, 12);
    break;

  case 2:
    memcpy(ImprimirLCD, "Calentar a", 11);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(Vaux2, 1), (char)223, Celcius_O_Farenheit(Vaux2, 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    sprintf(ImprimirLCD, "1 +%d", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 0, 2);
    sprintf(ImprimirLCD, "-%d 2", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if ((PosicionActual + 8) * SUMADOR_TEMP != Vaux2)
    {
      Vaux2 = (PosicionActual + 8) * SUMADOR_TEMP;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionActual = 0;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 9);

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(ImprimirLCD, "Cargar a", 10);
    Imprimir_LCD(ImprimirLCD, 4, 0);
    sprintf(ImprimirLCD, "%d%c ", Vaux1, '%');
    Imprimir_LCD(ImprimirLCD, 13, 0);
    memcpy(ImprimirLCD, "1 +25", 20);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-25 2", 20);
    Imprimir_LCD(ImprimirLCD, 15, 2);
    memcpy(ImprimirLCD, "3 Salir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if ((PosicionActual + 1) * SUMADOR_NIVEL != Vaux1)
    {
      Vaux1 = (PosicionActual + 1) * SUMADOR_NIVEL;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 4;
      PosicionActual = 0;
      lcd.clear();
      PosicionActual = 0;
    }
    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionActual = 0;
    }
    PosicionActual = Volver_a_Cero(PosicionActual, 4);

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 4:
    memcpy(ImprimirLCD, "Setear hora", 13);
    Imprimir_LCD(ImprimirLCD, 1, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    Imprimir_LCD(ImprimirLCD, 14, 0);
    memcpy(ImprimirLCD, "1 +1", 5);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-1 2", 5);
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 Seguir", 10);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (HoraASetear != Volver_a_Cero((hora_actual + PosicionActual), HORA_MAX))
    {
      HoraASetear = Volver_a_Cero((hora_actual + PosicionActual), HORA_MAX);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 5;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 3;
      lcd.clear();
      PosicionActual = 0;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, HORA_MAX);

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 5:
    memcpy(ImprimirLCD, "Setear min", 12);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    Imprimir_LCD(ImprimirLCD, 13, 0);
    memcpy(ImprimirLCD, "1 +1", 5);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-1 2", 5);
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 Seguir", 10);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (MinutoASetear != Volver_a_Cero((minutos_actual + PosicionActual), MINUTO_MAX))
    {
      MinutoASetear = Volver_a_Cero((minutos_actual + PosicionActual), MINUTO_MAX);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 6;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 4;
      lcd.clear();
      PosicionActual = 0;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, MINUTO_MAX);

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 6:
    memcpy(ImprimirLCD, "A las", 7);
    Imprimir_LCD(ImprimirLCD, 4, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    Imprimir_LCD(ImprimirLCD, 11, 0);
    memcpy(ImprimirLCD, "Calentar", 10);
    Imprimir_LCD(ImprimirLCD, 3, 1);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(Vaux2, 1), (char)223, Celcius_O_Farenheit(Vaux2, 2));
    Imprimir_LCD(ImprimirLCD, 13, 1);
    sprintf(ImprimirLCD, "LLenar %d%c", Vaux1, '%');
    Imprimir_LCD(ImprimirLCD, 5, 2);
    memcpy(ImprimirLCD, "3 Guardar", 11);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      lcd.clear();
      Flag = 7;
    }

    if (Pin_Entrada(7))
    {
      Flag = 5;
      lcd.clear();
      PosicionActual = 0;
      TiempoDeStandby = MiliSegundos;
    }
    break;

  case 7:
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    eep.write((LetraSeleccionada * 3) + 1, Hora_a_guardado(ImprimirLCD));

    eep.write((LetraSeleccionada * 3) + 2, Vaux1);
    eep.write((LetraSeleccionada * 3) + 3, Vaux2);

    Enviar_Serial(2, LetraSeleccionada);

    TiempoDeStandby = MiliSegundos;
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
    PosicionActual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(ImprimirLCD, "Nivel min", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c ", eep.read(12), '%');
    Imprimir_LCD(ImprimirLCD, 14, 0);
    memcpy(ImprimirLCD, "1 +25", 6);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-25 2", 6);
    Imprimir_LCD(ImprimirLCD, 15, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if ((PosicionActual)*SUMADOR_NIVEL != eep.read(12))
    {
      eep.write(12, (PosicionActual)*SUMADOR_NIVEL);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      eep.write(13, eep.read(12) + SUMADOR_NIVEL);
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7) == true || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      eep.write(12, Vaux1);
      eep.write(13, Vaux2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 4);

    break;

  case 2:
    memcpy(ImprimirLCD, "Nivel max", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c ", eep.read(13), '%');
    Imprimir_LCD(ImprimirLCD, 14, 0);
    memcpy(ImprimirLCD, "1 +25", 6);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-25 2", 6);
    Imprimir_LCD(ImprimirLCD, 15, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5) == true)
      PosicionActual -= 1;

    if ((PosicionActual + 1 + (eep.read(12) / SUMADOR_NIVEL)) * SUMADOR_NIVEL != eep.read(13))
    {
      eep.write(13, ((PosicionActual + 1 + (eep.read(12) / SUMADOR_NIVEL)) * SUMADOR_NIVEL));
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionActual = 0;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 4 - (eep.read(12) / SUMADOR_NIVEL));

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(12, Vaux1);
      eep.write(13, Vaux2);
      lcd.clear();
    }
    break;

  case 3:
    sprintf(ImprimirLCD, "A el %d%c", eep.read(12), '%');
    Imprimir_LCD(ImprimirLCD, 6, 0);
    sprintf(ImprimirLCD, "LLenar a %d%c", eep.read(13), '%');
    Imprimir_LCD(ImprimirLCD, 4, 1);
    memcpy(ImprimirLCD, "3 Guardar", 11);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 4;
      lcd.clear();
    }

    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionActual = 0;
    }
    break;

  case 4:
    Enviar_Serial(5, 0);
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
    eep.write(10, TEMP_MINIMO);
    PosicionActual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(ImprimirLCD, "Temp. min", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(eep.read(10), 1), (char)223, Celcius_O_Farenheit(eep.read(10), 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    sprintf(ImprimirLCD, "1 +%d", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 0, 1);
    sprintf(ImprimirLCD, "-%d 2", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 16, 1);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if ((PosicionActual + 8) * SUMADOR_TEMP != eep.read(10))
    {
      eep.write(10, (PosicionActual + 8) * SUMADOR_TEMP);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      eep.write(11, eep.read(10) + SUMADOR_TEMP);
      PosicionActual = 0;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      eep.write(10, Vaux1);
      eep.write(11, Vaux2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 8);
    break;

  case 2:
    memcpy(ImprimirLCD, "Temp. max", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(eep.read(11), 1), (char)223, Celcius_O_Farenheit(eep.read(11), 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    sprintf(ImprimirLCD, "1 +%d", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 0, 1);
    sprintf(ImprimirLCD, "-%d 2", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 16, 1);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if ((PosicionActual + 1 + (eep.read(10) / SUMADOR_TEMP)) * SUMADOR_TEMP != eep.read(11))
    {
      eep.write(11, (PosicionActual + 1 + (eep.read(10) / SUMADOR_TEMP)) * SUMADOR_TEMP);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionActual = 0;
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 16 - (eep.read(10) / SUMADOR_TEMP));

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(10, Vaux1);
      eep.write(11, Vaux2);
      lcd.clear();
    }
    break;

  case 3:
    sprintf(ImprimirLCD, "A los %d%c%c ", Celcius_O_Farenheit(eep.read(10), 1), (char)223, Celcius_O_Farenheit(0, 2));
    Imprimir_LCD(ImprimirLCD, 5, 0);
    sprintf(ImprimirLCD, "Calentar %d%c%c ", Celcius_O_Farenheit(eep.read(11), 1), (char)223, Celcius_O_Farenheit(eep.read(11), 2));
    Imprimir_LCD(ImprimirLCD, 3, 1);
    memcpy(ImprimirLCD, "3 Guardar", 11);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 4;
      lcd.clear();
    }

    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionActual = 0;
      TiempoDeStandby = MiliSegundos;
    }
    break;

  case 4:
    TiempoDeStandby = MiliSegundos;
    Enviar_Serial(6, 0);
    guardado_para_menus(true);
    break;
  }
}

void menu_modificar_hora_rtc(uint8_t hora, uint8_t minutos)
{
  switch (Flag)
  {
  case 0:
    HoraASetear = hora;
    MinutoASetear = minutos;
    Flag = 1;
    lcd.clear();
    PosicionActual = 0;
    break;
  case 1:
    memcpy(ImprimirLCD, "Cambiar hora", 14);
    Imprimir_LCD(ImprimirLCD, 1, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, minutos);
    Imprimir_LCD(ImprimirLCD, 14, 0);
    memcpy(ImprimirLCD, "1 +1", 5);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-1 2", 5);
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (HoraASetear != Volver_a_Cero(hora + PosicionActual, HORA_MAX))
    {
      HoraASetear = Volver_a_Cero(hora + PosicionActual, HORA_MAX);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 2;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, HORA_MAX);

    break;

  case 2:
    memcpy(ImprimirLCD, "Cambiar min", 12);
    Imprimir_LCD(ImprimirLCD, 1, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    Imprimir_LCD(ImprimirLCD, 14, 0);
    memcpy(ImprimirLCD, "1 +1", 5);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-1 2", 5);
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionActual += 1;
    if (Pin_Entrada(5))
      PosicionActual -= 1;

    if (MinutoASetear != Volver_a_Cero(minutos + PosicionActual, MINUTO_MAX))
    {
      MinutoASetear = Volver_a_Cero(minutos + PosicionActual, MINUTO_MAX);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      TiempoDeStandby = MiliSegundos;
      PosicionActual = 0;
    }
    if (Pin_Entrada(7))
    {
      Flag = 1;
      PosicionActual = 0;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, MINUTO_MAX);

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(ImprimirLCD, "Hora", 5);
    Imprimir_LCD(ImprimirLCD, 5, 0);
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    Imprimir_LCD(ImprimirLCD, 10, 0);
    memcpy(ImprimirLCD, "3 Guardar", 11);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 10);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 4;
      lcd.clear();
      PosicionActual = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionActual = 0;
    }
    break;

  case 4:
    now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), HoraASetear, MinutoASetear, now.second()));
    TiempoDeStandby = MiliSegundos;
    guardado_para_menus(false);
    break;
  }
}

void menu_activar_bomba()
{
  switch (Flag)
  {
  case 0:
    memcpy(ImprimirLCD, "Activar bomba", 14);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    if (eep.read(57) == 254)
    {
      memcpy(ImprimirLCD, "Bomba  activada", 16);
      Imprimir_LCD(ImprimirLCD, 2, 1);
    }
    if (eep.read(57) == 1)
    {
      memcpy(ImprimirLCD, "Bomba desactivada", 19);
      Imprimir_LCD(ImprimirLCD, 1, 1);
    }
    memcpy(ImprimirLCD, "1:Si", 5);
    Imprimir_LCD(ImprimirLCD, 3, 3);
    memcpy(ImprimirLCD, "2:No", 5);
    Imprimir_LCD(ImprimirLCD, 11, 3);

    if (Pin_Entrada(4))
      PosicionActual = 10;
    if (Pin_Entrada(5))
      PosicionActual = 5;

    if (PosicionActual > 8)
      eep.write(57, 254);
    if (PosicionActual <= 8)
      eep.write(57, 1);

    if (PosicionActual != Vaux1)
    {
      lcd.clear();
      Vaux1 = PosicionActual;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 1;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 16);
    break;
  case 1:
    TiempoDeStandby = MiliSegundos;
    guardado_para_menus(false);
    break;
  }
}

void menu_farenheit_celsius()
{
  switch (Flag)
  {
  case 0:
    memcpy(ImprimirLCD, "Cambiar unidad", 15);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    memcpy(ImprimirLCD, "Unidad actual", 14);
    Imprimir_LCD(ImprimirLCD, 2, 1);
    sprintf(ImprimirLCD, "%c%c", (char)223, Celcius_O_Farenheit(0, 2));
    Imprimir_LCD(ImprimirLCD, 16, 1);

    sprintf(ImprimirLCD, "1:%cC", (char)223);
    Imprimir_LCD(ImprimirLCD, 3, 3);
    sprintf(ImprimirLCD, "%cF:2", (char)223);
    Imprimir_LCD(ImprimirLCD, 13, 3);

    if (PosicionActual > 8)
      eep.write(56, 254);
    if (PosicionActual <= 8)
      eep.write(56, 1);

    if (Pin_Entrada(4))
      PosicionActual += 8;
    if (Pin_Entrada(5))
      PosicionActual -= 8;

    if (PosicionActual != Vaux1)
    {
      Vaux1 = PosicionActual;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 1;
      lcd.clear();
    }

    PosicionActual = Volver_a_Cero(PosicionActual, 16);
    break;

  case 1:
    PosicionActual = 0;
    guardado_para_menus(false);
    break;
  }
}

void menu_seteo_wifi()
{
  switch (Flag)
  {
  case 0:
    sprintf(ImprimirLCD, "%s", NombreWifi);
    Imprimir_LCD(ImprimirLCD, 6, 0);
    sprintf(ImprimirLCD, "%s", ContraWifi);
    Imprimir_LCD(ImprimirLCD, 6, 1);
    eep.readChars(59, ImprimirLCD, 18);
    Imprimir_LCD(ImprimirLCD, 4, 2);
    memcpy(ImprimirLCD, "SSID:", 6);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    memcpy(ImprimirLCD, "PASS:", 6);
    Imprimir_LCD(ImprimirLCD, 0, 1);
    memcpy(ImprimirLCD, "IP:", 4);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "3 Seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(6))
    {
      lcd.clear();
      Flag = 1;
      PosicionActual = 0;
    }
    if (Pin_Entrada(7))
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

    break;
  case 1:

    for (Vaux1 = 0; Vaux1 < 19; Vaux1++)
    {
      NombreWifi[Vaux1] = '\0';
      ContraWifi[Vaux1] = '\0';
    }
    LetraSeleccionada = 0;
    PosicionActual = 0;
    Vaux2 = 0;
    Flag = 2;
    break;
  case 2:
    memcpy(ImprimirLCD, "Nombre Wifi:", 13);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, "%s", NombreWifi);
    Imprimir_LCD(ImprimirLCD, 0, 1);

    memcpy(ImprimirLCD, "1 Mayus.", 10);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "Sig. 2", 8);
    Imprimir_LCD(ImprimirLCD, 14, 2);
    memcpy(ImprimirLCD, "3 Pass.", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Del. 4", 10);
    Imprimir_LCD(ImprimirLCD, 14, 3);

    LetraSeleccionada = PosicionActual / 2;
    PosicionActual = Volver_a_Cero(PosicionActual, 80);
    NombreWifi[Vaux2] = Retorno_Caracter(LetraSeleccionada, Mayusculas);
    if (Pin_Entrada(42))
      PosicionActual++;
    if (Pin_Entrada(4))
      Mayusculas = !Mayusculas;
    if (Pin_Entrada(5) && Vaux2 <= 19)
    {
      Vaux2++;
      LetraSeleccionada = 0;
    }
    if (Pin_Entrada(6) && Vaux2 >= 3)
    {
      NombreWifi[Vaux2] = '\0';
      lcd.clear();
      Flag = 3;
      Vaux2 = 0;
      LetraSeleccionada = 0;
    }
    if (Pin_Entrada(7) && Vaux2 > 0)
    {
      LetraSeleccionada = 0;
      NombreWifi[Vaux2] = '\0';
      Vaux2--;
      lcd.clear();
    }
    break;

  case 3:
    memcpy(ImprimirLCD, "Pass Wifi:", 11);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, "%s", ContraWifi);
    Imprimir_LCD(ImprimirLCD, 0, 1);
    memcpy(ImprimirLCD, "1 Mayus.", 10);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "Sig. 2", 8);
    Imprimir_LCD(ImprimirLCD, 14, 2);
    memcpy(ImprimirLCD, "3 Guardar", 11);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Del. 4", 10);
    Imprimir_LCD(ImprimirLCD, 14, 3);

    LetraSeleccionada = PosicionActual / 2;
    PosicionActual = Volver_a_Cero(PosicionActual, 80);

    ContraWifi[Vaux2] = Retorno_Caracter(LetraSeleccionada, Mayusculas);

    if (Pin_Entrada(42))
      PosicionActual++;
    if (Pin_Entrada(4))
      Mayusculas = !Mayusculas;
    if (Pin_Entrada(5) && Vaux2 <= 19)
    {
      Vaux2++;
      LetraSeleccionada = 0;
    }
    if (Pin_Entrada(6) && Vaux2 >= 8)
    {
      ContraWifi[Vaux2] = '\0';
      lcd.clear();
      Flag = 4;
    }
    if (Pin_Entrada(7) && Vaux2 > 0)
    {
      LetraSeleccionada = 0;
      ContraWifi[Vaux2] = '\0';
      Vaux2--;
      lcd.clear();
    }
    break;
  case 4:
    for (Vaux1 = 0; Vaux1 < 20; Vaux1++)
    {
      eep.writeChars(14, ContraWifi, 20);
      eep.writeChars(34, NombreWifi, 20);
    }
    Enviar_Serial(3, 0);
    Enviar_Serial(4, 0);
    TiempoDeStandby = MiliSegundos;
    PosicionActual = 0;
    guardado_para_menus(false);
    break;
  }
}

void guardado_para_menus(bool Menu)
{

  while (MiliSegundos <= TiempoDeStandby + TIEMPO_ESPERA_GUARDADO)
  {
    if (MiliSegundos - TiempoDeStandby <= 100)
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
      if (MiliSegundos % 125 == 0)
      {
        lcd.scrollDisplayLeft();
      }
    }
  }
  if (MiliSegundos > TiempoDeStandby + TIEMPO_ESPERA_GUARDADO)
  {

    if (Menu == true)
      Estadoequipo = menu1;

    if (Menu == false)
      Estadoequipo = menu2;

    Flag = 0;
    lcd.noAutoscroll();
    lcd.clear();
    TiempoDeStandby = MiliSegundos;
    funcionActual = posicion_inicial;
  }
}

//██████████████████████████████████████████FUNCIONES DE SOPORTE██████████████████████████████████████████
void Barra_de_carga(uint8_t column, uint8_t row, int16_t value)
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

int16_t Celcius_O_Farenheit(int8_t value, uint8_t function)
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

uint8_t Volver_a_Cero(int8_t actualpos, uint8_t maxpos)
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

void Imprimir_Hora(char buffer[20], uint8_t hora_entrada, uint8_t minuto_entrada)
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

char Retorno_Caracter(uint8_t Character_pos, bool mayus)
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

bool Pin_Entrada(uint8_t Wich_Button)
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

void Pin_Salida(uint8_t nat_pin, bool state)
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

void Imprimir_LCD(char buffer[20], uint8_t Column, uint8_t Row)
{
  lcd.setCursor(Column, Row);
  lcd.print(buffer);
}

//████████████████████████████████████████████COMUNICACIONES██████████████████████████████████████████

void Leer_Serial()
{
  bool datosObtenidos;
  char datos[18];
  uint8_t largoDatos;
  uint8_t slotEeprom;
  uint8_t i;

  largoDatos = Serial.available();

  for (i = 0; i <= largoDatos; i++)
  {
    if (i == 0)
      LetraSeleccionada = Serial.read();
    if (i == 1)
      Serial.read();
    if (i >= 2 && i < largoDatos)
      datos[i - 2] = Serial.read();
    if (i == largoDatos)
      datosObtenidos = true;
  }

  if (datosObtenidos == true)
  {
    switch (LetraSeleccionada)
    {         // dependiendo del char de comando
    case 'W': // calentamiento manual
      TemperaturaACalentar = datos[0] - 33;

      if (datos[1] == 'O')
        Calentar = false;

      if (datos[1] == 'I')
        Calentar = true;

      datosObtenidos = false;
      break;

    case 'F': // llenmado manual
      NivelALLenar = datos[0] - 33;

      if (datos[1] == 'O') // prendido
        LLenar = false;

      if (datos[1] == 'I') // apagado
        LLenar = true;

      datosObtenidos = false;
      break;

    case 'S': // H por hora
      slotEeprom = datos[0] - 48;
      if (slotEeprom <= 2 && slotEeprom >= 0)
      {
        eep.write((slotEeprom * 3) + 1, datos[1] - 33); // hora
        eep.write((slotEeprom * 3) + 2, datos[2] - 33); // nivel
        eep.write((slotEeprom * 3) + 3, datos[3] - 33); // temp
        datosObtenidos = false;
      }
      break;

    case 'H': // temp auto
      eep.write(10, datos[0] - 33);
      eep.write(11, datos[1] - 33);
      datosObtenidos = false;
      break;
    case 'C': // llenado auto
      eep.write(12, datos[0] - 33);
      eep.write(13, datos[1] - 33);
      datosObtenidos = false;
      break;
    case 'I': // Ip
      eep.writeChars(59, datos, 18);
      InternetDisponible = true;
      if (Estadoequipo == funciones && funcionActual == funcion_de_menu_seteo_wifi)
        lcd.clear();
      datosObtenidos = false;
      break;

    case 'E':
      datosObtenidos = false;
      break;
    }
  }
}

void Enviar_Serial(uint8_t WhatSend, uint8_t What_slot)
{
  char mensajeAEnviar[22];
  char calentando; 
  char llenando;

  if (Calentar)
    calentando = 'I';
  else
    calentando = 'O';

  if (LLenar)
    llenando = 'I';
  else
    calentando = 'O';

  switch (WhatSend)
  {

  case 1:
    sprintf(mensajeAEnviar, "U_%c%c%c%c", 128 + TemperaturaActual, 33 + NivelActual, calentando, llenando);
    break;
  case 2:
    sprintf(mensajeAEnviar, "K_%c%c%c%c", 33 + eep.read((What_slot * 3) + 1), 33 + eep.read((What_slot * 3) + 2), 33 + eep.read((What_slot * 3) + 3), What_slot + 48);
    break;
  case 3:
    sprintf(mensajeAEnviar, "N_%s", NombreWifi);
    break;
  case 4:
    sprintf(mensajeAEnviar, "P_%s", ContraWifi);
    break;
  case 5:
    sprintf(mensajeAEnviar, "C_%c%c", 33 + eep.read(12), 33 + eep.read(13));
    break;
  case 6:
    sprintf(mensajeAEnviar, "H_%c%c", 33 + eep.read(10), 33 + eep.read(11));
  default:
    break;
  }
  Serial.println(mensajeAEnviar);
}
