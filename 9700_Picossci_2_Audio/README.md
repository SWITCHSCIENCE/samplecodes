# Sample codes (Picossci_2_Audio)

Softwares for "Picossci 2 Audio"

+ lib ------------------------------ Arduino用ライブラリの格納ディレクトリ
   + Picossci_2_Audio -------------- このフォルダを Arduinoのライブラリフォルダにコピーして使用できます
      + examples ------------------- サンプルコードが格納されているフォルダ
        + Sound
          + main.cpp --------------- I2S音声出力のサンプルコード
+ platformio.ini ------------------- PlatformIOでのビルド用の定義

### 使用方法

このサンプルコードは二種類のボードマネージャに対応しています。
 - `Arduino` 公式の mbed 版
 - `earlephilhower` 氏の FreeRTOS 版 ( https://github.com/earlephilhower/arduino-pico )

このサンプルコードは Arduino ライブラリの形式になっています。
ライブラリの使用方法は `examples/Sound/main.cpp` 内にコメントとして記載しています。

### Arduino IDEで使用する場合

`/lib/Picossci_2_Audio` フォルダを Arduino のライブラリフォルダにコピーしてご利用ください。


### PlatformIO で使用する場合

必要に応じてご自身のプロジェクトフォルダに `/lib/Picossci_2_Audio` をコピーしてご利用ください。

なお、このフォルダをそのまま VisualStudioCode で開くことで、サンプルコードをビルドして試すことができます。

### GPIO割当ピンマップ

| GPIO | 機能 |
|------|------|
| 1 | I2S_DOUT |
| 2 | I2S_BCLK |
| 3 | I2S_LRCLK |
| 4 | I2C0_SDA |
| 5 | I2C0_SCL |
| 10 | SPI1_SCK |
| 11 | SPI1_TX |
| 12 | SPI1_RX |
| 13 | SD_CS |
| 16 | SW1-1 |
| 17 | SW1-T |
| 18 | SW1-2 |
| 19 | LCD_RS (D/C) |
| 20 | SPI0_RX |
| 21 | LCD_CS |
| 22 | SPI0_SCK |
| 23 | SPI0_TX |
| 26-29 | ピンヘッダJP4 |
