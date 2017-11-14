/*****************************************************************************
  BH1745NUC.ino

 Copyright (c) 2016 ROHM Co.,Ltd.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
******************************************************************************/
#include <Wire.h>
#include <BH1745NUC.h>

#define BH1745NUC_DEVICE_ADDRESS_38            (0x38)    // 7bit Addrss
#define BH1745NUC_DEVICE_ADDRESS_39            (0x39)    // 7bit Addrss

BH1745NUC bh1745nuc(BH1745NUC_DEVICE_ADDRESS_39);

void setup() {
  byte rc;

  Serial.begin(115200);
  while (!Serial);
  
  Wire.begin();
  
  rc = bh1745nuc.init();
}

void loop() {
  byte rc;
  unsigned short rgbc[4];
  
  rc = bh1745nuc.get_val(rgbc);
  if (rc == 0) {
    Serial.write("BH1745NUC (RED)   = ");
    Serial.println(rgbc[0]);
    Serial.write("BH1745NUC (GREEN) = ");
    Serial.println(rgbc[1]);
    Serial.write("BH1745NUC (BLUE)  = ");
    Serial.println(rgbc[2]);
    Serial.write("BH1745NUC (CLEAR) = ");
    Serial.println(rgbc[3]);    
    Serial.println();
  }
 
  delay(500);

}

