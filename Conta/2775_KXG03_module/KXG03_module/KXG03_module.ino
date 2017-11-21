#include <Wire.h>
#include <KXG03.h>

KXG03 kxg03;

void setup() {
  byte rc;

  Serial.begin(115200);
  while (!Serial);
  
  Wire.begin();
  
  rc = kxg03.init(KXG03_DEVICE_ADDRESS_4E);
}

void loop() {
  byte rc;
  float val[6];

  rc = kxg03.get_val(val);
  if (rc == 0) {
    Serial.print("X-Gyro: ");
    Serial.println(val[0], 2);
    Serial.print("Y-Gyro: ");
    Serial.println(val[1], 2);
    Serial.print("Z-Gyro: ");
    Serial.println(val[2], 2);
    Serial.print("X-Accelerator: ");
    Serial.println(val[3], 2);
    Serial.print("Y-Accelerator: ");
    Serial.println(val[4], 2);
    Serial.print("Z-Accelerator: ");
    Serial.println(val[5], 2);
    Serial.println(" ");
  }
  delay(100);
}

