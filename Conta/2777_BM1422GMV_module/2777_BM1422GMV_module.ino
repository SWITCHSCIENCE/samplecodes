#include <Wire.h>

uint8_t addr = 0x0e;
uint8_t data;

int16_t X;
int16_t Y;
int16_t Z;

void write_data(uint8_t addr, uint8_t reg, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t read_byte_data(uint8_t addr, uint8_t reg) {
  uint8_t data;
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr, 1);
  if (Wire.available() >= 1) {
    data = Wire.read();
  }
  return data;
}

int16_t read_word_data(uint8_t addr, uint8_t reg) {
  uint8_t data1,data2;
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr, 2);
  if (Wire.available() >= 2) {
    data1 = Wire.read();
    data2 = Wire.read();
  }
  return (int16_t)((data2 << 8) + data1);
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin(21, 22); //SDA,SCL  for ESP32
  Serial.begin(115200);
  pinMode(19, INPUT);

  Serial.println("Start.");

  write_data(addr, 0x1b, 0x80); // CNTL1
  delay(10);
  write_data(addr, 0x5c, 0x00); // CNTL4
  write_data(addr, 0x5d, 0x00);
  delay(10);
  write_data(addr, 0x1c, 0x08); // CNTL2
  delay(10);
  write_data(addr, 0x6c, 0x30); // OFF_X
  write_data(addr, 0x6d, 0x00);
  write_data(addr, 0x72, 0x30); // OFF_Y
  write_data(addr, 0x73, 0x00);
  write_data(addr, 0x78, 0x30); // OFF_Z
  write_data(addr, 0x79, 0x00);
  delay(10);
  write_data(addr, 0x1d, 0x40); // CNTL3

}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println(read_data(addr, 0x18));
  
  if (digitalRead(19) == LOW) {
    X = read_word_data(addr, 0x10);
    delay(1);
    Y = read_word_data(addr, 0x12);
    delay(1);
    Z = read_word_data(addr, 0x14);

    Serial.print("X=");
    Serial.print(X);
    Serial.print(" Y=");
    Serial.print(Y);
    Serial.print(" Z=");
    Serial.println(Z);

  }
  delay(10);
}
