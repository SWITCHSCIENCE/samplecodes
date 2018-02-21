#include <ESP8266WiFi.h>

const char* ssid = "ssid";
const char* password = "password";

int door_led = 13;                          //ドア開閉確認用LED
int batt_led = 5;                          //バッテリー低電圧警告用LED

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(door_led, OUTPUT);
  pinMode(batt_led, OUTPUT);
  digitalWrite(door_led, 0);
  digitalWrite(batt_led, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  // Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  //GET /close/370 HTTP/1.1 リクエストのフォーマット
  String req = client.readStringUntil('\r');
  int index = req.lastIndexOf("HTTP/1.1");
  double batt = 0.0;
  batt = (req.substring(index - 4, index - 1)).toInt() / 1023.0 * 10;
  //Serial.println(req);
  client.flush();
  if (batt < 3.3 && batt != 0) {                //バッテリー残量が3.3V以下なら （0の場合はエラー）
    digitalWrite(batt_led, 1);                  //LEDをつける。
  }
  else {
    digitalWrite(batt_led, 0);
  }

  // Match the request
  int val;
  if (req.indexOf("/open") != -1) {
    digitalWrite(door_led, 0);
    Serial.print("Door: open , Battery: ");
    Serial.print(batt);
    Serial.println(" V");
  }
  else if (req.indexOf("/close") != -1) {
    digitalWrite(door_led, 1);
    Serial.print("Door: close , Battery: ");
    Serial.print(batt);
    Serial.println(" V");
  }
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\n ";

  // Send the response to the client
  client.print(s);
  delay(1);

}

