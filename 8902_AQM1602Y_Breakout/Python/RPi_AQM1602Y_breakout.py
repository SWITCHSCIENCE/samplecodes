# -*- coding: utf-8 -*-

import smbus
from time import sleep

#=====================================================================
#  I/O
#=====================================================================
i2c = smbus.SMBus(1)
adrAQM1602 = 0x3E

#=====================================================================
#  AQM1602Y
#=====================================================================
def writeComLCD(com):
    i2c.write_i2c_block_data(adrAQM1602, 0x00, [com & 0xFF])
    sleep(0.001)

def writeDatLCD(dat):
    i2c.write_i2c_block_data(adrAQM1602, 0x40, [dat & 0xFF])
    sleep(0.001)

def clearLCD():
    writeComLCD(0x01)
    sleep(0.001)

def homeLCD():
    writeComLCD(0x02)
    sleep(0.001)

def LocateLCD(x, y):
    if y == 0:
        writeComLCD(0x80 + x)
    else:
        writeComLCD(0xC0 + x)

def putch(ch):
    writeDatLCD(ch)

def writeLineLCD(nL, lin):
    buf = lin.encode('utf-8')
    le = len(lin)
    if le <= 0:
        return False
    if le > 16:
        le = 16
    if nL == 0:
        writeComLCD(0x80)
    else:
        writeComLCD(0xC0)
    for idx in range(0, le):
        writeDatLCD(buf[idx])

def initLCD():
    sleep(0.04)
    writeComLCD(0x38)    # Function Set : 8bit / 2line
    writeComLCD(0x39)    # Function Set : IS=1 [extention mode set]
    writeComLCD(0x14)    # Internal OSC Frequency : BS=0 / F=0x04
    writeComLCD(0x7F)    # Contrast set : Contrast=0x0F [max]
    writeComLCD(0x56)    # Power/ICON/Contrast Control
    writeComLCD(0x6C)    # Follower control : Fon=1 / Rab=0x04
    sleep(0.2)
    writeComLCD(0x38)    # Function Set : IS=0 [extention mode cancel]
    writeComLCD(0x01)    # Clear Display
    writeComLCD(0x0C)    # Display ON
    sleep(0.002)

#=====================================================================
#  main
#=====================================================================

initLCD()

cnt = 0
x = 0
y = 0

writeLineLCD(0, "Hello, World!")
writeLineLCD(1, "0123456789ABCDEF")

sleep(2)

# -=-=-=-=-= loop =-=-=-=-=-
while True:
    if (cnt & 0x0007) == 0:
        LocateLCD(15, 0)
        putch((cnt >> 3) & 0x00FF)
        if (cnt & 0x0080) == 0:
            LocateLCD((cnt >> 3) & 0x0F, 1)
            putch(0x7E)
        else:
            LocateLCD(15 - ((cnt >> 3) & 0x0F), 1)
            putch(0x7F)

    cnt += 1
    sleep(0.1)