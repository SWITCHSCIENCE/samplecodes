#include "espr32conta.h"
#include <Wire.h>

// 使用するADC入力とLED
int sw_ch;
int _LED1 = IO2;
int _LED2 = IO3;

//------------------------------------------
// Main logics
//------------------------------------------

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  Serial.println("2x2_switch_example");
  //switchモジュールの検出
  if(read_analog(0) > 0x99){
    Serial.println("detected on M1");
    sw_ch = 0;
  }else if(read_analog(1) > 0x99){
    Serial.println("detected on M2");
    sw_ch = 1;
  }else{
    Serial.println("not detected");
    sw_ch = 0;
  }
 
  // LED PIN
  pinMode(_LED1, OUTPUT);
  pinMode(_LED2, OUTPUT);
  digitalWrite(_LED1, HIGH);
  digitalWrite(_LED2, HIGH);

  delay(1000);
}

void loop()
{
  int sw_no;
  int data;
  float value;
  char buf1[5];

  Serial.println("Scanning...");
  data = read_analog(sw_ch); // read from ADC ch
  value = data * (3.3 / 256);
  dtostrf(value,1,2,buf1);
  Serial.println("CH" + (String)sw_ch + ":" + (String)buf1 + " V");
  if(value < 0.21){
    sw_no = 1;
  }else if(value < 0.83){
    sw_no = 2;
  }else if(value < 1.65){
    sw_no = 3;
  }else if(value < 2.55){
    sw_no = 4;
  }else{
    sw_no = 0;
  }
  Serial.println("SW" + (String)sw_no + " is pressed");

  switch(sw_no){
    case 1:
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, HIGH);
    break;
    case 2:
    digitalWrite(_LED1, HIGH);
    digitalWrite(_LED2, LOW);
    break;
    case 3:
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, LOW);
    break;
    case 4:
    digitalWrite(_LED1, LOW);
    digitalWrite(_LED2, HIGH);
    delay(500);
    digitalWrite(_LED1, HIGH);
    digitalWrite(_LED2, LOW);
    delay(500);
    break;
    default:
    digitalWrite(_LED1, HIGH);
    digitalWrite(_LED2, HIGH);
    break;
  }

  delay(100);           // wait 5 seconds for next scan
}

//------------------------------------------
// device fonction
//------------------------------------------
int read_analog(int ch){
  byte error;
  int data;
  int add_NCD9830 = 0x48; //ADC の I2C アドレス
  int cmd = 0x80 + ((ch << 3) & 0x30) + ((ch << 6) & 0x40); //Channel Selectorの設定
  Wire.beginTransmission(add_NCD9830);  //IC2デバイスに接続
  Wire.write(cmd);  //I2Cデバイスのレジスタを指定
  error = Wire.endTransmission(false); //送信完了するが接続は維持
  /* error value
  * 0: success.
  * 1: data too long to fit in transmit buffer.
  * 2: received NACK on transmit of address.
  * 3: received NACK on transmit of data.
  * 4: other error.
  * 5: timeout
  */
  if(error == 0){
    Wire.requestFrom(add_NCD9830, 1); //IC2デバイスに指定したレジスタアドレスから指定サイズ分のデータ取得を要求
    data = Wire.read(); //データを取得 (受け取ったバイト数繰り返す)
  }else{
    data = -1;
  }  return data;
}
