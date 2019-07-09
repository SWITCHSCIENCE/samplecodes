/*****************************************************************************************
**                                                                                      **
**  LED Bar Test                                                                        **
**  Created : 2019/07/9 12:35:00                                                        **
**  Author  : O.aoki                                                                    **
**                                                                                      **
**  [ SPRESENSE + ベースシールド + 2x2タクトスイッチモジュール ]                        **
**  [ SPRESENSE : https://www.switch-science.com/catalog/3900/ ]                        **
**  [ ベースシールド : https://www.switch-science.com/catalog/3929/ ]                   **
**  [ LEDバーモジュール : https://www.switch-science.com/catalog/3681/ ]                **
**                                                                                      **
******************************************************************************************/
  #include <Wire.h>

  #define LED_BAR_ADDR 0x70 // in 7bit for Arduino

  #define MODE1 0x00
  #define MODE2 0x01
  #define PPWM0 0x02
  #define PPWM1 0x03
  #define PPWM2 0x04
  #define PPWM3 0x05
  #define PPWM4 0x06
  #define PPWM5 0x07
  #define PPWM6 0x08
  #define PPWM7 0x09
  #define PPWM8 0x0A
  #define PPWM9 0x0B
  #define PPWM10 0x0C
  #define PPWM11 0x0D
  #define PPWM12 0x0E
  #define PPWM13 0x0F
  #define PPWM14 0x10
  #define PPWM15 0x11
  #define GRPPWM 0x12
  #define GRPFREQ 0x13
  #define LEDOUT0 0x14
  #define LEDOUT1 0x15
  #define LEDOUT2 0x16
  #define LEDOUT3 0x17
  #define SUBADR1 0x18
  #define SUBADR2 0x19
  #define SUBADR3 0x1A
  #define ALLCALLADR 0x1B

  #define AI_ALLDIS B00000000
  #define AI_ALLEN  B10000000
  #define AI_BRI_EN B10100000
  #define AI_BLK_EN B11000000
  #define AI_BRI_BLK_EN B11100000

unsigned char led_dat[16];

/*---------------------------------------------------
    PCA9622DR
*/
//  I2C write routine
void I2Cwrite(byte id,byte reg,byte *data,byte datasize)
{
  Wire.beginTransmission(id);
  Wire.write(reg);
  Wire.write(data, datasize);
  Wire.endTransmission();
}

//  I2C read routine
void I2Cread(byte id,byte reg,byte *data,byte datasize)
{
  Wire.beginTransmission(id);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((int)id,(int)datasize, false);
  for(int i = 0; i < datasize; i++)
  {
    while(Wire.available()) data[i] = Wire.read();
  }
  Wire.endTransmission(true);
}

// Read LED data (buffer memory)
unsigned char ReadLEDdata(int num) {
    return led_dat[num & 0x0F];
}

// Write LED data (buffer memory)
void WriteLEDdata(int num, unsigned char dat) {
    led_dat[num & 0x0F] = dat;
}

// Clear LED data (buffer memory)
void ClearLEDdata() {
    for (int i = 0; i < 16; i++) led_dat[i] = 0;
}

// add LED data (buffer memory)
void AddLEDdata(unsigned short pat, unsigned char a) {
    for (int i = 0; i < 16; i++) {
        if ((pat & (0x0001 << i)) == (0x0001 << i)) led_dat[i] = (led_dat[i] + a) & 0x00FF;
    }
}

// Write LED data to LED Bar module
void WriteLED(int addr) {
    byte cmd[10];

    for (int i = 0; i < 10; i++) {
        cmd[i] = (char)led_dat[i];
    }
    I2Cwrite(addr, PPWM0 + 0x80, cmd, 10);
}

//  PCA9622DR Initialize
void  InitLEDdriver(int addr) {              // PCA9622DR Initialize
    byte    cmd[4];

    // Mode 0 Set
    cmd[0] = 0x00;              // SLEEP = 0
    I2Cwrite(addr, MODE1, cmd, 1);
    // Port Mode Configration
    cmd[0] = 0xAA;              // LED3 ,2 ,1 ,0  PWM(=10)
    cmd[1] = 0xAA;              // LED7 ,6 ,5 ,4  PWM(=10)
    cmd[2] = 0xAA;              // LED11,10,9 ,8  PWM(=10)
    cmd[3] = 0xAA;              // LED15,14,13,12 PWM(=10)
    I2Cwrite(addr, LEDOUT0 + 0x80, cmd, 4);
    // LED all clear
    ClearLEDdata();
    WriteLED(addr);
}

/*---------------------------------------------------
    Setup
---------------------------------------------------*/
void setup()
  {
    Wire.begin();

    InitLEDdriver(LED_BAR_ADDR);    // 初期化

    Serial.begin(115200);
    Serial.println("\n\nLED Bar test - 2018.9.21\n");
  }

/*---------------------------------------------------
    Main
---------------------------------------------------*/
void loop()
  {
    int   i;
    
    Serial.print("LED_BAR_ADDR : ");
    Serial.println(LED_BAR_ADDR, HEX);
    // 1bit づつ点灯
    for (i = 0; i < 10; i++) {
        while (ReadLEDdata(i) < 0xFF) { 
            AddLEDdata(0x0001 << i, 4);   //  LEDの輝度を 1ビットづつ 4 増加する（255まで）
            if (ReadLEDdata(i) > 0xFB) WriteLEDdata(i, 0xFF);
            WriteLED(LED_BAR_ADDR);
            delay(1);
        }
    }
    delay(250);
    // 1bit づつ消灯
    for (i = 0; i < 10; i++) {
        while (ReadLEDdata(i) > 0) {
            AddLEDdata(0x0001 << i, 0xFC);    // LEDの輝度を 1ビットづつ -4 増加する（0まで）
            if (ReadLEDdata(i) < 4) WriteLEDdata(i, 0);
            WriteLED(LED_BAR_ADDR);
            delay(1);
        }
    }
    delay(250);
    // 全点灯
    while (ReadLEDdata(0) < 0xFF) {
        AddLEDdata(0xFFFF, 1);    // LEDの輝度を 全ビット 1 増加する（255まで）
        WriteLED(LED_BAR_ADDR);
        delay(1);
    }
    delay(250);
    // 全消灯
    while (ReadLEDdata(0) > 0) {
        AddLEDdata(0xFFFF, 0xFF);   //LEDの輝度を 全ビット -1 増加する（0まで）
        WriteLED(LED_BAR_ADDR);
        delay(1);
    }
    delay(500);
  }


