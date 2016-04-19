Genuino101を使ってBLE経由でRapiroを制御してみる
====
Genuino101 の Bluetooth Low Energy 機能にスマートフォンなどからアクセスしてSerialに二足歩行ロボットRapiro用の制御コマンドを流す

使い方
----

+ `Rapiro_Genuino101_BLE.ino`をGenuino101に書き込む

+ Genuino101とRapiroを接続する

 + VCC,GND,TX,RX <-> VCC,GND,RX,TX

+ スマートフォンなどからGenuino101に接続する
 + Bluetoothデバイス名は`Genuino101-(数値)`や`Rapiro`となっているはずです

+ `UART Service service 1 tx`キャラクタリスティックに書き込んだ値によってRapiroが動作する
 + (例)`S`を送るとRapiroに`#M0`が送られる

対応サービス
----

+ NORDIC UART/Serial

  + [Documents(old)](https://devzone.nordicsemi.com/documentation/nrf51/6.0.0/s110/html/a00066.html)

  + [Documents(latest)](http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v11.0.0%2Fble_sdk_app_nus_eval.html&cp=4_0_0_4_2_2_18)

対応アプリ例
----

 + [Nordic Mobile Apps](https://www.nordicsemi.com/eng/Products/Nordic-mobile-Apps/nRF-Toolbox-App)

関連情報
----

 1. [スイッチサイエンスマガジン](http://mag.switch-science.com/)

 2. [Genuino101](https://www.switch-science.com/catalog/2670/)

 3. [Rapiro](https://www.switch-science.com/catalog/1550/)
