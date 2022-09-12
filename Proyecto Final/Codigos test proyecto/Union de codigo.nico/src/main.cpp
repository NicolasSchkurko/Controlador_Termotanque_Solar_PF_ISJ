
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

byte nono[8] = {
  B00000,
  B00000,
  B10001,
  B01010,
  B00100,
  B01010,
  B10001,
  B00000
};
byte bomba[8] = {
  B00000,
  B00000,
  B01110,
  B01110,
  B11111,
  B11111,
  B11111,
  B00000
};
byte wifi[8] = {
  B00000,
  B00000,
  B11110,
  B00001,
  B11001,
  B00101,
  B10101,
  B00000
};
byte no_wifi[8] = {
  B00000,
  B00101,
  B00010,
  B00101,
  B11000,
  B00100,
  B10100,
  B00000
};


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
uint8_t Posicion_actual;
uint8_t nivel_actual;
uint8_t temperatura_a_calentar;
uint8_t nivel_a_llenar;
int8_t temperatura_actual; // temp actual
uint8_t hora_to_modify, minuto_to_modify;

bool mayusculas = false;
bool llenar;
bool calentar;
bool esp_working =false;

//█████████████████████████████████████████████████████████████████████████████████

void setup()
{
  // Interrupcion cada 1 mili
  SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2 | 0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;
  // pulsadores pra manejar los menus//
  DDRD &= B00001111; 
  DDRB &= B00011100;

  PORTD |= B11110000; // setea pull up o pull down

  // pin  nivel
  pinMode(nivel_del_tanque, nivel_del_tanque); 
  // pines encoder
  attachInterrupt(PressedButton(40), doEncodeA, CHANGE);
  attachInterrupt(PressedButton(41), doEncodeB, CHANGE);
  // inicia sensor temp
  Wire.begin();       
  Sensor_temp.begin();
  Sensor_temp.requestTemperatures();
  temperatura_actual = Sensor_temp.getTempCByIndex(0);
  rtc.begin();        // inicializacion del rtc 
  // inicializacion del serial arduino-esp
  Serial.begin(9600); 
  Serial.setTimeout(200);

  lcd.init(); // Iniciacion del LCD
  lcd.createChar(2 , nono); 
  lcd.createChar(3 , bomba); 
  lcd.createChar(4 , wifi); 
  lcd.createChar(5 , no_wifi); 
  // Sensr de temperatura
  eep.readChars(14,password_wifi_setear, 20);
  eep.readChars(34,nombre_wifi_setear, 20);

  while (Vaux1<=7)
  {
    if (Vaux1==0)
        {
        Serial_Send_UNO(4, 0); 
        Vaux1++;
        }
    if (Vaux1==1)
      {
      Serial_Send_UNO(3, 0); 
      Vaux1++;
      }
    if (Vaux1==2)
      {
      Serial_Send_UNO(5, 0); 
      Vaux1++;
      }
    if (Vaux1==3)
      {
      Serial_Send_UNO(6, 0); 
      Vaux1++;
      }
    if (Vaux1==4)
      {
      Serial_Send_UNO(2, 1); 
      Vaux1++;
      }
    if (Vaux1==5)
      {
      Serial_Send_UNO(2, 2); 
      Vaux1++;
      }
    if (Vaux1==6)
      {
      Serial_Send_UNO(2, 3); 
      Vaux1++;
      }
    if (Vaux1==7)
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
  ControlOutput();

  if (Serial.available() > 0)Serial_Read_UNO(); // si recibe un dato del serial lo lee

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
      menu_de_auto_por_hora(now.hour(),now.minute());
      break;
    case llenado_auto:
      menu_de_llenado_auto();
      break;
    case calefaccion_auto:
      menu_de_calefaccion_auto();
      break;
    case funcion_de_menu_modificar_hora_rtc:
       menu_modificar_hora_rtc(now.hour(),now.minute());
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
    if (PressedButton(40) == PressedButton(41))
      Posicion_actual++;
    else
      Posicion_actual--;
}

void doEncodeB()
{
    if (PressedButton(40) != PressedButton(41))
      Posicion_actual++;
    else
      Posicion_actual--;
}

//███████████████████████████████████████CONTROL DE ENTRADAS/SALIDAS██████████████████████████████████████████


void Actualizar_entradas()
{ // Sexo y adaptarlo para no usar delay farenheit
  if (mili_segundos >= tiempo_sensores + tiempo_para_temperatura && Estadoequipo!=menu1 && Estadoequipo!=menu2)
  {
    Sensor_temp.requestTemperatures();
    temperatura_actual = Sensor_temp.getTempCByIndex(0);

    if (analogRead(nivel_del_tanque) < 100)nivel_actual = 0;
    if (analogRead(nivel_del_tanque) >= 100 && analogRead(nivel_del_tanque) < 256)nivel_actual = 25;
    if (analogRead(nivel_del_tanque) >= 256 && analogRead(nivel_del_tanque) < 512)nivel_actual = 50;
    if (analogRead(nivel_del_tanque) >= 512 && analogRead(nivel_del_tanque) < 768)nivel_actual = 75;
    if (analogRead(nivel_del_tanque) >= 768 && analogRead(nivel_del_tanque) <= 1024)nivel_actual = 100;
    tiempo_sensores = mili_segundos;
  } 
  // Actualiza el rtc
  now = rtc.now();
}

void ControlOutput()
{
  char Array_hora[6];

  if (temperatura_actual <= eep.read(10) || temperatura_actual < temperatura_a_calentar)
    calentar = true;
  if (temperatura_actual > eep.read(11) || temperatura_actual >= temperatura_a_calentar)
    calentar = false;
  PrintOutput(10, calentar);

  if (nivel_actual <= eep.read(12) || nivel_actual < nivel_a_llenar)
    llenar = true;
  if (nivel_actual > eep.read(13) || nivel_actual >= nivel_a_llenar)
    llenar = false;
  PrintOutput(11, llenar);
  if (eep.read(56)==254)
  PrintOutput(12, llenar);

  Printhora(Array_hora,now.hour(),now.minute());
  for (uint8_t i; i < 3; i++)
  {
    if (ArrayToChar(Array_hora) == eep.read(i + 1))
    {
      temperatura_a_calentar = eep.read(i + 2);
      nivel_a_llenar = eep.read(i + 3);
    }
  }
}


//███████████████████████████████████████████MENUES PRINCIPALES██████████████████████████████████████████

void standby()
{
  lcd.setCursor(5 ,3);
  if(esp_working)lcd.print(char(4));
  if(!esp_working)lcd.print(char(5));
  lcd.setCursor(6 ,3);
  if(eep.read(57)==254) lcd.print(char(3));
  if(eep.read(57)==1)lcd.print(char(2));
  

  sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(temperatura_actual,1), (char)223,CelciusOrFarenheit(temperatura_actual,2));
  PrintLCD(imprimir_lcd, 0, 0);
  sprintf(imprimir_lcd, "%d%c", nivel_actual, '%');
  for (Vaux2=0;imprimir_lcd[Vaux2]!='\0';Vaux2++){}
  PrintLCD(imprimir_lcd, 20-Vaux2, 0);
  
  lcd.setCursor(7,1);lcd.print(char(0));
  Printhora(imprimir_lcd,now.hour(),now.minute());PrintLCD(imprimir_lcd, 8, 1);

  if (PressedButton(1) == true|| PressedButton(2) == true|| PressedButton(3) == true)Posicion_actual+=1;
  
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
   if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
  {
      tiempo_de_standby = mili_segundos;
      Vaux1 = Posicion_actual;
      Estadoequipo = estado_standby;
  }
}

