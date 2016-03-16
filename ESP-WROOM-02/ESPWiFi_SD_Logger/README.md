ESP-WROOM-02を使ったWiFi機能付き温度ロガー
====
ESP-WROOM-02にSDカードおよびBME280(温度,気温,湿度センサ)を接続し、WEBブラウザから記録された各データをグラフ表示

使い方
----

+ [Arduino Time Library](http://playground.arduino.cc/code/time)を別途導入する

+ `/SDcard` の中身をすべて接続するSDカードのルートにコピーする

+ `ESPWiFi_SD_Logger.ino`を書き込む

+ IDEのシリアルモニタからIPアドレスを確認

+ `http://(IP)/graph.htm` でグラフ画面にアクセス

 + 表示するファイルの変更が可能

 + ファイル名は例えば2016年2月の場合、`2016-02.csv`となるので`2016-02`と画面には入力

 + サンプルデータとして`2016-02.csv`を同梱

関連情報
----

 1. [スイッチサイエンスマガジン](http://mag.switch-science.com/)

 2. [ESP-WROOM-02 開発ボード](https://www.switch-science.com/catalog/2500/)
