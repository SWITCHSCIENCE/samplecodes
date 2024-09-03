#include <SPI.h>

const int MA735_CS = SS;
SPISettings settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin();  // MISO 16 MOSI 19 SCK 18
  pinMode(MA735_CS, OUTPUT);
  digitalWrite(MA735_CS, HIGH);

  delay(5000);
  printHelp();
}

uint32_t angle_interval = 0;
uint32_t angle_last = 0;

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // シリアルから1行読み込む
    input.trim();                                 // 前後の空白を削除

    // 入力をスペースで分割
    int spaceIndex1 = input.indexOf(' ');
    int spaceIndex2 = input.indexOf(' ', spaceIndex1 + 1);

    String command = input.substring(0, spaceIndex1);
    String arg1 = (spaceIndex1 == -1) ? "" : input.substring(spaceIndex1 + 1, spaceIndex2);
    String arg2 = (spaceIndex2 == -1) ? "" : input.substring(spaceIndex2 + 1);

    // コマンドに応じた処理を実行
    executeCommand(command, arg1, arg2);
  }
  if (angle_interval) {
    uint32_t curr = millis();
    uint32_t diff = curr - angle_last;
    if (diff >= angle_interval) {
      Serial.println(readAngle());
      angle_last = curr;
    }
  }
}

void printHelp() {
  Serial.println("MA735 Register Settings available commands:");
  Serial.println("d");
  Serial.println("  print all registers");
  Serial.println("m interval");
  Serial.println("  Outputs the angle at interval.");
  Serial.println("r reg");
  Serial.println("  Reads the register at address 'reg'.");
  Serial.println("w reg val");
  Serial.println("  Writes 'val' to the register at address 'reg'.");
}

int parseArgument(String arg) {
  if (arg.startsWith("0x")) {
    return (int)strtol(arg.c_str(), NULL, 16);  // 16進数として解析
  } else {
    return arg.toInt();  // 10進数として解析
  }
}

void executeCommand(String command, String arg1, String arg2) {
  if (command == "d") {
    Serial.print("0x00 Z[7:0]\n");
    Serial.println(readReg(0x0), HEX);
    Serial.print("0x01 Z[15:8]\n");
    Serial.println(readReg(0x1), HEX);
    Serial.print("0x02 BCT[7:0]\n");
    Serial.println(readReg(0x2), HEX);
    Serial.print("0x03 ETY:1 ETX:0\n");
    Serial.println(readReg(0x3), HEX);
    Serial.print("0x04 PPT[1:0]:6 ILIP[3:0]:2\n");
    Serial.println(readReg(0x4), HEX);
    Serial.print("0x05 PPT[9:2]\n");
    Serial.println(readReg(0x5), HEX);
    Serial.print("0x06 MGLT[2:0]:5 MGHT[2:0]:2\n");
    Serial.println(readReg(0x6), HEX);
    Serial.print("0x09 RD:7\n");
    Serial.println(readReg(0x9), HEX);
    Serial.print("0x0E FW[7:0]\n");
    Serial.println(readReg(0xE), HEX);
    Serial.print("0x10 HYS[7:0]\n");
    Serial.println(readReg(0x10), HEX);
    Serial.print("0x1B MGH:7 MGL:6\n");
    Serial.println(readReg(0x1B), HEX);
  } else if (command == "m") {
    angle_interval = parseArgument(arg1);
  } else if (command == "w") {
    uint8_t reg = parseArgument(arg1);
    uint8_t val = parseArgument(arg2);
    Serial.println(writeReg(reg, val), HEX);
  } else if (command == "r") {
    uint8_t reg = parseArgument(arg1);
    Serial.println(readReg(reg), HEX);
  } else {
    printHelp();
  }
}

uint8_t readReg(uint8_t reg) {
  SPI.beginTransaction(settings);
  digitalWrite(MA735_CS, LOW);
  SPI.transfer16(0x4000 | (reg & 0x1f) << 8);
  digitalWrite(MA735_CS, HIGH);

  delayMicroseconds(100);

  digitalWrite(MA735_CS, LOW);
  uint16_t rd = SPI.transfer16(0);
  digitalWrite(MA735_CS, HIGH);
  SPI.endTransaction();
  return rd >> 8;
}

uint8_t writeReg(uint8_t reg, uint8_t val) {
  SPI.beginTransaction(settings);
  digitalWrite(MA735_CS, LOW);
  SPI.transfer16(0x8000 | (reg & 0x1f) << 8 | val);
  digitalWrite(MA735_CS, HIGH);

  delay(20);

  digitalWrite(MA735_CS, LOW);
  uint16_t rd = SPI.transfer16(0);
  digitalWrite(MA735_CS, HIGH);
  SPI.endTransaction();
  return rd >> 8;
}

uint16_t readAngle() {
  SPI.beginTransaction(settings);
  digitalWrite(MA735_CS, LOW);
  uint16_t rd = SPI.transfer16(0);  // 角度の読み込み
  digitalWrite(MA735_CS, HIGH);
  SPI.endTransaction();
  return rd;
}
