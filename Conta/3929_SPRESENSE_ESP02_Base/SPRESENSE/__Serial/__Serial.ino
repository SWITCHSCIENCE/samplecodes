/*************************************************************************************
**                                                                                  **
**  Serial Test                                                                     **
**  Created : 2019/07/9 12:20:00                                                    **
**  Author  : O.aoki                                                                **
**                                                                                  **
**  [ SPRESENSE + ベースシールド + 2x2タクトスイッチモジュール ]                    **
**  [ SPRESENSE : https://www.switch-science.com/catalog/3900/ ]                    **
**  [ ベースシールド : https://www.switch-science.com/catalog/3929/ ]               **
**                                                                                  **
**************************************************************************************/
uint32_t    cnt;
int         ldat;

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
  Serial.begin(115200);     // USB Serial
  Serial2.begin(115200);    // HW Serial (D0/D1)
  
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);

  Serial.println("\n\nSerial Test - 2018.10.9\n");
  ldat = 0;
  cnt = 0;
}

/*------------------------------------------------------------------------------------
    Main
------------------------------------------------------------------------------------*/
void loop() {
  byte  dat;

  //PrintHex((cnt & 0xFF), 2);
  //Serial.print("\n");
  if (Serial.available()) Serial2.write(Serial.read());
  if (Serial2.available()) Serial.write(Serial2.read());

  digitalWrite(LED1, (cnt >> 3) & 0x01);
  digitalWrite(LED0, cnt & 0x01);
  cnt++;
  delay(40);
}



