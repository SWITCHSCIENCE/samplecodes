from machine import Pin, UART, Timer
import micropython,time

#=====================================================================
#  I/O
#=====================================================================
# GPIO (LED)
led0 = Pin(25, Pin.OUT)
led1 = Pin(24, Pin.OUT)

# Analog-In (SW array)
ain0 = machine.ADC(0)
ain1 = machine.ADC(1)
ain2 = machine.ADC(2)
ain3 = machine.ADC(3)

# GPIO (Matrix LED : Row)
ROW0 = Pin(23, Pin.OUT)
ROW1 = Pin(22, Pin.OUT)
ROW2 = Pin(21, Pin.OUT)
ROW3 = Pin(20, Pin.OUT)
ROW4 = Pin(19, Pin.OUT)
ROW5 = Pin(18, Pin.OUT)
ROW6 = Pin(17, Pin.OUT)
ROW7 = Pin(16, Pin.OUT)

# GPIO (Matrix LED : Collom)
COL0 = Pin(15, Pin.OUT)
COL1 = Pin(14, Pin.OUT)
COL2 = Pin(13, Pin.OUT)
COL3 = Pin(12, Pin.OUT)
COL4 = Pin(11, Pin.OUT)
COL5 = Pin(10, Pin.OUT)
COL6 = Pin( 9, Pin.OUT)
COL7 = Pin( 8, Pin.OUT)

# UART
uart0 = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))

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
led0.value(1)
led1.value(1)

ROW0.value(1)
ROW1.value(1)
ROW2.value(1)
ROW3.value(1)
ROW4.value(1)
ROW5.value(1)
ROW6.value(1)
ROW7.value(1)

COL0.value(0)
COL1.value(0)
COL2.value(0)
COL3.value(0)
COL4.value(0)
COL5.value(0)
COL6.value(0)
COL7.value(0)

bdat = [0,0,0,0,0,0,0,0]
scan = 0
rbuff = ''
rstr = []
testFlag = 1

#=====================================================================
#  Serial sub routine
#=====================================================================
#----- 整数値かどうかを判定 -----#
def isint(s):
    try:
        int(s, 10)  # 試しにint関数で文字列を変換
    except ValueError:
        return False  # 失敗すれば False
    else:
        return True  # 上手くいけば True

#----- String Receive -----#
def UART0read():
    global rbuff
    global rstr

    while uart0.any():
        cha = uart0.read(1)
        if cha >= b'a' and cha <= b'z':
            cha = cha.upper()

        if cha == b'R' or cha == b'W':
            rbuff = ''
            rbuff += str(cha, 'utf-8')
        elif cha == b'\r' or cha == b'\n':
            rstr = rbuff.split(",")
            return True
        else :
            rbuff += str(cha, 'utf-8')

    return False

#----- Command Execute -----#
def ComExe():
    global bdat
    global rstr
    global testFlag

    if len(rstr) == 2 and rstr[0] == 'R':
        if isint(rstr[1]) :
            cnum = int(rstr[1])
        else :
            cnum = 255

        if cnum == 0:
            uart0.write('R,0,' + hex(ain0.read_u16()) + '\r\n')
        elif cnum == 1:
            uart0.write('R,1,' + hex(ain1.read_u16()) + '\r\n')
        elif cnum == 2:
            uart0.write('R,2,' + hex(ain2.read_u16()) + '\r\n')
        elif cnum == 3:
            uart0.write('R,3,' + hex(ain3.read_u16()) + '\r\n')

    elif len(rstr) == 3 and rstr[0] == 'W':
        testFlag = 0
        if isint(rstr[1]) :
            cnum = int(rstr[1])
        else :
            cnum = 255
        if isint(rstr[2]) :
            cdat = int(rstr[2])
        else :
            cdat = 0

        if cnum >= 0 and cnum <= 63:
            if (cdat & 0x01) == 1:
                bdat[(cnum >> 3) & 0x07] |= 0x01 << (cnum & 0x07)
            else :
                bdat[(cnum >> 3) & 0x07] &= ~(0x01 << (cnum & 0x07))
        elif cnum >= 70 and cnum <= 77:
            bdat[cnum - 70] = cdat
        elif cnum == 90:
            led0.value(1 - (cdat & 0x01))
        elif cnum == 91:
            led1.value(1 - (cdat & 0x01))
        elif cnum == 98:
            if (cdat & 0x01) == 1:
                led0.value(0)
                led1.value(0)
                for i in range(8):
                    bdat[i] = 0xFF
            else :
                led0.value(1)
                led1.value(1)
                for i in range(8):
                    bdat[i] = 0
        elif cnum == 99:
            if cdat == 1:
                testFlag = 1
            elif cdat == 2:
                testFlag = 2

