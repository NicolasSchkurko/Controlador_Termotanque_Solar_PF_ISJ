
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
char mensajeAEnviar[24];
bool InternetDisponible = false;
uint8_t Enviar_Wifi_Serial;
uint8_t TiempoPulsadorEncoder;
uint8_t ItemSeleccionado = 0;
uint32_t TiempoDeStandby;
int8_t TemperaturaActual; // temp actual
uint8_t PosicionEntradas; // es el "sumador" del encoder
uint32_t tiempoSensores;
bool Mayusculas = false;
uint8_t PosicionActual2; // por cambiar
uint8_t PosicionActual;
uint32_t MiliSegundos;
uint8_t MinutoASetear;
uint8_t NivelASetear2;
uint8_t NivelASetear;
uint8_t TempASetear2;
uint8_t TempASetear;
uint8_t HoraASetear;
uint8_t GuardadoDeHora;
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
  // pulsadores
  DDRD &= B00001111; // 0 input, 1 output; de derecha a izquierda del 0 al 7
  DDRB &= B11111110; // De derecha a izquierda del 8 al 13
  // setea pull up o pull down
  PORTD |= B11110000;// De derecha a izquierda del 0 al 7
  PORTB |= B00000001; // 1 pull up 0 pull down; De derecha a izquierda del 8 al 13
  // pines encoder
  attachInterrupt(Pin_Entrada(2), EncoderPinA, CHANGE);
  attachInterrupt(Pin_Entrada(3), EncoderPinB, CHANGE);
  // inicia sensor temp
  Wire.begin();
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  TemperaturaActual = Sensor_temp.getTempCByIndex(0);
  // inicia rtc
  rtc.begin();
  // inicia serial
  Serial.begin(9600);
  // Iniciacion del LCD
  lcd.init();
  lcd.createChar(0, hora);
  lcd.createChar(3, wifi);
  lcd.createChar(4, barra_abajo);
  lcd.createChar(5, barra_derecha);
  lcd.createChar(6, barra_izquierda);
  lcd.createChar(7, barra_arriba);
  eep.readChars(16, ContraWifi, 20);
  eep.readChars(37, NombreWifi, 20);
  // Inicializa todo para iniciar el programa correctamente
  MiliSegundos = 0;
  Enviar_Wifi_Serial = 0;
  TiempoDeStandby = MiliSegundos;
  PosicionActual = PosicionEntradas;
  Estadoequipo = estado_inicial;
}

void loop()
{
  Actualizar_entradas();
  Actualizar_salidas();

  // si recibe un dato del serial lo lee
  Leer_Serial();

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
      //---------------- Menu Basico ---------------------
    case posicion_inicial:
      Estadoequipo = estado_inicial;
      break;
    case llenado_manual:
      eep.write(14, menu_de_llenado_manual());
      break;
    case calefaccion_manual:
      eep.write(15, menu_de_calefaccion_manual());
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
    //---------------- Menu avanzado ---------------------
    case funcion_de_menu_modificar_hora_rtc:
      menu_modificar_hora_rtc(now.hour(), now.minute());
      break;
    case funcion_farenheit_celsius:
      menu_farenheit_celsius();
      break;
    case funcion_activar_bomba:
      menu_activar_bomba();
      break;
    case funcion_de_menu_seteo_wifi:
      menu_seteo_wifi();
      break;
    }
    break;
  }
}

//███████████████████████████████████████CONTROL DE ENTRADAS/SALIDAS██████████████████████████████████████████
bool datosenviados;
void Actualizar_entradas()
{
  if (MiliSegundos >= tiempoSensores + TIEMPO_LECTURA_TEMP + 1000)
  {
    // Lee el sensor de temp y envia al esp el estado actual (temp y nivel actual)
    if (Estadoequipo == estado_Standby || Estadoequipo == estado_inicial)
      Sensor_temp.requestTemperatures();
    TemperaturaActual = Sensor_temp.getTempCByIndex(0);
    tiempoSensores = MiliSegundos;
    datosenviados = false;
  }
  if (MiliSegundos >= tiempoSensores + TIEMPO_LECTURA_TEMP && !datosenviados)
  {
    // Lee el sensor de temp y envia al esp el estado actual (temp y nivel actual)
    Enviar_Serial(0, 0);
    datosenviados = true;
    if (Enviar_Wifi_Serial < 3)
      Enviar_Wifi_Serial++;
  }
  // Envia mensajes al esp al iniciar el controlador
  if (Enviar_Wifi_Serial < 2)
  {
    if (Enviar_Wifi_Serial == 0 && !InternetDisponible)
      Enviar_Serial(1, 0);
    if (Enviar_Wifi_Serial == 1 && !InternetDisponible)
      Enviar_Serial(2, 0);
  }

  NivelActual = map(analogRead(SENSOR_NIVEL), 0, 1024, 0, 100);
  now = rtc.now(); // Actualiza el rtc
}

