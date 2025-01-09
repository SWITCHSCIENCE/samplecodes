// Open loop motor control example
#include <SimpleFOC.h>

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(7, 5.6, 260);  // BDUAV 2206-260KV 14 poles 5.6Ω 260KV
// BLDCMotor motor = BLDCMotor(7, 0.368, 1200);  // Surpass Hobby 2204-1200KV 14 poles 0.368Ω 1200KV
BLDCDriver3PWM driver = BLDCDriver3PWM(17, 18, 47);
// DRV8311Hのローサイド電流センサを使用
// 第一引数は1AあたりのmV比、GAINピンの設定（HiZ）に合わせてここでは1000とする
LowsideCurrentSense currentSense = LowsideCurrentSense(1000.0, 4, 5, 6);

// target variable
float target_velocity = 2 * PI;

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char *cmd) {
  command.scalar(&target_velocity, cmd);
}
void doLimit(char *cmd) {
  command.scalar(&motor.voltage_limit, cmd);
}
void doCurrent(char *cmd) {
  command.scalar(&motor.current_limit, cmd);
}

void setup() {
  // use monitoring with serial
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  // limit the maximal dc voltage the driver can set
  // as a protection measure for the low-resistance motors
  // this value is fixed on startup
  driver.voltage_limit = 6;
  if (!driver.init()) {
    Serial.println("Driver init failed!");
    return;
  }
  // link the motor and the driver
  motor.linkDriver(&driver);
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

  // limiting motor movements
  // limit the voltage to be set to the motor
  // start very low for high resistance motors
  // current = voltage / resistance, so try to be well under 1Amp
  motor.voltage_limit = 3;  // [V]

  // DRV8311Hの最大電流以下に設定
  // このサンプルでは電流センサは使わずブラシレスモーターの巻き線抵抗とKV値から推定した電流値でトルク制御を行う。
  // 巻き線抵抗が指定されていない場合、current_limitの値は無視される。
  // ドローン用のブラシレスモーターは巻き線抵抗の値が非常に低いため電流を流し過ぎないよう注意してください。
  motor.current_limit = 0.5;  // [A]

  // open loop control config
  motor.controller = MotionControlType::velocity_openloop;

  // init motor hardware
  if (!motor.init()) {
    Serial.println("Motor init failed!");
    return;
  }

  // link current sense and the driver
  currentSense.linkDriver(&driver);

  // initialise the current sensing
  if (currentSense.init() == 0) {
    Serial.println("Current sense init fail.");
  }

  // add target command T
  command.add('T', doTarget, "target velocity");
  command.add('L', doLimit, "voltage limit");
  command.add('C', doCurrent, "current limit");

  Serial.println("Motor ready!");
  Serial.println("Set target velocity [rad/s]");
  _delay(1000);
}

void loop() {
  // open loop velocity movement
  // using motor.voltage_limit and motor.velocity_limit
  // to turn the motor "backwards", just set a negative target_velocity
  motor.move(target_velocity);

  // user communication
  command.run();

  // 各相の電流を出力する
  static uint32_t last = 0;
  uint32_t curr = micros();
  if (curr - last > 10000) {
    PhaseCurrent_s currents = currentSense.getPhaseCurrents();
    Serial.print(currents.a * 1000);  // milli Amps
    Serial.print("\t");
    Serial.print(currents.b * 1000);  // milli Amps
    Serial.print("\t");
    Serial.println(currents.c * 1000);  // milli Amps
    last = curr;
  }
}