#=====================================================================
#  Matrix LED Sub routine
#=====================================================================
#----- Row-pins Encoder -----#
def RowDrive(dat):
    ROW0.value(1)
    ROW1.value(1)
    ROW2.value(1)
    ROW3.value(1)
    ROW4.value(1)
    ROW5.value(1)
    ROW6.value(1)
    ROW7.value(1)
    if (dat & 0x07) == 0:
        ROW0.value(0)
    elif (dat & 0x07) == 1:
        ROW1.value(0)
    elif (dat & 0x07) == 2:
        ROW2.value(0)
    elif (dat & 0x07) == 3:
        ROW3.value(0)
    elif (dat & 0x07) == 4:
        ROW4.value(0)
    elif (dat & 0x07) == 5:
        ROW5.value(0)
    elif (dat & 0x07) == 6:
        ROW6.value(0)
    elif (dat & 0x07) == 7:
        ROW7.value(0)

#----- Column-pins Output -----#
def ColumnDrive(dat):
    COL0.value(0)
    COL1.value(0)
    COL2.value(0)
    COL3.value(0)
    COL4.value(0)
    COL5.value(0)
    COL6.value(0)
    COL7.value(0)
    if (dat & 0x01) == 0x01:
        COL0.value(1)
    if (dat & 0x02) == 0x02:
        COL1.value(1)
    if (dat & 0x04) == 0x04:
        COL2.value(1)
    if (dat & 0x08) == 0x08:
        COL3.value(1)
    if (dat & 0x10) == 0x10:
        COL4.value(1)
    if (dat & 0x20) == 0x20:
        COL5.value(1)
    if (dat & 0x40) == 0x40:
        COL6.value(1)
    if (dat & 0x80) == 0x80:
        COL7.value(1)

#----- Matrix-LED Main Drive -----#
def DriveBitMap():
    global scan

    ROW0.value(1)
    ROW1.value(1)
    ROW2.value(1)
    ROW3.value(1)
    ROW4.value(1)
    ROW5.value(1)
    ROW6.value(1)
    ROW7.value(1)
    ColumnDrive(bdat[scan])
    RowDrive(scan)
    scan = (scan + 1) & 0x07

#----- Test Motion 1 : WaveFLow -----#
def WaveFlow(tmp):
    if (tmp & 0x0000001F) == 0:
        for i in range(7, 0, -1):
            bdat[i] = bdat[i - 1]
        tmp >>= 5;
        if (tmp & 0x00000008) == 0:
            bdat[0] = 0x01 << (tmp & 0x07)
        else :
            bdat[0] = 0x01 << (7 - (tmp & 0x07));

#----- Test Motion 2 : Cross <-> Circle -----#
def CrossCircle(tmp):
    if (tmp >> 8) & 0x00000001 == 1:
        bdat[0] = 0x18
        bdat[1] = 0x24
        bdat[2] = 0x42
        bdat[3] = 0x81
        bdat[4] = 0x81
        bdat[5] = 0x42
        bdat[6] = 0x24
        bdat[7] = 0x18
    else :
        bdat[0] = 0x81
        bdat[1] = 0x42
        bdat[2] = 0x24
        bdat[3] = 0x18
        bdat[4] = 0x18
        bdat[5] = 0x24
        bdat[6] = 0x42
        bdat[7] = 0x81

#=====================================================================
#  main loop
#=====================================================================
uart0.write('\n\r# Picossci Matrix LED Ready.\n\r')

#----- Repeat Timer (Hz) -----#
beetTimer.init(mode=Timer.PERIODIC, period=1, callback=beetFunc )
cnt = 0

while True:
    # -=-=-= 1mS インターバル処理 =-=-=-
    if timerFlag == True:
        DriveBitMap()
        if testFlag == 1:
            led0.value((cnt >> 7) & 0x00000001)
            led1.value((cnt >> 8) & 0x00000001)
            WaveFlow(cnt)
        elif testFlag == 2:
            led0.value((cnt >> 8) & 0x00000001)
            led1.value((cnt >> 7) & 0x00000001)
            CrossCircle(cnt)
        cnt += 1
        timerFlag = False

    # -=-=-= インターバル以外の処理 =-=-=-
    if UART0read() == True :
        ComExe()
