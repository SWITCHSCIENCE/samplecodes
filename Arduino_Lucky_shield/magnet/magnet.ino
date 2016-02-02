/*
  MAG3110 Breakout Example Code

  by: Aaron Weiss, aaron at sparkfun dot com
      SparkFun Electronics 2011
  date: 9/6/11
  license: beerware, if you use this code and happen to meet me, you
           can by me a beer

  The code reads the raw 16-bit x, y, and z values and prints them
  out. This sketch does not use the INT1 pin, nor does it poll for
  new data.

*/

/******************************************************************************
// スイッチサイエンス　マガジン
// http://mag.switch-science.com/2016/01/30/lucky-shield-ehou/
// 較正の際には、センサをまんべんなく回転させる必要があります。
// 「8の字を描くように回す」「各軸ごとに数回回す」などの方法があります。
// こうすることで、地磁気以外の成分をおおよそ取り除くことができます。
******************************************************************************/

#include <Wire.h>
#include <math.h>

#define MAG_ADDR  0x0E //7-bit address for the MAG3110, doesn't change
#define IOEX_ADDR 0x20 // IOエクスパンダのアドレス

#define BUZZER_PIN 5

#define EHOU -15 // 南-西-北周りを正とする度数法
#define CALIB_TIME 10000 // キャリブレーションの時間(ms)

void datasend(int,int,int *,int);
void dataread(int,int,int *,int);
int32_t x_ave,y_ave,z_ave;
int32_t x_max,x_min,y_max,y_min,z_max,z_min;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  config();            // turn the MAG3110 on

  int config1 = B00001000;//io0をPIRピン覗いて出力に
  datasend(IOEX_ADDR,0x06,&config1,1);
  mag_calib();
}

void loop()
{
  // 較正用スイッチ(SW1)の監視
  int ioexpand_port[2];
  dataread(IOEX_ADDR,0x00,ioexpand_port,2);
  bool usersw1 = ioexpand_port[1] & B00100000;

  // 押されてるかどうか
  if(!usersw1)
  {
    mag_calib();
  }else{
    print_values();
  }
}

// 磁気の測定を行って最大値と最小値を測定
// そこから測定レンジを較正
void mag_calib()
{
  x_max= readx();x_min= readx();
  y_max= ready();y_min= ready();
  z_max= readz();z_min= readz();
  uint32_t times = millis();
  int32_t sampling=0;

  // user LED1 を光らせる
  int ioexpand_port[2];
  ioexpand_port[0]= B00101111;
  datasend(IOEX_ADDR,0x02,ioexpand_port,1);

  Serial.print("start calibration...");
  tone(BUZZER_PIN,400,20); // 較正化開始の合図
  while((millis() - times)<CALIB_TIME)
  {
    int32_t x,y,z;
    x = readx();
    y = ready();
    z = readz();
    if(x>x_max) x_max = x;
    if(x<x_min) x_min = x;
    if(y>y_max) y_max = y;
    if(y<y_min) y_min = y;
    if(z>z_max) z_max = z;
    if(z<z_min) z_min = z;
    sampling++;
  }
  x_ave =(x_max+ x_min)/2;
  y_ave =(y_max+ y_min)/2;
  z_ave =(z_max+ z_min)/2;
  tone(BUZZER_PIN,800,20); // 較正終了の合図

  Serial.print(sampling);
  Serial.println("point sampled");

  Serial.print("ave:x=");
  Serial.print(x_ave);
  Serial.print(",");
  Serial.print("y=");
  Serial.print(y_ave);
  Serial.print(",");
  Serial.print("z=");
  Serial.println(z_ave);

  Serial.print("max:x=");
  Serial.print(x_max);
  Serial.print(",");
  Serial.print("y=");
  Serial.print(y_max);
  Serial.print(",");
  Serial.print("z=");
  Serial.println(z_max);

  Serial.print("min:x=");
  Serial.print(x_min);
  Serial.print(",");
  Serial.print("y=");
  Serial.print(y_min);
  Serial.print(",");
  Serial.print("z=");
  Serial.println(z_min);

  // LED1消灯
  ioexpand_port[0] = B00111111;
  datasend(IOEX_ADDR,0x02,ioexpand_port,1);
}

void config(void)
{
  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x11);              // cntrl register2
  Wire.write(0x80);              // send 0x80, enable auto resets
  Wire.endTransmission();       // stop transmitting

  delay(15);

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x10);              // cntrl register1
  Wire.write(1);                 // send 0x01, active mode
  Wire.endTransmission();       // stop transmitting
}



void print_values(void)
{
  double x,y,z;
  x=  (readx()-x_ave)*10.0/(x_max-x_min); // 最大値と最小値をレンジとする割合*10
  y = (ready()-y_ave)*10.0/(y_max-y_min);
  z = (readz()-z_ave)*10.0/(z_max-z_min);
  double rad = atan2(y,x); // ラジアンを取得
  double theta = rad * 180.0 /M_PI; // 度数法に変換
  Serial.println(theta);

  // 恵方+-1°で判断
  if( ((int)theta > (EHOU-1)) & ((int)theta < (EHOU+1)) )
  {
    tone(BUZZER_PIN,1000,10);
    int port = B00011111; // LED2点灯
    datasend(IOEX_ADDR,0x02,&port,1);
  }else{
    int port = B00111111; // LED2消灯
    datasend(IOEX_ADDR,0x02,&port,1);
  }
}

int readx(void)
{
  int xl, xh;  //define the MSB and LSB

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x01);              // x MSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    xh = Wire.read(); // receive the byte
  }

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x02);              // x LSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    xl = Wire.read(); // receive the byte
  }

  int xout = (xl|(xh << 8)); //concatenate the MSB and LSB
  return xout;
}

int ready(void)
{
  int yl, yh;  //define the MSB and LSB

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x03);              // y MSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    yh = Wire.read(); // receive the byte
  }

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x04);              // y LSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    yl = Wire.read(); // receive the byte
  }

  int yout = (yl|(yh << 8)); //concatenate the MSB and LSB
  return yout;
}

int readz(void)
{
  int zl, zh;  //define the MSB and LSB

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x05);              // z MSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    zh = Wire.read(); // receive the byte
  }

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x06);              // z LSB reg
  Wire.endTransmission();       // stop transmitting

  delayMicroseconds(2); //needs at least 1.3us free time between start and stop

  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may send less than requested
  {
    zl = Wire.read(); // receive the byte
  }

  int zout = (zl|(zh << 8)); //concatenate the MSB and LSB
  return zout;
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
