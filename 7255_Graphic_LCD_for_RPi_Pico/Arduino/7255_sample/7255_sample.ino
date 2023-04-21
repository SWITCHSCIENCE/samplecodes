#include <Arduino.h>
#include <SPI.h>

static constexpr const uint8_t PICO_AQM1248_PIN_MISO =  4;
static constexpr const uint8_t PICO_AQM1248_PIN_CS   =  5;
static constexpr const uint8_t PICO_AQM1248_PIN_SCK  =  6;
static constexpr const uint8_t PICO_AQM1248_PIN_MOSI =  7;
static constexpr const uint8_t PICO_AQM1248_PIN_DC   =  9;
static constexpr const uint8_t PICO_AQM1248_PIN_RST  = 10;

static constexpr const uint8_t PICO_AQM1248_PIN_LED1 = 16;
static constexpr const uint8_t PICO_AQM1248_PIN_LED2 =  8;

static constexpr const uint8_t PICO_AQM1248_PIN_SW1  =  0;
static constexpr const uint8_t PICO_AQM1248_PIN_SW2  =  1;
static constexpr const uint8_t PICO_AQM1248_PIN_SW3  =  2;
static constexpr const uint8_t PICO_AQM1248_PIN_SW4  =  3;
static constexpr const uint8_t PICO_AQM1248_PIN_SW_A = 11;  // up
static constexpr const uint8_t PICO_AQM1248_PIN_SW_B = 13;  // right
static constexpr const uint8_t PICO_AQM1248_PIN_SW_C = 15;  // left
static constexpr const uint8_t PICO_AQM1248_PIN_SW_D = 14;  // down
static constexpr const uint8_t PICO_AQM1248_PIN_SW_P = 12;  // push