void menu_basico()
{
  const char *menuprincipal[maxY_menu1] = {
    "C manual",
    "H manual",
    "H & F in H",
    "C segun lleno",
    "H segun temp",
    "menu avanzado",
    "volver"
  };
  switch (Flag)
  {
  case 0:
    tiempo_de_standby = mili_segundos;
    lcd.clear();
    Flag = 1;
    break;
  case 1:
    sprintf(imprimir_lcd,">%s",  menuprincipal[ReturnToCero(Vaux1, maxY_menu1)]);     PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd,"%s",  menuprincipal[ReturnToCero(Vaux1 + 1, maxY_menu1)]);  PrintLCD(imprimir_lcd, 1, 1);
    sprintf(imprimir_lcd,"%s",  menuprincipal[ReturnToCero(Vaux1 + 2, maxY_menu1)]);  PrintLCD(imprimir_lcd, 1, 2);
    sprintf(imprimir_lcd,"%s", menuprincipal[ReturnToCero(Vaux1 + 3, maxY_menu1)]);   PrintLCD(imprimir_lcd, 1, 3);

    if (Posicion_actual / 2 != Vaux1)
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual / 2;
    }

    if (PressedButton(1))Posicion_actual+=2; // suma 2 al encoder
    if (PressedButton(2))Posicion_actual-=2; ; // resta 2 al encoder

    Posicion_actual=ReturnToCero(Posicion_actual,maxY_menu1 * 2);

    if (PressedButton(3))
    {
      Flag = Vaux1 + 2;
      lcd.clear();
    }

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual;
      Estadoequipo = estado_standby;
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
    "Setear hora display",
    "Cambiar unidad",
    "Activar la bomba",
    "conexion wifi",
    "volver"
  };
  switch (Flag)
  {
  case 0:
    tiempo_de_standby = mili_segundos;
    Posicion_actual=0;
    lcd.clear();
    Flag = 1;
    break;
  case 1:
    sprintf(imprimir_lcd, ">%s", menuavanzado[ReturnToCero(Vaux1, maxY_menu2)]);    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd,"%s", menuavanzado[ReturnToCero(Vaux1 + 1, maxY_menu2)]);  PrintLCD(imprimir_lcd, 1, 1);
    sprintf(imprimir_lcd,"%s", menuavanzado[ReturnToCero(Vaux1 + 2, maxY_menu2)]);  PrintLCD(imprimir_lcd, 1, 2);
    sprintf(imprimir_lcd,"%s",menuavanzado[ReturnToCero(Vaux1 + 3, maxY_menu2)]);   PrintLCD(imprimir_lcd, 1, 3);


    if (Posicion_actual / 2 != Vaux1)
    {
      tiempo_de_standby = mili_segundos;
      lcd.clear();
      Vaux1 = Posicion_actual / 2;
    }

    if (PressedButton(1)) Posicion_actual+=2; // suma 1 a Vaux1
    if (PressedButton(2)) Posicion_actual+=2; // resta 1 a Vaux1

    Posicion_actual=ReturnToCero(Posicion_actual,maxY_menu2 * 2);

    if (PressedButton(3))
    {
      Flag = Vaux1 + 2;
      lcd.clear();
    } 

    if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
    {
      tiempo_de_standby = mili_segundos;
      Vaux1 = Posicion_actual;
      lcd.clear();
      Estadoequipo = estado_standby;
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
    Posicion_actual=0;
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
    if (nivel_actual > min_nivel) Vaux1 = nivel_actual;
    if (nivel_actual <= min_nivel) Vaux1 = min_nivel;
    lcd.clear();
    Posicion_actual=0;
    Flag = 1;
    break;
  case 1:
    memcpy(imprimir_lcd, "Nivel a llenar:", 16);  PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d%c ", Vaux1, '%');    PrintLCD(imprimir_lcd, 16, 0);
    memcpy(imprimir_lcd, "Sumar 25 con 1", 15);   PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 25 con 2", 16);  PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 16);  PrintLCD(imprimir_lcd, 0, 3);

    if ((Posicion_actual + 1) * sumador_nivel != Vaux1)Vaux1 = (Posicion_actual + 1) * sumador_nivel;

    if (PressedButton(1)) Posicion_actual+=1;
    if (PressedButton(2)) Posicion_actual-=1;

    Posicion_actual=ReturnToCero(Posicion_actual, 4);
    
    if (PressedButton(3))
    {
      Flag = 2;
      lcd.clear();
    }
    if (PressedButton(4))
    {
      Estadoequipo = menu1;
      lcd.clear();
    }
    break;

  case 2:

    memcpy(imprimir_lcd, "Llenar hasta:", 14);  PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d%c", Vaux1, '%');  PrintLCD(imprimir_lcd, 14, 0);
    memcpy(imprimir_lcd, "Confirmar?", 11);     PrintLCD(imprimir_lcd, 5, 3);

    if (PressedButton(3))
    {
      lcd.clear();
      Flag = 3;
    }

    if (PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
    }
    break;

  case 3:
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
    if (min_temp > temperatura_actual)  Vaux2 = min_temp;
    else  Vaux2 = temperatura_actual;
    lcd.clear();
    Posicion_actual=0;
    Flag = 1;
    break;

  case 1:

    memcpy(imprimir_lcd, "Calentar a", 11);           PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 14);        PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 15);       PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 16);      PrintLCD(imprimir_lcd, 0, 3);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2,1), (char)223,CelciusOrFarenheit(Vaux2,2));
    PrintLCD(imprimir_lcd, 12, 0);

    if ((Posicion_actual + 8) * sumador_temperatura != Vaux2)Vaux2 = (Posicion_actual + 8) * sumador_temperatura;

    if (PressedButton(1))Posicion_actual+=1;
    if (PressedButton(2))Posicion_actual-=1;

    Posicion_actual=ReturnToCero(Posicion_actual,9);

    if (PressedButton(3))
    {
      Flag = 2;
      lcd.clear();
    }

    if (PressedButton(4))
    {
      Estadoequipo = menu1;
      Flag = 0;
      lcd.clear();
    }

    break;

  case 2:
    memcpy(imprimir_lcd, "Calentar a", 11);PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Confirmar?", 11);PrintLCD(imprimir_lcd, 5, 3);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2,1), (char)223,CelciusOrFarenheit(Vaux2,2));
    
    PrintLCD(imprimir_lcd, 12, 0);

    if (PressedButton(3))
    {
      Flag = 3;
      lcd.clear();
    }

    if (PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
    }
    break;

  case 3:
    guardado_para_menus(true);
    return Vaux2;
    break;
  }
}

