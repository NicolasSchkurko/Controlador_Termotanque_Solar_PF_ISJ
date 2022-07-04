#include <OneWire.h>

int oneWirePin = 7;

OneWire ds(oneWirePin); 

int temp_0 [2]; //arrays to store temeratures

void read_temp(byte addr[8], int number); //reads temerature on given address and saves it to the array (temp_0 or temp_1)
void send_for_temp(byte addr[8]); //asks for temerature on given address

long previousMillis = 0;        // will store last time DS was updated
long interval = 1000;           // interval at which to read temp (milliseconds)

void setup() {
  Serial.begin (9600); //for debug

ds.search(ds_addr0); //find addresses of 2 devices we have

send_for_temp(ds_addr0); //ask firs and second for temperature
/* this is more elegant: have to try it
  ds.reset();                  
  ds.skip();                       // tell all sensors on bus
  ds.write(0x44,0);            // to convert temperature

*/
}

void loop() {
//do your cool stuff here


   if (millis() - previousMillis > interval) {
     previousMillis = millis();   
//reading data from old requests:
read_temp(ds_addr0, 0);

//sending new requests:
send_for_temp(ds_addr0);
   }
//print data from arrays - if it's not updated - will print old   
Serial.print(temp_0[0]); 
Serial.print("."); 
Serial.print(temp_0[1]); 
Serial.print(" ");
}

void send_for_temp(byte addr[8]){
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
}

void read_temp(byte addr[8], int number){
  byte i;
  byte data[12];
  
  ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for (i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  
int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;

  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = 50 * TReading; // multiply by 100 * 0.5  
  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;

  //code we acrually need:
  
  temp_0[0] = Whole;
  temp_0[1] = Fract;
  }
}