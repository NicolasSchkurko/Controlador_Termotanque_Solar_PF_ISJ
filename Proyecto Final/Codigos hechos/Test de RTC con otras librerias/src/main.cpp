#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

bool getTime(const char *str);
bool getDate(const char *str);

tmElements_t tm;

void setup() {
  bool parse=false;
  bool config=false;

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }

  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
  if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }
  /*SREG = (SREG & 0b01111111);
  TIMSK2 = TIMSK2|0b00000001;
  TCCR2B = 0b00000011;
  SREG = (SREG & 0b01111110) | 0b10000000;*/
}

void loop() {
  getTime(__TIME__);
  Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}




/*#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h> // a basic DS1307 library that returns time as a time_t
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR 0x27

LiquidCrystal_I2C lcd(0x20,20,4);

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  setSyncProvider(RTC.get); // Vamos a usar el RTC
  setTime(10, 40, 00, 19, 9, 2016); // Las 10:40:00 del dia 19 de Sep de 2016
  if (timeStatus() != timeSet)
  Serial.println("Unable to sync with the RTC");
  else
  Serial.println("RTC has set the system time");
}

void printDigits(int digits)
{
  lcd.print(":");
  if (digits < 10)
  lcd.print('0');
  lcd.print(digits);
}

void digitalClockDisplay()
{
  lcd.home (); // go home
  lcd.print(hour());
  printDigits(minute());
  printDigits(second());
  lcd.setCursor ( 0, 1 ); // go to the 2nd line
  lcd.print(day());
  lcd.print("/");
  lcd.print(month());
  lcd.print("/");
  lcd.print(year());
}

void loop()
{
  digitalClockDisplay();
  delay(1000);
}
*/