void menu_de_auto_por_hora(uint8_t hora_actual, uint8_t minutos_actual)
{
  switch (Flag)
  /*  Vaux1=Nivel a setear
      Vaux2=temp a setear */
  {
  case 0:
    hora_to_modify = hora_actual;
    minuto_to_modify = minutos_actual;
    Posicion_actual=0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(imprimir_lcd, "Seleccionar: slot", 13);                                    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d", Vaux2 + 1);                                           PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "1:", 20);                                                   PrintLCD(imprimir_lcd, 0, 1);
    //Save a int convierte el char de guardado en 1 la hora y en 2 el minuto guardado en la eeprom, por ende esto imprime la hora y los minutos
    Printhora(imprimir_lcd, SaveToUINT(1, eep.read(1)), SaveToUINT(2, eep.read(2)));  PrintLCD(imprimir_lcd, 3, 1);
    memcpy(imprimir_lcd, "2:", 20);                                                   PrintLCD(imprimir_lcd, 0, 2);
    Printhora(imprimir_lcd, SaveToUINT(1, eep.read(4)), SaveToUINT(2, eep.read(4)));  PrintLCD(imprimir_lcd, 3, 2);
    memcpy(imprimir_lcd, "3:", 20);                                                   PrintLCD(imprimir_lcd, 0, 3);
    Printhora(imprimir_lcd, SaveToUINT(1, eep.read(8)), SaveToUINT(2, eep.read(8)));  PrintLCD(imprimir_lcd, 3, 3);

    Vaux2 = Posicion_actual / 4;

    if (PressedButton(1))Posicion_actual+=4;
    if (PressedButton(2))Posicion_actual-=4;

    if (PressedButton(3))
    {
      Vaux2 = min_temp;
      Vaux1 = min_nivel;
      Posicion_actual=0;
      lcd.clear();
      Flag = 2;
    }

    if (PressedButton(4))
    {
      Posicion_actual=0;
      Flag = 0;
      lcd.clear();
      Estadoequipo = menu1;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,12);
    break;

  case 2:
    memcpy(imprimir_lcd, "Temp. max:", 11);     PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);  PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20); PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);PrintLCD(imprimir_lcd, 0, 3);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2,1), (char)223,CelciusOrFarenheit(Vaux2,2));
    PrintLCD(imprimir_lcd, 12, 0);

    if ((Posicion_actual + 8) * sumador_temperatura != Vaux2)
    {
      Vaux2 = (Posicion_actual + 8) * sumador_temperatura;
    }

    if (PressedButton(1))Posicion_actual+=1;
    if (PressedButton(2))Posicion_actual-=1;

    if (PressedButton(3))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual=0;
    }

    if (PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual=0;
    }

    Posicion_actual=ReturnToCero(Posicion_actual,9);
    break;

  case 3:
    memcpy(imprimir_lcd, "Nivel max", 20);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d%c ", Vaux1, '%');
    PrintLCD(imprimir_lcd, 10, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);
    PrintLCD(imprimir_lcd, 0, 3);

    if ((Posicion_actual + 1) * sumador_nivel != Vaux1)
    {
      Vaux1 = (Posicion_actual + 1) * sumador_nivel;
    }

    if (PressedButton(1))Posicion_actual+=1;
    if (PressedButton(2))Posicion_actual-=1;

    if (PressedButton(3))
    {
      Flag = 4;
      Posicion_actual = 0;
      lcd.clear();
      Posicion_actual = 0;
    }
    if (PressedButton(4))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual=0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,4);
    break;

  case 4:
    memcpy(imprimir_lcd, "Setear hora:", 20);                   PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);  PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "aumentar con 1", 20);                 PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "disminuir con 2", 20);                PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);                 PrintLCD(imprimir_lcd, 0, 3);

    hora_to_modify = ReturnToCero((hora_actual + Posicion_actual), hora_max);

    if (PressedButton(1))Posicion_actual+=1;
    if (PressedButton(2))Posicion_actual-=1;
    if (PressedButton(3))
    {
      Flag = 5;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,hora_max);
    break;

  case 5:
    memcpy(imprimir_lcd, "Setear min:", 20);PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "aumentar con 1", 20);PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "disminuir con 2", 20);PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);PrintLCD(imprimir_lcd, 0, 3);

    minuto_to_modify = ReturnToCero((minutos_actual + Posicion_actual), minuto_max);

    if (PressedButton(1))Posicion_actual+=1;
    if (PressedButton(2))Posicion_actual-=1;
    if (PressedButton(3))
    {
      Flag = 6;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 4;
      lcd.clear();
      Posicion_actual = 0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,minuto_max);
    break;

  case 6:
    memcpy(imprimir_lcd, "A las:", 20);                           PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);    PrintLCD(imprimir_lcd, 7, 0);
    memcpy(imprimir_lcd, "Calentar:", 20);                        PrintLCD(imprimir_lcd, 0, 1);
    sprintf(imprimir_lcd, "Llenar: %d%c", Vaux1, '%');            PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar?", 20);                       PrintLCD(imprimir_lcd, 5, 3);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(Vaux2,1), (char)223,CelciusOrFarenheit(Vaux2,2));
    PrintLCD(imprimir_lcd, 11, 1);

    if (PressedButton(3))
    {
      lcd.clear();
      Flag = 7;
      Posicion_actual = 0;
      mili_segundos = tiempo_de_standby;
    }

    if (PressedButton(4))
    {
      Flag = 5;
      lcd.clear();
      Posicion_actual = 0;
    }
    break;

  case 7:
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);

    eep.write((Vaux2 * 3) + 1, ArrayToChar(imprimir_lcd));
    eep.write((Vaux2 * 3) + 2, Vaux1);
    eep.write((Vaux2 * 3) + 3, Vaux2);

    guardado_para_menus(true);
    break;
  }
}

