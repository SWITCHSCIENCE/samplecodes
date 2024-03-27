#include "espr32s3conta.h"
#include <Wire.h>

//------------------------------------------
// Main logics
//------------------------------------------

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  Serial.println("I2C Scanner");
  delay(1000);
}

void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.println("");
      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found\n");
  }else{
    Serial.println("done\n");
  }
  delay(5000);           // wait 5 seconds for next scan
}
