import spidev
import struct
from time import sleep, time
from collections import deque

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 16000000
spi.mode = 0

# OPMODE_CFG CONV_MODE = 0 Manual Mode
spi.xfer2([0x08, 0x04, 0x00])

# SEQUENCE_CFG SEQ_MODE = 2 On-the-fly sequence mode
spi.xfer2([0x08, 0x10, 0x02])

# DATA_CFG APPEND_STATUS = 1 4-bit channel ID is appended to ADC data.
spi.xfer2([0x08, 0x02, 0x10])

# 読み込むチャンネル
chs = deque()
chs.append(0)
chs.append(1)
chs.append(2)
chs.append(3)
chs.append(4)
chs.append(5)
chs.append(6)
chs.append(7)

# 最初の入力チャネル設定
spi.xfer2([0x80 | (chs[0] << 3), 0])

# 設定したチャンネルを末尾に移動
chs.rotate(-1)

try:
    while True:
        data = []
        for ch in chs:
            data.extend(spi.xfer2([0x80 | (ch << 3), 0]))
        print(*[h[0] >> 4 for h in struct.iter_unpack('>H', bytes(data))],sep=',')
except:
    pass

spi.close()