void menu_de_llenado_auto()
{
  switch (Flag)
  {
  case 0:
    Vaux1 = eep.read(12);
    Vaux2 = eep.read(13);
    eep.write(12, min_nivel);
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(imprimir_lcd, "Nivel min:", 20);             PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d%c ", eep.read(12), '%');  PrintLCD(imprimir_lcd, 11, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);          PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20);         PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);        PrintLCD(imprimir_lcd, 0, 3);

    if ((Posicion_actual + 1) * sumador_nivel != eep.read(12))
    {
      eep.write(12, (Posicion_actual + 1) * sumador_nivel);
    }

    if (PressedButton(1) == true)Posicion_actual +=1;
    if (PressedButton(2) == true)Posicion_actual -=1;
    if (PressedButton(3) == true)
    {
      Flag = 2;
      eep.write(13, eep.read(12) + sumador_nivel);
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4) == true)
    {
      eep.write(12, Vaux1);
      eep.write(13, Vaux2);
      Estadoequipo = menu1;
      Flag = 0;
      funcionActual = posicion_inicial;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual=ReturnToCero(Posicion_actual,3); 
    break;

  case 2:
    memcpy(imprimir_lcd, "Nivel max:", 20);PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "%d%c", eep.read(13), '%');
    PrintLCD(imprimir_lcd, 11, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);
    PrintLCD(imprimir_lcd, 0, 3);

    if ((Posicion_actual + 1 + (eep.read(12) / sumador_nivel)) * sumador_nivel != eep.read(13))
    {
      eep.write(13, ((Posicion_actual + 1 + (eep.read(12) / sumador_nivel)) * sumador_nivel));
    }

    if (PressedButton(1) == true)Posicion_actual +=1;
    if (PressedButton(2) == true)Posicion_actual -=1;
    if (PressedButton(3))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }

    Posicion_actual=ReturnToCero(Posicion_actual,4 - (eep.read(12) / sumador_nivel)); 
    break;

  case 3:
    sprintf(imprimir_lcd, "Al llegar a %d%c", eep.read(12), '%');
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd, "LLenar hasta %d%c", eep.read(13), '%');
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Confirmar?", 20);
    PrintLCD(imprimir_lcd, 5, 3);

    if (PressedButton(3))
    {
      Flag = 4;
      lcd.clear();
      Posicion_actual = 0;
      mili_segundos = tiempo_de_standby;
    }

    if (PressedButton(4))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
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
    Vaux1 = eep.read(10);
    Vaux2 = eep.read(11);
    eep.write(10, min_temp);
    Posicion_actual = 0;
    lcd.clear();
    Flag = 1;
    break;

  case 1:
    memcpy(imprimir_lcd, "Temp. min:", 20);
    PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);
    PrintLCD(imprimir_lcd, 0, 3);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(10),1), (char)223,CelciusOrFarenheit(eep.read(10),2));
    PrintLCD(imprimir_lcd, 12, 0);

    if ((Posicion_actual + 8) * sumador_temperatura != eep.read(10))
    {
      eep.write(10, (Posicion_actual + 8) * sumador_temperatura);
    }

    if (PressedButton(1))Posicion_actual +=1;
    if (PressedButton(2))Posicion_actual -=1;
    if (PressedButton(3))
    {
      Flag = 2;
      eep.write(11, eep.read(10) + sumador_temperatura);
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      eep.write(10, Vaux1);
      eep.write(11, Vaux1);
      Estadoequipo = menu1;
      Flag = 0;
      funcionActual = posicion_inicial;
      lcd.clear();
      Posicion_actual = 0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,8); 
    break;

  case 2:
    memcpy(imprimir_lcd, "Temp. max:", 20);
    PrintLCD(imprimir_lcd, 0, 0);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(11),1), (char)223,CelciusOrFarenheit(eep.read(11),2));
    PrintLCD(imprimir_lcd, 12, 0);

    memcpy(imprimir_lcd, "Sumar 5 con 1", 20);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "Restar 5 con 2", 20);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 20);
    PrintLCD(imprimir_lcd, 0, 3);

    if ((Posicion_actual + 1 + (eep.read(10) / sumador_temperatura)) * sumador_temperatura != eep.read(11))
    {
      eep.write(11, (Posicion_actual + 1 + (eep.read(10) / sumador_temperatura)) * sumador_temperatura);
    }

    if (PressedButton(1))
      Posicion_actual +=1;
    if (PressedButton(2))
      Posicion_actual -=1;
    if (PressedButton(3))
    {
      Flag = 3;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,16 - (eep.read(10) / sumador_temperatura)); 

    break;

  case 3:
    memcpy(imprimir_lcd, "A los:", 20);
    PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Calentar a:", 20);
    PrintLCD(imprimir_lcd, 0, 1);

    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(10),1), (char)223,CelciusOrFarenheit(0,2));
    PrintLCD(imprimir_lcd, 7, 0);
    sprintf(imprimir_lcd, "%d%c%c ", CelciusOrFarenheit(eep.read(10),1), (char)223,CelciusOrFarenheit(eep.read(10),2));
    PrintLCD(imprimir_lcd, 12, 1);
  
    memcpy(imprimir_lcd, "Confirmar con 3", 20);
    PrintLCD(imprimir_lcd, 0, 3);

    if (PressedButton(3))
    {
      Flag = 4;
      lcd.clear();
      Posicion_actual = 0;
      mili_segundos = tiempo_de_standby;
    }

    if (PressedButton(4))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }
    break;

  case 4:
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
    memcpy(imprimir_lcd, "Setear hora:", 15);
    PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify,minutos);
    PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "aumentar con 1", 15);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "disminuir con 2", 16);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 16);
    PrintLCD(imprimir_lcd, 0, 3);

    hora_to_modify = ReturnToCero(hora + Posicion_actual, hora_max);

    if (PressedButton(1))
      Posicion_actual +=1;
    if (PressedButton(2))
      Posicion_actual -=1;
    if (PressedButton(3))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Estadoequipo = menu2;
      Flag = 0;
      funcionActual = posicion_inicial;
      lcd.clear();
      mili_segundos = tiempo_de_standby;
      Posicion_actual = 0;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,hora_max); 
    break;

  case 2:
    memcpy(imprimir_lcd, "Setear min:", 12);
    PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 12, 0);
    memcpy(imprimir_lcd, "aumentar con 1", 15);
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "disminuir con 2", 16);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "Confirmar con 3", 16);
    PrintLCD(imprimir_lcd, 0, 3);

    minuto_to_modify = ReturnToCero(minutos + Posicion_actual, minuto_max);

    if (PressedButton(1))
      Posicion_actual +=1;
    if (PressedButton(2))
      Posicion_actual -=1;
    if (PressedButton(3))
    {
      Flag = 1;
      lcd.clear();
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 3;
      lcd.clear();
      mili_segundos = tiempo_de_standby;
      Posicion_actual = 0;
    }

    Posicion_actual=ReturnToCero(Posicion_actual,minuto_max); 
    break;

  case 3:
    memcpy(imprimir_lcd, "hora Actual:", 13);
    PrintLCD(imprimir_lcd, 0, 0);
    Printhora(imprimir_lcd, hora_to_modify, minuto_to_modify);
    PrintLCD(imprimir_lcd, 13, 0);
    memcpy(imprimir_lcd, "Confirmar?", 11);
    PrintLCD(imprimir_lcd, 5, 3);

    if (PressedButton(3))
    {
      Flag = 4;
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), hora_to_modify, minuto_to_modify, now.second()));
      lcd.clear();
      mili_segundos = tiempo_de_standby;
      Posicion_actual = 0;
    }

    if (PressedButton(4))
    {
      Flag = 2;
      lcd.clear();
      Posicion_actual = 0;
    }
    break;

  case 4:
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
    PrintLCD(imprimir_lcd, 0, 0);
    if (eep.read(57)==254)
    {
      memcpy(imprimir_lcd, "Bomba Activada", 15);
      PrintLCD(imprimir_lcd, 0, 1);
    }
    if (eep.read(57)==1)
    {
      memcpy(imprimir_lcd, "Bomba Desactivada", 18);
      PrintLCD(imprimir_lcd, 0, 1);
    }
    memcpy(imprimir_lcd, "1:Si", 5);
    PrintLCD(imprimir_lcd, 3, 3);
    memcpy(imprimir_lcd, "2:No", 5);
    PrintLCD(imprimir_lcd, 11, 3);

    if (PressedButton(1))Posicion_actual=10;
    if (PressedButton(2))Posicion_actual=5;
    if (Posicion_actual > 8)eep.write(57,254);
    if (Posicion_actual <= 8)eep.write(57,1);
    if (PressedButton(3) || PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
      mili_segundos = tiempo_de_standby;
    }
    Posicion_actual=ReturnToCero(Posicion_actual,16); 
    break;
  case 1:
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
    PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "Unidad actual", 14);
    PrintLCD(imprimir_lcd, 0, 1);

    sprintf(imprimir_lcd, "%c%c", (char)223, CelciusOrFarenheit(0,2));
    PrintLCD(imprimir_lcd, 15, 1);

    sprintf(imprimir_lcd, "1:%cC", (char)223);
    PrintLCD(imprimir_lcd, 3, 3);
    sprintf(imprimir_lcd, "2:%cF", (char)223);
    PrintLCD(imprimir_lcd, 13, 3);

    if (Posicion_actual > 8)
      eep.write(56,254);
    if (Posicion_actual <= 8)
      eep.write(56,1);

    if (PressedButton(1))Posicion_actual+=8;
    if (PressedButton(2))Posicion_actual-=8;
    if (PressedButton(3) || PressedButton(4))
    {
      Flag = 1;
      lcd.clear();
      mili_segundos = tiempo_de_standby;
    }

    Posicion_actual=ReturnToCero(Posicion_actual,16); 
    break;

  case 1:
    guardado_para_menus(false);
    break;
  }
}

