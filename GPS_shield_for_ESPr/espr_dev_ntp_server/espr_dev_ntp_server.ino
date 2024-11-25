#include <WiFi.h>
#include <NetworkUdp.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

const char *ssid = "your-ssid";
const char *password = "your-password";

boolean wifi_connected = false;

void setup() {
  Serial.begin(115200);
  delay(5000);
  initGNSS();
  connectToWiFi(ssid, password);
}

void loop() {
  processGNSS();
  if (wifi_connected) {
    processNTPRequest();
  }
  delay(1);
}
