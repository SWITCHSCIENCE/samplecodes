
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}

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
  if(door_state == 0){
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
  String url = "/cgi-bin/door.py/?";
  if(door_state == 0){                        
    url += "state=close";
  }
  else{
    url += "state=open";
  }
    url += "&batt=";
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

  if(door_state == 0){
    ESP.deepSleep(5*60*1000*1000, WAKE_RF_DEFAULT);         //ドアが閉じている間は5分毎に起動
  }
  else{
    ESP.deepSleep(60*60*1000*1000, WAKE_RF_DEFAULT);        //ドアが開いている間は1時間毎に起動
  }
  delay(1000);
  
}

