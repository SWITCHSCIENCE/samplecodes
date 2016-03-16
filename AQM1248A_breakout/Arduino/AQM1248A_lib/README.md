AQM1248A Simple Library
====
AQM1248A 小型グラフィック液晶をArduinoで扱うためのライブラリ

使い方
----

 + [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) の導入

 + `#include "AQM1248A.h"`をIDEの先頭に追加

サンプル
----

 + `examples/graphictest/graphictest.ino`
  円、三角形、角丸四角形、sin波の表示

注意
----

 + 本液晶は表示用RAMからデータを読み取れないため、プログラム内部に仮想的に表示画面を用意している

 + この仮想画面がおよそ800バイトを占める

 + 仮想画面を液晶に反映させるために、`showPic()`を呼ぶ必要がある


関連情報
----

1. [AQM1248A小型グラフィック液晶ボード](https://www.switch-science.com/catalog/2608/)
