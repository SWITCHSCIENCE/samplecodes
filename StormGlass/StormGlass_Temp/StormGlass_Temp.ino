#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#define PIN 6//NeoPixelのシグナルピン
#define BME280_ADDRESS 0x76  // BME280のI2Cアドレス
 
unsigned long int hum_raw, temp_raw, pres_raw;
signed long int t_fine;
unsigned long  runtime_next;
unsigned long  interval = 60 * 60 * 1000L;  // 60分×60秒×1000ms
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// 変換用変数
uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int8_t  dig_H1;
int16_t dig_H2;
int8_t  dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int8_t  dig_H6;

//温度の範囲の設定
double HIGHtemp=35;//この温度を超えると真っ赤になる
double LOWtemp=20;//この温度を下回ると真っ青になる

//色を決定するのに使用する変数
double T1=0;//温度を0～255の範囲で表現した場合の格納場所
double R=0;
double G=0;
double B=0;


double temp_act = 0.0, press_act = 0.0, hum_act = 0.0; //センサーの値を入れる変数


void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket

  // End of trinket special code

  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t mode = 3;               //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable
 
  uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
  uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
  uint8_t ctrl_hum_reg  = osrs_h;
  
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  Wire.begin();
  Serial.begin(9600);
 
  writeReg(0xF2, ctrl_hum_reg);
  writeReg(0xF4, ctrl_meas_reg);
  writeReg(0xF5, config_reg);
  readTrim();                   // Read trim data from BME280
}

void loop() {

  signed long int temp_cal;
  readData();
  temp_cal = calibration_T(temp_raw);
  temp_act = (double)temp_cal / 100.0;
  Serial.print("TEMP : ");
  Serial.print(temp_act);
  Serial.print("　　");
  Serial.println(T1);
  color();//色を温度あわせて変化させる関数
}

// Fill the dots one after the other with a color


void color()
{

  //T1に温度を0～255で表現したものを代入
  if(temp_act<LOWtemp)
  {
    T1=0;
  }
  else if(temp_act>HIGHtemp)
  {
    T1=255;
  }
  else 
  {
    T1=255*(temp_act-LOWtemp)/(HIGHtemp-LOWtemp);
  }

  
  calRGB(T1);//T1の値からR,G,Bの値の決定する関数

  //LEDの点灯
  for(int i=0; i<strip.numPixels(); i++) 
  {
     strip.setPixelColor(i, strip.Color(R,G,B));
  }
  strip.show();
  delay(50);
  
}

//-------------------------------------------------
//Tが255のときは真っ赤、0のときは真っ青になり、真ん中のときは緑になるようにR,G,Bを決定する
// --------------------------------------------------------------
void calRGB(int T)
{
  if(T<128)
  {
    R=0;
    G=2*T;
    B=256-2*T;
  }
  else 
  {
    B=0;
    R=2*T-256;
    G=-2*T+512;
  }
  if(R>255)
  {
    R=255;
  }
  if(G>255)
  {
    G=255;
  }
  if(B>255)
  {
    B=255;
  }
}


//-------------------------------------------------
// チップ固有の調整用データを読み出す
// --------------------------------------------------------------
void readTrim()
{
  uint8_t data[33], i = 0;
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0x88);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 24);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xA1);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 1);
  data[i] = Wire.read();
  i++;
 
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xE1);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 7);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  dig_T1 = (data[1] << 8) | data[0];
  dig_T2 = (data[3] << 8) | data[2];
  dig_T3 = (data[5] << 8) | data[4];
  dig_P1 = (data[7] << 8) | data[6];
  dig_P2 = (data[9] << 8) | data[8];
  dig_P3 = (data[11] << 8) | data[10];
  dig_P4 = (data[13] << 8) | data[12];
  dig_P5 = (data[15] << 8) | data[14];
  dig_P6 = (data[17] << 8) | data[16];
  dig_P7 = (data[19] << 8) | data[18];
  dig_P8 = (data[21] << 8) | data[20];
  dig_P9 = (data[23] << 8) | data[22];
  dig_H1 = data[24];
  dig_H2 = (data[26] << 8) | data[25];
  dig_H3 = data[27];
  dig_H4 = (data[28] << 4) | (0x0F & data[29]);
  dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
  dig_H6 = data[31];
}
 
// --------------------------------------------------------------
// BME280のレジスタに書き込む
// --------------------------------------------------------------
void writeReg(uint8_t reg_address, uint8_t data)
{
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}
 
// --------------------------------------------------------------
// BME280のデータを読み出す
// --------------------------------------------------------------
void readData()
{
  int i = 0;
  uint32_t data[8];
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xF7);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 8);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
  temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  hum_raw  = (data[6] << 8) | data[7];
}
 
// --------------------------------------------------------------
// 温度を計算する
// --------------------------------------------------------------
signed long int calibration_T(signed long int adc_T)
{
  signed long int var1, var2, T;
  var1 = ((((adc_T >> 3) - ((signed long int)dig_T1 << 1))) * ((signed long int)dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((signed long int)dig_T1)) * ((adc_T >> 4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;
 
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}
 

 

