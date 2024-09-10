## サンプルコード for スイッチサイエンスDRV8311H搭載3相ブラシレスモータードライバモジュール

オープンソースのフィールド指向制御（FOC）ブラリ[Simple FOC](https://docs.simplefoc.com/)を使った[スイッチサイエンスDRV8311H搭載3相ブラシレスモータードライバモジュール](https://www.switch-science.com/products/9877)のサンプルコードです。

- open_loop_velocity_example
  - オープンループ（位置センサーを使わない）の速度制御サンプルです。
  - シリアルコンソールから速度指示を行い任意の速度（ラジアン/秒）でブラシレスモーターを回転させます。
- open_loop_position_example
  - オープンループ（位置センサーを使わない）の位置制御サンプルです。
  - シリアルコンソールから位置指示を行い任意の位置（ラジアン）にブラシレスモーターを回転させます。
  - 1回転は2 * PIラジアンです。
- current_sense_low_side
  - DRV8311Hに内蔵された電流センサを使用して、ブラシレスモーター各相の電流をシリアルプロッターにプロットします。
- full_control_serial
  - 磁気角度センサ（MA735）と電流センサを使用したフルモーションコントロールのサンプルです。
  - シリアルコンソールからコマンド指示で自由にブラシレスモーターを制御できます。
  - コマンドの仕様は[こちら](https://docs.simplefoc.com/commander_motor)を参照してください。

### 動作確認環境

- [スイッチサイエンスDRV8311H搭載3相ブラシレスモータードライバモジュール](https://www.switch-science.com/products/9877)
- [ESPr Developer S3](https://www.switch-science.com/products/8514)
- [ジンバルモーター](https://www.amazon.co.jp/dp/B089SYW5W3)
- Arduino IDE 2.3.2
- arduino-esp32 v3.0.4
- Simple FOC v2.3.4
