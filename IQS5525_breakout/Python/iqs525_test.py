import readline
import pyxel
import math
import serial
import time
import sys

pyxel.init(256, 256)
try:
    ser = serial.Serial('/dev/tty.usbserial-D306JGM3',115200,timeout = 1)
except Exception as error:
    print("No board connection")
    sys.exit()
time.sleep(3)
raw_data = ''
ser.reset_input_buffer()
packet_size = 40

while True:
    if ser.in_waiting > 0:
        raw_buffer = ser.readline().decode('utf-8')
        if len(raw_buffer) < packet_size:
            continue
        sensor_data = raw_buffer[0:packet_size-2].strip().split(',')
        ser.reset_input_buffer()
        print(len(raw_buffer))
        print(raw_buffer)
        print(sensor_data)

        relX = int(sensor_data[0])
        if relX > 32768:
            relX = relX - 65536
        relY = int(sensor_data[1])
        if relY > 32768:
            relY = relY - 65536
        absX = int(sensor_data[2])
        if absX > 32768:
            absX = absX - 65536
        absY = int(sensor_data[3])
        if absY > 32768:
            absY = absY - 65536
        data0 = int(sensor_data[6])
        data1 = int(sensor_data[7])
        print("Rel-X",relX," Rel-Y",relY, end='')
        print(" Abs-X",int(sensor_data[2])," Abs-Y",int(sensor_data[3]), end='')
        print(" Str",int(sensor_data[4])," Area",int(sensor_data[5]), end='')
        print(" Data0",data0," Data1",data1)

        pyxel.cls(0)
        pyxel.text(1,1,"Abs X,Y:",3)
        pyxel.text(1,7,"Rel X,Y:",3)
        pyxel.text(1,13,"Event 0:",3)
        pyxel.text(1,19,"Event 1:",3)
        pyxel.pset(absX/4,absY/4,7)
        if (data0 & 0x01) != 0:
            pyxel.circb(absX/4,absY/4,10,3)
        if (data0 & 0x02) != 0:
            pyxel.circ(absX/4,absY/4,10,3)
        pyxel.text(35,1,str(absX)+","+str(absY),7)
        pyxel.text(35,7,str(relX)+","+str(relY),7)
        pyxel.text(35,13,format(data0,'08b'),7)
        pyxel.text(35,19,format(data1,'08b'),7)
        pyxel.flip()
