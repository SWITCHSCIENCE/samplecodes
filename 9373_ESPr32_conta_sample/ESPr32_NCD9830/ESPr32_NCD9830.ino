#include "espr32conta.h"
#include <Wire.h>

//------------------------------------------
// Main logics
//------------------------------------------

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  Serial.println("NCD9830_examle");
}

void loop()
{
  int data;
  float value;
  char buf1[5];

  Serial.println("Scanning...");

  for(int ch = 0; ch < 8; ch++ ) 
  {
    data = read_analog(ch); // read from ADC ch
//    Serial.println("CH" + (String)ch + ":" + (String)data);
    if(data >= 0){
      value = data * (3.3 / 256);
      dtostrf(value,1,2,buf1);
      Serial.println("CH" + (String)ch + ":" + (String)buf1 + " V");
    }else{
      Serial.println("NCD9830 error");
    }
  }
  delay(2000);           // wait 5 seconds for next scan
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
  }
  return data;
}
