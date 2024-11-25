NetworkUDP ntpClient;
const int ntpPort = 123;

void connectToWiFi(const char *ssid, const char *pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // 古い設定を削除
  WiFi.disconnect(true);
  // WiFiイベントハンドラーを登録（別スレッドから呼ばれる）
  WiFi.onEvent(WiFiEvent);
  //接続開始
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

// 注意: WiFiEventはFreeRTOSの別タスクから呼び出される
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      //UDPパケットを初期化
      ntpClient.begin(ntpPort);
      wifi_connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      wifi_connected = false;
      break;
    default: break;
  }
}

// NTPパケットの受信処理
// リクエストがあったらGPSと同期した今の時間を返す！
void processNTPRequest() {
  int packetSize = ntpClient.parsePacket();
  if (packetSize) {
    // 受信時刻
    uint32_t seconds, microseconds;
    uint32_t recvtime = micros();
    bool valid = getUnixEpoch(&seconds, &microseconds);
    uint32_t currentNTPTime = seconds + 2208988800UL;  // 1970年1月1日からの秒数に変換

    const int NTP_PACKET_SIZE = 48;
    byte packetBuffer[NTP_PACKET_SIZE];
    ntpClient.read(packetBuffer, packetSize);

    if (packetSize == NTP_PACKET_SIZE) {
      if (valid) {
        // NTPレスポンスパケットの作成
        packetBuffer[0] = 0x24;  // LI = 00 VN = 100 Mode = 010
        // Stratum 1 サーバーの階層
        packetBuffer[1] = 1;
        // Poll Interval 16秒 ポーリング間隔（2のべき乗秒）
        packetBuffer[2] = 4;
        // Precision -20 時刻精度（2の負のべき乗秒）
        packetBuffer[3] = 0xec;
        // Root Delay サーバーから基準サーバーまでの往復遅延
        packetBuffer[4] = 0;
        packetBuffer[5] = 0;
        packetBuffer[6] = 0;
        packetBuffer[7] = 0;
        // Root Dispersion サーバーの時刻誤差範囲
        packetBuffer[8] = 0;
        packetBuffer[9] = 0;
        packetBuffer[10] = 0;
        packetBuffer[11] = 0;
        // Reference Identifier 基準サーバーの識別子
        packetBuffer[12] = 'G';
        packetBuffer[13] = 'P';
        packetBuffer[14] = 'S';
        packetBuffer[15] = ' ';
        // Reference Timestamp 基準タイムスタンプ（サーバーが最後に同期された時刻）
        convert_to_fixed_timestamp(currentNTPTime, 0, &packetBuffer[16]);
        // Origin Timestamp クライアントがリクエストを送信した時刻
        packetBuffer[24] = packetBuffer[40];
        packetBuffer[25] = packetBuffer[41];
        packetBuffer[26] = packetBuffer[42];
        packetBuffer[27] = packetBuffer[43];
        packetBuffer[28] = packetBuffer[44];
        packetBuffer[29] = packetBuffer[45];
        packetBuffer[30] = packetBuffer[46];
        packetBuffer[31] = packetBuffer[47];
        // Receive Timestamp 受信タイムスタンプ（リクエストをサーバーが受け取った時刻）
        convert_to_fixed_timestamp(currentNTPTime, microseconds, &packetBuffer[32]);
        // Transmit Timestamp 送信タイムスタンプ（サーバーが応答をクライアントに送信した時刻）
        microseconds += micros() - recvtime;
        if (microseconds >= 1000000) {
          currentNTPTime += 1;
          microseconds -= 1000000;
        }
        convert_to_fixed_timestamp(currentNTPTime, microseconds, &packetBuffer[40]);

        // NTPパケットを送信
        ntpClient.beginPacket(ntpClient.remoteIP(), ntpClient.remotePort());
        ntpClient.write(packetBuffer, 48);
        ntpClient.endPacket();

        debugPrintNTPPacket(packetBuffer);
      }
    }
  }
}

void convert_to_fixed_timestamp(uint32_t sec, uint32_t msec, uint8_t *buffer) {
  // 64ビット固定小数点の整数部（上位32ビット）は秒
  uint64_t fixed_value = (uint64_t)sec << 32;

  // 小数部（下位32ビット）はマイクロ秒を0xFFFFFFFF / 1秒でスケール
  fixed_value |= ((uint64_t)msec << 32) / 1000000;

  // 64ビット値をビッグエンディアン形式で8バイト配列に格納
  for (int i = 0; i < 8; i++) {
    buffer[7 - i] = (uint8_t)(fixed_value >> (i * 8));
  }
}

// NTPパケットの各フィールドを表示
void debugPrintNTPPacket(byte *packet) {

  Serial.print("Leap Indicator: ");
  Serial.println((packet[0] >> 6) & 0b11);  // 上位2ビット

  Serial.print("Version Number: ");
  Serial.println((packet[0] >> 3) & 0b111);  // 中間3ビット

  Serial.print("Mode: ");
  Serial.println(packet[0] & 0b111);  // 下位3ビット

  Serial.print("Stratum: ");
  Serial.println(packet[1]);

  Serial.print("Poll Interval: ");
  Serial.println(packet[2]);

  Serial.print("Precision: ");
  Serial.println(packet[3]);

#define DUMP_DWORD(start) \
  Serial.printf("%02x %02x %02x %02x\n", packet[start], packet[start + 1], packet[start + 2], packet[start + 3]); \
  Serial.print("Root Delay: ");
  DUMP_DWORD(4);

  Serial.print("Root Dispersion: ");
  DUMP_DWORD(8);

  Serial.print("Reference ID: ");
  DUMP_DWORD(12);

#define DUMP_TIMESTAMP(start) \
  for (int i = start; i < start + 8; i++) { \
    Serial.print(packet[i], HEX); \
    Serial.print(" "); \
  } \
  Serial.println();

  Serial.print("Reference Timestamp: ");
  DUMP_TIMESTAMP(16);

  Serial.print("Origin Timestamp: ");
  DUMP_TIMESTAMP(24);

  Serial.print("Receive Timestamp: ");
  DUMP_TIMESTAMP(32);

  Serial.print("Transmit Timestamp: ");
  DUMP_TIMESTAMP(40);
}