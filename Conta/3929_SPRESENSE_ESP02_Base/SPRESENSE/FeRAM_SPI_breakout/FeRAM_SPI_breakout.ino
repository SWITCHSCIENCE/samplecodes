/*************************************************************************************
**                                                                                  **
**  FeRAM SPI Test                                                                  **
**  Created : 2019/07/9 12:30:00                                                    **
**  Author  : O.aoki                                                                **
**                                                                                  **
**  [ SPRESENSE + ベースシールド + 2x2タクトスイッチモジュール ]                    **
**  [ SPRESENSE : https://www.switch-science.com/catalog/3900/ ]                    **
**  [ ベースシールド : https://www.switch-science.com/catalog/3929/ ]               **
**  [ 1Mbit Fe-RAMモジュール : https://www.switch-science.com/catalog/3682/ ]       **
**  [ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]  **
**                                                                                  **
**************************************************************************************/
#include <SPI.h>
uint32_t    cnt;
uint32_t    addr;
uint32_t    indx;
byte        sw;
byte        dsw;
byte        buf[16];

/*------------------------------------------------------------------------------------
    1Mbit FeRAM [MR45V100A]
*/
//#define _SS         8        //  SPIチャンネルの SS(chipSelect)信号 => SSA (8)
#define _SS         7        //  SPIチャンネルの SS(chipSelect)信号 => SSB (7)

#define CMD_WREN    (0x06)  // 0000 0110 Set Write Enable Latch
#define CMD_WRDI    (0x04)  // 0000 0100 Write Disable
#define CMD_RDSR    (0x05)  // 0000 0101 Read Status Register
#define CMD_WRSR    (0x01)  // 0000 0001 Write Status Register
#define CMD_READ    (0x03)  // 0000 0011 Read Memory Data
#define CMD_WRITE   (0x02)  // 0000 0010 Write Memory Data
#define CMD_FSTRD   (0x0B)  // 0000 1011 Fast Read from Memory Array
#define CMD_RDID    (0x9F)  // 1001 1111 Read device ID
#define CMD_SLEEP   (0xB9)  // 1011 1001 Enter Sleep Mode
 
#define SR_WPEN     (0b10000000)
#define SR_BP1      (0b00001000)
#define SR_BP0      (0b00000100)
#define SR_WEL      (0b00000010)

//  ---- 1byte write
void  FeRAMwrite(uint32_t addr, byte data)
{
  digitalWrite(_SS, LOW);
  SPI.transfer(CMD_WREN);
  digitalWrite(_SS, HIGH);
  delayMicroseconds(5);

  digitalWrite(_SS, LOW);
  SPI.transfer(CMD_WRITE);
  SPI.transfer((addr & 0x00FF0000) >> 16);  // アドレス上位
  SPI.transfer((addr & 0x0000FF00) >>  8);  // アドレス中位
  SPI.transfer((addr & 0x000000FF)      );  // アドレス下位
  SPI.transfer(data);                       // Write Data
  digitalWrite(_SS, HIGH);
  delayMicroseconds(5);
}
 
//  ---- 1byte read
byte  FeRAMread(uint32_t addr)
{
  byte  ch[5];

  digitalWrite(_SS, LOW);
//  SPI.transfer(CMD_READ);
  ch[0] = SPI.transfer(CMD_READ);
  ch[1] = SPI.transfer((addr & 0x00FF0000) >> 16);  // アドレス上位
  ch[2] = SPI.transfer((addr & 0x0000FF00) >>  8);  // アドレス中位
  ch[3] = SPI.transfer((addr & 0x000000FF)      );  // アドレス下位
  ch[4] = SPI.transfer(0x00);                  // Write dummy 0x00, Reade Data
  digitalWrite(_SS, HIGH);
  delayMicroseconds(5);
  /*
  for (int i = 0; i < 5; i++) {
    PrintHex(ch[i], 2);
    Serial.print(" ");
  }
  */
  return ch[4];
}

//  ---- block write
void FeRAMblockWrite(uint32_t addr, byte *data, uint32_t length)
{
    uint32_t idx;

    for (idx = 0; idx < length; idx++) {
        FeRAMwrite(addr + idx, *(data + idx));
    }
}

//  ---- block read
void FeRAMblockRead(uint32_t addr, byte *data, uint32_t length)
{
    uint32_t idx;

    for (idx = 0; idx < length; idx++) {
        *(data + idx) = FeRAMread(addr + idx);
    }
}

//  ---- FeRAM initialize
void  InitFeRAM(void)
{
  // FeRAM : Write Disable
  digitalWrite(_SS, LOW);
  SPI.transfer(CMD_WRDI);
  digitalWrite(_SS, HIGH);
  delay(1);

  // FeRAM : Write Enable
  digitalWrite(_SS, LOW);
  SPI.transfer(CMD_WREN);
  digitalWrite(_SS, HIGH);
  delay(1);

  // FeRAM : Write Status Register
/*
  digitalWrite(_SS, LOW);
  SPI.transfer(CMD_WRSR);
  SPI.transfer(SR_WEL);
  digitalWrite(_SS, HIGH);
  delay(1);
/**/
}

/*------------------------------------------------------------------------------------
    SW (2x2 Switch) Read
*/
#define _ASW        A0    //  スイッチモジュールのアナログ入力チャンネル
#define _SL1        4     //  スイッチモジュールのLED1
#define _SL2        2     //  スイッチモジュールのLED2

