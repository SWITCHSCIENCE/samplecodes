/*************************************************************************************
**                                                                                  **
**  2x2 Switch Test                                                                 **
**  Created : 2018/09/18 12:20:00                                                   **
**  Author  : O.aoki                                                                **
**                                                                                  **
**  [ Arduino Uno + ベースシールド + 2x2タクトスイッチモジュール ]                  **
**  [ Arduino Uno : https://www.switch-science.com/catalog/789/ ]                   **
**  [ ベースシールド : https://www.switch-science.com/catalog/2769/ ]               **
**  [ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]  **
**                                                                                  **
**************************************************************************************/
const int _ASW = 0;   //  スイッチモジュールのアナログ入力チャンネル
const int _LED1 = 1;
const int _LED2 = 0;

/*---------------------------------------------------
    SW (2x2 Switch) Read
*/
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

/*---------------------------------------------------
    Setup
---------------------------------------------------*/
void setup() {
  pinMode(_LED1, OUTPUT);
  pinMode(_LED2, OUTPUT);

  digitalWrite(_LED1, LOW);
  digitalWrite(_LED2, LOW);
}

/*---------------------------------------------------
    Main
---------------------------------------------------*/
void loop() {
  char  sw;

  sw = SW_read();

  if        (sw == 1) {
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, LOW);
  } else if (sw == 2) {
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, HIGH);
  } else if (sw == 3) {
    digitalWrite(_LED1, HIGH);
    digitalWrite(_LED2, LOW);
  } else if (sw == 4) {
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, LOW);
  } else {
    digitalWrite(_LED1, HIGH);
    digitalWrite(_LED2, HIGH);
  }
  delay(20);
}