void menu_seteo_wifi()
{
  switch (Flag)
  {
  case 0:
    sprintf(imprimir_lcd,"%s", nombre_wifi_setear);   PrintLCD(imprimir_lcd, 6, 0);
    sprintf(imprimir_lcd,"%s", password_wifi_setear); PrintLCD(imprimir_lcd, 6, 1);
    sprintf(imprimir_lcd,"%d:%d:%d:%d   ", eep.read(58),eep.read(59),eep.read(60),eep.read(61)); PrintLCD(imprimir_lcd, 3, 2);
    memcpy(imprimir_lcd, "SSID:", 6);                 PrintLCD(imprimir_lcd, 0, 0);
    memcpy(imprimir_lcd, "PASS:", 6);                 PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "IP:", 11);                  PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "modificar?", 11);           PrintLCD(imprimir_lcd, 5, 3);
    if (PressedButton(3))
    {
      lcd.clear();
      Flag = 1;
      Posicion_actual = 0;
    }
    if (PressedButton(4))
    {
      Estadoequipo = menu2;
      Flag = 0;
      funcionActual = posicion_inicial;
      lcd.clear();
    }

    break;
  case 1:

    for (Vaux2 = 0; Vaux1 < 20; Vaux1++)
    {
      nombre_wifi_setear[Vaux1] = '\0';
      password_wifi_setear[Vaux1] = '\0';
    }
    Vaux2 = 0;
    Actualchar = 0;
    Flag = 2;
    Posicion_actual = 0;
    break;
  case 2:
    memcpy(imprimir_lcd, "Nombre Wifi:", 13);PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd,"%s", nombre_wifi_setear); PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "modificar?", 11);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "3:Si", 5);
    PrintLCD(imprimir_lcd, 3, 3);
    memcpy(imprimir_lcd, "4:No", 5);
    PrintLCD(imprimir_lcd, 11, 3);

    Actualchar = Posicion_actual / 2;
    Posicion_actual=ReturnToCero(Posicion_actual,80);
    Vaux2 = ReturnToCero(Vaux2, 20);

    nombre_wifi_setear[Vaux2] = Character_Return(Actualchar, mayusculas);

    if (PressedButton(1))
      mayusculas = !mayusculas;
    if (PressedButton(2) == true && Vaux2 <= 19)
    {
      Vaux2++;
      Actualchar = 0;
    }
    if (PressedButton(3))
    {
      nombre_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 3;
      Vaux2 = 0;
      Actualchar = 0;
    }
    if (PressedButton(4))
    {
      nombre_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 0;
      Vaux2 = 0;
      Actualchar = 0;
    }
    break;

  case 3:
    memcpy(imprimir_lcd, "Pass Wifi:", 11);
    PrintLCD(imprimir_lcd, 0, 0);
    sprintf(imprimir_lcd,"%s", password_wifi_setear); 
    PrintLCD(imprimir_lcd, 0, 1);
    memcpy(imprimir_lcd, "modificar?", 11);
    PrintLCD(imprimir_lcd, 0, 2);
    memcpy(imprimir_lcd, "3:Si", 5);
    PrintLCD(imprimir_lcd, 3, 3);
    memcpy(imprimir_lcd, "4:No", 5);
    PrintLCD(imprimir_lcd, 11, 3);

    Actualchar = Posicion_actual / 2;
    Posicion_actual=ReturnToCero(Posicion_actual,80);
    Vaux2 = ReturnToCero(Vaux2, 20);

    password_wifi_setear[Vaux2] = Character_Return(Actualchar, mayusculas);

    if (PressedButton(1))
      mayusculas = !mayusculas;
    if (PressedButton(2))
    {
      Vaux2++;
      Actualchar = 0;
    }
    if (PressedButton(3))
    {
      password_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 4;
    }
    if (PressedButton(4))
    {
      nombre_wifi_setear[Vaux2] = '\0';
      lcd.clear();
      Flag = 2;
      Vaux2 = 0;
      Actualchar = 0;
    }
    break;
  case 4:
    for (Vaux1 = 0; Vaux1 < 20; Vaux1++)
    {
      eep.writeChars(14,password_wifi_setear,20);
      eep.writeChars(34,nombre_wifi_setear,20);
    }
    Serial_Send_UNO(6, 0);
    guardado_para_menus(false);
    break;
  }
}

