# Sample codes (Picossci_NTSC)

Softwares for "Picossci NTSC"

+ lib ------------------------------ Arduino用ライブラリの格納ディレクトリ
   + Picossci_NTSC ----------------- このフォルダを Arduinoのライブラリフォルダにコピーして使用できます
      + examples ------------------- サンプルコードが格納されているフォルダ
        + HowToUse
          + main.cpp --------------- 基本的な使用方法のサンプルプログラム
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


