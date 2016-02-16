/*******************************************************************************
  // SWITCHSCIENCE wiki -- http://trac.switch-science.com/
  // PCA9622DR Arduino Sample
  *******************************************************************************/
  #include <Wire.h>

  #define PCA9622DR_ADDR 0x70 // in 7bit for Arduino

  #define MODE1 0x00
  #define MODE2 0x01
  #define PWM0 0x02
  #define PWM1 0x03
  #define PWM2 0x04
  #define PWM3 0x05
  #define PWM4 0x06
  #define PWM5 0x07
  #define PWM6 0x08
  #define PWM7 0x09
  #define PWM8 0x0A
  #define PWM9 0x0B
  #define PWM10 0x0C
  #define PWM11 0x0D
  #define PWM12 0x0E
  #define PWM13 0x0F
  #define PWM14 0x10
  #define PWM15 0x11
  #define GRPPWM 0x12
  #define GRPFREQ 0x13
  #define LEDOUT0 0x14
  #define LEDOUT1 0x15
  #define LEDOUT2 0x16
  #define LEDOUT3 0x17
  #define SUBADR1 0x18
  #define SUBADR2 0x19
  #define SUBADR3 0x1A
  #define ALLCALLADR 0x1B

  #define AI_ALLDIS B00000000
  #define AI_ALLEN  B10000000
  #define AI_BRI_EN B10100000
  #define AI_BLK_EN B11000000
  #define AI_BRI_BLK_EN B11100000



  void datasend(int,int,int *,int);
  void dataread(int,int,int *,int);

  int ledOut[4];
  int ledPWM[16];
  int initdata[2];
  void setup()
  {
    Serial.begin(9600);
    Wire.begin();

    // 初期化
    initdata[0] = B10000001;// 4bit目のデフォルトはSLEEP=1
    initdata[1] = B00100101;// 5bit目のDMBLNK=1で全体点滅を有効
    datasend(PCA9622DR_ADDR, MODE1 | AI_ALLEN, initdata, 2); // 全領域インクリメント

    // 調光用デューティー比の設定
    ledPWM[4] = 0x00;
    ledPWM[5] = 0x01;
    ledPWM[6] = 0x04;
    ledPWM[7] = 0x10;

    ledPWM[8] = 0x10;
    ledPWM[9] = 0x30;
    ledPWM[10] = 0x80;
    ledPWM[11] = 0xD0;

    ledPWM[12] = 0xFF;
    ledPWM[13] = 0x70;
    ledPWM[14] = 0x30;
    ledPWM[15] = 0x10;
    datasend(PCA9622DR_ADDR, PWM0 | AI_ALLEN,ledPWM,16);

    // 全体点滅の設定
    int grp_pwm = 0x80; // 5:5
    datasend(PCA9622DR_ADDR, GRPPWM,&grp_pwm,1);
    int grp_freq = 0x2F; // (0x2F+1)/24 秒周期 = 2秒周期
    datasend(PCA9622DR_ADDR, GRPFREQ,&grp_freq,1);


    // LED出力設定
    ledOut[0] = B01010101;  // LEDの個別調光や全体の点滅(PWM)は影響されない
    ledOut[1] = B10101010;  // LEDは個別調光に影響される
    ledOut[2] = B10101010;
    ledOut[3] = B11111111;  // LEDは個別調光と全体の点滅(PWM)に影響される
    datasend(PCA9622DR_ADDR,LEDOUT0 | AI_ALLEN,ledOut,4);

    Serial.println("LED turned ON, PWM, etc...");
  }

  void loop()
  {
    // 点滅機能はPCA9622内蔵機能なのでloop部分は何もせずとも点滅します
  }

  //送信用関数(デバイスアドレス,レジスタアドレス,&送信用データ,送信データ数)
  void datasend(int id,int reg,int *data,int datasize)
  {
    Wire.beginTransmission(id);
    Wire.write(reg);
    for(int i=0;i<datasize;i++)
    {
      Wire.write(data[i]);
    }
    Wire.endTransmission();
  }

  //受信用関数(デバイスアドレス,レジスタアドレス,&受信用データ,受信データ数)
  void dataread(int id,int reg,int *data,int datasize)
  {
    Wire.beginTransmission(id);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(id, datasize, false);
    for(int i=0;i<datasize;i++)
    {
      data[i] = Wire.read();
    }
    Wire.endTransmission(true);
  }
