from machine import UART, PWM, Pin
import time

#=====================================================================
#  I/O
#=====================================================================
# led0 = Pin(25, Pin.OUT)
led0 = PWM(Pin(25))
led0.freq(100)

#UART
u0 = UART(0,baudrate=115200,tx=Pin(0),rx=Pin(1))

#=====================================================================
#  Initialize
#=====================================================================

rbuff = bytes()
ldir = 1
ldat = 0
cnt = 0

u0.write('Host sample.\r\n')

#=====================================================================
#  main loop
#=====================================================================

while True:
    if (cnt & 0x007F) == 0:
        u0.write('W,4,2000\r\n')
        u0.write('W,5,2000\r\n')
        u0.write('W,6,2000\r\n')
        u0.write('W,7,2000\r\n')
        u0.write('W,14,0\r\n')
    elif (cnt & 0x007F) == 0x0040:
        u0.write('W,4,5000\r\n')
        u0.write('W,5,5000\r\n')
        u0.write('W,6,5000\r\n')
        u0.write('W,7,5000\r\n')
        u0.write('W,14,1\r\n')

    if (cnt & 0x0FFF) == 0x0020:
        u0.write('W,13,0\r\n')
        u0.write('W,15,0\r\n')
    elif (cnt & 0x0FFF) == 0x0420:
        u0.write('W,13,5000\r\n')
        u0.write('W,15,1\r\n')
    elif (cnt & 0x0FFF) == 0x0820:
        u0.write('W,13,0\r\n')
        u0.write('W,15,0\r\n')
    elif (cnt & 0x0FFF) == 0x0C20:
        u0.write('W,13,-5000\r\n')
        u0.write('W,15,1\r\n')

    if ldir == 1:
        ldat += 0x1111
        if ldat > 0xFFFF:
            ldat = 0xFFFF
            ldir = 0
    else :
        ldat -= 0x1111
        if ldat < 0:
            ldat = 0
            ldir = 1
    led0.duty_u16(ldat)

    cnt += 1
    time.sleep(0.01)