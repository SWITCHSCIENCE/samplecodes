// use SimpleFOC

#include <SPI.h>
#include <SimpleFOC.h>

const int MA735_CS = SS;
SPISettings settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);

// sensor reading function example
//  for the magnetic sensor with analog communication
// returning an angle in radians in between 0 and 2PI
float readSensor() {
  SPI.beginTransaction(settings);
  digitalWrite(MA735_CS, LOW);
  float angle = SPI.transfer16(0) * _2PI / 65536.0;
  digitalWrite(MA735_CS, HIGH);
  SPI.endTransaction();
  return angle;
}

// sensor intialising function
void initSensor() {
  SPI.begin();
  pinMode(MA735_CS, OUTPUT);
  digitalWrite(MA735_CS, HIGH);
}

// generic sensor class contructor
// - read sensor callback
// - init sensor callback (optional)
GenericSensor sensor = GenericSensor(readSensor, initSensor);

void setup() {
  // monitoring port
  Serial.begin(115200);

  // if callbacks are not provided in the constructor
  // they can be assigned directly:
  //sensor.readCallback = readSensor;
  //sensor.initCallback = initSensor;

  sensor.init();

  Serial.println("Sensor ready");
  _delay(1000);
}

void loop() {
  // iterative function updating the sensor internal variables
  // it is usually called in motor.loopFOC()
  sensor.update();

  // display the angle and the angular velocity to the terminal
  Serial.print(sensor.getAngle());
  Serial.print("\t");
  Serial.println(sensor.getVelocity());
  delay(20);
}