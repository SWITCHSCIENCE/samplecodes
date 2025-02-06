#include <Wire.h>

// 追加ライブラリ
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <WiFiEspAT.h>

// .inoと同じ階層に保存
#include "picossci_secrets.h"
#include "index_html.h"

// BME680の設定
Adafruit_BME680 bme(&Wire1);
#define SEALEVELPRESSURE_HPA (1013.25)

// I2C通信ピン設定
#define SDA_PIN 6
#define SCL_PIN 7

// WiFi、ESPの設定
#define ESP_SERIAL Serial2
const long AT_BAUD_RATE = 115200;
char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS; 
WiFiServer server(80);
const int MAX_CLIENTS = 3;
const int CLIENT_CONN_TIMEOUT = 3600;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Wire1.setSDA(SDA_PIN);
  Wire1.setSCL(SCL_PIN);
  ESP_SERIAL.begin(AT_BAUD_RATE);
  WiFi.init(ESP_SERIAL);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println();
  Serial.println("WiFi connected.");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(ip);
  server.begin(MAX_CLIENTS, CLIENT_CONN_TIMEOUT);
  Serial.println("Server started. Access in browser: http://" + ip.toString() + "/");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    IPAddress clientIP = client.remoteIP();
    Serial.print("New client: ");
    Serial.println(clientIP);

    boolean currentLineIsBlank = false;
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        request += c;

        // リクエストヘッダの解析
        if (c == '\n') {
          if (currentLineIsBlank) {
            break;
          }
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    // リクエストの解析
    Serial.println("Parsing request...");
    String requestLine = getRequestLine(request);
    String method = getMethod(requestLine);
    String path = getPath(requestLine);
    Serial.print("Method: ");
    Serial.println(method);
    Serial.print("Path: ");
    Serial.println(path);

    if (method == "GET") {
      if (path == "/") {
        // インデックスページを送信
        sendIndexPage(client);
      }
      else if (path == "/data") {
        // センサーデータをJSON形式で送信
        sendSensorData(client);
      }
      else {
        // 404 Not Found
        sendNotFound(client);
      }
    }
    else {
      // 405 Method Not Allowed
      sendMethodNotAllowed(client);
    }

    // クライアントとの接続終了
    client.stop();
    Serial.println("Client disconnected");
  }
}

// リクエストから最初のリクエストラインを取得
String getRequestLine(String request) {
  int pos = request.indexOf('\n');
  if (pos != -1) {
    return request.substring(0, pos);
  }
  return "";
}

// リクエストラインからメソッドを取得
String getMethod(String requestLine) {
  int pos = requestLine.indexOf(' ');
  if (pos != -1) {
    return requestLine.substring(0, pos);
  }
  return "";
}

// リクエストラインからパスを取得
String getPath(String requestLine) {
  int start = requestLine.indexOf(' ');
  if (start != -1) {
    int end = requestLine.indexOf(' ', start + 1);
    if (end != -1) {
      return requestLine.substring(start + 1, end);
    }
  }
  return "";
}

// インデックスページを送信
void sendIndexPage(WiFiClient client) {
  Serial.println("Sending index page in chunks...");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  const size_t chunkSize = 512;
  size_t totalLength = strlen(INDEX_HTML);
  
  for (size_t i = 0; i < totalLength; i += chunkSize) {
    size_t bytesToSend = ((totalLength - i) < chunkSize) ? (totalLength - i) : chunkSize;
    char buffer[chunkSize];
    memcpy(buffer, INDEX_HTML + i, bytesToSend);
    client.write(buffer, bytesToSend);
    delay(1);
  }
  
  Serial.println("Index page sent.");
}

void sendSensorData(WiFiClient client) {
  Serial.println("Sending sensor data as JSON...");
  float temp = 0.0, hum = 0.0, pres = 0.0, gas_kohms = 0.0, alt = 0.0, disidx = 0.0;
  bool hasData = false;
  if (bme.performReading()) {
    temp      = bme.temperature;
    hum       = bme.humidity;
    pres      = bme.pressure / 100.0;     // hPa
    gas_kohms = bme.gas_resistance / 1000.0;
    alt       = bme.readAltitude(SEALEVELPRESSURE_HPA);
    hasData   = true;
  }
  disidx = 0.81*temp + 0.01*hum*(0.99*temp - 14.3) + 46.3;

  // JSONレスポンスの作成
  String json = "{";
  if (hasData) {
    json += "\"temperature\":" + String(temp, 2) + ",";
    json += "\"humidity\":" + String(hum, 2) + ",";
    json += "\"pressure\":" + String(pres, 2) + ",";
    json += "\"gas_resistance\":" + String(gas_kohms, 2) + ",";
    json += "\"altitude\":" + String(alt, 2) + ",";
    json += "\"discomfort_index\":" + String(disidx, 2);
  }
  else {
    json += "\"error\":\"Failed to read from BME680 sensor.\"";
  }
  json += "}";

  // HTTPレスポンス送信
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(json);
}

// 404 Not Found を送信
void sendNotFound(WiFiClient client) {
  Serial.println("Sending 404 Not Found...");
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html><head><title>404 Not Found</title></head><body>");
  client.println("<h1>404 Not Found</h1>");
  client.println("<p>The requested URL was not found on this server.</p>");
  client.println("</body></html>");
}

// 405 Method Not Allowed を送信
void sendMethodNotAllowed(WiFiClient client) {
  Serial.println("Sending 405 Method Not Allowed...");
  client.println("HTTP/1.1 405 Method Not Allowed");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html><head><title>405 Method Not Allowed</title></head><body>");
  client.println("<h1>405 Method Not Allowed</h1>");
  client.println("<p>The requested method is not allowed for the URL.</p>");
  client.println("</body></html>");
}
