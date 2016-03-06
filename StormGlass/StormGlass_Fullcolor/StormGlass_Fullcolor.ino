#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#define PIN 6
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


int tempTable[] = {20, 30};                      // 温度範囲の指定
long baroTable[] = {950, 1050};                  // 気圧範囲の指定

double X[3][16]={
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

double R1=0;
double G1=0;
double B2=0;

double temp_act = 0.0, press_act = 0.0, hum_act = 0.0; //最終的に表示される値を入れる変数
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
  unsigned long int press_cal, hum_cal;
 
  readData();
 
  temp_cal = calibration_T(temp_raw);
  press_cal = calibration_P(pres_raw);
  hum_cal = calibration_H(hum_raw);
  temp_act = (double)temp_cal / 100.0;
  press_act = (double)press_cal / 100.0;
  hum_act = (double)hum_cal / 1024.0;
  
  Serial.print("TEMP : ");
  Serial.print(temp_act);
  Serial.print(" DegC  PRESS : ");
  Serial.print(press_act);
  Serial.print(" hPa  HUM : ");
  Serial.print(hum_act);
  Serial.println(" %");
  Serial.print("R ");
  Serial.print(R1);
  Serial.print(" G: ");
  Serial.print(G1);
  Serial.print(" B ");
  Serial.print(B2);
  Serial.println(" %");
  color();
}

// Fill the dots one after the other with a color


void color()
{
  
 //温度の範囲を参考に温度を0～255の範囲で表現
  if(temp_act<tempTable[0])
  {
    R1=0;
  }
  else if(temp_act>tempTable[1])
  {
    R1=255;
  }
  else
  {
    R1=255*(temp_act-tempTable[0])/(tempTable[1]-tempTable[0]);
  }

  //気圧の範囲を参考に気圧を0～255の範囲で表現
  if(press_act<baroTable[0])
  {
    G1=0;
  }
  else if(press_act>baroTable[1])
  {
    G1=255;
  }
  else
  {
    G1=255*(press_act-baroTable[0])/(baroTable[1]-baroTable[0]);
  }

  //湿度を0～255の範囲で表現
  B2=255*hum_act/100;


  //気温、気圧、湿度をR,G,Bに依存させひとつの色として気象を表現する。

  
  //配列の最初の3つは次のように固定する。
  for(int i =0;i<3;i++)
  {
    X[i][0]=100;
    X[i][1]=255;
    X[i][2]=100;
  }
  
  //X[0]の行には気温、X[1]の行には気圧、X[2]の行には湿度を格納する。
  for(int i=3;i<16;i++)
  {
    X[0][i]=R1;
    X[1][i]=G1;
    X[2][i]=B2;
  }
  //配列に格納された色をぐるぐる回す。
  int k=0;
  for(int i=0; i<strip.numPixels(); i++)
  {
    
    for(int j=0; j<strip.numPixels(); j++) 
    {
      k=i+j;
      if(k>15)
      {
        k-=16;
      }
      strip.setPixelColor(j, strip.Color(X[0][k],X[1][k],X[2][k]));
    }
    strip.show();
      delay(50);
    
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
 
// --------------------------------------------------------------
// 気圧を計算する
// --------------------------------------------------------------
unsigned long int calibration_P(signed long int adc_P)
{
  signed long int var1, var2;
  unsigned long int P;
  var1 = (((signed long int)t_fine) >> 1) - (signed long int)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((signed long int)dig_P6);
  var2 = var2 + ((var1 * ((signed long int)dig_P5)) << 1);
  var2 = (var2 >> 2) + (((signed long int)dig_P4) << 16);
  var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((signed long int)dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((signed long int)dig_P1)) >> 15);
  if (var1 == 0)
  {
    return 0;
  }
  P = (((unsigned long int)(((signed long int)1048576) - adc_P) - (var2 >> 12))) * 3125;
  if (P < 0x80000000)
  {
    P = (P << 1) / ((unsigned long int) var1);
  }
  else
  {
    P = (P / (unsigned long int)var1) * 2;
  }
  var1 = (((signed long int)dig_P9) * ((signed long int)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
  var2 = (((signed long int)(P >> 2)) * ((signed long int)dig_P8)) >> 13;
  P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
  return P;
}
 
// --------------------------------------------------------------
// 湿度を計算する
// --------------------------------------------------------------
unsigned long int calibration_H(signed long int adc_H)
{
  signed long int v_x1;
 
  v_x1 = (t_fine - ((signed long int)76800));
  v_x1 = (((((adc_H << 14) - (((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) +
            ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) *
                (((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + (( signed long int)2097152)) *
                ((signed long int) dig_H2) + 8192) >> 14));
  v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
  v_x1 = (v_x1 < 0 ? 0 : v_x1);
  v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
  return (unsigned long int)(v_x1 >> 12);
}
