/*
 SPRESENSEアドオン BME280 https://www.switch-science.com/catalog/3928/ 用サンプルソース

 OLEDの制御部分はmgo_tecさんのソースコード
 https://www.mgo-tec.com/blog-entry-ssd1306-revalidation-i2c-esp32-esp8266.html を
 元に、いくつかのパラメータをこのOLED用に変更しています。

 Font dataは https://github.com/emutyworks/8x8DotJPFont のものを抜粋して使っています。

*/

#include "Wire.h"
#include <SSCI_BME280.h>

SSCI_BME280 bme280;
const uint8_t BME_i2c_addr = 0x76;        //I2C Address
const uint8_t ADDRES_OLED =  0x3C;
const int SDA_OLED =  5;
const int SCL_OLED =  4;
const uint32_t Frequensy_OLED = 400000; //Max=400kHz


// Font data from https://github.com/emutyworks/8x8DotJPFont
uint8_t Font[17][8] = {
  { 0x00, 0x3e, 0x41, 0x41, 0x41, 0x41, 0x3e, 0x00 }, /* 0x39 ０ */
  { 0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00 }, /* 0x3A １ */
  { 0x00, 0x62, 0x51, 0x51, 0x49, 0x49, 0x46, 0x00 }, /* 0x3B ２ */
  { 0x00, 0x22, 0x41, 0x49, 0x49, 0x49, 0x36, 0x00 }, /* 0x3C ３ */
  { 0x00, 0x30, 0x28, 0x24, 0x22, 0x7f, 0x20, 0x00 }, /* 0x3D ４ */
  { 0x00, 0x2f, 0x45, 0x45, 0x45, 0x45, 0x39, 0x00 }, /* 0x3E ５ */
  { 0x00, 0x3e, 0x49, 0x49, 0x49, 0x49, 0x32, 0x00 }, /* 0x3F ６ */
  { 0x00, 0x01, 0x01, 0x61, 0x19, 0x05, 0x03, 0x00 }, /* 0x40 ７ */
  { 0x00, 0x36, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00 }, /* 0x41 ８ */
  { 0x00, 0x26, 0x49, 0x49, 0x49, 0x49, 0x3e, 0x00 }, /* 0x42 ９ */
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* 0x00    */
  { 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* 0x04 ． */
  { 0x02, 0x05, 0x02, 0x3c, 0x42, 0x42, 0x24, 0x00 }, /* 0x1B ℃ */
  { 0x42, 0x25, 0x12, 0x08, 0x24, 0x52, 0x21, 0x00 }, /* 0x20 ％ */
  { 0x3f, 0x04, 0x38, 0x00, 0x3f, 0x09, 0x06, 0x00 },  /* 0x48 h *//* 0x30 P */
  { 0x18, 0x24, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* 0x41 a */
  { 0xff, 0xfb, 0x88, 0xc0, 0xe0, 0xf6, 0xff, 0xff }  /* 0x41 Logo */
};

void setup() {
  Serial.begin(115200);

  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t bme280mode = 3;         //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable


  Wire.begin();
  bme280.setMode(BME_i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);
  bme280.readTrim();
  delay(100);

  SSD1306_Init(); //OLED ssd1306 初期化
  delay(10);
  Clear_Display_All();
  //  Display_Pic();
  //  Clear_Display_All();
  title_disp();
}

void loop() {
  double temp_act, press_act, hum_act; //最終的に表示される値を入れる変数
  bme280.readData(&temp_act, &press_act, &hum_act);

  temp_disp(temp_act);
  press_disp(press_act);
  hum_disp(hum_act);
  //  Serial.print("TEMP : ");
  //  Serial.print(temp_act);
  //  Serial.print(" DegC  PRESS : ");
  //  Serial.print(press_act);
  //  Serial.print(" hPa  HUM : ");
  //  Serial.print(hum_act);
  //  Serial.println(" ");
  delay(1000);

}
//*******************************************

void disp_char(int fontNum) {
  uint8_t b = 0, dummy = 0;
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

  for (int m = 0; m < 8; m++) {
    b = Font[fontNum][m];
    Wire.write(b); //SSD1306のGDRAM にデータ書き込み
  }
  Wire.endTransmission(); //これが送信されて初めてディスプレイに表示される
}

//*******************************************
void title_disp() {
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xB0 | 0); //set page start address←垂直開始位置はここで決める(B0～B7)
  Wire.write((byte)0b00000000);
  Wire.write(0x21); //set Column Address
  Wire.write((byte)(8 * 0)); //Column Start Address←水平開始位置はここで決める(0～127)
  Wire.write(127); //Column Stop Address　画面をフルに使う
  Wire.endTransmission();
  disp_char(16);
}

//*******************************************
void temp_disp(double temp_act) {
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xB0 | 1); //set page start address←垂直開始位置はここで決める(B0～B7)
  Wire.write((byte)0b00000000);
  Wire.write(0x21); //set Column Address
  Wire.write((byte)8 * 3); //Column Start Address←水平開始位置はここで決める(0～127)
  Wire.write(127); //Column Stop Address　画面をフルに使う
  Wire.endTransmission();

  String tempStr = String(temp_act, 2);
  for (int i = 0; i < tempStr.length(); i++) {
    char c = tempStr.charAt(i);
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        disp_char(c - '0');
        break;
      case '.':
        disp_char(11);
        break;
    }
  }
  disp_char(12); // ℃
  Serial.println(tempStr);
}
//*******************************************
void hum_disp(double hum_act) {
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xB0 | 2); //set page start address←垂直開始位置はここで決める(B0～B7)
  Wire.write((byte)0b00000000);
  Wire.write(0x21); //set Column Address
  Wire.write((byte)8 * 3); //Column Start Address←水平開始位置はここで決める(0～127)
  Wire.write(127); //Column Stop Address　画面をフルに使う
  Wire.endTransmission();

  String humStr = String(hum_act, 2);
  for (int i = 0; i < humStr.length(); i++) {
    char c = humStr.charAt(i);
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        disp_char(c - '0');
        break;
      case '.':
        disp_char(11);
        break;
    }
  }
  disp_char(13); // ％
}
//*******************************************
void press_disp(double press_act) {
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xB0 | 3); //set page start address←垂直開始位置はここで決める(B0～B7)
  Wire.write((byte)0b00000000);
  Wire.write(0x21); //set Column Address
  Wire.write((byte)8 * 1); //Column Start Address←水平開始位置はここで決める(0～127)
  Wire.write(127); //Column Stop Address　画面をフルに使う
  Wire.endTransmission();

  String pressStr = String(press_act, 2);
  for (int i = 0; i < pressStr.length(); i++) {
    char c = pressStr.charAt(i);
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        disp_char(c - '0');
        break;
      case '.':
        disp_char(11);
        break;
    }
  }
  disp_char(14); // hP
  disp_char(15); // a
}
//*******************************************
void Display_Pic() {
  int i, j, m, n;
  uint8_t b = 0, dummy = 0;

  for (i = 0; i < 4; i++) {
    Wire.beginTransmission(ADDRES_OLED);
    Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
    Wire.write(0xB0 | i); //set page start address←垂直開始位置はここで決める(B0～B7)
    Wire.write((byte)0b00000000);
    Wire.write(0x21); //set Column Address
    Wire.write((byte)0); //Column Start Address←水平開始位置はここで決める(0～127)
    Wire.write(127); //Column Stop Address　画面をフルに使う
    Wire.endTransmission();

    for (j = 0; j < 16; j++) { //column = 8byte x 16 ←8バイト毎に水平に連続で16個表示
      Wire.beginTransmission(ADDRES_OLED);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

      for (m = 7; m >= 0; m--) {
        for (n = 0; n < 8; n++) { //描画1バイトを縦列に変換
          dummy = ( Font[0][n] >> m ) & 0x01;
          if (dummy > 0) {
            b = b | (dummy << n);
          }
        }
        Wire.write(b); //SSD1306のGDRAM にデータ書き込み
        b = 0;
      }
      Wire.endTransmission(); //これが送信されて初めてディスプレイに表示される
      delay(500);
    }
  }
}
//**************************************************
void Clear_Display_All() {
  uint8_t i, j, k;

  for (i = 0; i < 8; i++) { //Page(0-7)
    Wire.beginTransmission(ADDRES_OLED);
    Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
    Wire.write(0xB0 | i); //set page start address(B0～B7)
    Wire.write((byte)0b00000000);
    Wire.write(0x21); //set Column Address
    Wire.write((byte)0); //Column Start Address(0-127)
    Wire.write(127); //Column Stop Address(0-127)
    Wire.endTransmission();

    for (j = 0; j < 16; j++) { //column = 8byte x 16
      Wire.beginTransmission(ADDRES_OLED);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data)
      for (k = 0; k < 8; k++) { //continue to 31byte
        Wire.write(byte(0x00));
      }
      Wire.endTransmission();
    }
  }
}
//*******************************************
void SSD1306_Init() {
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xAE); //display off
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0xD5);
  Wire.write(0x80);
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0xA8); //Set Multiplex Ratio  0xA8, 0x1F
  Wire.write(0x1F); //64MUX
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0xD3); //Set Display Offset 0xD3, 0x00
  Wire.write((byte)0x00);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0x40); //Set Display Start Line 0x40
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0xA1); //Set Segment re-map 0xA0/0xA1
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0xC8); //Set COM Output Scan Direction 0xC0,/0xC8
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0xDA); //Set COM Pins hardware configuration 0xDA, 0x02
  Wire.write(0b00000010);
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0x81); //Set Contrast Control 0x81, default=0x7F
  Wire.write(0x8f); //0-255
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0xA4); //Disable Entire Display On
  Wire.write((byte)0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0xA6); //Set Normal Display 0xA6, Inverse display 0xA7
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0xD5); //Set Display Clock Divide Ratio/Oscillator Frequency 0xD5, 0x80
  Wire.write(0b10000000);
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0x20); //Set Memory Addressing Mode
  Wire.write(0x10); //Page addressing mode
  Wire.endTransmission();
  Wire.beginTransmission(ADDRES_OLED);
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0x22); //Set Page Address
  Wire.write((byte)0); //Start page set
  Wire.write(7); //End page set
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write((byte)0); //Column Start Address
  Wire.write(127); //Column Stop Address
  Wire.write((byte)0b00000000); //control byte, Co bit = 0 (continue), D/C# = 0 (command)
  Wire.write(0x8D); //Set Enable charge pump regulator 0x8D, 0x14
  Wire.write(0x14);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
  Wire.write(0xAF); //Display On 0xAF
  Wire.endTransmission();
}

