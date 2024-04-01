from smbus2 import SMBus, i2c_msg
from time import sleep, time
import struct

devadr = 0x18
bus = SMBus(1)

def writeReg(reg, value):
    msg = i2c_msg.write(devadr, [0x08, reg, value])
    bus.i2c_rdwr(msg)

def readADC(length):
    msg = i2c_msg.read(devadr, length)
    bus.i2c_rdwr(msg)
    return [h[0]>>4 for h in struct.iter_unpack('>H', bytes(msg))]

# ABORT_SEQUENCE
writeReg(0x1F, 0x01)

# OPMODE_SEL Manual Mode with CH0 only
writeReg(0x1C, 0x00)

# START_SEQUENCE start
writeReg(0x1E, 0x01)

while True:
    print(*readADC(2), sep = ',')