byte   SW_read(void) {
    int adat;
                                        // ベースシールドに載せた場合モジュールの駆動電圧は 3.3V です
                                        // これらの電圧は計算上の大まかな数字です。現物に合わせて
                                        // 微調整してください。
    adat  = analogRead(_ASW);
    if      (adat < 0x020) return 1;    // 0.21V 未満なら SW1 が押された
    else if (adat < 0x100) return 2;    // 0.21V ～ 0.83V なら SW2 が押された
    else if (adat < 0x200) return 3;    // 0.83V ～ 1.65V なら SW3 が押された
    else if (adat < 0x300) return 4;    // 1.65V ～ 2.60V なら SW4 が押された
    return 0;                           // 2.60V 以上ならスイッチが押されていない
}

/*------------------------------------------------------------------------------------
    Serial Sub
*/
// 桁付き16進表示（ 0 fill型 ）
void  PrintHex(uint32_t dat, int dig) {
  if ((dig >= 1) && (dig <= 8)) {
    for (int i = dig; i > 0; i--) {
      Serial.print((dat >> ((i - 1) * 4)) & 0x0000000F, HEX);
    }
  }
}
/*------------------------------------------------------------------------------------
    Setup
------------------------------------------------------------------------------------*/
void setup() {
  pinMode(_SS, OUTPUT);
  digitalWrite(_SS, HIGH);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
//  SPI.setBitOrder(MSBFIRST);            // 最上位ビットから転送
//  SPI.setDataMode(SPI_MODE0);           // クロック極性：LOW   クロックエッジ：立ち上がり
//  SPI.setDataMode(SPI_MODE1);           // クロック極性：LOW   クロックエッジ：立ち下がり
//  SPI.setDataMode(SPI_MODE2);           // クロック極性：HIGH   クロックエッジ：立ち下がり
//  SPI.setDataMode(SPI_MODE3);           // クロック極性：HIGH   クロックエッジ：立ち上がり
  InitFeRAM();

  Serial.begin(115200);
  Serial.println("\n\nFeRAM SPI Test - 2018.9.25\n");

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(_SL1, OUTPUT);
  pinMode(_SL2, OUTPUT);
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(_SL1, HIGH);
  digitalWrite(_SL2, HIGH);

  cnt = 0;
  addr = 0x00000000;
  indx = 0x00000000;
  sw = 0;
  dsw = 0;
}

/*------------------------------------------------------------------------------------
    Main
------------------------------------------------------------------------------------*/
void loop() {
  byte  dat;
  dsw = sw;
  sw = SW_read();

  if (dsw == 0) {
    switch (sw) {
      case 1 :    // -=-= SW1 =-=-
        indx = 0;
        Serial.print("-- Memory Dump ------\n");
/*
        dat = FeRAMread(addr);
        PrintHex(addr, 6);
        Serial.print(" : ");
        PrintHex(dat, 2);
        Serial.print("\n");
/**/
/**/
        for (int j = 0; j < 16; j++) {
          PrintHex(addr + indx, 6);
          Serial.print(" : ");
          FeRAMblockRead(addr + indx, buf, 16);
          for (int i = 0; i < 16; i++) {
            PrintHex(buf[i], 2);
            Serial.print(" ");
          }
          Serial.print("\n");
          indx += 16;
        }
/**/
        Serial.print("\n");
        break;
      case 2 :    // -=-= SW2 =-=-
        indx = 0;
        buf[0] = cnt & 0x000000FF;
        Serial.print("-- Memory Write ------\n");
        for (int j = 0; j < 16; j++) {
          PrintHex(addr + indx, 6);
          Serial.print(" : ");
          PrintHex(buf[0], 2);
          Serial.print(" ");
          for (int i = 1; i < 16; i++) {
            buf[i] = (buf[i - 1] + 1) & 0x000000FF;
            PrintHex(buf[i], 2);
            Serial.print(" ");
          }
          FeRAMblockWrite(addr + indx, buf, 16);
          indx += 16;
          buf[0] = (buf[15] + 1) & 0x000000FF;
          Serial.print("\n");
        }
        Serial.print("\n");
        break;
      case 3 :    // -=-= SW3 =-=-
        addr = (addr + 0x00001000) & 0x0001FFFF;
        Serial.print("-- address increment ------\n");
        Serial.print("addr = ");
        PrintHex(addr, 6);
        Serial.print("\n\n");
        break;
      case 4 :    // -=-= SW4 =-=-
        Serial.print("-- test pattern write ------\n");
        for (addr = 0; addr < 0x00020000; addr += 0x00001000) {
          for (int i = 0; i < 16; i++) buf[i] = (cnt + i) & 0x000000FF;
          buf[11] = buf[10];
          buf[12] = 0;
          buf[13] = (addr >> 16) & 0x000000FF;
          buf[14] = (addr >>  8) & 0x000000FF;
          buf[15] = (addr >>  0) & 0x000000FF;
          FeRAMblockWrite(addr, buf, 16);
          Serial.print("addr = ");
          PrintHex(addr, 6);
          Serial.print(" : ");
          PrintHex(buf[0], 2);
          if ((addr & 0x00003000) == 0x00003000) {
            Serial.print("\n");
          } else {
            Serial.print("  ");
          }
          cnt++;
        }
        addr = 0;
        Serial.print(" - OK.\n\n");
        break;
      default :
        break;
    }
  }
  digitalWrite(LED1, (cnt >> 3) & 0x01);
  digitalWrite(_SL2, (cnt >> 4) & 0x01);
  digitalWrite(LED0, cnt & 0x01);
  digitalWrite(_SL1, (cnt >> 2) & 0x01);
  cnt++;
  delay(20);
}


