///////////////////////////////////////////////////////
//      Picossci DC/Servo Motor Driver               //
//      Sample program [ArduinoIDE]                  //
//      O.Aoki [ switch-science ]                    //
///////////////////////////////////////////////////////
#include <hardware/pwm.h>

// pins for UART 0:
const int TX0_pin = 0;
const int RX0_pin = 1;
// pins for PWM output:
const int PWM0A_pin = 16;
const int PWM0B_pin = 17;
const int PWM1A_pin = 18;
const int PWM1B_pin = 19;
const int PWM2A_pin = 20;
const int PWM2B_pin = 23;
const int PWM3A_pin = 22;
const int PWM3B_pin = 19;
const int Servo0_pin = 8;
const int Servo1_pin = 9;
const int Servo2_pin = 10;
const int Servo3_pin = 11;
const int Servo4_pin = 12;
const int Servo5_pin = 13;
const int Servo6_pin = 14;
const int Servo7_pin = 15;
// pins for the LEDs:
const int LED1_pin = 25;
const int LED2_pin = 24;
// pins for Analog input:
const int AIN0_pin = 26;
const int AIN1_pin = 27;
const int AIN2_pin = 28;
const int AIN3_pin = 29;

const char SCHAR = ',';

// PWMスライスを取得
unsigned int MOTOR0 = pwm_gpio_to_slice_num(PWM0A_pin);
unsigned int MOTOR1 = pwm_gpio_to_slice_num(PWM1A_pin);
unsigned int MOTOR2 = pwm_gpio_to_slice_num(PWM2A_pin);
unsigned int MOTOR3 = pwm_gpio_to_slice_num(PWM3A_pin);
unsigned int SERVO01 = pwm_gpio_to_slice_num(Servo0_pin);
unsigned int SERVO23 = pwm_gpio_to_slice_num(Servo2_pin);
unsigned int SERVO45 = pwm_gpio_to_slice_num(Servo4_pin);
unsigned int SERVO67 = pwm_gpio_to_slice_num(Servo6_pin);

String ss = "";
bool stringComplete = false;
unsigned int len = 0;
unsigned int comm = 0;
unsigned int pnum = 0;
int dat = 0;
unsigned int cnt = 0;

