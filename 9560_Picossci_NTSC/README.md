# Sample codes (Picossci_NTSC)

Softwares for "Picossci NTSC"

+ lib ------------------------------ Arduino用ライブラリの格納ディレクトリ
   + Picossci_NTSC ----------------- このフォルダを Arduinoのライブラリフォルダにコピーして使用できます
      + examples ------------------- サンプルコードが格納されているフォルダ
        + HowToUse
          + main.cpp --------------- 基本的な使用方法のサンプルコード
        + NoLibrarySample
          + main.cpp --------------- ライブラリを使用せずユーザーコードのみで映像と音声を出力するサンプルコード 
+ platformio.ini ------------------- PlatformIOでのビルド用の定義

### 使用方法

このこのサンプルコードは二種類のボードマネージャに対応しています。
 - `Arduino` 公式の mbed 版
 - `earlephilhower` 氏の FreeRTOS 版 ( https://github.com/earlephilhower/arduino-pico )

このサンプルコードは Arduino ライブラリの形式になっています。
ライブラリの使用方法は `examples/HowToUse/main.cpp` 内にコメントとして記載しています。

### Arduino IDEで使用する場合

`/lib/Picossci_NTSC` フォルダを Arduino のライブラリフォルダにコピーしてご利用ください。


### PlatformIO で使用する場合

必要に応じてご自身のプロジェクトフォルダに `/lib/Picossci_NTSC` をコピーしてご利用ください。

なお、このフォルダをそのまま VisualStudioCode で開くことで、サンプルコードをビルドして試すことができます。

### GPIO割当ピンマップ

0-7 = R2R DAC
8 = LCD RS
9 = LCD CS
10 = SPI1 SCK
11 = SPI1 MOSI TX
12 = SPI1 MISO RX
13 = SD CS
14 = LED 2 (PWM7A)
15 = LED 3 (PWM7B)
16 = I2S BCLK
17 = I2S LRCLK
18 = I2S DOUT / JP1
19-21 = JP1
22 = SW 3
23 = SW 2
24 = SW 1
25 = LED 1 (PWM4B)
26-29 = JP1
