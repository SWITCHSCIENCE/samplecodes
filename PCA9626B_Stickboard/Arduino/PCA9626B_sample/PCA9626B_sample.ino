/*******************************************************************************
// SWITCHSCIENCE wiki -- http://trac.switch-science.com/
// PCA9626B Arduino Sample
*******************************************************************************/
#include <Wire.h>

#define PCA9626B_ADDR 0x40 // in 7bit for Arduino


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
#define PWM16 0x12
#define PWM17 0x13
#define PWM18 0x14
#define PWM19 0x15
#define PWM20 0x16
#define PWM21 0x17
#define PWM22 0x18
#define PWM23 0x19
#define GRPPWM 0x1A
#define GRPFREQ 0x1B
#define CHASE 0x1C
#define LEDOUT0 0x1D
#define LEDOUT1 0x1E
#define LEDOUT2 0x1F
#define LEDOUT3 0x20
#define LEDOUT4 0x21
#define LEDOUT5 0x22
#define SUBADR1 0x23
#define SUBADR2 0x24
#define SUBADR3 0x25
#define ALLCALLADR 0x26

#define AI_ALLEN  B10000000

#define AI_BRI_EN B00100000
#define AI_CHASE_EN B01000000

int chase  =0x00;
int chaseMode;

int dmblink_off = B00000101;
int dmblink_on = B00100101;


void datasend(int,int,int *,int);
void dataread(int,int,int *,int);

int ledOut[6];
int ledPWM[24];
int initdata[2];
void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // 初期化
  initdata[0] = B10000001;// 4bit目のデフォルトはSLEEP=1
  initdata[1] = B00000101;// 5bit目のDMBLNK=1で全体点滅を有効/インクリメント機能ONの時は全レジスタ領域が対象
  datasend(PCA9626B_ADDR, MODE1 | AI_ALLEN, initdata, 2); // 全領域インクリメント

  // 調光用デューティー比の設定
  ledPWM[0]  = 0xFF; // Blue
  ledPWM[1]  = 0xFF; // Green
  ledPWM[2]  = 0xFF; // Red

  ledPWM[3] = 0x00; // Blue
  ledPWM[4] = 0xFF; // Green
  ledPWM[5] = 0x00; // Red

  ledPWM[6] = 0xFF; // Blue
  ledPWM[7] = 0x00; // Green
  ledPWM[8] = 0x00; // Red

  ledPWM[9] = 0x00; // Blue
  ledPWM[10] = 0x00; // Green
  ledPWM[11] = 0xFF; // Red

  ledPWM[12] = 0xFF; // Blue
  ledPWM[13] = 0xFF; // Green
  ledPWM[14] = 0x00; // Red

  ledPWM[15] = 0x00; // Blue
  ledPWM[16] = 0xFF; // Green
  ledPWM[17] = 0xFF; // Red

  ledPWM[18] = 0xFF; // Blue
  ledPWM[19] = 0x00; // Green
  ledPWM[20] = 0xFF; // Red

  ledPWM[21] = 0xFF; // Blue
  ledPWM[22] = 0xFF; // Green
  ledPWM[23] = 0xFF; // Red

  datasend(PCA9626B_ADDR, PWM0 | AI_ALLEN,ledPWM,24);

  // 全体点滅の設定
  int grp_pwm = 0x80; // 5:5
  datasend(PCA9626B_ADDR, GRPPWM,&grp_pwm,1);
  int grp_freq = 0x2F; // (0x2F+1)/24 秒周期 = 2秒周期
  datasend(PCA9626B_ADDR, GRPFREQ,&grp_freq,1);

  // CHASEモード初期化
  int chase_init = 0x00;
  datasend(PCA9626B_ADDR, CHASE,&chase_init,1);

  // LED出力設定
  //ledOut[0]= B01010101;  // LEDは個別調光に影響されない、つまり全色点灯し白色に見える
  ledOut[0]= B11111111;  // LEDは個別調光と全体の点滅(PWM)に影響される
  ledOut[1] = B11111111; // よって色の制御ができる
  ledOut[2] = B11111111;
  ledOut[3] = B11111111;
  ledOut[4] = B11111111;
  ledOut[5] = B11111111;
  datasend(PCA9626B_ADDR,LEDOUT0 | AI_ALLEN,ledOut,6);

  Serial.println("LED turned ON, PWM, etc...");
}

void loop()
{
  switch(chaseMode)
  {
    case 1: // CHASE機能有効
        // 全体点滅機能とCHASE機能は同時になされます
        // 見やすくするためにDMBLNKをOFFにしておきます
        datasend(PCA9626B_ADDR,MODE2, &dmblink_off,1);

        // 調光機能も有効になっているので見やすくするために全LEDを強制点灯させます
        ledOut[0] = B01010101;
        ledOut[1] = B01010101;
        ledOut[2] = B01010101;
        ledOut[3] = B01010101;
        ledOut[4] = B01010101;
        ledOut[5] = B01010101;
        datasend(PCA9626B_ADDR,LEDOUT0 | AI_ALLEN,ledOut,6);

        // CHSEモードの点灯の種類をすべて表示します
        for(chase=0;chase<0x90;chase++)
        {
          datasend(PCA9626B_ADDR,CHAS,&chase,1);
          delay(500);
        }
        // CHASEモード無効化
        chaseMode = 0;
        break;

    case 0: // CHASE機能無効
        // LEDの調光機能、全体点灯をONにし、色の制御を有効にします
        datasend(PCA9626B_ADDR,MODE2, &dmblink_on,1);
        ledOut[0] = B11111111;
        ledOut[1] = B11111111;
        ledOut[2] = B11111111;
        ledOut[3] = B11111111;
        ledOut[4] = B11111111;
        ledOut[5] = B11111111;
        datasend(PCA9626B_ADDR,LEDOUT0 | AI_ALLEN,ledOut,6);
        // 点滅機能はPCA9626内蔵機能なのでloop部分は何もせずとも点滅します
        delay(10000);
        // CHASEモード有効化
        chaseMode = 1;
        break;

    default:
        break;
  }
}



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
