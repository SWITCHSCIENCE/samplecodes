#! /usr/bin/env python
# -*- coding: utf-8 -*-
#*************************************************************************************
#	RTC Backup test
#	Created : 2018/09/28 17:15:00
#	Author  : O.aoki
#	[ Raspberry Pi 2 + Conta HAT + 2x2タクトスイッチモジュール ]
#	[ Raspberry Pi 2 : https://www.switch-science.com/catalog/2910/ ]
#	[ Conta HAT : https://www.switch-science.com/catalog/3677/ ]
#	[ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]
#*************************************************************************************

import  time
import  sys
import  smbus

#-------------------------------------------------------------------------------------
#	SW (2x2 Switch) Read
#-------------------------------------------------------------------------------------
bus = smbus.SMBus(1)
add_NCD9830 = 0x48        # ADC の I2C アドレス

# スイッチのアナログ入力
def read_analog(ch):
        cmd = 0x80 + ((ch << 3) & 0x30) + ((ch << 6) & 0x40)
        bus.write_byte(add_NCD9830, cmd)
        data = bus.read_byte(add_NCD9830)
        return data

# スイッチ入力のデコード
def read_sw(ch):
        dat = read_analog(ch)
        if dat < 0x10:             # 0.206V 未満なら SW1 が押された
                return 1	
        elif dat < 0x50:           # 0.206V ～ 1.03V なら SW2 が押された
                return 2
        elif dat < 0x90:           # 1.03V ～ 1.86V なら SW3 が押された 
                return 3
        elif dat < 0xd0:           # 1.86V ～ 2.68V なら SW4 が押された
                return 4
        else:
                return 0           # 2.68V 以上ならスイッチが押されていない

# Conta HAT に載せた場合モジュールの駆動電圧は 3.3V です
# これらの電圧は計算上の大まかな数字です。現物に合わせて
# 微調整してください。

#-------------------------------------------------------------------------------------
#	main 
#-------------------------------------------------------------------------------------
if __name__ == '__main__':
        try:
                while True:
                        sw = read_sw(7)    # M1 のソケットに刺した場合、ADC は 7ch
                        print(sw)
                        time.sleep(0.2)

        except KeyboardInterrupt:
                spi.close()
                sys.exit(0)
