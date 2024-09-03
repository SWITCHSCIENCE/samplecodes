// use SimpleFOC
#include <SimpleFOC.h>

// A GPIO22
// B GPIO20
// Z GPIO23
// Encoder encoder = Encoder(22, 20, 1024, 23);

// A GPIO22
// B GPIO20
// Z not use
Encoder encoder = Encoder(22, 20, 1024);

// interrupt routine intialisation
void doA() {
  encoder.handleA();
}
void doB() {
  encoder.handleB();
}
void doIndex() {
  encoder.handleIndex();
}

void setup() {
  // monitoring port
  Serial.begin(115200);

  // enable/disable quadrature mode
  encoder.quadrature = Quadrature::ON;

  // check if you need internal pullups
  encoder.pullup = Pullup::USE_EXTERN;

  // initialise encoder hardware
  encoder.init();

  // hardware interrupt enable
  encoder.enableInterrupts(doA, doB, doIndex);

  Serial.println("Encoder ready");
  _delay(1000);
}

void loop() {
  // iterative function updating the sensor internal variables
  // it is usually called in motor.loopFOC()
  // not doing much for the encoder though
  encoder.update();

  // display the angle and the angular velocity to the terminal
  Serial.print(encoder.getAngle(), 7);
  Serial.print("\t");
  Serial.println(encoder.getVelocity(), 7);

  delay(20);
}
