#include <OneWire.h>
#include <DallasTemperature.h>
#define TEMP_RESOLUTION 9
OneWire  ds(9);  // on pin 9 (a 4.7K resistor is necessary)

unsigned long start;
int end = 800;

byte addr[8],    
     data[12],   //porque son 7 para sacar el dato de la temperatura y los otros 2 son para sacar el signo y la resolucion
     i;

int antiDither(int newVal);

void setup()
{
  Serial.begin(9600);
  Serial.println("\n"__FILE__"\n");
  ds.search(addr);
}
void loop()
{
  if (millis() - start > end)
  {
    start += end;
    int val, newVal;
    ds.reset();
    ds.select(addr);   //selecciona la cuireccion de la entrada para poder comunicarse
    ds.write(0xBE);         // Read Scratchpad
    for ( i = 0; i < 9; i++)   // we need 9 bytes, read conversion results here,
      data[i] = ds.read();     // 800 ms after conversion start
    newVal = data[1] << 8 | data[0];   
    val = antiDither(newVal);   
    Serial.println();
    Serial.print(val);
    Serial.print(" = ");
    Serial.print(val >> 4);
    Serial.print(".");
    Serial.println(val & 0x000F, DEC);
    ds.reset();
    ds.select(addr);
    ds.write(0x44); // start conversion
  }
}

int antiDither(int newVal) // prevents +1 <-> -1 dither
{
  static int val = 0, oldVal = 0;
  if (newVal != val && newVal != oldVal)
  {
    oldVal = val;
    val = newVal;
  }
  return val;
}