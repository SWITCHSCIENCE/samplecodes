#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "Arduino.h"

#include "esp32-hal.h"

#define LED_PIN 0
#define RMT_RX_PIN 10
#define RMT_MEM_RX RMT_MEM_128

#define RECV_BUF_LEN 128
rmt_data_t recv_buf[RECV_BUF_LEN];
rmt_obj_t* rmt_recv = NULL;

EventGroupHandle_t events;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  events = xEventGroupCreate();

  // 初期化
  if ((rmt_recv = rmtInit(RMT_RX_PIN, RMT_RX_MODE, RMT_MEM_RX)) == NULL) {
    Serial.println("init receiver failed\n");
  }
  // 受信tick設定(1us)
  rmtSetTick(rmt_recv, 1000);
  // フィルターしきい値(この設定より短いパルスを除外)※ソースクロックで指定（80MHz）
  rmtSetFilter(rmt_recv, true, 255);
  // 受信アイドルしきい値(20,000us)
  rmtSetRxThreshold(rmt_recv, 20000);
  // 受信開始（非同期）
  memset(recv_buf, 0, sizeof(recv_buf));
  rmtReadAsync(rmt_recv, recv_buf, RECV_BUF_LEN, events, false, 0);
}

void loop() {
  // 受信したか確認
  EventBits_t ev = xEventGroupWaitBits(events, RMT_FLAG_RX_DONE, 1, 1, 20 / portTICK_PERIOD_MS);
  if (ev & RMT_FLAG_RX_DONE) {
    // LED点灯
    digitalWrite(LED_PIN, LOW);
    // 受信データを出力（出力は送信側スケッチにそのままコピペ可
    if (Serial) {
      for (int i = 0; i < RECV_BUF_LEN; i++) {
        if (recv_buf[i].duration0 || recv_buf[i].duration1) {
          Serial.printf("{%d,%d,%d,%d},",
                        recv_buf[i].duration0, recv_buf[i].level0,
                        recv_buf[i].duration1, recv_buf[i].level1);
        }
      }
      Serial.println();
    }
    // LED消灯
    digitalWrite(LED_PIN, HIGH);

    // 再度受信開始
    memset(recv_buf, 0, sizeof(recv_buf));
    rmtReadAsync(rmt_recv, recv_buf, RECV_BUF_LEN, events, false, 0);
  }
}
