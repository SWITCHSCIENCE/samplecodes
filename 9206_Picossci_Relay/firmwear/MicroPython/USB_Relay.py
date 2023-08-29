#=====================================================================
#    Picossci Relay
#    Sample program [MicroPython]
#    O.Aoki [ switch-science ]
#    2023-08-25
#=====================================================================

from machine import Pin, UART, Timer
import micropython
import time
import sys
import uselect

#=====================================================================
#  I/O
#=====================================================================
# --- LED (GPIO)
led1 = Pin(25, Pin.OUT)
led2 = Pin(14, Pin.OUT)
led3 = Pin(15, Pin.OUT)

# --- Switch (GPIO)
sw1 = Pin(24, Pin.IN)
sw2 = Pin(23, Pin.IN)
sw3 = Pin(22, Pin.IN)

# --- Relay (GPIO)
relay1 = Pin(17, Pin.OUT)
relay2 = Pin(16, Pin.OUT)

# --- Analog-In (GPIO)
ain0 = machine.ADC(0)
ain1 = machine.ADC(1)
ain2 = machine.ADC(2)
ain3 = machine.ADC(3)

# UART
#uart0 = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))
spoll = uselect.poll()
spoll.register(sys.stdin, uselect.POLLIN)

# Timer
beetTimer = Timer()
timerFlag = False

#=====================================================================
#  Timer Sub Function
#=====================================================================
def beetFunc(beetTimer):
    global timerFlag

    timerFlag = True
    
#=====================================================================
#  Initialize
#=====================================================================
# --- LED port (GPIO)
led1.value(1)
led2.value(1)
led3.value(1)

# --- Relay port (GPIO)
relay1.value(0)
relay2.value(0)

# --- relay registert (global)
relay1reg = 0;
relay2reg = 0;
# --- led registert (global)
led1reg = 0;
led2reg = 0;
led3reg = 0;
LEDflag = 1;
# --- sw input registert (global)
sw1d = 0xFF
sw2d = 0xFF
sw3d = 0xFF
# --- USB-serial work (global)
rbuff = ''
rstr = []
# --- time counter (global)
cnt = 0

#=====================================================================
#  Serial sub routine
#=====================================================================
#----- 整数値かどうかを判定 -----#
def isint(s):
    try:
        int(s, 10)  # 試しにint関数で文字列を整数に変換
    except ValueError:
        return False  # 失敗すれば False
    else:
        return True  # 上手くいけば True

#----- String Receive -----#
def UART0read():
    global rbuff
    global rstr

    if not spoll.poll(0):
        return False

    cha = sys.stdin.read(1).upper()
    if cha == 'W' or cha == 'R' :
        rbuff = cha
    elif cha == '\n' or cha == '\r' :
        rstr = rbuff.split(",")
        rbuff = ''
        return True
    else :
        rbuff += cha

    return False