void guardado_para_menus(bool Menu)
{

  while (mili_segundos <= tiempo_de_standby + tiempo_de_espera_menu)
  {
    memcpy(imprimir_lcd, "Guardando...", 13);
    PrintLCD(imprimir_lcd, 4, 0);
  }

  if (mili_segundos >= tiempo_de_standby + tiempo_de_espera_menu)
  {

    if (Menu == true)
    {
      Estadoequipo = menu1;
      Flag = 0;
    }

    if (Menu == false)
    {
      Estadoequipo = menu2;
      Flag = 0;
    }

    funcionActual = posicion_inicial;
    lcd.clear();
    tiempo_de_standby = mili_segundos;
  }
}

//██████████████████████████████████████████FUNCIONES DE SOPORTE██████████████████████████████████████████

int16_t CelciusOrFarenheit(int8_t value, uint8_t function){
  if(function==1){
    if (eep.read(56)==254)return ((9 * value) / 5) + 32;
    if (eep.read(56)==1)return value;
  }
  if(function==2){
    if (eep.read(56)==254)return'F';
    if (eep.read(56)==1)return 'C';
  }
}

uint8_t ReturnToCero(int8_t actualpos, uint8_t maxpos)
{
  if (actualpos >= maxpos)
  {
    return 0 + actualpos % maxpos;
  }
  if (actualpos < 0)
  {
    return maxpos + actualpos;
  }
  if (actualpos >= 0 && actualpos < maxpos)
    return actualpos;
  return (0);
}

