/*****************************************************************************
  RPR-0521RS.ino

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
#include <RPR-0521RS.h>

RPR0521RS rpr0521rs;

void setup() {
  byte rc;

  Serial.begin(115200);		/*  Modify : set the serial speed to 11500 kbps  */
  while (!Serial);
  
  Wire.begin();
  
  rc = rpr0521rs.init();
}

void loop() {
  byte rc;
  unsigned short ps_val;
  float als_val;
  byte near_far;
  
  rc = rpr0521rs.get_psalsval(&ps_val, &als_val);
  if (rc == 0) {
    Serial.print(F("RPR-0521RS (Proximity)     = "));
    Serial.print(ps_val);
    Serial.print(F(" [count]"));
    near_far = rpr0521rs.check_near_far(ps_val);
    if (near_far == RPR0521RS_NEAR_VAL) {
      Serial.println(F(" Near"));
    } else {
      Serial.println(F(" Far"));
    }
    
    if (als_val != RPR0521RS_ERROR) {
      Serial.print(F("RPR-0521RS (Ambient Light) = "));
      Serial.print(als_val);
      Serial.println(F(" [lx]"));
      Serial.println();
    }
  }
 
  delay(500);

}