///////////////////////////////////////////////////////
// setup routine                                     //
///////////////////////////////////////////////////////
void setup() {
  // ---- initialize serial(UART0) ----
  Serial1.setTX(TX0_pin);
  Serial1.setRX(RX0_pin);
  Serial1.begin(115200);
  // ---- initialize DC-Motor(PWM) ----
  // PWMの周期設定
  pwm_set_clkdiv(MOTOR0, 20.3);                   // PWM0 : 100Hz(10mS)
  pwm_set_wrap(MOTOR0, 65535);                    // PWM0範囲:0-FFFF
  pwm_set_clkdiv(MOTOR1, 20.3);                   // PWM1 : 100Hz(10mS)
  pwm_set_wrap(MOTOR1, 65535);                    // PWM1範囲:0-FFFF
  pwm_set_clkdiv(MOTOR2, 20.3);                   // PWM2 : 100Hz(10mS)
  pwm_set_wrap(MOTOR2, 65535);                    // PWM2範囲:0-FFFF
  pwm_set_clkdiv(MOTOR3, 20.3);                   // PWM3 : 100Hz(10mS)
  pwm_set_wrap(MOTOR3, 65535);                    // PWM3範囲:0-FFFF
  // PWMのPIN設定
  gpio_set_function(PWM0A_pin, GPIO_FUNC_PWM);    // MOTOR0A : PWM0A
  gpio_set_function(PWM0B_pin, GPIO_FUNC_PWM);    // MOTOR0B : PWM0B
  pwm_set_chan_level(MOTOR0, PWM_CHAN_A, 0);      // MOTOR0A 初期値 : 0
  pwm_set_chan_level(MOTOR0, PWM_CHAN_B, 0);      // MOTOR0B 初期値 : 0
  pwm_set_enabled(MOTOR0, true);                  // PWM0 開始
  gpio_set_function(PWM1A_pin, GPIO_FUNC_PWM);    // MOTOR1A : PWM0A
  gpio_set_function(PWM1B_pin, GPIO_FUNC_PWM);    // MOTOR1B : PWM0B
  pwm_set_chan_level(MOTOR1, PWM_CHAN_A, 0);      // MOTOR1A 初期値 : 0
  pwm_set_chan_level(MOTOR1, PWM_CHAN_B, 0);      // MOTOR1B 初期値 : 0
  pwm_set_enabled(MOTOR1, true);                  // PWM1 開始
  gpio_set_function(PWM2A_pin, GPIO_FUNC_PWM);    // MOTOR2A : PWM0A
  gpio_set_function(PWM2B_pin, GPIO_FUNC_PWM);    // MOTOR2B : PWM0B
  pwm_set_chan_level(MOTOR2, PWM_CHAN_A, 0);      // MOTOR2A 初期値 : 0
  pwm_set_chan_level(MOTOR2, PWM_CHAN_B, 0);      // MOTOR2B 初期値 : 0
  pwm_set_enabled(MOTOR2, true);                  // PWM2 開始
  gpio_set_function(PWM3A_pin, GPIO_FUNC_PWM);    // MOTOR3A : PWM0A
  gpio_set_function(PWM3B_pin, GPIO_FUNC_PWM);    // MOTOR3B : PWM0B
  pwm_set_chan_level(MOTOR3, PWM_CHAN_A, 0);      // MOTOR3A 初期値 : 0
  pwm_set_chan_level(MOTOR3, PWM_CHAN_B, 0);      // MOTOR3B 初期値 : 0
  pwm_set_enabled(MOTOR3, true);                  // PWM3 開始
  // ---- initialize Servo(PWM) ----
  // PWMの周期設定
  pwm_set_clkdiv(SERVO01, 40.6);                  // PWM4 : 50Hz(20mS)
  pwm_set_wrap(SERVO01, 65535);                   // PWM4範囲:0-FFFF
  pwm_set_clkdiv(SERVO23, 40.6);                  // PWM5 : 50Hz(20mS)
  pwm_set_wrap(SERVO01, 65535);                   // PWM5範囲:0-FFFF
  pwm_set_clkdiv(SERVO45, 40.6);                  // PWM6 : 50Hz(20mS)
  pwm_set_wrap(SERVO45, 65535);                   // PWM6範囲:0-FFFF
  pwm_set_clkdiv(SERVO67, 40.6);                  // PWM7 : 50Hz(20mS)
  pwm_set_wrap(SERVO67, 65535);                   // PWM7範囲:0-FFFF
  // PWMのPIN設定
  gpio_set_function(Servo0_pin, GPIO_FUNC_PWM);   // Servo0 : PWM4A
  gpio_set_function(Servo1_pin, GPIO_FUNC_PWM);   // Servo1 : PWM4B
  pwm_set_chan_level(SERVO01, PWM_CHAN_A, 0);     // Servo0 初期値 : 0
  pwm_set_chan_level(SERVO01, PWM_CHAN_B, 0);     // Servo1 初期値 : 0
  pwm_set_enabled(SERVO01, true);                 // PWM4 開始
  gpio_set_function(Servo2_pin, GPIO_FUNC_PWM);   // Servo2 : PWM5A
  gpio_set_function(Servo3_pin, GPIO_FUNC_PWM);   // Servo3 : PWM5B
  pwm_set_chan_level(SERVO23, PWM_CHAN_A, 0);     // Servo2 初期値 : 0
  pwm_set_chan_level(SERVO23, PWM_CHAN_B, 0);     // Servo3 初期値 : 0
  pwm_set_enabled(SERVO23, true);                 // PWM5 開始
  gpio_set_function(Servo4_pin, GPIO_FUNC_PWM);   // Servo4 : PWM6A
  gpio_set_function(Servo5_pin, GPIO_FUNC_PWM);   // Servo5 : PWM6B
  pwm_set_chan_level(SERVO45, PWM_CHAN_A, 0);     // Servo4 初期値 : 0
  pwm_set_chan_level(SERVO45, PWM_CHAN_B, 0);     // Servo5 初期値 : 0
  pwm_set_enabled(SERVO45, true);                 // PWM6 開始
  gpio_set_function(Servo6_pin, GPIO_FUNC_PWM);   // Servo6 : PWM7A
  gpio_set_function(Servo7_pin, GPIO_FUNC_PWM);   // Servo7 : PWM7B
  pwm_set_chan_level(SERVO67, PWM_CHAN_A, 0);     // Servo6 初期値 : 0
  pwm_set_chan_level(SERVO67, PWM_CHAN_B, 0);     // Servo7 初期値 : 0
  pwm_set_enabled(SERVO67, true);                 // PWM7 開始
  // ---- initialize LED(GPIO) ----
  pinMode(LED1_pin, OUTPUT);
  pinMode(LED2_pin, OUTPUT);
  digitalWrite(LED1_pin, HIGH);
  digitalWrite(LED2_pin, HIGH);

  Serial1.println("\n\r# Picossci DC/Servo Motor Driver Ready.\n\r");
}

///////////////////////////////////////////////////////
// sub routine                                       //
///////////////////////////////////////////////////////
/////----- String to Number -----/////
int Buff2Num(int *pp, int se,char *sb) {
  int num = 0;
  int ddir = 0;

  if (sb[*pp] == '-') {
    ddir = -1;
    *pp += 1;
  } else {
    ddir = 1;
  }
  if (*pp >= se) {
    num = -1;
  } else {
    for (int i = *pp; i < se; i++) {
      if ((sb[i] >= '0') and (sb[i] <= '9')) {
        num += (int)sb[i] - 0x30;
        if ((i + 1 == se) or (sb[i + 1] == SCHAR)) {
          num *= ddir; 
          *pp = i + 2;
          break; 
        }
        num *= 10;
      }
    }
  }

  return num;
}

