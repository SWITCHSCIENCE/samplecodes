
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char *ssid = "ssid";
const char *password = "password";

ESP8266WebServer server ( 80 );
IRsend irsend(14);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  irsend.begin();
  Serial.println("");

  //wait for connection
  while( WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/",handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  char temp[800];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  char message[20];
  String(server.arg(0)).toCharArray(message,20);

  if(server.arg(0).indexOf("Power") != -1){
    Serial.println("Power");
    irsend.sendPanasonic(0x555A,0xF148688B);
    delay(10);
    irsend.sendPanasonic(0x555A,0xF148688B);
  }
  else if(server.arg(0).indexOf("Vol UP") != -1){
    Serial.println("Vol UP");
    irsend.sendPanasonic(0x555A,0xF148288F);
  }
  else if(server.arg(0).indexOf("Vol Down") != -1){
    Serial.println("Vol Down");
    irsend.sendPanasonic(0x555A,0xF148A887);
  }
  
  snprintf ( temp, 800,

"<html>\
  <head>\
    <title>ESPr IR Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>ESPr IR DEMO</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>BUTTON :%s</p>\
    <form action=\"/\" method=\"post\">\
      <input type=\"submit\" name=\"button1\" value=\"Power\" style=\"width:30%; height:100px\">\
      <input type=\"submit\" name=\"button2\" value=\"Vol UP\" style=\"width:30%; height:100px\">\
      <input type=\"submit\" name=\"button3\" value=\"Vol Down\" style=\"width:30%; height:100px\">\
    </form>\
  </body>\
</html>",

    hr, min % 60, sec % 60 ,message
    
  );
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );

}
