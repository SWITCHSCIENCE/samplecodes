#include "Arduino.h"
#include "esp32-hal.h"

#define RMT_TX_PIN 1
#define RMT_MEM_TX RMT_MEM_128

/*信号がうまく届かない時は送信回数を増やす*/
#define RMT_SEND_CNT 1

rmt_obj_t* rmt_send = NULL;

rmt_data_t send_data[] = {
  { 10343, 0, 5010, 1 },
  { 750, 0, 1831, 1 },
  { 718, 0, 1859, 1 },
  { 717, 0, 1882, 1 },
  { 723, 0, 542, 1 },
  { 746, 0, 542, 1 },
  { 747, 0, 1830, 1 },
  { 718, 0, 1881, 1 },
  { 695, 0, 1882, 1 },
  { 724, 0, 541, 1 },
  { 747, 0, 541, 1 },
  { 747, 0, 1830, 1 },
  { 747, 0, 1853, 1 },
  { 695, 0, 571, 1 },
  { 746, 0, 541, 1 },
  { 747, 0, 542, 1 },
  { 746, 0, 542, 1 },
  { 746, 0, 1854, 1 },
  { 694, 0, 1882, 1 },
  { 695, 0, 1859, 1 },
  { 718, 0, 572, 1 },
  { 745, 0, 1853, 1 },
  { 694, 0, 594, 1 },
  { 724, 0, 541, 1 },
  { 747, 0, 1830, 1 },
  { 718, 0, 593, 1 },
  { 724, 0, 541, 1 },
  { 747, 0, 541, 1 },
  { 747, 0, 1853, 1 },
  { 695, 0, 594, 1 },
  { 723, 0, 1830, 1 },
  { 718, 0, 1858, 1 },
  { 719, 0, 593, 1 },
  { 724, 0, 0, 1 },
};

void setup() {
  // 初期化
  if ((rmt_send = rmtInit(RMT_TX_PIN, RMT_TX_MODE, RMT_MEM_TX)) == NULL) {
    Serial.println("init sender failed\n");
  }

  // 送信tick設定（1us）
  rmtSetTick(rmt_send, 1000);

  // 送信キャリア設定(38kHz, 1/3duty => on 8.771 off 17.543) ※ソースクロックで指定（80MHz）
  rmtSetCarrier(rmt_send, true, 1, 17543 / 12.5, 8771 / 12.5);

  // 赤外線受光モジュールの出力がアクティブローなので信号を反転する
  rmt_data_t sigbuf[128];
  memcpy(sigbuf, send_data, sizeof(send_data));
  int num_data = sizeof(send_data) / sizeof(send_data[0]);
  for (int i = 0; i < num_data; ++i) {
    sigbuf[i].level0 = ~sigbuf[i].level0;
    sigbuf[i].level1 = ~sigbuf[i].level1;
  }

  // 起動時に送信
  for (int i = 0; i < RMT_SEND_CNT; i++) {
    rmtWriteBlocking(rmt_send, sigbuf, num_data);
    delay(103);
  }

  // 終了処理
  rmtDeinit(rmt_send);
}

void loop() {
  // 送信終わったらディープスリープに入る
  esp_deep_sleep_start();
  delay(1000);
}
