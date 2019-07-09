/*************************************************************************************
**                                                                                  **
**  2x2 Switch Test                                                                 **
**  Created : 2019/07/9 12:25:00                                                    **
**  Author  : O.aoki                                                                **
**                                                                                  **
**  [ SPRESENSE + ベースシールド + 2x2タクトスイッチモジュール ]                    **
**  [ SPRESENSE : https://www.switch-science.com/catalog/3900/ ]                    **
**  [ ベースシールド : https://www.switch-science.com/catalog/3929/ ]               **
**  [ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]  **
**                                                                                  **
**************************************************************************************/
#define _ASW        A0    //  スイッチモジュールのアナログ入力チャンネル
#define _SL1        4     //  スイッチモジュールのLED1
#define _SL2        2     //  スイッチモジュールのLED2
uint32_t    cnt;
int         ldat;

/*------------------------------------------------------------------------------------
    SW (2x2 Switch) Read
*/
byte   SW_read(void) {
    int adat;
                                        // ベースシールドに載せた場合モジュールの駆動電圧は 3.3V です
                                        // これらの電圧は計算上の大まかな数字です。現物に合わせて
                                        // 微調整してください。
    adat  = analogRead(_ASW);
    PrintHex(adat, 4);
    Serial.print(" ");
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
  Serial.begin(115200);
  Serial.print("\n\nStart!\n");

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(_SL1, OUTPUT);
  pinMode(_SL2, OUTPUT);
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(_SL1, HIGH);
  digitalWrite(_SL2, HIGH);

  Serial.println("\n\nSerial Test - 2018.10.9\n");
  ldat = 0;
  cnt = 0;
}

/*---------------------------------------------------
    Main
---------------------------------------------------*/
void loop() {
  byte  sw;

  sw = SW_read();

  if        (sw == 1) {
    Serial.println(" 1xxx ");
  } else if (sw == 2) {
    Serial.println(" 01xx ");
  } else if (sw == 3) {
    Serial.println(" 001x ");
  } else if (sw == 4) {
    Serial.println(" 0001 ");
  } else {
    Serial.println(" 0000 ");
  }
  digitalWrite(LED1, (cnt >> 3) & 0x01);
  digitalWrite(_SL2, (cnt >> 4) & 0x01);
  digitalWrite(LED0, cnt & 0x01);
  digitalWrite(_SL1, (cnt >> 1) & 0x01);
  cnt++;
  delay(40);
}

