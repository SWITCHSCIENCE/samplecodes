/*****************************************************************************
  BM1383AGLV.ino

 Copyright (c) 2017 ROHM Co.,Ltd.

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
#include <BM1383AGLV.h>

BM1383AGLV bm1383aglv;

void setup() {
  byte rc;

  Serial.begin(115200);		/*  Modify : set the serial speed to 11500 kbps  */
  while (!Serial);

  Wire.begin();

  rc = bm1383aglv.init();

  if (rc != 0) {
    Serial.println(F("BM1383AGLV initialization failed"));
    Serial.flush();
  }

}

void loop() {
  byte rc;
  float press = 0, temp = 0;

  rc = bm1383aglv.get_val(&press, &temp);
  if (rc == 0) {
    Serial.write("BM1383AGLV (PRESS) = ");
    Serial.print(press);
    Serial.println(" [hPa]");
    Serial.write("BM1383AGLV (TEMP) =  ");
    Serial.print(temp);
    Serial.println(" [degrees Celsius]");
    Serial.println();
  }

  delay(500);

}
