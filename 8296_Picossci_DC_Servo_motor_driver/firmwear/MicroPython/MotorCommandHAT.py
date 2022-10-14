#=====================================================================
#      Picossci DC/Servo Motor Driver
#      Sample program [MicroPython]
#      O.Aoki [ switch-science ]
#=====================================================================

from machine import UART, PWM, Pin
import time

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

# Servo(PWM0A ~ PWM7B)
servo0 = PWM(Pin(8))
servo0.freq(50)
servo1 = PWM(Pin(9))
servo1.freq(50)
servo2 = PWM(Pin(10))
servo2.freq(50)
servo3 = PWM(Pin(11))
servo3.freq(50)
servo4 = PWM(Pin(12))
servo4.freq(50)
servo5 = PWM(Pin(13))
servo5.freq(50)
servo6 = PWM(Pin(14))
servo6.freq(50)
servo7 = PWM(Pin(15))
servo7.freq(50)

motor0A = PWM(Pin(16))
motor0A.freq(100)
motor0B = PWM(Pin(17))
motor0B.freq(100)
motor1A = PWM(Pin(18))
motor1A.freq(100)
motor1B = PWM(Pin(19))
motor1B.freq(100)
motor2A = PWM(Pin(20))
motor2A.freq(100)
motor2B = PWM(Pin(21))
motor2B.freq(100)
motor3A = PWM(Pin(22))
motor3A.freq(100)
motor3B = PWM(Pin(23))
motor3B.freq(100)

#UART
uart0 = UART(0,baudrate=115200,tx=Pin(0),rx=Pin(1))

#=====================================================================
#  Initialize
#=====================================================================

led0.value(1)
led1.value(1)

servo0.duty_u16(0)
servo1.duty_u16(0)
servo2.duty_u16(0)
servo3.duty_u16(0)
servo4.duty_u16(0)
servo5.duty_u16(0)
servo6.duty_u16(0)
servo7.duty_u16(0)

motor0A.duty_u16(0)
motor0B.duty_u16(0)
motor1A.duty_u16(0)
motor1B.duty_u16(0)
motor2A.duty_u16(0)
motor2B.duty_u16(0)
motor3A.duty_u16(0)
motor3B.duty_u16(0)

rbuff = bytes()
cha = 0
cha2 = 0
pnum = 0
ddir = 1
dat = 0

#=====================================================================
#  Sub Function
#=====================================================================

def isint(s):  # 整数値かどうかを判定
    try:
        int(s, 10)  # 試しにint関数で文字列を変換
    except ValueError:
        return False  # 失敗すれば False
    else:
        return True  # 上手くいけば True

#=====================================================================
#  main loop
#=====================================================================

uart0.write('UART TEST\r\n')

while True:
    if uart0.any() > 0:
        cha = uart0.read(1)
        cha2 = cha.upper()
        if cha2 == b'R' or cha2 == b'W':
            rbuff = bytes()
            rbuff += cha2
        elif cha2 == b'\r' or cha2 == b'\n':
            rbuff = rbuff.decode('utf-8')
            rstr = rbuff.split(",")
            
            if len(rstr) >= 2 and isint(rstr[1]):
                pnum = int(rstr[1])
            else :
                pnum = 99
            
            if len(rstr) >= 3 and isint(rstr[2]):
                ddir = 1
                if int(rstr[2]) < 0:
                    ddir = -1
                dat = abs(int(rstr[2]))
                if dat >= 0xFFFF:
                    dat = 0xFFFF
            
            rbuff = bytes()
            
            if len(rstr) == 2 and rstr[0] == 'R':
                if pnum == 0:
                    uart0.write('R,0,' + str(ain0.read_u16()) + '\r\n')
                elif pnum == 1:
                    uart0.write('R,1,' + str(ain1.read_u16()) + '\r\n')
                elif pnum == 2:
                    uart0.write('R,2,' + str(ain2.read_u16()) + '\r\n')
                elif pnum == 3:
                    uart0.write('R,3,' + str(ain3.read_u16()) + '\r\n')
            elif len(rstr) == 3 and rstr[0] == 'W':
                if pnum == 15:
                    led1.value(1 - (dat % 2))
                elif pnum == 14:
                    led0.value(1 - (dat % 2))
                elif pnum == 0:
                    servo0.duty_u16(dat)
                elif pnum == 1:
                    servo1.duty_u16(dat)
                elif pnum == 2:
                    servo2.duty_u16(dat)
                elif pnum == 3:
                    servo3.duty_u16(dat)
                elif pnum == 4:
                    servo4.duty_u16(dat)
                elif pnum == 5:
                    servo5.duty_u16(dat)
                elif pnum == 6:
                    servo6.duty_u16(dat)
                elif pnum == 7:
                    servo7.duty_u16(dat)
                elif pnum == 10:
                    if ddir > 0:
                        motor0A.duty_u16(dat)
                        motor0B.duty_u16(0)
                    else :
                        motor0A.duty_u16(0)
                        motor0B.duty_u16(dat)
                elif pnum == 11:
                    if ddir > 0:
                        motor1A.duty_u16(dat)
                        motor1B.duty_u16(0)
                    else :
                        motor1A.duty_u16(0)
                        motor1B.duty_u16(dat)
                elif pnum == 12:
                    if ddir > 0:
                        motor2A.duty_u16(dat)
                        motor2B.duty_u16(0)
                    else :
                        motor2A.duty_u16(0)
                        motor2B.duty_u16(dat)
                elif pnum == 13:
                    if ddir > 0:
                        motor3A.duty_u16(dat)
                        motor3B.duty_u16(0)
                    else :
                        motor3A.duty_u16(0)
                        motor3B.duty_u16(dat)
        else :
            rbuff += cha2
