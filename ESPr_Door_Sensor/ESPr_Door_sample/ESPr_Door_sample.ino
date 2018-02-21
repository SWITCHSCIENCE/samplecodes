
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#define CLOSE 0
const char* ssid     = "ssid";
const char* password = "password";

const char* host = "host IP address";

WiFiClient client;

boolean flag = false;
int LED = 4;
int reed_sw = 5;

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);

  // We start by connecting to a WiFi network
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(reed_sw,INPUT);
  digitalWrite(LED,LOW);
}

int value = 0;

void loop() {
  int door_state;
  door_state = digitalRead(reed_sw);

  Serial.print("Door State:");
  if(door_state == CLOSE){
    Serial.println("Close");
  }
  else{
    Serial.println("Open");
  }

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url;
  if(door_state == 0){                        
    url += "/close/";
  }
  else{
    url += "/open/";
  }
  url += system_adc_read();               //バッテリーの電圧1/10

  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  Serial.println();
  Serial.println("closing connection");

  if(door_state == CLOSE){
    Serial.println("DEEP SLEEP 60s");
    //ESP.deepSleep(60 * 1000 * 1000, WAKE_RF_DEFAULT);         //ドアが閉じている間は1分毎に起動
    ESP.deepSleep(0, WAKE_RF_DEFAULT);                          //ドアが閉じている間はドアが開くまで待機
  }
  else{
    Serial.println("DEEP SLEEP");
    ESP.deepSleep(0, WAKE_RF_DEFAULT);        //ドアが開いている間はドアが閉じるまで待機
  }
  delay(1000);
  
}

