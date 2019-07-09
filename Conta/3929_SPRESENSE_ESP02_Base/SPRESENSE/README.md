# SPRESENSE_ESP02_Base

"SPRESENSE_ESP02_Base" is a baseboard equipped with ESP-WROOM-02 for SPRESENSE. The baseboard can also carry a small Conta standard board with sensors etc.


## How to use
[SPRESENSE用 Contaベースボード](https://www.switch-science.com/catalog/3929/) (Japanese)


## Buy
[SPRESENSE用 Contaベースボード](https://www.switch-science.com/catalog/3929/) for Japan  


## Sample codes (Arduino IDE for SPRESENSE)

SPRESENSE
|- 2x2_switch_module    [ 2x2 switch test. Digital out, analog In test. ]
|                         2x2 switch を使ったデジタル出力とアナログ入力のテストプログラムです。
|
|- FeRAM_SPI_breakout   [ FeRAM + 2x2 switch test. SPI connection access test to FeRAM. ]
|                         SPI 接続を使って FeRAM にアクセスするテストプログラムです。
|
|- LED_Bar_module       [ LED bar test. I2C connection access test to LED bar. ]
|                         I2C 接続を使って LED バーモジュール を制御するテストプログラムです。
|
|- RTC_module           [ RTC + 2x2 switch test. I2C connection access test to RTC. ]
|                         I2C 接続を使ってリアルタイムクロック(RTC) にアクセスするテストプログラムです。
|
|- __Serial             [ Serial test. Connect the ESP-WROOM-02 serial to the USB serial via SPRESENSE. ]
                          SPRESENSE を経由してESP-WROOM-02 と USBシリアルを接続するテストプログラムです。