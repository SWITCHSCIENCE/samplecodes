////////////////////////////////////////////
//      Picossci Matrix LED               //
//      Sample program [ArduinoIDE]       //
//      O.Aoki [ switch-science ]         //
////////////////////////////////////////////

// pins for UART 0
#define PIN_TX0 0
#define PIN_RX0 1
// pins for Native LED
#define PIN_LED1 25
#define PIN_LED2 24
// pins for Matrix LED Row [sink]
#define PIN_ROW0 23
#define PIN_ROW1 22
#define PIN_ROW2 21
#define PIN_ROW3 20
#define PIN_ROW4 19
#define PIN_ROW5 18
#define PIN_ROW6 17
#define PIN_ROW7 16
// pins for Matrix LED Column [source]
#define PIN_COL0 15
#define PIN_COL1 14
#define PIN_COL2 13
#define PIN_COL3 12
#define PIN_COL4 11
#define PIN_COL5 10
#define PIN_COL6 9
#define PIN_COL7 8
// pins for Test toggle
#define PIN_TRI1 4
#define PIN_TRI2 5
#define PIN_TRI3 6
// pins for Analog input
#define PIN_AIN0 26
#define PIN_AIN1 27
#define PIN_AIN2 28
#define PIN_AIN3 29

struct repeating_timer st_tm1ms;

