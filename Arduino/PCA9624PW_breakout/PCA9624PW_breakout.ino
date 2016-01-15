#include <Wire.h>

//コントロールレジスタ
#define MODE1       0x00
#define MODE2       0x01
#define PPWM0       0x02
#define GRPPWM      0x0A
#define GRPFREQ     0x0B
#define LEDOUT0     0x0C
#define LEDOUT1     0x0D
#define SUBADR1     0x0E
#define SUBADR2     0x0F
#define SUBADR3     0x10
#define ALLCALLADR  0x11

//スレーブアドレス
#define addr        0x60

//LEDの数
#define MAX_LED     8

static unsigned char led_data[MAX_LED]; //LEDの明るさ設定用配列

//------ PCA9624PW 初期化 ------//
void initLEDdriver(){
  char cmd[3];

  //MODE1レジスタ設定
  cmd[0] = MODE1;     
  cmd[1] = 0x00;      //SLEEP = 0
  i2cWrite(addr,cmd,2);

  //ポートモード設定
  cmd[0] = LEDOUT0 | 0x80;  //LEDOUT0から連続書き込み
  cmd[1] = 0xAA;            //LED 3, 2, 1, 0 : PWM(=10)
  cmd[2] = 0xAA;            //LED 7, 6, 5, 4 : PWM(=10)
  i2cWrite(addr,cmd,3);

  clearLEDdata();           //LEDの明るさ用データ初期化
  writeLED(addr,led_data,0);               //明るさデータをPCA9624に反映
  
}

void i2cWrite(char slave_address,char *data, int num){
  Wire.beginTransmission(slave_address);
  for(int i=0;i<num;i++){
    Wire.write(*(data+i));
  }
  Wire.endTransmission();
}

void clearLEDdata(){
  for(int i=0;i<MAX_LED;i++){
    led_data[i] = 0;
  }
}

void writeLED(char slave_address, unsigned char *data, int offset){
  char cmd[9];
  int i;
  cmd[0] = PPWM0 | 0x80;   //PWM0レジスタから連続書き込み
  for(i=0;i<8;i++){
    cmd[i+1] = data[offset+i]; 
  }
  i2cWrite(slave_address, cmd, 9);
}

void setup() {
  Wire.begin();
  initLEDdriver();
}

void loop() {
  int i;

  for(int i=0;i<8;i++){       //LED0から7まで順番に明るくする
    led_data[i] = 35 * i + 10;
  }
  writeLED(addr,led_data,0);  //PCA9624に反映
  delay(2000);

  while(1){                     //LED7が消えるまでループ
    for(int i=0;i<8;i++){       //全てのLEDの明るさを1減らす
      if(led_data[i] > 0){
        led_data[i]--;
      }
    }
    writeLED(addr,led_data,0);  //PCA9624に反映
    delay(10);
    if(led_data[7] == 0){       //LED7が消えたらループ終了
      break;
    }
  }
  delay(1000);
}