uint8_t SaveToUINT(uint8_t function, uint8_t save)
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

int8_t ArrayToChar(char buffer[20]) //// ya arregle lo de colver
{
  uint8_t hora;
  uint8_t minuto;
  uint8_t resto;
  uint8_t NumeroFinal; // solo una variable (_convert  nos evita modificar variables globales como bldos)
    hora = (buffer[0] - '0') * 10; // toma el valor del primer digito del string y lo convierte en int (numero de base 10)
    hora += (buffer[1] - '0');     // toma el valor del segundo digito
    hora = hora * 4;        // multiplica la hora x 4 (la proporcionalidad esta en la documentacion)

    minuto = (buffer[3] - '0') * 10; // lo mismo que en el var 1 pero con minutos (10:[puntero aca]0)
    minuto += (buffer[4] - '0');     // lo mismo que en el var 1 pero con minutos
    resto = minuto % 15;             // saca el Vaux4 (ejemplo 7/5 Vaux4 2)
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
  buffer[0] = slot + '0';
  hora_entrada = hora_entrada - (slot * 10);
  buffer[1] = hora_entrada + '0';
  buffer[2] = ':';
  slot = minuto_entrada / 10;
  buffer[3] = slot + '0';
  minuto_entrada = minuto_entrada - (slot * 10);
  buffer[4] = minuto_entrada + '0';
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
  case 1:
    if ((PIND & (1 << PD4)) == 0)
    {
      while ((PIND & (1 << PD4)) == 0){}
      return true;
    }
    else return false;
    break;

  case 2:
    if ((PIND & (1 << PD5)) == 0)
    {
      while ((PIND & (1 << PD5)) == 0){}
      return true;
    }
    else return false;
    break;
  
  case 3:
    if ((PIND & (1 << PD6)) == 0)
    {
      while ((PIND & (1 << PD6)) == 0){}
      return true;
    }
    else return false;
    break;
  case 4:
    if ((PIND & (1 << PD7)) == 0)
    {
      while ((PIND & (1 << PD7)) == 0){}
      return true;
    }
    else return false;
    break;
  case 40:
    if ((PIND & (1 << PD2)) == 0)return true;
    else return false;
    break;
  case 41:
    if ((PIND & (1 << PD3)) == 0)return true;
    else return false;
    break;
  default:
    return false;
    break;
  }
}

