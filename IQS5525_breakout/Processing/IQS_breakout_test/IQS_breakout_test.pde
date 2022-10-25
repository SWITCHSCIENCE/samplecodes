/**
 * Simple Read
 * 
 * Read data from the serial port and change the color of a rectangle
 * when a switch connected to a Wiring or Arduino board is pressed and released.
 * This example works with the Wiring / Arduino program that follows below.
 */


import processing.serial.*;

Serial myPort;  // Create object from Serial class
int val;      // Data received from the serial port
PFont myFont;

void setup() 
{
  size(1026, 1026);
  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  String portName = Serial.list()[2];
  printArray(Serial.list());
  myPort = new Serial(this, portName, 115200);
  myPort.buffer(37);

  myFont = createFont("Meiryo", 12);
  textFont(myFont);
  println(portName);
}

void draw()
{
  String s = "";
  int b = 1;
  while ( myPort.available() > 0 && b == 1) {  // If data is available,
    val = myPort.read();         // read it and store it in val
    if (val == '\n')
    {
      b = 0;
    }
    s = s + char(val);
  }
  if (s == "")
  {
    return;
  }
  if (s.length() < 37)
  {
    return;
  }

  println(s);
  String[] xy = splitTokens(s, ",: ");
  int x = 0, y = 0;
  if (4 < xy.length)
  {
    x = int(xy[2]);
    y = int(xy[3]);
  }

  //draw frame
  background(255, 255, 255);
  fill(0, 0, 100);
  rect(0, 0, 1024, 1024);

  fill(255, 255, 255);
  rect(x-1, y-1, 3, 3);

  textSize(12);
  text(s, 200, 50);
  text("(" + x + "," + y + ")", x+5, y+15);
  myPort.clear();
}



/*

 // Wiring / Arduino Code
 // Code for sensing a switch status and writing the value to the serial port.
 
 int switchPin = 4;                       // Switch connected to pin 4
 
 void setup() {
 pinMode(switchPin, INPUT);             // Set pin 0 as an input
 Serial.begin(9600);                    // Start serial communication at 9600 bps
 }
 
 void loop() {
 if (digitalRead(switchPin) == HIGH) {  // If switch is ON,
 Serial.write(1);               // send 1 to Processing
 } else {                               // If the switch is not ON,
 Serial.write(0);               // send 0 to Processing
 }
 delay(100);                            // Wait 100 milliseconds
 }
 
 */
