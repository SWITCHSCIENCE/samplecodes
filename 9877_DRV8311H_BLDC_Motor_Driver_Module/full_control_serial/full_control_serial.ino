#include <SPI.h>
#include <SimpleFOC.h>

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(7, 5.6, 260);  // BDUAV 2206-260KV 14 poles 5.6Ω 260KV
BLDCMotor motor = BLDCMotor(7, 0.368, 1200);  // Surpass Hobby 2204-1200KV 14 poles 0.368Ω 1200KV
BLDCDriver3PWM driver = BLDCDriver3PWM(17, 18, 47);
// Inline current sensor instance
LowsideCurrentSense currentSense = LowsideCurrentSense(1000.0, 4, 5, 6);  // 1000mV/A, 4:ADC1_CH3, 5:ADC1_CH4, 6:ADC1_CH5

SPISettings settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
const int MA735_CS = SS;

// sensor reading function for the magnetic sensor with analog communication
// returning an angle in radians in between 0 and 2PI
float readSensor() {
  digitalWrite(MA735_CS, LOW);
  SPI.beginTransaction(settings);
  float angle = SPI.transfer16(0) * _2PI / 65536.0;
  SPI.endTransaction();
  digitalWrite(MA735_CS, HIGH);
  return angle;
}

// sensor intialising function
void initSensor() {
  // SS 10
  // MOSI 11
  // MISO 13
  // SCK 12
  pinMode(MA735_CS, OUTPUT);
  digitalWrite(MA735_CS, HIGH);
  SPI.begin();
}

// generic sensor class contructor
// - read sensor callback
// - init sensor callback (optional)
GenericSensor sensor = GenericSensor(readSensor, initSensor);

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) {
  command.motor(&motor, cmd);
}

void setup() {
  // initialize encoder sensor hardware
  sensor.init();

  // link the motor to the sensor
  motor.linkSensor(&sensor);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  driver.init();

  // link the motor and the driver
  motor.linkDriver(&driver);
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

  // aligning voltage [V]
  motor.voltage_sensor_align = _isset(motor.phase_resistance) ? 1.0f * motor.phase_resistance : 2;

  // index search velocity [rad/s]
  motor.velocity_index_search = 3;

  // choose FOC modulation
  // motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  // set control loop type to be used
  motor.controller = MotionControlType::torque;

  // contoller configuration
  // default parameters in defaults.h

  // velocity PI controller parameters
  motor.PID_velocity.P = 0.1f;
  motor.PID_velocity.I = 10.0f;

  // maximal voltage to be set to the motor
  motor.voltage_limit = _isset(motor.phase_resistance) ? 5.0f * motor.phase_resistance : 3;

  // DRV8311Hの最大電流以下に設定
  // ドローン用のブラシレスモーターは巻き線抵抗の値が非常に低いため電流を流し過ぎないよう注意してください。
  motor.current_limit = 1;  // [A]

  // angle P controller
  motor.P_angle.P = 20.0f;

  // maximal velocity of the position control
  motor.velocity_limit = 20;

  // use monitoring with serial
  Serial.begin(921600);
  // comment out if not needed
  motor.useMonitoring(Serial);
  motor.monitor_variables = 0;

  // initialize motor
  motor.init();

  // link current sense and the driver
  currentSense.linkDriver(&driver);

  // initialise the current sensing
  if (currentSense.init() == 0) {
    Serial.println("Current sense init fail.");
    return;
  }

  // link current sense and the driver
  motor.linkCurrentSense(&currentSense);

  // align encoder and start FOC
  motor.initFOC();

  // add target command M
  command.add('M', doTarget, "motor control");
  // command.verbose = VerboseMode::machine_readable;

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the motor control using serial terminal:"));
  _delay(1000);
}

void loop() {
  // main FOC algorithm function
  motor.loopFOC();

  // Motion control function
  motor.move();

  // シリアルプロッターでモーターの変数を監視するために使用する機能ですが、
  // 送信頻度を上げるとモーター制御に遅れが生じノイズの原因になります。
  motor.monitor();

  // user communication
  command.run();
}
