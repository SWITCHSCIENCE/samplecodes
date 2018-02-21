/*****************************************************************************
  BD1020.ino

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
#include <BD1020.h>

int tempout_pin = A0;		/*  Modify : Temperature value is taken from pin A0  */

BD1020 bd1020;

void setup() {

  Serial.begin(115200);		/*  Modify : set the serial speed to 11500 kbps  */
  while (!Serial);

  bd1020.init(tempout_pin);

  Serial.println("BD1020HFV Sample");

}

void loop() {
  float temp;

  bd1020.get_val(&temp);
  Serial.print("BD1020HFV Temp=");
  Serial.print(temp);
  Serial.print("  [degrees Celsius], ADC=");
  Serial.println(bd1020.temp_adc);
  
  delay(500);
}