void Actualizar_salidas()
{
  char textoHora[6];

  if (TemperaturaActual < eep.read(15)) // temp a llenar
    Calentar = true;
  else if (TemperaturaActual <= eep.read(10)) // Temp minima
    eep.write(15, eep.read(11));              // Temp maxima
  else if (TemperaturaActual >= eep.read(15))
  {
    Calentar = false;
    eep.write(15, 0);
  }

  if (NivelActual < eep.read(14)) // nivel a llenar
    LLenar = true;
  else if (NivelActual <= eep.read(12)) // Nivel minimo
    eep.write(14, eep.read(13));        // Nivel maximo
  else if (NivelActual >= eep.read(14))
  {
    LLenar = false;
    eep.write(14, 0);
  }

  if (eep.read(59) == 254)  // eep57 es la activacion de la bomba, si esta en 254 (valor maximo) equivale a un booleano en true
    Pin_Salida(12, LLenar); // bomba

  Pin_Salida(11, LLenar);   // electrovalvula
  Pin_Salida(10, Calentar); // resistencia

  // Verifica si la hora actual es similar a alguna de las horas guardadas, en ese caso activa el calentamiento/llenado
  Imprimir_Hora(textoHora, now.hour(), now.minute());
  for (uint8_t i = 0; i < 3; i++)
  {
    if (Hora_a_guardado(textoHora) == eep.read(i * 3 + 1)) // compara hora con las guardadas y activa segun hora, multiplica por 3 para agrupar entre los tres "perfiles".
    {
      eep.write(14, eep.read(i * 3 + 2));
      eep.write(15, eep.read(i * 3 + 3));
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
    PosicionEntradas++;
  else
    PosicionEntradas--;
}

void EncoderPinB()
{
  if (Pin_Entrada(2) != Pin_Entrada(3))
    PosicionEntradas++;
  else
    PosicionEntradas--;
}

//███████████████████████████████████████████MENUES PRINCIPALES██████████████████████████████████████████
void Standby()
{
  // imprime carga o espera temperatura
  lcd.setCursor(5, 3);
  if (Calentar)
    lcd.write(char(94));
  else
    lcd.write(char(61));

  // imprime C o F dependiendo de la unidad seteada
  lcd.setCursor(7, 3);
  lcd.print(char(Celcius_O_Farenheit(TemperaturaActual, 2)));

  // imprime bomba o bomba desactivada titilando con una cruz
  lcd.setCursor(12, 3);
  if (eep.read(59) == 254)
    lcd.print(char(244));
  else
  {
    if (now.second() % 2 == 1)
      lcd.write(char(244));
    else
      lcd.write('x');
  }

  // imprime carga o espera llenado
  lcd.setCursor(14, 3);
  if (LLenar)
    lcd.write(char(94));
  else
    lcd.write(char(61));

  // imprime logo wifi
  lcd.setCursor(9, 3);
  lcd.print(char(3));
  // imprime cruz o carita sonriente si hay o no wifi (cambiar iconos)
  lcd.setCursor(10, 3);
  if (InternetDisponible)
    lcd.print(char(175));
  else
    lcd.write('x');

  // imprime el valor de temp
  sprintf(ImprimirLCD, "%d%c", Celcius_O_Farenheit(TemperaturaActual, 1), char(223));
  Imprimir_LCD(ImprimirLCD, 0, 0);

  // Proceso de impresion del nivel nivel
  /*Como el nivel lo queriamos imprimir necesitabamos detectar la cantidad de caracteres que utilizaba  -/
  /-  para poder dejarlo lo mas a la derecha posible, para ello imprimimos en el array de imprimir el   -/
  /-  valor actual del nivel y con un for detectamos la posicion en la que hay un /0, con eso logramos  -/
  /-  saber cuantos caracteres hay, y haciendo 20 (cant max de filas en el display) menos los caracteres-/
  /-  que ocupa imprimir el valor logramos imprimir lo mas a la derecha posible el nivel actual         */

  sprintf(ImprimirLCD, " %d%c", NivelActual, '%');
  for (PosicionActual2 = 0; ImprimirLCD[PosicionActual2] != '\0'; PosicionActual2++)
  { }                                                   // Detecta espacios en blanco para imprimirlo a la derecha
  Imprimir_LCD(ImprimirLCD, 20 - PosicionActual2, 0); // imprime nivel a la derecha

  // imprime relojito
  lcd.setCursor(7, 1);
  lcd.print(char(0));

  // imprime hora
  Imprimir_Hora(ImprimirLCD, now.hour(), now.minute());
  Imprimir_LCD(ImprimirLCD, 8, 1);

  Barra_de_carga(1, 1, TemperaturaActual); // imprime barra segun la temperatura
  Barra_de_carga(16, 1, NivelActual);

  // detecta si se presiono algun boton
  if (Pin_Entrada(4) || Pin_Entrada(5) || Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42)){
    PosicionEntradas += 1;
    MiliSegundos=0;
    tiempoSensores = MiliSegundos;
  }
  // Si se presiono algun boton cambia de estado
  if (PosicionActual != PosicionEntradas)
  {
    switch (Estadoequipo)
    {
    case estado_Standby:
      Estadoequipo = estado_inicial;
      break;
    case estado_inicial:
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
      break;
    default:
      Estadoequipo = estado_Standby;
      break;
    }
    PosicionActual = PosicionEntradas;
    TiempoDeStandby = MiliSegundos;
  }
  // Caso contrario va un estado hacia atras
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
    PosicionActual = 0;
    PosicionEntradas = 0;
    Flag = 1;
    break;
  case 1:
    // imprime las opciones del menu
    sprintf(ImprimirLCD, ">%s", Menuprincipal[Volver_a_Cero(PosicionActual, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(PosicionActual + 1, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 1);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(PosicionActual + 2, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 2);
    sprintf(ImprimirLCD, "%s", Menuprincipal[Volver_a_Cero(PosicionActual + 3, COLUMNAS_MAXIMAS_M1)]);
    Imprimir_LCD(ImprimirLCD, 1, 3);

    // mueven el menu

    if (Pin_Entrada(4))
      PosicionEntradas += 2; // baja una columna

    if (Pin_Entrada(5))
      PosicionEntradas -= 2; // sube una columna

    if (TiempoPulsadorEncoder != 200 && MiliSegundos >= TiempoDeStandby + 250)
      TiempoPulsadorEncoder = 200; // pulsador encoder == 200 mili segundos

    if (Pin_Entrada(42) && MiliSegundos >= TiempoDeStandby + TiempoPulsadorEncoder) // con el pulsador del encoder va aumentando la velocidad con el tiempo
    {                                                                               // va sumando +2 mientras acelera la diferencia en el que se suma 2
      PosicionEntradas += 2;
      if (TiempoPulsadorEncoder == 200)
        TiempoPulsadorEncoder -= 20;
      if (TiempoPulsadorEncoder == 180)
        TiempoPulsadorEncoder -= 30;
      if (TiempoPulsadorEncoder == 150)
        TiempoPulsadorEncoder -= 50;
      if (TiempoPulsadorEncoder <= 100 && TiempoPulsadorEncoder >= 10)
        TiempoPulsadorEncoder -= 10;
    }
    if (PosicionEntradas >= 14 && PosicionEntradas <= 28)
      PosicionEntradas=0;
    if (PosicionEntradas >= 29)
      PosicionEntradas=13;

    if (PosicionEntradas / 2 != PosicionActual) // cada 2 pulsos del encoder baja/sube una columna del menu
    {
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
      PosicionActual = PosicionEntradas / 2;
    }

    // Selecciona algun item
    if (Pin_Entrada(6))
    {
      TiempoDeStandby = MiliSegundos;
      Flag = PosicionActual + 2;
      lcd.clear();
    }

    // Si no se selecciona ni aprieta nada vuelve al estado inicial
    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_MENUES)
    {
      lcd.clear();
      TiempoDeStandby = MiliSegundos;
      PosicionActual = PosicionEntradas;
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
    PosicionActual = PosicionEntradas;
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
  case 0: // inicializa variables
    TiempoDeStandby = MiliSegundos;
    PosicionActual = 0;
    PosicionEntradas = 0;
    lcd.clear();
    Flag = 1;
    break;
  case 1:
    // imprime opciones menu
    sprintf(ImprimirLCD, ">%s", Menuavanzado[Volver_a_Cero(PosicionActual, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 0);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(PosicionActual + 1, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 1);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(PosicionActual + 2, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    sprintf(ImprimirLCD, " %s", Menuavanzado[Volver_a_Cero(PosicionActual + 3, COLUMNAS_MAXIMAS_M2)]);
    Imprimir_LCD(ImprimirLCD, 0, 3);

    // mueven el menu
    if (Pin_Entrada(4))
      PosicionEntradas += 2; // suma 1 a PosicionActual
    if (Pin_Entrada(5))
      PosicionEntradas -= 2; // resta 1 a PosicionActual

    if (TiempoPulsadorEncoder != 200 && MiliSegundos >= TiempoDeStandby + 250)
      TiempoPulsadorEncoder = 200;

    if (Pin_Entrada(42) && MiliSegundos >= TiempoDeStandby + TiempoPulsadorEncoder)
    {
      PosicionEntradas += 2;
      if (TiempoPulsadorEncoder == 200)
        TiempoPulsadorEncoder -= 20;
      if (TiempoPulsadorEncoder == 180)
        TiempoPulsadorEncoder -= 30;
      if (TiempoPulsadorEncoder == 150)
        TiempoPulsadorEncoder -= 50;
      if (TiempoPulsadorEncoder <= 100 && TiempoPulsadorEncoder >= 10)
        TiempoPulsadorEncoder -= 10;
    }
    if (PosicionEntradas >= 10 && PosicionEntradas <= 20)
      PosicionEntradas=0;
    if (PosicionEntradas >= 21)
      PosicionEntradas=9;

    if (PosicionEntradas / 2 != PosicionActual)
    {
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
      PosicionActual = PosicionEntradas / 2;
    }

    // Elije una opcion
    if (Pin_Entrada(6))
    {
      Flag = PosicionActual + 2;
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
    }

    // Standby
    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_MENUES)
    {
      TiempoDeStandby = MiliSegundos;
      PosicionActual = PosicionEntradas;
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
    PosicionEntradas = 0;
    Estadoequipo = menu1;
    break;
  }
}

//███████████████████████████████████████████MENUES FUNCIONALES██████████████████████████████████████████

uint8_t menu_de_llenado_manual()
{
  switch (Flag)
  {
  case 0:
    if (NIVEL_MINIMO > NivelActual)
      NivelASetear = TEMP_MINIMO; //Temp minimo?
    else
      NivelASetear = NivelActual;
    NivelASetear = NIVEL_MINIMO;
    lcd.clear();
    PosicionEntradas = 0;
    Flag = 1;
    break;
  case 1:
    memcpy(ImprimirLCD, "LLenar hasta", 14);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c ", NivelASetear, '%');
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
      PosicionEntradas += 1;

    if (Pin_Entrada(5))
      PosicionEntradas -= 1;


    if (PosicionEntradas >= 4 && PosicionEntradas <= 8)
      PosicionEntradas=0;
    if (PosicionEntradas >= 9)
      PosicionEntradas=3;
    if ((PosicionEntradas + 1) * SUMADOR_NIVEL != NivelASetear)
    {
      NivelASetear = (PosicionEntradas + 1) * SUMADOR_NIVEL; // actualiza el nivel a setear
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

    break;

  case 2:

    memcpy(ImprimirLCD, "LLenar hasta:", 14);
    Imprimir_LCD(ImprimirLCD, 1, 0);
    sprintf(ImprimirLCD, "%d%c", NivelASetear, '%');
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
      PosicionEntradas = 0;
      TiempoDeStandby = MiliSegundos;
      lcd.clear();
    }
    break;

  case 3:
    TiempoDeStandby = MiliSegundos;
    guardado_para_menus(true);
    return NivelASetear;
    Flag = 0;
    break;
  }
}

uint8_t menu_de_calefaccion_manual()
{
  switch (Flag)
  {
  case 0:
    if (TEMP_MINIMO > TemperaturaActual)
      TempASetear = TEMP_MINIMO;
    else
      TempASetear = (TemperaturaActual / 5) * 5;
    lcd.clear();
    PosicionEntradas = 0;
    Flag = 1;
    break;

  case 1:

    memcpy(ImprimirLCD, "Calentar a", 11);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(TempASetear, 1), (char)223, Celcius_O_Farenheit(TempASetear, 2));
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
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

  
    if (PosicionEntradas >= 9 && PosicionEntradas <= 18)
      PosicionEntradas=0;
    if (PosicionEntradas >= 19)
      PosicionEntradas=8;
    
    if ((PosicionEntradas + 8) * SUMADOR_TEMP != TempASetear)
    {
      TempASetear = (PosicionEntradas + 8) * SUMADOR_TEMP; // actualiza el nivel a setear
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      PosicionEntradas = 0;
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
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(TempASetear, 1), (char)223, Celcius_O_Farenheit(TempASetear, 2));
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
      PosicionEntradas = 0;
      TiempoDeStandby = 0;
      lcd.clear();
    }
    break;

  case 3:
    TiempoDeStandby = MiliSegundos;
    guardado_para_menus(true);
    return TempASetear;
    Flag = 0;
    break;
  }
}

void menu_de_auto_por_hora(uint8_t hora_actual, uint8_t minutos_actual)
{
  switch (Flag)
  {
  case 0:
    HoraASetear = 0;
    MinutoASetear = 0;
    PosicionEntradas = 0;
    lcd.clear();
    TiempoDeStandby = MiliSegundos;
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
    Imprimir_LCD(ImprimirLCD, 0, ItemSeleccionado + 1);

    if (Pin_Entrada(4))
      PosicionEntradas += 4;
    if (Pin_Entrada(5))
      PosicionEntradas -= 4;
    if (Pin_Entrada(42))
      PosicionEntradas += 1;

    if (Pin_Entrada(6))
    {
      TempASetear = TEMP_MINIMO;
      NivelASetear = NIVEL_MINIMO;
      PosicionEntradas = 0;
      lcd.clear();
      Flag = 2;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      lcd.clear();
    }
    ItemSeleccionado = PosicionEntradas / 4;
    if (PosicionEntradas >= 12 && PosicionEntradas <= 30 )
      PosicionEntradas=1;
    if (PosicionEntradas >= 31)
      PosicionEntradas=11;

    if (PosicionActual2 != ItemSeleccionado)
    {
      memcpy(ImprimirLCD, " ", 2);
      Imprimir_LCD(ImprimirLCD, 0, PosicionActual2 + 1);
      PosicionActual2 = ItemSeleccionado;
      TiempoDeStandby = MiliSegundos;
    }

    break;

  case 2:
    memcpy(ImprimirLCD, "Calentar a", 11);
    Imprimir_LCD(ImprimirLCD, 2, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(TempASetear, 1), (char)223, Celcius_O_Farenheit(TempASetear, 2));
    Imprimir_LCD(ImprimirLCD, 13, 0);
    sprintf(ImprimirLCD, "1 +%d", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 0, 2);
    sprintf(ImprimirLCD, "-%d 2", Celcius_O_Farenheit(5, 3));
    Imprimir_LCD(ImprimirLCD, 16, 2);
    memcpy(ImprimirLCD, "3 seguir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 9 && PosicionEntradas <= 18 )
      PosicionEntradas=0;
    if (PosicionEntradas >= 19)
      PosicionEntradas=8;

    if ((PosicionEntradas + 8) * SUMADOR_TEMP != TempASetear)
    {
      TiempoDeStandby = MiliSegundos;
      TempASetear = (PosicionEntradas + 8) * SUMADOR_TEMP;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionEntradas = 0;
    }

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
    sprintf(ImprimirLCD, "%d%c ", NivelASetear, '%');
    Imprimir_LCD(ImprimirLCD, 13, 0);
    memcpy(ImprimirLCD, "1 +25", 20);
    Imprimir_LCD(ImprimirLCD, 0, 2);
    memcpy(ImprimirLCD, "-25 2", 20);
    Imprimir_LCD(ImprimirLCD, 15, 2);
    memcpy(ImprimirLCD, "3 Salir", 9);
    Imprimir_LCD(ImprimirLCD, 0, 3);
    memcpy(ImprimirLCD, "Volver 4", 9);
    Imprimir_LCD(ImprimirLCD, 12, 3);

    if ((PosicionEntradas + 1) * SUMADOR_NIVEL != NivelASetear)
    {
      NivelASetear = (PosicionEntradas + 1) * SUMADOR_NIVEL;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 4;
      PosicionEntradas = 0;
      lcd.clear();
      PosicionEntradas = 0;
    }
    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionEntradas = 0;
    }
    if (PosicionEntradas >= 4 && PosicionEntradas <= 8 )
      PosicionEntradas=0;
    if (PosicionEntradas >= 9)
      PosicionEntradas=3;
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

    if (HoraASetear !=  PosicionEntradas)
    {
      HoraASetear = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }
    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 5;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 3;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (PosicionEntradas >= 24 && PosicionEntradas <= 48)
      PosicionEntradas=0;
    if (PosicionEntradas >= 49)
      PosicionEntradas=23;

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

    if (MinutoASetear != PosicionEntradas)
    {
      MinutoASetear = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (Pin_Entrada(6))
    {
      Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
      GuardadoDeHora = Hora_a_guardado(ImprimirLCD);
      if (GuardadoDeHora != eep.read(1) && GuardadoDeHora != eep.read(4) && GuardadoDeHora != eep.read(7))
      {
        Flag = 6;
        lcd.clear();
        PosicionEntradas = 0;
      }
      else
      {
        if(GuardadoDeHora == eep.read((ItemSeleccionado * 3) + 1))
        {
          Flag = 6;
          lcd.clear();
          PosicionEntradas = 0;
        }
        else
        {
          memcpy(ImprimirLCD, "Hora ya usada", 15);
          Imprimir_LCD(ImprimirLCD, 3, 1);
        }
      }

    }

    if (Pin_Entrada(7))
    {
      Flag = 4;
      lcd.clear();
      PosicionEntradas = 0;
    }
    if (PosicionEntradas >= 60 && PosicionEntradas <= 80)
      PosicionEntradas=0;
    if (PosicionEntradas >= 81)
      PosicionEntradas=59;

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
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(TempASetear, 1), (char)223, Celcius_O_Farenheit(TempASetear, 2));
    Imprimir_LCD(ImprimirLCD, 13, 1);
    sprintf(ImprimirLCD, "LLenar %d%c", NivelASetear, '%');
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
      PosicionEntradas = 0;
      TiempoDeStandby = MiliSegundos;
    }
    break;

  case 7:
    Imprimir_Hora(ImprimirLCD, HoraASetear, MinutoASetear);
    eep.write((ItemSeleccionado * 3) + 1, Hora_a_guardado(ImprimirLCD));

    eep.write((ItemSeleccionado * 3) + 2, NivelASetear);
    eep.write((ItemSeleccionado * 3) + 3, TempASetear);

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
    NivelASetear = eep.read(12);  // nivel minimo
    NivelASetear2 = eep.read(13); // nivel maximo
    PosicionEntradas = 0;
    TiempoDeStandby = MiliSegundos;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(ImprimirLCD, "Nivel min", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c  ", eep.read(12), '%');
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
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 4 && PosicionEntradas <= 8)
      PosicionEntradas=0;
    if (PosicionEntradas >= 9)
      PosicionEntradas=3;

    if ((PosicionEntradas)*SUMADOR_NIVEL != eep.read(12))
    {
      eep.write(12, (PosicionEntradas)*SUMADOR_NIVEL);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      eep.write(13, eep.read(12) + SUMADOR_NIVEL);
      lcd.clear();
      PosicionEntradas = (eep.read(12) / SUMADOR_NIVEL)+1;
    }

    if (Pin_Entrada(7) == true || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      eep.write(12, NivelASetear);
      eep.write(13, NivelASetear2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

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
      PosicionEntradas += 1;
    if (Pin_Entrada(5) == true)
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 5 && PosicionEntradas <= 10)
      PosicionEntradas=(eep.read(12)/SUMADOR_NIVEL)+1;
    if(eep.read(12)==0)
    {
    if (PosicionEntradas >= 9)
      PosicionEntradas= 4;
    }
    else
    {
    if (PosicionEntradas < eep.read(12)/SUMADOR_NIVEL)
      PosicionEntradas= 4;
    }

    if (PosicionEntradas * SUMADOR_NIVEL != eep.read(13))
    {
      eep.write(13, PosicionEntradas * SUMADOR_NIVEL);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(12, NivelASetear);
      eep.write(13, NivelASetear2);
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
      PosicionEntradas = 0;
    }
    break;

  case 4:
    guardado_para_menus(true);
    break;
  }
}

void menu_de_calefaccion_auto()
{
  switch (Flag)
  {
  case 0:
    TempASetear = eep.read(10);
    TempASetear2 = eep.read(11);
    eep.write(10, TEMP_MINIMO);
    PosicionEntradas = 8;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(ImprimirLCD, "Temp. min", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(eep.read(10), 1), (char)223, Celcius_O_Farenheit(0, 2));
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
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 16)
      PosicionEntradas=8;
    if (PosicionEntradas < 8)
      PosicionEntradas=15;

    if (PosicionEntradas * SUMADOR_TEMP != eep.read(10))
    {
      eep.write(10, PosicionEntradas * SUMADOR_TEMP);
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 2;
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      eep.write(10, TempASetear);
      eep.write(11, TempASetear2);
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }
    break;

  case 2:
    memcpy(ImprimirLCD, "Temp. max", 11);
    Imprimir_LCD(ImprimirLCD, 3, 0);
    sprintf(ImprimirLCD, "%d%c%c ", Celcius_O_Farenheit(eep.read(11), 1), (char)223, Celcius_O_Farenheit(0, 2));
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
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 17)
      PosicionEntradas= (eep.read(10)+SUMADOR_TEMP) / SUMADOR_TEMP;
    if (PosicionEntradas <= eep.read(10) / SUMADOR_TEMP)
      PosicionEntradas= 16;

    if (PosicionEntradas * SUMADOR_TEMP != eep.read(11))
    {
      eep.write(11, PosicionEntradas * SUMADOR_TEMP);
      TiempoDeStandby = MiliSegundos;
    }
    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      lcd.clear();
      PosicionEntradas= eep.read(10) / SUMADOR_TEMP;
    }
  
    
    if (MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 0;
      Estadoequipo = menu1;
      eep.write(10, TempASetear);
      eep.write(11, TempASetear2);
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
      PosicionEntradas = 0;
      TiempoDeStandby = MiliSegundos;
    }
    break;

  case 4:
    TiempoDeStandby = MiliSegundos;
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
    PosicionEntradas = 0;
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

    if (PosicionEntradas >= 24 && PosicionEntradas <= 48)
      PosicionEntradas=0;
    if (PosicionEntradas >= 49)
      PosicionEntradas=23;

    if (HoraASetear != PosicionEntradas)
    {
      HoraASetear = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(4) || Pin_Entrada(42))
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (Pin_Entrada(6))
    {
      Flag = 2;
      lcd.clear();
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

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
      PosicionEntradas += 1;
    if (Pin_Entrada(5))
      PosicionEntradas -= 1;

    if (PosicionEntradas >= 60 && PosicionEntradas <= 70)
      PosicionEntradas=0;
    if (PosicionEntradas >= 71)
      PosicionEntradas=59;

    if (MinutoASetear != PosicionEntradas)
    {
      MinutoASetear = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6))
    {
      Flag = 3;
      lcd.clear();
      TiempoDeStandby = MiliSegundos;
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 1;
      PosicionEntradas = 0;
      lcd.clear();
    }


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
      PosicionEntradas = 0;
    }

    if (Pin_Entrada(7))
    {
      Flag = 2;
      lcd.clear();
      PosicionEntradas = 0;
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
    if (eep.read(59) == 254)
    {
      memcpy(ImprimirLCD, "Bomba  activada", 16);
      Imprimir_LCD(ImprimirLCD, 2, 1);
    }
    if (eep.read(59) == 1)
    {
      memcpy(ImprimirLCD, "Bomba desactivada", 19);
      Imprimir_LCD(ImprimirLCD, 1, 1);
    }
    memcpy(ImprimirLCD, "1:Si", 5);
    Imprimir_LCD(ImprimirLCD, 3, 3);
    memcpy(ImprimirLCD, "2:No", 5);
    Imprimir_LCD(ImprimirLCD, 11, 3);

    if (Pin_Entrada(4))
      PosicionEntradas = 10;
    if (Pin_Entrada(5))
      PosicionEntradas = 5;

    if (PosicionEntradas > 8)
      eep.write(59, 254);
    if (PosicionEntradas <= 8)
      eep.write(59, 1);


    if (PosicionEntradas >= 16 && PosicionEntradas <= 32)
      PosicionEntradas=0;
    if (PosicionEntradas >= 33)
      PosicionEntradas=15;
    if (PosicionEntradas != PosicionActual)
    {
      lcd.clear();
      PosicionActual = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 1;
      lcd.clear();
    }
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

    if (PosicionEntradas > 8)
      eep.write(58, 254);
    if (PosicionEntradas <= 8)
      eep.write(58, 1);

    if (Pin_Entrada(4))
      PosicionEntradas += 8;
    if (Pin_Entrada(5))
      PosicionEntradas -= 8;

    if (PosicionEntradas >= 16 && PosicionEntradas <= 32)
      PosicionEntradas=0;
    if (PosicionEntradas >= 33)
      PosicionEntradas=15;

    if (PosicionEntradas != PosicionActual)
    {
      PosicionActual = PosicionEntradas;
      TiempoDeStandby = MiliSegundos;
    }

    if (Pin_Entrada(6) || Pin_Entrada(7) || Pin_Entrada(42) || MiliSegundos >= TiempoDeStandby + TIEMPO_ESPERA_FUNCIONES)
    {
      Flag = 1;
      lcd.clear();
    }

    break;

  case 1:
    PosicionEntradas = 0;
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
    eep.readChars(60, ImprimirLCD, 22);
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
      PosicionEntradas = 0;
    }
    if (Pin_Entrada(7))
    {
      Estadoequipo = menu2;
      Flag = 0;
      lcd.clear();
    }

    break;
  case 1:

    for (PosicionActual = 0; PosicionActual < 20; PosicionActual++)
    {
      NombreWifi[PosicionActual] = 0;
      ContraWifi[PosicionActual] = 0;
    }
    ItemSeleccionado = 0;
    PosicionEntradas = 0;
    PosicionActual2 = 0;
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

    ItemSeleccionado = PosicionEntradas / 2;
    if (PosicionEntradas >= 81 && PosicionEntradas <= 91)
      PosicionEntradas=0;
    if (PosicionEntradas >= 92)
      PosicionEntradas=80;
    NombreWifi[PosicionActual2] = Retorno_Caracter(ItemSeleccionado, Mayusculas);
    if (Pin_Entrada(42))
      PosicionEntradas++;
    if (Pin_Entrada(4))
      Mayusculas = !Mayusculas;
    if (Pin_Entrada(5) && PosicionActual2 <= 19)
    {
      PosicionActual2++;
      ItemSeleccionado = 0;
    }
    if (Pin_Entrada(6) && PosicionActual2 >= 3)
    {
      NombreWifi[PosicionActual2] = 0;
      lcd.clear();
      Flag = 3;
      PosicionActual2 = 0;
      ItemSeleccionado = 0;
    }
    if (Pin_Entrada(7) && PosicionActual2 > 0)
    {
      ItemSeleccionado = 0;
      NombreWifi[PosicionActual2] = 0;
      PosicionActual2--;
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

    ItemSeleccionado = PosicionEntradas / 2;
    if (PosicionEntradas >= 81 && PosicionEntradas <= 91)
      PosicionEntradas=0;
    if (PosicionEntradas >= 92)
      PosicionEntradas=80;
    ContraWifi[PosicionActual2] = Retorno_Caracter(ItemSeleccionado, Mayusculas);

    if (Pin_Entrada(42))
      PosicionEntradas++;
    if (Pin_Entrada(4))
      Mayusculas = !Mayusculas;
    if (Pin_Entrada(5) && PosicionActual2 <= 19)
    {
      PosicionActual2++;
      ItemSeleccionado = 0;
    }
    if (Pin_Entrada(6) && PosicionActual2 >= 8)
    {
      ContraWifi[PosicionActual2] = 0;
      lcd.clear();
      Flag = 4;
    }
    if (Pin_Entrada(7) && PosicionActual2 > 0)
    {
      ItemSeleccionado = 0;
      ContraWifi[PosicionActual2] = 0;
      PosicionActual2--;
      lcd.clear();
    }
    break;
  case 4:
    for (PosicionActual = 0; PosicionActual < 20; PosicionActual++)
    {
      eep.writeChars(16, ContraWifi, 20);
      eep.writeChars(37, NombreWifi, 20);
    }
    Enviar_Wifi_Serial=0;
    TiempoDeStandby = MiliSegundos;
    PosicionEntradas = 0;
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
    if (eep.read(58) == 254)
      return ((9 * value) / 5) + 32;
    if (eep.read(58) == 1)
      return value;
  }
  if (function == 2)
  {
    if (eep.read(58) == 254)
      return 'F';
    if (eep.read(58) == 1)
      return 'C';
  }
  if (function == 3)
  {
    if (eep.read(58) == 254)
      return ((9 * value) / 5);
    if (eep.read(58) == 1)
      return value;
  }
}

uint8_t Volver_a_Cero(uint8_t actualpos, uint8_t maxpos)
{
  if (actualpos >= maxpos && actualpos <= 254 - (254 % maxpos))
  {
    return actualpos % maxpos;
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

int8_t Hora_a_guardado(char buffer[20]) //// ya arregle lo de volver
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
  String mensaje;
  mensaje = Serial.readString();
  Serial.println(mensaje);
}

void Enviar_Serial(uint8_t WhatSend, uint8_t What_slot)
{
}
