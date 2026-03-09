/*
 * RP2350 8ch Temperature Logger (Experimental)
 * * [NOTICE]
 * 1-Wire communication errors or occasional system freezes have been observed.
 * This code is provided for reference only. Use it at your own risk.
 * * センサーの通信エラーやフリーズが発生する場合があります。
 * 本コードは参考用であり、動作を保証するものではありません。自己責任でご使用ください。
 */
#include <LovyanGFX.hpp>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

// --- LovyanGFX 設定 ---
class LGFX_AQM1248A : public lgfx::LGFX_Device {
  lgfx::Panel_ST7565 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
public:
  LGFX_AQM1248A(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = 0; cfg.spi_mode = 3; cfg.freq_write = 1000000;
      cfg.pin_sclk = 18; cfg.pin_mosi = 19; cfg.pin_miso = -1; cfg.pin_dc = 22;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 21; cfg.pin_rst = 20; cfg.panel_width = 128; cfg.panel_height = 48;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX_AQM1248A display;
LGFX_Sprite canvas(&display);
RTC_DS3231 rtc;

#define SW_LEFT   8
#define SW_PUSH   9
#define SW_RIGHT  10
#define LED_GREEN 26
#define LED_RED   23
#define SD_CS     29

// --- 1-Wire 構成 ---
const int BUS_PINS[6] = {1, 41, 43, 45, 47, 3};
OneWire oneWires[6];
DallasTemperature sensors[6];
const char* CH_NAMES[8] = {"CN1", "CN5", "CN6", "CN7", "CN8", "CN3_1", "CN3_2", "CN3_3"};
const int CH_BUS_IDX[8] = {0, 1, 2, 3, 4, 5, 5, 5};
const int CH_DEV_IDX[8] = {0, 0, 0, 0, 0, 0, 1, 2};
float temperatures[8] = {0.0};

// --- 共有変数 (volatile必須) ---
volatile uint32_t core0_heartbeat = 0;
volatile bool core0_frozen = false;

// --- 状態管理 ---
int currentPortIdx = 0;
int logInterval = 1;
bool configMode = false;
int configStep = 0; 
DateTime editTime;
bool sdActive = false;
int lastLoggedMin = -1;

unsigned long lastTempUpdate = 0;

// ================================================================
// Core 1: 監視用コア (Watchdog)
// ================================================================
void setup1() {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
}

void loop1() {
  // 5秒間 Core 0 の心拍(heartbeat)が更新されなければフリーズとみなす
  if (millis() - core0_heartbeat > 5000) {
    core0_frozen = true;
    digitalWrite(LED_RED, HIGH); // 赤LED点灯（リセットはしない）
  } else {
    // 正常時は消灯
    if (!core0_frozen) digitalWrite(LED_RED, LOW);
  }
  delay(500);
}

// ================================================================
// Core 0: メイン処理コア
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("System Booting...");

  pinMode(SW_LEFT, INPUT_PULLUP);
  pinMode(SW_PUSH, INPUT_PULLUP);
  pinMode(SW_RIGHT, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);

  EEPROM.begin(256);
  logInterval = EEPROM.read(0);
  if (logInterval < 1 || logInterval > 60) logInterval = 1;

  display.init();
  canvas.createSprite(128, 48);
  
  Wire.setSDA(12);
  Wire.setSCL(13);
  Wire.begin();
  if(!rtc.begin()) Serial.println("RTC ERROR");

  for(int b=0; b<6; b++) {
    oneWires[b].begin(BUS_PINS[b]);
    sensors[b].setOneWire(&oneWires[b]);
    sensors[b].begin();
    sensors[b].setWaitForConversion(false);
  }

  SPI1.setRX(28); SPI1.setSCK(30); SPI1.setTX(31);
  if (SD.begin(SD_CS, SPI1)) {
    sdActive = true;
    if (!SD.exists("/datalog.csv")) {
      File f = SD.open("/datalog.csv", FILE_WRITE);
      f.println("Time,CN1,CN5,CN6,CN7,CN8,CN3_1,CN3_2,CN3_3");
      f.close();
    }
  }

  core0_heartbeat = millis(); 
  Serial.println("Core 0 Ready.");
}

void logToSD(DateTime now) {
  // デバッグ用：どこで止まるかシリアルに出す
  Serial.println("> Starting SD Log...");
  
  String dataString = now.timestamp(DateTime::TIMESTAMP_FULL) + ",";
  Serial.printf("[%02d:%02d:%02d] Data: ", now.hour(), now.minute(), now.second());

  for (int i = 0; i < 8; i++) {
    dataString += String(temperatures[i], 1);
    Serial.printf("%s:%.1f ", CH_NAMES[i], temperatures[i]);
    if (i < 7) dataString += ",";
  }
  Serial.println("");

  if (sdActive) {
    File dataFile = SD.open("/datalog.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println("> SD Write Success.");
    } else {
      Serial.println("> SD File Open Failed.");
    }
  }
}

void loop() {
  // 心拍更新（生きてる証拠）
  core0_heartbeat = millis();

  DateTime now = rtc.now();

  // --- 温度更新 (2秒おきに緩和) ---
  if (millis() - lastTempUpdate > 2000) {
    Serial.println("> Updating Temperatures...");
    for(int i=0; i<8; i++) {
      float t = sensors[CH_BUS_IDX[i]].getTempCByIndex(CH_DEV_IDX[i]);
      temperatures[i] = (t != DEVICE_DISCONNECTED_C && t != 85.0) ? t : -999.0;
    }
    for(int b=0; b<6; b++) sensors[b].requestTemperatures();
    lastTempUpdate = millis();
    digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
  }

  // --- ロギング判定 (秒指定を廃止し、分が変わった瞬間に判定) ---
  if (now.minute() % logInterval == 0 && now.minute() != lastLoggedMin) {
    logToSD(now);
    lastLoggedMin = now.minute();
  }

  // --- UI/ボタン処理 ---
  handleButtons(now);

  delay(50); 
}

void handleButtons(DateTime now) {
  static unsigned long pushStartTime = 0;
  static bool isPushing = false;
  
  bool leftPressed = (digitalRead(SW_LEFT) == LOW);
  bool rightPressed = (digitalRead(SW_RIGHT) == LOW);
  bool pushDown = (digitalRead(SW_PUSH) == LOW);

  if (pushDown) {
    if (!isPushing) { pushStartTime = millis(); isPushing = true; }
    if (!configMode && (millis() - pushStartTime > 1000)) {
      configMode = true; configStep = 0; editTime = now; isPushing = false;
      while(digitalRead(SW_PUSH) == LOW) core0_heartbeat = millis(); // 待機中も心拍更新
    }
  } else if (isPushing) {
    if (configMode) {
      configStep++;
      if (configStep > 5) {
        rtc.adjust(editTime);
        EEPROM.write(0, logInterval); EEPROM.commit();
        configMode = false;
      }
    }
    isPushing = false;
  }

  if (configMode) {
    if (leftPressed) { changeConfig(-1); delay(150); }
    if (rightPressed) { changeConfig(1); delay(150); }
    drawConfig();
  } else {
    if (leftPressed) { currentPortIdx = (currentPortIdx + 7) % 8; delay(200); }
    if (rightPressed) { currentPortIdx = (currentPortIdx + 1) % 8; delay(200); }
    drawMonitor(now);
  }
}

void changeConfig(int dir) {
  int y = editTime.year(), m = editTime.month(), d = editTime.day(), hh = editTime.hour(), mm = editTime.minute();
  switch(configStep) {
    case 0: logInterval = constrain(logInterval + dir, 1, 60); break;
    case 1: y += dir; break;
    case 2: m = constrain(m + dir, 1, 12); break;
    case 3: d = constrain(d + dir, 1, 31); break;
    case 4: hh = (hh + dir + 24) % 24; break;
    case 5: mm = (mm + dir + 60) % 60; break;
  }
  editTime = DateTime(y, m, d, hh, mm, 0);
}

void drawMonitor(DateTime now) {
  canvas.clear();
  canvas.setCursor(0, 0);
  canvas.printf("%02d/%02d %02d:%02d:%02d  %s", now.month(), now.day(), now.hour(), now.minute(), now.second(), sdActive ? "SD:OK" : "SD:NG");
  canvas.drawFastHLine(0, 10, 128, 1);
  canvas.setCursor(0, 14);
  canvas.printf("CH%d:%s (Log:%dm)", currentPortIdx + 1, CH_NAMES[currentPortIdx], logInterval);
  canvas.setCursor(0, 26);
  canvas.setTextSize(2);
  if (temperatures[currentPortIdx] < -100) canvas.print("No Conn.");
  else canvas.printf("%.1f C", temperatures[currentPortIdx]); 
  canvas.setTextSize(1);
  canvas.pushSprite(0, 0);
}

void drawConfig() {
  canvas.clear();
  canvas.fillScreen(lgfx::color565(40, 40, 40));
  canvas.setCursor(0, 0);
  canvas.printf("[SETTING] %s", (configStep == 0) ? "Interval" : "RTC");
  canvas.drawFastHLine(0, 10, 128, 1);
  canvas.setCursor(0, 20);
  canvas.setTextSize(2);
  if (configStep == 0) canvas.printf("%d min", logInterval);
  else if (configStep == 1) canvas.printf("%04d", editTime.year());
  else if (configStep == 2) canvas.printf("%02d", editTime.month());
  else if (configStep == 3) canvas.printf("%02d", editTime.day());
  else if (configStep == 4) canvas.printf("%02d :", editTime.hour());
  else if (configStep == 5) canvas.printf(": %02d", editTime.minute());
  canvas.setTextSize(1);
  canvas.pushSprite(0, 0);
}