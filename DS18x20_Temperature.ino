#include <OneWire.h>
#include <Time.h>

OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)
byte sensor1[] = {40, 255, 78, 129, 161, 21, 4, 122};
byte sensor2[] = {40, 255, 110, 94, 161, 21, 3, 154};

void setup(void) {
  Serial.begin(9600);
}

void loop(void) {
  digitalClockDisplay();
  
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if ( !ds.search(addr)) {
    //    Serial.println("No more addresses.");
    //    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

//  if (byte_array_eq(sensor1, addr, 8)) {
//    Serial.print("Sensor1: ");
//  } else 
  if (byte_array_eq(sensor2, addr, 8)) {
    Serial.print("Sensor2: ");
  } else {
    Serial.print("Unknown: ");  
  }
  
//  Serial.print("(");
//  for ( i = 0; i < 8; i++) {
//    Serial.write(' ');
//    Serial.print(addr[i]);
//  }
//  Serial.print(") ");
//
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  //  Serial.println();

  // the first ROM byte indicates which chip
  //  switch (addr[0]) {
  //    case 0x10:
  //      Serial.println("  Chip = DS18S20");  // or old DS1820
  //      type_s = 1;
  //      break;
  //    case 0x28:
  //      Serial.println("  Chip = DS18B20");
  //      type_s = 0;
  //      break;
  //    case 0x22:
  //      Serial.println("  Chip = DS1822");
  //      type_s = 0;
  //      break;
  //    default:
  //      Serial.println("Device is not a DS18x20 family device.");
  //      return;
  //  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  //  Serial.print("  Data = ");
  //  Serial.print(present, HEX);
  //  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //    Serial.print(data[i], HEX);
    //    Serial.print(" ");
  }
  //  Serial.print(" CRC=");
  //  Serial.print(OneWire::crc8(data, 8), HEX);
  //  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  //  Serial.print("  Temperature = ");
  //  Serial.print(celsius);
  //  Serial.print(" Celsius, ");
  Serial.println(fahrenheit);
  //  Serial.print(" F");
  //  Serial.println("");
  Serial.flush();
}

bool byte_array_eq(byte const *x, byte const *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
