/*************************************************************************************
**                                                                                  **
**  RTC Backup test                                                                 **
**  Created : 2018/09/25 15:25:00                                                   **
**  Author  : O.aoki                                                                **
**                                                                                  **
**  [ Arduino Uno + ベースシールド + 2x2タクトスイッチモジュール ]                  **
**  [ Arduino Uno : https://www.switch-science.com/catalog/789/ ]                   **
**  [ ベースシールド : https://www.switch-science.com/catalog/2769/ ]               **
**  [ RTC モジュール : https://www.switch-science.com/catalog/3770/ ]               **
**  [ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]  **
**                                                                                  **
**************************************************************************************/
#include <Wire.h>

typedef struct
{
    char    s;
    char    m;
    char    h;
    char    d;
    char    wd;
    char    mm;
    char    y;
} dt_dat;

dt_dat      dt;
char        sw;
char        dsw;

/*------------------------------------------------------------------------------------
    RTC [PCF2129AT]
*/
#define RTC_DEVICE_ADDRESS    (0x51)  // RTC の I2C アドレス (7bit)

//  Control registers
#define RTC_CONTROL1          (0x00)
#define RTC_CONTROL2          (0x01)
#define RTC_CONTROL3          (0x02)
//  Time and date registers
#define RTC_SECONDS           (0x03)
#define RTC_MINUTES           (0x04)
#define RTC_HOURS             (0x05)
#define RTC_DAYS              (0x06)
#define RTC_WEEKDAYS          (0x07)
#define RTC_MONTHS            (0x08)
#define RTC_YEARS             (0x09)

//  ---- RTC Time Read
void  GetTime(dt_dat *dt) {
  byte  dat[7];

  // I2C access (read)
  Wire.beginTransmission(RTC_DEVICE_ADDRESS);
  Wire.write(RTC_SECONDS);
  Wire.endTransmission(true);
  Wire.requestFrom(RTC_DEVICE_ADDRESS, 7, false);
  for (int i = 0; i < 7; i++) {
    dat[i] = Wire.read();
  }
  Wire.endTransmission(true);
  // BCD to byte
  dt->s  = ((dat[0] >> 4) * 10) + (dat[0] & 0x0F);
  dt->m  = ((dat[1] >> 4) * 10) + (dat[1] & 0x0F);
  dt->h  = ((dat[2] >> 4) * 10) + (dat[2] & 0x0F);
  dt->d  = ((dat[3] >> 4) * 10) + (dat[3] & 0x0F);
  dt->wd =                        (dat[4] & 0x0F);
  dt->mm = ((dat[5] >> 4) * 10) + (dat[5] & 0x0F);
  dt->y  = ((dat[6] >> 4) * 10) + (dat[6] & 0x0F);
}

//  ---- RTC Time Write
void  SetTime(dt_dat *dt) {
  byte  dat[7];

  // byte to BCD
  dat[0] = ((dt->s  / 10) * 0x10) + (dt->s  % 10);
  dat[1] = ((dt->m  / 10) * 0x10) + (dt->m  % 10);
  dat[2] = ((dt->h  / 10) * 0x10) + (dt->h  % 10);
  dat[3] = ((dt->d  / 10) * 0x10) + (dt->d  % 10);
  dat[4] =                          (dt->wd % 10);
  dat[5] = ((dt->mm / 10) * 0x10) + (dt->mm % 10);
  dat[6] = ((dt->y  / 10) * 0x10) + (dt->y  % 10);
  // I2C access (write)
  Wire.beginTransmission(RTC_DEVICE_ADDRESS);
  Wire.write(RTC_SECONDS);
  for (int i = 0; i < 7; i++) {
    Wire.write(dat[i]);
  }
  Wire.endTransmission();
}

/*------------------------------------------------------------------------------------
    SW (2x2 Switch) Read
*/
#define _ASW        0     //  スイッチモジュールのアナログ入力チャンネル

char   SW_read(void) {
    int adat;
                                        // ベースシールドに載せた場合モジュールの駆動電圧は 3.3V です
                                        // これらの電圧は計算上の大まかな数字です。現物に合わせて
                                        // 微調整してください。
    adat  = analogRead(_ASW);
    if      (adat < 0x040) return 1;    // 0.21V 未満なら SW1 が押された
    else if (adat < 0x100) return 2;    // 0.21V ～ 0.83V なら SW2 が押された
    else if (adat < 0x200) return 3;    // 0.83V ～ 1.65V なら SW3 が押された
    else if (adat < 0x280) return 4;    // 1.65V ～ 2.1V なら SW4 が押された
    return 0;                           // 2.1V 以上ならスイッチが押されていない
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

// 桁付き10進表示（ 0 fill型 ）
void  PrintDec(uint32_t dat, int dig) {
  uint32_t  dec = 1000000000;

  if ((dig >= 1) && (dig <= 10)) {
    for (int i = 10; i > 0; i--) {
      if (dig >= i) {
        Serial.print(dat / dec, dec);
      }
      dat = dat % dec;
      dec /= 10;
    }
  }
}

// 時間表示
void  PrintTime(void) {
  PrintDec(2000 + dt.y, 4);
  Serial.print("/");
  PrintDec(dt.mm, 2);
  Serial.print("/");
  PrintDec(dt.d, 2);
  Serial.print("(");
  PrintDec(dt.wd, 2);
  Serial.print(") ");
  PrintDec(dt.h, 2);
  Serial.print(":");
  PrintDec(dt.m, 2);
  Serial.print(":");
  PrintDec(dt.s, 2);
  Serial.print("\n");
}

/*------------------------------------------------------------------------------------
    Setup
------------------------------------------------------------------------------------*/
void setup() {
  Wire.begin();

  Serial.begin(115200);
  Serial.println("\n\nRTC Backup test - 2018.09.25\n");

  sw = 0;
  dsw = 0;
}

/*------------------------------------------------------------------------------------
    Main
------------------------------------------------------------------------------------*/
void loop() {
  dsw = sw;
  sw = SW_read();
  if (dsw == 0) {
    switch (sw) {
      case 1 :    // -=-= SW1 =-=-
        Serial.print("-- Read RTC.\n");
        GetTime(&dt);
        break;
      case 2 :    // -=-= SW2 =-=-
        Serial.print("-- Print d-data.\n");
        PrintTime();
        break;
      case 3 :    // -=-= SW3 =-=-
        Serial.print("-- Read RTC & Print.\n");
        GetTime(&dt);
        PrintTime();
        break;
      case 4 :    // -=-= SW4 =-=-
        Serial.print("-- Init time (Set Time).\n");
        dt.y  = 18;
        dt.mm = 9;
        dt.d  = 25;
        dt.wd = 2;
        dt.h  = 12;
        dt.m  = 0;
        dt.s  = 0;
        SetTime(&dt);
        PrintTime();
        break;
      default :
        break;
    }
  }
  delay(20);
}