static constexpr const uint8_t pic[6][128] ={{0,0,0,0,128,192,192,224,224,224,224,224,224,224,224,224,224,96,0,0,0,0,0,0,0,0,0,0,0,0,128,224,224,224,224,224,224,224,224,192,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,31,3,0,0,0,0,0,0,0,0,0,0,224,248,254,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,60,126,126,231,231,199,199,206,142,140,0,0,3,63,255,252,0,0,240,254,30,62,248,240,0,0,254,255,15,0,0,255,255,255,0,7,7,7,7,255,255,255,7,7,7,7,0,248,252,254,15,7,7,7,7,15,30,28,0,0,0,255,255,255,224,224,224,224,255,255,255,0,0,0,0,0,0,0},
{0,0,0,0,255,255,255,255,255,255,255,255,191,135,128,128,128,128,128,128,128,128,0,0,0,0,7,7,7,7,7,135,199,231,247,251,255,255,255,255,255,255,0,0,0,0,0,0,0,0,12,28,60,56,48,48,49,57,63,31,15,0,0,0,7,63,62,62,63,7,0,0,7,63,62,62,63,7,0,0,0,63,63,63,0,0,0,0,0,63,63,63,0,0,0,0,0,3,15,31,28,56,56,56,56,28,30,14,0,0,0,63,63,63,0,0,0,0,63,63,63,0,0,0,0,0,0,0},
{0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,127,31,3,0,0,128,192,224,240,248,252,254,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,224,176,24,8,8,8,8,16,48,0,0,0,192,48,16,8,8,8,24,16,96,0,0,0,248,0,0,0,248,8,8,8,8,8,8,8,0,0,0,248,16,96,192,0,0,0,0,0,248,0,0,0,192,48,16,8,8,8,24,16,96,0,0,248,8,8,8,8,8,8,8,0,0,0,0,0,0,0},
{0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,239,227,240,248,252,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,96,193,129,3,2,2,6,132,204,120,0,15,112,192,128,0,0,0,0,192,96,0,0,0,255,0,0,0,255,2,2,2,2,2,2,2,0,0,0,255,0,0,0,3,12,24,96,192,255,0,0,15,112,192,128,0,0,0,0,192,96,0,0,255,2,2,2,2,2,2,2,0,0,0,0,0,0,0},
{0,0,0,0,1,3,3,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,3,3,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
};

SPISettings spi_setting(10000000, MSBFIRST, SPI_MODE3);

#if defined(ARDUINO_ARCH_MBED)
// for mbed official version
  arduino::MbedSPI hw_spi(PICO_AQM1248_PIN_MISO, PICO_AQM1248_PIN_MOSI, PICO_AQM1248_PIN_SCK);
#else
// for earlephilhower version
  SPIClassRP2040 hw_spi(spi0, PICO_AQM1248_PIN_MISO, PICO_AQM1248_PIN_CS, PICO_AQM1248_PIN_SCK, PICO_AQM1248_PIN_MOSI);
#endif

void initLCD(){

  digitalWrite(PICO_AQM1248_PIN_CS,LOW);
  digitalWrite(PICO_AQM1248_PIN_DC,LOW);

  hw_spi.transfer(0xAE);           //Display = OFF
  hw_spi.transfer(0xA0);           //ADC = normal
  hw_spi.transfer(0xC8);           //Common output = revers
  hw_spi.transfer(0xA3);           //LCD bias = 1/7

  //内部レギュレータON
  hw_spi.transfer(0x2C);
  delay(2);
  hw_spi.transfer(0x2E);
  delay(2);
  hw_spi.transfer(0x2F);

  //コントラスト設定
  hw_spi.transfer(0x23);           //Vo voltage regulator internal resistor ratio set
  hw_spi.transfer(0x81);           //Electronic volume mode set
  hw_spi.transfer(0x1C);           //Electronic volume register set

  //表示設定
  hw_spi.transfer(0xA4);           //Display all point ON/OFF = normal
  hw_spi.transfer(0x40);           //Display start line = 0
  hw_spi.transfer(0xA6);           //Display normal/revers = normal
  hw_spi.transfer(0xAF);           //Dsiplay = ON

  digitalWrite(PICO_AQM1248_PIN_CS, HIGH);
}

void setup() {
  pinMode(PICO_AQM1248_PIN_LED1, OUTPUT);
  pinMode(PICO_AQM1248_PIN_LED2, OUTPUT);
  pinMode(PICO_AQM1248_PIN_DC, OUTPUT);
  pinMode(PICO_AQM1248_PIN_CS, OUTPUT);
  pinMode(PICO_AQM1248_PIN_RST, OUTPUT);
  digitalWrite(PICO_AQM1248_PIN_CS, HIGH);
  digitalWrite(PICO_AQM1248_PIN_RST, HIGH);

  hw_spi.begin();

  initLCD();              //LCD初期化
}

void loop() {
  delay(16);

  static uint32_t count = 0;

  count++;

  // スイッチの状態に応じてLED点滅周期を変更する
  uint8_t bits1 = 0;
  if (!digitalRead(PICO_AQM1248_PIN_SW_A)) { bits1 |= 0b00000100; }
  if (!digitalRead(PICO_AQM1248_PIN_SW_B)) { bits1 |= 0b00001000; }
  if (!digitalRead(PICO_AQM1248_PIN_SW_C)) { bits1 |= 0b00010000; }
  if (!digitalRead(PICO_AQM1248_PIN_SW_D)) { bits1 |= 0b00100000; }
  if (!digitalRead(PICO_AQM1248_PIN_SW_P)) { bits1 |= 0b01000000; }
  digitalWrite(PICO_AQM1248_PIN_LED1, !(bits1 & count));

  uint8_t bits2 = 0;
  if (!digitalRead(PICO_AQM1248_PIN_SW1 )) { bits2 |= 0b00000100; }
  if (!digitalRead(PICO_AQM1248_PIN_SW2 )) { bits2 |= 0b00001000; }
  if (!digitalRead(PICO_AQM1248_PIN_SW3 )) { bits2 |= 0b00010000; }
  if (!digitalRead(PICO_AQM1248_PIN_SW4 )) { bits2 |= 0b00100000; }
  digitalWrite(PICO_AQM1248_PIN_LED2, !(bits2 & count));

  // ロゴ画像の表示位置を算出する
  int8_t x_pos = count;
  x_pos = x_pos >= 0 ? x_pos : 0;

  hw_spi.beginTransaction(spi_setting);
  digitalWrite(PICO_AQM1248_PIN_CS,LOW);

  // ロゴ画像の転送処理ループ(縦に分割されているためループで6回分送信する)
  for(int page=0;page<6;page++){
    // 128バイト分のバッファにロゴデータを準備する。(表示位置に変化を加えてスクロールさせる)
    uint8_t buf[128];
    memcpy(buf, &pic[page][x_pos], 128 - x_pos);
    if (x_pos != 0) {
      memcpy(&buf[128 - x_pos], pic[page], x_pos);
    }

    digitalWrite(PICO_AQM1248_PIN_DC,LOW);
    hw_spi.transfer(0xb0+page);        //ページアドレス選択
    hw_spi.transfer(0x10);             //カラムアドレス上位4bit = 0000
    hw_spi.transfer(0x00);             //カラムアドレス下位4bit = 0000

    // ロゴ画像データを転送する
    digitalWrite(PICO_AQM1248_PIN_DC,HIGH);
    hw_spi.transfer(buf, 128);
  }
  digitalWrite(PICO_AQM1248_PIN_CS,HIGH);
  hw_spi.endTransaction();
}