#----- Command Execute -----#
def ComExe():
    global rstr
    global relay1reg, relay2reg
    global led1reg, led2reg, led3reg, LEDflag

    if len(rstr) == 2 and rstr[0] == 'R':
        cnum = 255
        if isint(rstr[1]) : cnum = int(rstr[1])

        if   cnum ==  1 : print('R,1,'  + str(relay1reg) + '\r\n')        # Read Relay port Register 1
        elif cnum ==  2 : print('R,2,'  + str(relay2reg) + '\r\n')        # Read Relay port Register 2
        elif cnum ==  4 : print('R,4,'  + str(led1reg) + '\r\n')          # Read LED port Register 1
        elif cnum ==  5 : print('R,5,'  + str(led2reg) + '\r\n')          # Read LED port Register 2
        elif cnum ==  6 : print('R,6,'  + str(led3reg) + '\r\n')          # Read LED port Register 3
        elif cnum == 80 : print('R,80,' + str(ain0.read_u16()) + '\r\n')  # Read Analog port 0 (AIN0)
        elif cnum == 81 : print('R,81,' + str(ain1.read_u16()) + '\r\n')  # Read Analog port 1 (AIN1)
        elif cnum == 82 : print('R,82,' + str(ain2.read_u16()) + '\r\n')  # Read Analog port 2 (AIN2)
        elif cnum == 83 : print('R,83,' + str(ain3.read_u16()) + '\r\n')  # Read Analog port 3 (AIN3)
        elif cnum == 90 : print('R,90,' + str(LEDflag) + '\r\n')          # Read LED flag Register

    elif len(rstr) == 3 and rstr[0] == 'W':
        cnum = 255
        cdat = 0
        if isint(rstr[1]) : cnum = int(rstr[1])
        if isint(rstr[2]) : cdat = int(rstr[2])

        if   cnum ==  1 : relay1reg = cdat & 0x01  # Write data to Relay port register 1
        elif cnum ==  2 : relay2reg = cdat & 0x01  # Write data to Relay port register 2
        elif cnum ==  4 : led1reg = cdat & 0x01    # Write data to LED port register 1
        elif cnum ==  5 : led2reg = cdat & 0x01    # Write data to LED port register 2
        elif cnum ==  6 : led3reg = cdat & 0x01    # Write data to LED port register 3
        elif cnum == 11 : relay1reg = 1  # Set Relay port register 1
        elif cnum == 12 : relay2reg = 1  # Set Relay port register 2
        elif cnum == 13 :                # Set all Relay port register
            relay1reg = 1
            relay2reg = 1
        elif cnum == 14 : led1reg = 1    # Set LED port register 1
        elif cnum == 15 : led2reg = 1    # Set LED port register 2
        elif cnum == 16 : led3reg = 1    # Set LED port register 3
        elif cnum == 17 :                # Set all LED port register
            led1reg = 1
            led2reg = 1
            led3reg = 1
        elif cnum == 21 : relay1reg = 0  # Reset Relay port register 1
        elif cnum == 22 : relay2reg = 0  # Reset Relay port register 2
        elif cnum == 23 :                # Reset all Relay port register
            relay1reg = 0
            relay2reg = 0
        elif cnum == 24 : led1reg = 0    # Reset LED port register 1
        elif cnum == 25 : led2reg = 0    # Reset LED port register 1
        elif cnum == 26 : led3reg = 0    # Reset LED port register 1
        elif cnum == 27 :                # Reset all LED port register
            led1reg = 0
            led2reg = 0
            led3reg = 0
        elif cnum == 31 : relay1reg ^= 0x01  # Toggle Relay port register 1
        elif cnum == 32 : relay2reg ^= 0x01  # Toggle Relay port register 2
        elif cnum == 33 :                    # Toggle all Relay port register
            relay1reg ^= 0x01
            relay2reg ^= 0x01
        elif cnum == 34 : led1reg ^= 0x01    # Toggle LED port register 1
        elif cnum == 35 : led2reg ^= 0x01    # Toggle LED port register 2
        elif cnum == 36 : led3reg ^= 0x01    # Toggle LED port register 3
        elif cnum == 37 :                    # Toggle all LED port register
            led1reg ^= 0x01
            led2reg ^= 0x01
            led3reg ^= 0x01
        elif cnum == 90 : LEDflag = cdat & 0x01  # Write data to LED flag register
        elif cnum == 99 :  # All Register Reset
            relay1reg = 0
            relay2reg = 0
            led1reg = 0
            led2reg = 0
            led3reg = 0
            LEDflag = 0

#=====================================================================
#  main loop
#=====================================================================
time.sleep_ms(500)
print('\n\r# Picossci Relay Ready.\n\r')

#----- Start Repeat Timer (10mS) -----#
beetTimer.init(mode=Timer.PERIODIC, period=10, callback=beetFunc )
cnt = 0

while True:
    # -=-=-= 10mS インターバル処理 =-=-=-
    if timerFlag == True:
        sw1d = (sw1d << 1) | (sw1.value() & 0x01)
        sw2d = (sw2d << 1) | (sw2.value() & 0x01)
        sw3d = (sw3d << 1) | (sw3.value() & 0x01)
        if (sw1d & 0x00FF) == 0x0080 : relay1reg ^= 0x01
        if (sw2d & 0x00FF) == 0x0080 : LEDflag ^= 0x01
        if (sw3d & 0x00FF) == 0x0080 : relay2reg ^= 0x01

        relay1.value(relay1reg & 0x01)
        relay2.value(relay2reg & 0x01)

        if LEDflag == 1:
            led1.value((cnt >> 4) & 0x01)
            led2.value((~relay2reg) & 0x01)
            led3.value((~relay1reg) & 0x01)
        else :
            led1.value((~led1reg) & 0x01)
            led2.value((~led2reg) & 0x01)
            led3.value((~led3reg) & 0x01)

        cnt += 1
        timerFlag = False

    # -=-=-= インターバル以外の処理 =-=-=-
    if UART0read() == True :
        ComExe()
