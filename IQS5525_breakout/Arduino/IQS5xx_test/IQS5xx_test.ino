#include <Wire.h>

//! SystemControl0 bit definitions
//
#define ACK_RESET           0x80

/******************** DEVICE INFO REGISTERS ***************************/
#define ProductNumber_adr   0x0000  //(READ)      //2 BYTES;
#define GestureEvents0_adr  0x000D  //(READ)
#define SystemControl0_adr  0x0431  //(READ/WRITE)
#define SystemControl1_adr  0x0432  //(READ/WRITE)
#define SystemConfig0_adr   0x058E  //(READ/WRITE/E2)
#define TotalRx_adr         0x063D  //(READ/WRITE/E2)
#define RxMapping_adr       0x063F  //(READ/WRITE/E2) //10 BYTES;
#define TxMapping_adr       0x0649  //(READ/WRITE/E2) //15 BYTES;

#define END_WINDOW          (uint16_t)0xEEEE

#define SWIPE_Y_NEG         0x20
#define SWIPE_Y_POS         0x10
#define SWIPE_X_POS         0x08
#define SWIPE_X_NEG         0x04
#define TAP_AND_HOLD        0x02
#define SINGLE_TAP          0x01

uint8_t IQS5xx_addr = 0x74;
uint8_t Data_Buff[44];
uint8_t wdata[32] = {0};
uint8_t rdata[32] = {0};
char buf[32];

void RDY_wait() {
  while (digitalRead(2) == 0)
  {
    delay(1);
  }
}

void I2C_Write(int IQS_addr, uint8_t *data , int num) {
  Wire.beginTransmission(IQS5xx_addr);
  Wire.write(IQS_addr >> 8);
  Wire.write(IQS_addr & 0x00ff);
  for (int i = 0; i < num; i++) {
    Wire.write(*data);
    data++;
  }
  Wire.endTransmission();
}

int I2C_Read(int IQS_addr, uint8_t *data, int num) {
  int i = 0;
  Wire.beginTransmission(IQS5xx_addr);
  Wire.write(IQS_addr >> 8);
  Wire.write(IQS_addr & 0x00ff);
  Wire.endTransmission();
  int result = Wire.requestFrom(IQS5xx_addr, num);
  while (Wire.available())  {  // 要求より短いデータが来る可能性あり
    data[i] = Wire.read();      // 1バイトを受信
    i += 1;
  }
  return result;
}

// End Communication Window
void Close_Comms() {
  wdata[0] = 0;
  I2C_Write(END_WINDOW, wdata, 1);
}

// IQS5xx_AcknowledgeReset
void IQS5xx_AcknowledgeReset() {
  RDY_wait();
  wdata[0] = 0x80;
  I2C_Write(SystemControl0_adr, wdata, 1);
}

// =============================================================

void setup() {
//  Wire.begin();
  Wire.begin(4,5); // ESP02 SDA,SCL
//  Wire.begin(25,26); // ESP32
  Serial.begin(115200);
  pinMode(2, INPUT);  // Ready Pin

  // IQS5xx_AcknowledgeReset
  IQS5xx_AcknowledgeReset();

  // IQS5xx_CheckVersion
  I2C_Read(ProductNumber_adr, rdata, 6);
  sprintf(buf, "Product number : %02x%02x\nProject number : %02x%02x\nversion : %d.%d\n", rdata[0], rdata[1], rdata[2], rdata[3], rdata[4], rdata[5]);
  Serial.println(buf);

  // End Communication Window
  Close_Comms();


  // Set Total Rx/Tx
  RDY_wait();

  wdata[0] = 5;
  wdata[1] = 5;
  I2C_Write(TotalRx_adr, wdata, 2); // 0x063D

  // RxMapping_adr
  RDY_wait();

  wdata[0] = 0;
  wdata[1] = 1;
  wdata[2] = 2;
  wdata[3] = 3;
  wdata[4] = 4;
  I2C_Write(RxMapping_adr, wdata, 5); // 0x063F

  // TxMapping_adr
  RDY_wait();

  wdata[0] = 0;
  wdata[1] = 1;
  wdata[2] = 2;
  wdata[3] = 3;
  wdata[4] = 4;
  I2C_Write(TxMapping_adr, wdata, 5); // 0x0649
  Close_Comms();

  // SystemConfig0_adr
  RDY_wait();

  wdata[0] = 0x4A;
  I2C_Write(SystemConfig0_adr, wdata, 1); // 0x058E
  Close_Comms();

  IQS5xx_AcknowledgeReset();

}

void loop() {
  RDY_wait();
  I2C_Read(GestureEvents0_adr, Data_Buff, 44);

  Close_Comms();

  //    i = 0;
  //    while (i < 44) {
  //      Serial.print(Data_Buff[i], HEX);
  //      Serial.print(" ");
  //      i += 1;
  //    }
  //    Serial.println();
  //  switch (Data_Buff[0])
  //  {
  //    case SINGLE_TAP   :   Serial.print("Single Tap  ");
  //      break;
  //    case TAP_AND_HOLD :   Serial.print("Tap & Hold  ");
  //      break;
  //    case SWIPE_X_NEG  :   Serial.print("Swipe X-    ");
  //      break;
  //    case SWIPE_X_POS  :   Serial.print("Swipe X+    ");
  //      break;
  //    case SWIPE_Y_POS  :   Serial.print("Swipe Y+    ");
  //      break;
  //    case SWIPE_Y_NEG  :   Serial.print("Swipe Y-    ");
  //      break;
  //  }
  //  if (Data_Buff[0] > 0) {
  //    Serial.print(" ");
  //    Serial.print("Fingers : ");
  //    Serial.println(Data_Buff[4]);
  //    Serial.println();
  //  }
  int i16Rel1X = ((Data_Buff[5] << 8) | (Data_Buff[6]));
  int i16Rel1Y = ((Data_Buff[7] << 8) | (Data_Buff[8]));
  int i16Abs1X = ((Data_Buff[9] << 8) | (Data_Buff[10]));
  int i16Abs1Y = ((Data_Buff[11] << 8) | (Data_Buff[12]));
  int i16Tstr1 = ((Data_Buff[13] << 8) | (Data_Buff[14]));
  int i16Tarea1 = (Data_Buff[15]);

  if (i16Abs1X != -1 && i16Abs1Y != -1) {
    sprintf(buf, "%5d,%5d,%5d,%5d : %3d,%3d : %2x,%2x", i16Rel1X, i16Rel1Y, i16Abs1X, i16Abs1Y, i16Tstr1, i16Tarea1, Data_Buff[0], Data_Buff[1]);
    Serial.print(buf);
    Serial.println();
  }
}