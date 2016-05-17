/*
  2016/05/11 Switch Science,inc.
  Arduino Yún Bridge example for Rapiro

  Possible commands created in this shetch:
  
  "/arduino/motion/0"     -> Initial Position
  "/arduino/motion/1"     -> Move Forward
  "/arduino/motion/2"     -> Move Backward
  "/arduino/motion/3"     -> Turn Left
  "/arduino/motion/4"     -> Turn Right
  "/arduino/motion/5"     -> Wave Both Hands in a vertical direction
  "/arduino/motion/6"     -> Wave Rigth Hand
  "/arduino/motion/7"     -> Wave Both Hands in a horizontal direction
  "/arduino/motion/8"     -> Wave Left Hand
  "/arduino/motion/9"     -> Catch Action

*/

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <SoftwareSerial.h>

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;
SoftwareSerial rapiroSerial(2,4);

String motionName[10] = { "Initial Position",
                          "Move Forward",
                          "Move Backward",
                          "Turn Left",
                          "Turn Right",
                          "Wave Both Hands in a vertical direction",
                          "Wave Right Hand",
                          "Wave Both Hands in a horizontal direction",
                          "Wave Left Hand",
                          "Catch Action"};


void setup() {
  rapiroSerial.begin(57600);
  // Bridge startup
  Bridge.begin();

  
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(BridgeClient client) {
  // read the command
  String command = client.readStringUntil('/');
  //client.println(command);
  // is "motion" command?
  if (command == "motion") {
    motionCommand(client);
  }
}

void motionCommand(BridgeClient client) {
  int value;

  // Read motion number
   value = client.parseInt();
   if(value >= 0 && value <= 10){
      client.println(motionName[value]);
      rapiroSerial.print("#M");
      rapiroSerial.print(value);
   }
   else{
      client.println("Motion Number must be in the range 0-9.");
   }
}




