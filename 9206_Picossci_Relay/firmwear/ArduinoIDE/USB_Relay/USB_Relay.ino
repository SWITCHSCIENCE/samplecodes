////////////////////////////////////////////
//      Picossci Relay                    //
//      Sample program [ArduinoIDE]       //
//      O.Aoki [ switch-science ]         //
////////////////////////////////////////////

// pins for LED
#define LED1_pin    25
#define LED2_pin    14
#define LED3_pin    15
// pins for Lever SW
#define SW1_pin     24
#define SW2_pin     23
#define SW3_pin     22
// pins for Relay Port
#define RELAY1_pin  17
#define RELAY2_pin  16
// pins for Analog input
#define AIN0_pin    26
#define AIN1_pin    27
#define AIN2_pin    28
#define AIN3_pin    29

struct repeating_timer st_tm1ms;
bool timerFlag = false;
volatile word tm_1ms;   // 1msダウンカウントタイマー

const char SCHAR = ',';
String ss = "";
bool stringComplete = false;
word len = 0;
word comm = 0;
word cnum = 0;
int dat = 0;

byte relay1reg = 0;
byte relay2reg = 0;
byte led1reg = 0;
byte led2reg = 0;
byte led3reg = 0;
byte LEDflag = 1;
byte sw1d = 0xFF;
byte sw2d = 0xFF;
byte sw3d = 0xFF;

int cnt = 0;

///////////////////////////////////////////////////////
// setup routine                                     //
///////////////////////////////////////////////////////
void setup() {
  // ---- initialize serial (USB-Serial) ----
  Serial.begin(115200);
  // ---- initialize LED (GPIO) ----
  pinMode(LED1_pin, OUTPUT);
  pinMode(LED2_pin, OUTPUT);
  pinMode(LED3_pin, OUTPUT);
  digitalWrite(LED1_pin, HIGH);
  digitalWrite(LED2_pin, HIGH);
  digitalWrite(LED3_pin, HIGH);
  // ---- initialize Lever Switch (GPIO) ----
  pinMode(SW1_pin, INPUT);
  pinMode(SW2_pin, INPUT);
  pinMode(SW3_pin, INPUT);
  // ---- initialize Relay Port (GPIO) ----
  pinMode(RELAY1_pin, OUTPUT);
  pinMode(RELAY2_pin, OUTPUT);
  digitalWrite(RELAY1_pin, LOW);
  digitalWrite(RELAY2_pin, LOW);
  //  1msサイクルでタイマー割り込み
  add_repeating_timer_us(995, Timer, NULL, &st_tm1ms);

  delay(1000);
  Serial.println("\n\r# Picossci Relay Ready.\n\r");
}

///////////////////////////////////////////////////////
// Interrupt routine [ 1mS ]                         //
///////////////////////////////////////////////////////
bool Timer(struct repeating_timer *t) {
  timerFlag = true;
  return true;
}

