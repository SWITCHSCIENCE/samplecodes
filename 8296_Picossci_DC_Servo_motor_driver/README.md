# Sample codes (Picossci DC Servo motor driver)

Softwares for Picossci DC Servo motor driver

+ example -------------------------- Picossci DC Servo motor driver のホスト側サンプルプログラムの格納ディレクトリ
   +  RPi_4_Host.py ------------------- Raspberry Pi 4 用 サーボx4、DCモーターx1、LEDx2を制御するホスト側サンプルプログラム（Python3）
   +  RPi_Pico_Host.py ---------------- Raspberry Pi Pico 用 サーボx4、DCモーターx1、LEDx2を制御するホスト側サンプルプログラム（MicroPython）

+ firmwear ------------------------- Picossci DC Servo motor driver の初期ファームウェアの格納ディレクトリ
   + ArduinoIDE ----------------------- Arduino言語版
       + MotorCommandHAT.ino -------------- 初期ファームウェア (Arduino言語版)
       + MotorCommandHAT.ino.generic.uf2 -- ファームウェアバイナリー
   + MicroPython ---------------------- MicroPython言語版
       + MotorCommandHAT.py --------------- 初期ファームウェア (MicroPython版)
       + rp2-pico-20220618-v1.19.1.uf2 ---- RP2040 MicroPython
   + Command.txt ---------------------- 初期ファームウェアの制御コマンドについて
