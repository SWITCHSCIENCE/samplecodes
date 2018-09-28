#! /usr/bin/env python
# -*- coding: utf-8 -*-
#*************************************************************************************
#	RTC Backup test
#	Created : 2018/09/28 17:55:00
#	Author  : O.aoki
#	[ Raspberry Pi 2 + Conta pHAT + 2x2タクトスイッチモジュール ]
#	[ Raspberry Pi 2 : https://www.switch-science.com/catalog/2910/ ]
#	[ Conta pHAT : https://www.switch-science.com/catalog/3678/ ]
#	[ 2x2タクトスイッチモジュール : https://www.switch-science.com/catalog/3680/ ]
#	[ 1Mbit FeRAM : https://www.switch-science.com/catalog/3682/ ]
#*************************************************************************************

import  time
import  sys
import  smbus
import  spidev

#-------------------------------------------------------------------------------------
#	FeRAM Tools

spi = spidev.SpiDev()
CMD_WREN = 0x06
CMD_WRDI = 0x04
CMD_RDSR = 0x05
CMD_WRSR = 0x01
CMD_READ = 0x03
CMD_WRITE = 0x02

# FeRAM 初期化
def feram_init():
        spi.open(0,0)
        spi.max_speed_hz = 1000000
        spi.bits_per_word = 8
        spi.mode = 3

        spi.xfer2([CMD_WREN])
        time.sleep(0.01)

# FeRAM バイトライト
def feram_write(addr, data):
        spi.xfer2([CMD_WREN])
        spi.xfer2([CMD_WRITE, (addr & 0x00FF0000) >> 16, (addr & 0x0000FF00) >> 8, (addr & 0x000000FF), data])

# FeRAM バイトリード
def feram_read(addr):
        ret = spi.xfer2([CMD_READ, (addr & 0x00FF0000) >> 16, (addr & 0x0000FF00) >> 8, (addr & 0x000000FF), 0])
        return ret

# FeRAM ブロックライト
def feram_write_block(addr, data):
        buff = [CMD_WRITE, (addr & 0x00FF0000) >> 16, (addr & 0x0000FF00) >> 8, (addr & 0x000000FF)]
        buff.extend(data)
        spi.xfer2([CMD_WREN])
        spi.xfer2(buff)

# FeRAM ブロックリード
def feram_read_block(addr, len):
        buff = [CMD_READ, (addr & 0x00FF0000) >> 16, (addr & 0x0000FF00) >> 8, (addr & 0x000000FF)]
        buff.extend(range(len))
        dat = spi.xfer2(buff)
        ret = dat[4:]
        return ret

#-------------------------------------------------------------------------------------
#	SW (2x2 Switch) Tools

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
                feram_init()
                addr = 0x00000000
                sw = 0
                dsw = 0
                cnt = 0

                while True:
                        dsw = sw
                        sw = read_sw(0)
                        if dsw == 0:
                                # SW1 が押されたとき addr 番地から 256 バイト表示する
                                if sw == 1:
                                        print "-- Memory Dump ------"
                                        for i in range(0, 0x100, 0x10):
                                                dat = feram_read_block(addr + i, 16)
                                                print "{:06X} :".format((addr + i)),
                                                for j in range(16):
                                                        print "{:02X}".format(dat[j]),
                                                print ""
                                        print ""
                                # SW2 が押されたとき addr 番地から 256 バイト書き込む
                                elif sw == 2:
                                        print "-- Memory Write ------"
                                        for i in range(0, 0x100, 0x10):
                                                print "{:06X} :".format((addr + i)),
                                                buff = []
                                                for j in range(16):
                                                        buff.append((cnt + i + j) & 0x000000FF)
                                                        print "{:02X}".format(buff[j]),
                                                feram_write_block(addr + i, buff)
                                                print ""
                                        print ""
                                # SW3 が押されたとき addr の値を 0x00001000 増やす
                                elif sw == 3:
                                        print "-- Address Increment ------"
                                        addr = (addr + 0x00001000) & 0x0001FFFF
                                        print "addr = {:06X}".format(addr)
                                        print ""
                                # SW4 が押されたとき FeRAM 全体にテストパターンを書き込む
                                elif sw == 4:
                                        print "-- Test Pattern Write ------"
                                        cn = cnt
                                        for addr in range(0, 0x00020000, 0x00001000):
                                                buff = []
                                                for i in range(16):
                                                        buff.append((cn + i) & 0x000000FF)
                                                buff[11] = buff[10]
                                                buff[12] = 0
                                                buff[13] = (addr >> 16) & 0x000000FF
                                                buff[14] = (addr >>  8) & 0x000000FF
                                                buff[15] = (addr >>  0) & 0x000000FF
                                                cn += 1
                                                feram_write_block(addr, buff)
                                                print "addr = {0:06X} : {1:02X}".format(addr, buff[0])
                                        addr = 0
                                        print ""
                        cnt += 1
        except KeyboardInterrupt:
                print('\n')
                spi.close()
                sys.exit(0)