byte tdat[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
volatile byte bdat[8];
volatile byte scan = 0;

const char SCHAR = ',';

String ss = "";
bool stringComplete = false;
word len = 0;
word comm = 0;
word pnum = 0;
int dat = 0;
byte testFlag = 1;
bool timerFlag = false;
volatile word tm_1ms;   // 1msダウンカウントタイマー
unsigned int cnt = 0;

///////////////////////////////////////////////////////
// setup routine                                     //
///////////////////////////////////////////////////////
void setup() {
  // ---- initialize serial(UART0) ----
  Serial1.setTX(PIN_TX0);
  Serial1.setRX(PIN_RX0);
  Serial1.begin(115200);
  // ---- initialize Native LED(GPIO) ----
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED1, HIGH);
  digitalWrite(PIN_LED2, HIGH);
  // ---- initialize Matrix LED Row(GPIO) ----
  pinMode(PIN_ROW0, OUTPUT);
  pinMode(PIN_ROW1, OUTPUT);
  pinMode(PIN_ROW2, OUTPUT);
  pinMode(PIN_ROW3, OUTPUT);
  pinMode(PIN_ROW4, OUTPUT);
  pinMode(PIN_ROW5, OUTPUT);
  pinMode(PIN_ROW6, OUTPUT);
  pinMode(PIN_ROW7, OUTPUT);
  digitalWrite(PIN_ROW0, HIGH);
  digitalWrite(PIN_ROW1, HIGH);
  digitalWrite(PIN_ROW2, HIGH);
  digitalWrite(PIN_ROW3, HIGH);
  digitalWrite(PIN_ROW4, HIGH);
  digitalWrite(PIN_ROW5, HIGH);
  digitalWrite(PIN_ROW6, HIGH);
  digitalWrite(PIN_ROW7, HIGH);
  // ---- initialize Matrix LED Column(GPIO) ----
  pinMode(PIN_COL0, OUTPUT);
  pinMode(PIN_COL1, OUTPUT);
  pinMode(PIN_COL2, OUTPUT);
  pinMode(PIN_COL3, OUTPUT);
  pinMode(PIN_COL4, OUTPUT);
  pinMode(PIN_COL5, OUTPUT);
  pinMode(PIN_COL6, OUTPUT);
  pinMode(PIN_COL7, OUTPUT);
  digitalWrite(PIN_COL0, LOW);
  digitalWrite(PIN_COL1, LOW);
  digitalWrite(PIN_COL2, LOW);
  digitalWrite(PIN_COL3, LOW);
  digitalWrite(PIN_COL4, LOW);
  digitalWrite(PIN_COL5, LOW);
  digitalWrite(PIN_COL6, LOW);
  digitalWrite(PIN_COL7, LOW);

  for (int i = 0; i < 8; i++) bdat[i] = 0;

  //  1msサイクルでタイマー割り込み
  add_repeating_timer_us(995, Timer, NULL, &st_tm1ms);

  Serial1.println("\n\r# Picossci Matrix LED Ready.\n\r");
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
  if ((rbuff[0] == 'W') and (rbuff[1] == SCHAR)) {
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
        rstr += (String)analogRead(PIN_AIN0);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 1:
        rstr += "1,";
        rstr += (String)analogRead(PIN_AIN1);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 2:
        rstr += "2,";
        rstr += (String)analogRead(PIN_AIN2);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      case 3:
        rstr += "3,";
        rstr += (String)analogRead(PIN_AIN3);
        rstr += "\n\r";
        Serial1.print(rstr);
        break;
      default :
        break;
    }
  } else if ((comm == 1) and (len == 3)) {
    switch (pnum) {
      case 70: bdat[0] = byte(dat & 0x00FF); break;
      case 71: bdat[1] = byte(dat & 0x00FF); break;
      case 72: bdat[2] = byte(dat & 0x00FF); break;
      case 73: bdat[3] = byte(dat & 0x00FF); break;
      case 74: bdat[4] = byte(dat & 0x00FF); break;
      case 75: bdat[5] = byte(dat & 0x00FF); break;
      case 76: bdat[6] = byte(dat & 0x00FF); break;
      case 77: bdat[7] = byte(dat & 0x00FF); break;
      case 90: digitalWrite(PIN_LED1, 1 - (dat & 0x0001)); break;
      case 91: digitalWrite(PIN_LED2, 1 - (dat & 0x0001)); break;
      case 99: testFlag = byte(dat & 0x00FF); break;
      case 98:
          if (dat) {
            digitalWrite(PIN_LED1, LOW);
            digitalWrite(PIN_LED2, LOW);
            for (int i = 0; i < 8; i++) bdat[i] = 0xFF;
          } else {
            digitalWrite(PIN_LED1, HIGH);
            digitalWrite(PIN_LED2, HIGH);
            for (int i = 0; i < 8; i++) bdat[i] = 0;
          }
          testFlag = 0;
        break;
      default :
          if ((pnum >= 0) && (pnum <= 63)) {
            if (dat & 0x0001) {
              bdat[(pnum >> 3) & 0x07] |= byte(0x01 << (pnum & 0x07));
            } else {
              bdat[(pnum >> 3) & 0x07] &= byte(~(0x01 << (pnum & 0x07)));
            }
          }
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
// Matrix LED Sub routine                            //
///////////////////////////////////////////////////////
/////----- Row-pins Encoder -----/////
void RowDrive(byte dat) {
  digitalWrite(PIN_ROW0, HIGH);
  digitalWrite(PIN_ROW1, HIGH);
  digitalWrite(PIN_ROW2, HIGH);
  digitalWrite(PIN_ROW3, HIGH);
  digitalWrite(PIN_ROW4, HIGH);
  digitalWrite(PIN_ROW5, HIGH);
  digitalWrite(PIN_ROW6, HIGH);
  digitalWrite(PIN_ROW7, HIGH);

  switch (dat) {
    case 0: digitalWrite(PIN_ROW0, LOW); break;
    case 1: digitalWrite(PIN_ROW1, LOW); break;
    case 2: digitalWrite(PIN_ROW2, LOW); break;
    case 3: digitalWrite(PIN_ROW3, LOW); break;
    case 4: digitalWrite(PIN_ROW4, LOW); break;
    case 5: digitalWrite(PIN_ROW5, LOW); break;
    case 6: digitalWrite(PIN_ROW6, LOW); break;
    case 7: digitalWrite(PIN_ROW7, LOW); break;
    default : break;
  }
}

/////----- Column-pins Output -----/////
void ColumnDrive(byte dat) {
  digitalWrite(PIN_COL0, LOW);
  digitalWrite(PIN_COL1, LOW);
  digitalWrite(PIN_COL2, LOW);
  digitalWrite(PIN_COL3, LOW);
  digitalWrite(PIN_COL4, LOW);
  digitalWrite(PIN_COL5, LOW);
  digitalWrite(PIN_COL6, LOW);
  digitalWrite(PIN_COL7, LOW);

  if ((dat & 0x01) == 0x01) digitalWrite(PIN_COL0, HIGH);
  if ((dat & 0x02) == 0x02) digitalWrite(PIN_COL1, HIGH);
  if ((dat & 0x04) == 0x04) digitalWrite(PIN_COL2, HIGH);
  if ((dat & 0x08) == 0x08) digitalWrite(PIN_COL3, HIGH);
  if ((dat & 0x10) == 0x10) digitalWrite(PIN_COL4, HIGH);
  if ((dat & 0x20) == 0x20) digitalWrite(PIN_COL5, HIGH);
  if ((dat & 0x40) == 0x40) digitalWrite(PIN_COL6, HIGH);
  if ((dat & 0x80) == 0x80) digitalWrite(PIN_COL7, HIGH);
}

void DriveBitMap() {
  digitalWrite(PIN_ROW0, HIGH);
  digitalWrite(PIN_ROW1, HIGH);
  digitalWrite(PIN_ROW2, HIGH);
  digitalWrite(PIN_ROW3, HIGH);
  digitalWrite(PIN_ROW4, HIGH);
  digitalWrite(PIN_ROW5, HIGH);
  digitalWrite(PIN_ROW6, HIGH);
  digitalWrite(PIN_ROW7, HIGH);
  digitalWrite(PIN_COL0, LOW);
  digitalWrite(PIN_COL1, LOW);
  digitalWrite(PIN_COL2, LOW);
  digitalWrite(PIN_COL3, LOW);
  digitalWrite(PIN_COL4, LOW);
  digitalWrite(PIN_COL5, LOW);
  digitalWrite(PIN_COL6, LOW);
  digitalWrite(PIN_COL7, LOW);

  if ((bdat[scan] & 0x01) == 0x01) digitalWrite(PIN_COL0, HIGH);
  if ((bdat[scan] & 0x02) == 0x02) digitalWrite(PIN_COL1, HIGH);
  if ((bdat[scan] & 0x04) == 0x04) digitalWrite(PIN_COL2, HIGH);
  if ((bdat[scan] & 0x08) == 0x08) digitalWrite(PIN_COL3, HIGH);
  if ((bdat[scan] & 0x10) == 0x10) digitalWrite(PIN_COL4, HIGH);
  if ((bdat[scan] & 0x20) == 0x20) digitalWrite(PIN_COL5, HIGH);
  if ((bdat[scan] & 0x40) == 0x40) digitalWrite(PIN_COL6, HIGH);
  if ((bdat[scan] & 0x80) == 0x80) digitalWrite(PIN_COL7, HIGH);
  switch (scan) {
    case 0: digitalWrite(PIN_ROW0, LOW); break;
    case 1: digitalWrite(PIN_ROW1, LOW); break;
    case 2: digitalWrite(PIN_ROW2, LOW); break;
    case 3: digitalWrite(PIN_ROW3, LOW); break;
    case 4: digitalWrite(PIN_ROW4, LOW); break;
    case 5: digitalWrite(PIN_ROW5, LOW); break;
    case 6: digitalWrite(PIN_ROW6, LOW); break;
    case 7: digitalWrite(PIN_ROW7, LOW); break;
    default : break;
  }
  scan = (scan + 1) & 0x07;
}

void CrossCircle(word tmp) {
  if ((tmp >> 8) & 0x00000001) {
    bdat[0] = 0x18;
    bdat[1] = 0x24;
    bdat[2] = 0x42;
    bdat[3] = 0x81;
    bdat[4] = 0x81;
    bdat[5] = 0x42;
    bdat[6] = 0x24;
    bdat[7] = 0x18;
  } else {
    bdat[0] = 0x81;
    bdat[1] = 0x42;
    bdat[2] = 0x24;
    bdat[3] = 0x18;
    bdat[4] = 0x18;
    bdat[5] = 0x24;
    bdat[6] = 0x42;
    bdat[7] = 0x81;
  }
}

void WaveFlow(word tmp) {
  if ((tmp & 0x0000001F) == 0) {
    for (int i = 7; 0 < i; i -= 1) {
      bdat[i] = bdat[i - 1];
    }
    tmp >>= 5;
    if ((tmp & 0x00000008) == 0) {
      bdat[0] = 0x01 << (tmp & 0x07);
    } else {
      bdat[0] = 0x01 << (7 - (tmp & 0x07));
    }
  }
}

///////////////////////////////////////////////////////
// Main loop [forever]                               //
///////////////////////////////////////////////////////
void loop() {
  word tmp;

  // -=-=-= 1mS インターバル処理 =-=-=-
  if (timerFlag == true) {
    DriveBitMap();

    if (testFlag) {
      digitalWrite(PIN_LED2, ((cnt >> 7) & 0x00000001)); // LED2を点滅
      digitalWrite(PIN_LED1, ((cnt >> 8) & 0x00000001)); // LED1を点滅
      if (testFlag == 1) WaveFlow(cnt);
      if (testFlag == 2) CrossCircle(cnt);
    }

    cnt++;
    timerFlag = false;
  }

  // -=-=-= インターバル以外の処理 =-=-=-
  UART0read();
  if (stringComplete) ComAnalize();
  if (stringComplete) ComExe();

  
}