void PrintOutput(uint8_t Wich_pin, bool state)
{
  switch (Wich_pin)
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
      ActualIndividualDataPos=0;
    }
    if (i == 1)Serial.read();
    if (i >= 2 && i<seriallength){                                         // si no es nungun caracter especial:
      Individualdata[ActualIndividualDataPos] = Serial.read(); // copia al individual
      ActualIndividualDataPos++;   
    }
    if(i==seriallength)Take_Comunication_Data = true;
  }

  if (Take_Comunication_Data == true)
  {
    switch (Actualchar)
    { // dependiendo del char de comando
    case 'W':
      temperatura_a_calentar = Individualdata[0];
      if (Individualdata[1] == 'O')
        calentar = true;
      if (Individualdata[1] == 'I')
        calentar = false;
      Take_Comunication_Data = false;
      break;

    case 'F':
      nivel_a_llenar = Individualdata[0];
      if (Individualdata[1] == 'O')
        llenar = true;
      if (Individualdata[1] == 'I')
        llenar = false;
      Take_Comunication_Data = false;
      break;

    case 'S':
    // vaux2 guarda el slot (1,2 o 3) de la eeprom
       saveslot= Individualdata[0];
      if (saveslot <= 2 && saveslot >= 0)
      {
        eep.write((saveslot * 3) + 1, Individualdata[1]);
        eep.write((saveslot * 3) + 2, Individualdata[2]);
        eep.write((saveslot * 3) + 3, Individualdata[3]);
        Take_Comunication_Data = false;
      }
      break;

    case 'H':
      eep.write(10, Individualdata[0]);
      eep.write(11, Individualdata[1]);
      Take_Comunication_Data = false;
      break;
    case 'C':
      eep.write(12, Individualdata[0]);
      eep.write(13, Individualdata[1]);
      Take_Comunication_Data = false;
      break;
    case 'I':
      eep.write(58,Individualdata[0]);
      eep.write(59,Individualdata[1]);
      eep.write(60,Individualdata[2]);
      eep.write(61,Individualdata[3]);
      esp_working=true;
      if(Estadoequipo==funciones && funcionActual==funcion_de_menu_seteo_wifi)lcd.clear();
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
  char calentando,llenando;
  if (calentar)calentando='I';
  else calentando='O';
  if (llenar)llenando='I';
  else calentando='O';
  
    switch (WhatSend)
    {
    case 1:
      sprintf(OutputMessage, "U_%c%c%c%c", nivel_actual, nivel_actual,calentando,llenando);       
      break;
    case 2:
      sprintf(OutputMessage, "K_%c%c%c%c", eep.read((What_slot * 3) + 1), eep.read((What_slot * 3) + 2), eep.read((What_slot * 3) + 3), What_slot);
      break;
    case 3:
      sprintf(OutputMessage, "N_%s",nombre_wifi_setear);
      break;
    case 4:
      sprintf(OutputMessage, "P_%s",password_wifi_setear);
      break;
    case 5:
      sprintf(OutputMessage, "H_%c%c",eep.read(10),eep.read(11));
      break;
    case 6:
      sprintf(OutputMessage, "C_%c%c",eep.read(12),eep.read(13));
    default:
      break;
    }
    Serial.println(OutputMessage);
}
