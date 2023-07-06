import RPi.GPIO as GPIO
import time

Pwm0Pin_0 = 12 # GPIO 18
Pwm0Pin_1 = 32 # GPIO 12
Pwm1Pin_0 = 33 # GPIO 13
Pwm1Pin_1 = 35 # GPIO 19

GPIO.setmode(GPIO.BOARD)
GPIO.setup(Pwm0Pin_0, GPIO.OUT)
GPIO.output(Pwm0Pin_0, GPIO.LOW)  
GPIO.setup(Pwm0Pin_1, GPIO.OUT)
GPIO.output(Pwm0Pin_1, GPIO.LOW)  
GPIO.setup(Pwm1Pin_0, GPIO.OUT)
GPIO.output(Pwm1Pin_0, GPIO.LOW)  
GPIO.setup(Pwm1Pin_1, GPIO.OUT)
GPIO.output(Pwm1Pin_1, GPIO.LOW)  

p0_0 = GPIO.PWM(Pwm0Pin_0, 100)
p0_0.start(0)
p0_1 = GPIO.PWM(Pwm0Pin_1, 100)
p1_0 = GPIO.PWM(Pwm1Pin_0, 100)
p1_0.start(0)
p1_1 = GPIO.PWM(Pwm1Pin_1, 100)
flag = 0

try:
    while True:
        for dc in range(0, 31, 4):
            if flag == 0:
                p0_0.ChangeDutyCycle(dc)
                p1_0.ChangeDutyCycle(31-dc)
            else:
                p0_1.ChangeDutyCycle(dc)
                p1_1.ChangeDutyCycle(31-dc)
            time.sleep(0.05)
        time.sleep(1)
        for dc in range(30, -1, -4):
            if flag == 0:
                p0_0.ChangeDutyCycle(dc)
                p1_0.ChangeDutyCycle(30-dc)
            else:
                p0_1.ChangeDutyCycle(dc)
                p1_1.ChangeDutyCycle(30-dc)
            time.sleep(0.05)
        time.sleep(1)
        if flag == 0:
            p0_0.stop()
            p0_1.start(0)
            p1_0.stop()
            p1_1.start(0)
            flag = 1
        else:
            p0_0.start(0)
            p0_1.stop()
            p1_0.start(0)
            p1_1.stop()
            flag = 0
except KeyboardInterrupt:
    pass
p0_0.stop()
p1_0.stop()
GPIO.cleanup()