/////----- String Receive -----/////
void UART0read(void) {
  while(Serial1.available()) {
    char inChar = (char)Serial1.read();
    if ((inChar == '\n') or(inChar == '\r')) stringComplete = true;
    else if ((inChar >= '0') and (inChar <= '9')) ss += inChar;
    else if ((inChar >= 'A') and (inChar <= 'Z')) ss += inChar;
    else if ((inChar >= 'a') and (inChar <= 'z')) ss += (char)((int)inChar - 0x20);
    else if (inChar == '-') ss += inChar;
    else if (inChar == SCHAR) ss += inChar;
  }
}

/////----- Command Analize -----/////
void ComAnalize(void) {
  char rbuff[255];
  int ssend = ss.length();
  if (ssend >= 254) ssend = 254;
  ss.toCharArray(rbuff, ssend + 1);  // <- +1 ??
  len = 0;
  // --- Command Detection
  if ((len == 0) and (rbuff[0] == 'W') and (rbuff[1] == SCHAR)) {
    len = 1;
    comm = 1;
  } else if ((rbuff[0] == 'R') and (rbuff[1] == SCHAR)) {
    len = 1;
    comm = 2;
  }
  // --- Port Number Detection
  int bp = 2;
  if ((len == 1) and (bp < ssend)) {
    pnum = Buff2Num(&bp, ssend, rbuff);
    if (pnum < 0) len = 0; else len = 2;
  }
  // --- data Detection
  if ((len == 2) and (bp < ssend)) {
    dat = Buff2Num(&bp, ssend, rbuff);
    len = 3;
  }
}

/////----- Command Execute -----/////
void ComExe() {
  if ((comm == 2) and (len == 2)) {
    String rstr = "R,";
    switch (pnum) {
      case 0:
        rstr += "0,";
        rstr += (String)analogRead(AIN0_pin);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 1:
        rstr += "1,";
        rstr += (String)analogRead(AIN1_pin);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 2:
        rstr += "2,";
        rstr += (String)analogRead(AIN2_pin);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 3:
        rstr += "3,";
        rstr += (String)analogRead(AIN3_pin);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      default :
        break;
    }
  } else if ((comm == 1) and (len == 3)) {
    if ((pnum >= 0) and (pnum <=7)) dat &= 0x0000FFFF;
    switch (pnum) {
      case 14: digitalWrite(LED1_pin, 1 - (dat & 0x0001)); break;
      case 15: digitalWrite(LED2_pin, 1 - (dat & 0x0001)); break;
      case 0:  pwm_set_chan_level(SERVO01, PWM_CHAN_A, dat); break;
      case 1:  pwm_set_chan_level(SERVO01, PWM_CHAN_B, dat); break;
      case 2:  pwm_set_chan_level(SERVO23, PWM_CHAN_A, dat); break;
      case 3:  pwm_set_chan_level(SERVO23, PWM_CHAN_B, dat); break;
      case 4:  pwm_set_chan_level(SERVO45, PWM_CHAN_A, dat); break;
      case 5:  pwm_set_chan_level(SERVO45, PWM_CHAN_B, dat); break;
      case 6:  pwm_set_chan_level(SERVO67, PWM_CHAN_A, dat); break;
      case 7:  pwm_set_chan_level(SERVO67, PWM_CHAN_B, dat); break;
      case 10:
        if (dat >= 0) {
          pwm_set_chan_level(MOTOR0, PWM_CHAN_A, dat);
          pwm_set_chan_level(MOTOR0, PWM_CHAN_B, 0);
        } else {
          pwm_set_chan_level(MOTOR0, PWM_CHAN_A, 0);
          pwm_set_chan_level(MOTOR0, PWM_CHAN_B, -dat);
        }
        break;
      case 11:
        if (dat >= 0) {
          pwm_set_chan_level(MOTOR1, PWM_CHAN_A, dat);
          pwm_set_chan_level(MOTOR1, PWM_CHAN_B, 0);
        } else {
          pwm_set_chan_level(MOTOR1, PWM_CHAN_A, 0);
          pwm_set_chan_level(MOTOR1, PWM_CHAN_B, -dat);
        }
        break;
      case 12:
        if (dat >= 0) {
          pwm_set_chan_level(MOTOR2, PWM_CHAN_A, dat);
          pwm_set_chan_level(MOTOR2, PWM_CHAN_B, 0);
        } else {
          pwm_set_chan_level(MOTOR2, PWM_CHAN_A, 0);
          pwm_set_chan_level(MOTOR2, PWM_CHAN_B, -dat);
        }
        break;
      case 13:
        if (dat >= 0) {
          pwm_set_chan_level(MOTOR3, PWM_CHAN_A, dat);
          pwm_set_chan_level(MOTOR3, PWM_CHAN_B, 0);
        } else {
          pwm_set_chan_level(MOTOR3, PWM_CHAN_A, 0);
          pwm_set_chan_level(MOTOR3, PWM_CHAN_B, -dat);
        }
        break;
      default :
        break;
    }
  }
  ss = "";
  stringComplete = false;
  len = 0;
  comm = 0;
  pnum = 0;
  dat = 0;
}
///////////////////////////////////////////////////////
// the loop routine runs over and over again forever //
///////////////////////////////////////////////////////
void loop() {
  UART0read();
  if (stringComplete) ComAnalize();
  if (stringComplete) ComExe();
}