///////////////////////////////////////////////////////
// Serial sub routine                                //
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
void USBserialRead(void) {
  while(Serial.available()) {
    char inChar = (char)Serial.read();
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
  if ((rbuff[0] == 'W') and (rbuff[1] == SCHAR)) {
    len = 1;
    comm = 1;
  } else if ((rbuff[0] == 'R') and (rbuff[1] == SCHAR)) {
    len = 1;
    comm = 2;
  }
  // --- Command Number Detection
  int bp = 2;
  if ((len == 1) and (bp < ssend)) {
    cnum = Buff2Num(&bp, ssend, rbuff);
    if (cnum < 0) len = 0; else len = 2;
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
    switch (cnum) {
      case  1: // Read Relay port Register 1
        rstr += "1,";
        rstr += (String)(relay1reg);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case  2: // Read Relay port Register 2
        rstr += "2,";
        rstr += (String)(relay2reg);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case  4: // Read LED port Register 1
        rstr += "4,";
        rstr += (String)(led1reg);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case  5: // Read LED port Register 2
        rstr += "5,";
        rstr += (String)(led2reg);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case  6: // Read LED port Register 3
        rstr += "6,";
        rstr += (String)(led3reg);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case 80: // Read Analog port 0 (AIN0)
        rstr += "80,";
        rstr += (String)analogRead(AIN0_pin);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case 81: // Read Analog port 1 (AIN1)
        rstr += "81,";
        rstr += (String)analogRead(AIN1_pin);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case 82: // Read Analog port 2 (AIN2)
        rstr += "82,";
        rstr += (String)analogRead(AIN2_pin);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case 83: // Read Analog port 3 (AIN3)
        rstr += "83,";
        rstr += (String)analogRead(AIN3_pin);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      case 90: // Read LED flag Register
        rstr += "90,";
        rstr += (String)(LEDflag);
        rstr += "\n\r";
        Serial.print(rstr);
        break;
      default :
        break;
    }
  } else if ((comm == 1) and (len == 3)) {
    switch (cnum) {
      case  1: relay1reg = dat & 0x0001; break; // Write data to Relay port register 1
      case  2: relay2reg = dat & 0x0001; break; // Write data to Relay port register 2
      case  4: led1reg = dat & 0x0001; break; // Write data to LED port register 1
      case  5: led2reg = dat & 0x0001; break; // Write data to LED port register 2
      case  6: led3reg = dat & 0x0001; break; // Write data to LED port register 3
      case 11: relay1reg = 1; break; // Set Relay port register 1
      case 12: relay1reg = 1; break; // Set Relay port register 2
      case 13: // Set all Relay port register
        relay1reg = 1;
        relay2reg = 1;
        break;
      case 14: led1reg = 1; break; // Set LED port register 1
      case 15: led2reg = 1; break; // Set LED port register 2
      case 16: led3reg = 1; break; // Set LED port register 3
      case 17: // Set all Relay port register
        led1reg = 1;
        led2reg = 1;
        led3reg = 1;
        break;
      case 21: relay1reg = 0; break; // Reset Relay port register 1
      case 22: relay1reg = 0; break; // Reset Relay port register 2
      case 23: // Reset all Relay port register
        relay1reg = 0;
        relay2reg = 0;
        break;
      case 24: led1reg = 0; break; // Reset LED port register 1
      case 25: led2reg = 0; break; // Reset LED port register 2
      case 26: led3reg = 0; break; // Reset LED port register 3
      case 27: // Reset all LED port register
        led1reg = 0;
        led2reg = 0;
        led3reg = 0;
        break;
      case 31: relay1reg = 1 - relay1reg; break; // Toggle Relay port register 1
      case 32: relay1reg = 1 - relay2reg; break; // Toggle Relay port register 2
      case 33: // Toggle all Relay port register
        relay1reg = 1 - relay1reg;
        relay2reg = 1 - relay2reg;
        break;
      case 34: led1reg = 1 - led1reg; break; // Toggle LED port register 1
      case 35: led2reg = 1 - led2reg; break; // Toggle LED port register 2
      case 36: led3reg = 1 - led3reg; break; // Toggle LED port register 3
      case 37: // Toggle all Relay port register
        led1reg = 1 - led1reg;
        led2reg = 1 - led2reg;
        led3reg = 1 - led3reg;
        break;
      case 90: LEDflag = dat & 0x0001; break; // Write data to LED flag register
      case 99: // All Register Reset
        relay1reg = 0;
        relay2reg = 0;
        led1reg = 0;
        led2reg = 0;
        led3reg = 0;
        LEDflag = 0;
        break;
      default :
        break;
    }
  }
  ss = "";
  stringComplete = false;
  len = 0;
  comm = 0;
  cnum = 0;
  dat = 0;
}

///////////////////////////////////////////////////////
// Main loop [forever]                               //
///////////////////////////////////////////////////////
void loop() {
  // -=-=-= 1mS インターバル処理 =-=-=-
  if (timerFlag == true) {
    if ((sw1d & 0x00FF) == 0x0080) relay1reg ^= 0x01;
    if ((sw2d & 0x00FF) == 0x0080) LEDflag ^= 0x01;
    if ((sw3d & 0x00FF) == 0x0080) relay2reg ^= 0x01;

    digitalWrite(RELAY1_pin, relay1reg);
    digitalWrite(RELAY2_pin, relay2reg);
    if (LEDflag == 1) {
      digitalWrite(LED1_pin, ((cnt >> 7) & 0x00000001));
      digitalWrite(LED2_pin, (~relay2reg) & 0x01);
      digitalWrite(LED3_pin, (~relay1reg) & 0x01);
    } else {
      digitalWrite(LED1_pin, (~led1reg) & 0x01);
      digitalWrite(LED2_pin, (~led2reg) & 0x01);
      digitalWrite(LED3_pin, (~led3reg) & 0x01);
    }

    sw1d = (sw1d << 1) | (digitalRead(SW1_pin) & 0x01);
    sw2d = (sw2d << 1) | (digitalRead(SW2_pin) & 0x01);
    sw3d = (sw3d << 1) | (digitalRead(SW3_pin) & 0x01);
    cnt++;
    timerFlag = false;
  }

  // -=-=-= インターバル以外の処理 =-=-=-
  USBserialRead();
  if (stringComplete) ComAnalize();
  if (stringComplete) ComExe();
}
