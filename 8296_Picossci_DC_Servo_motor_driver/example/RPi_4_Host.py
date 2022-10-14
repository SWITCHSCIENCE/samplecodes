import serial
import time

# シリアル通信ベースクラス
class SerialBase:
    def __init__(self):
        # commの初期化
        self.comm = ''
        # ポートオープンフラグ
        self.isOpen = False
        # タイムアウトフラグ
        self.isTimeOut = False
        # Threadの終了フラグ
        self.isStop = False
        # 受信データバッファ
        self.receiveBuffer = bytearray()

    def IsOpen(self):
        return self.isOpen

    def IsTimeOut(self):
        return self.isTimeOut

    def IsStop(self):
        return self.isStop

    def Stop(self):
        self.isStop = True

    def Write(self, data):
        self.comm.write(data)

    def Read(self):
        return self.comm.read()

    def Open(self, ser, port, baud='115200'):
        try:
            self.comm = ser
            self.comm = serial.Serial(port, baud, timeout=0.01)
            self.isOpen = True
        except Exception as err:
            self.isOpen = False
        return self.isOpen

def main():
    print("Host sample.\r\n")

    # シリアルクラス
    ser = SerialBase()
    ser.Open(ser, '/dev/serial0', '115200')

#     ser.Write(b'\r\nStart.\r\n')

    cnt = 0
    time.sleep(1)

    while True:
        if (cnt & 0x007F) == 0:
            ser.Write("w,4,2000\r\n".encode("utf-8"))
            ser.Write("w,5,2000\r\n".encode("utf-8"))
            ser.Write("w,6,2000\r\n".encode("utf-8"))
            ser.Write("w,7,2000\r\n".encode("utf-8"))
            ser.Write("w,14,0\r\n".encode("utf-8"))
        elif (cnt & 0x007F) == 0x0040:
            ser.Write("w,4,5000\r\n".encode("utf-8"))
            ser.Write("w,5,5000\r\n".encode("utf-8"))
            ser.Write("w,6,5000\r\n".encode("utf-8"))
            ser.Write("w,7,5000\r\n".encode("utf-8"))
            ser.Write("w,14,1\r\n".encode("utf-8"))

        if (cnt & 0x0FFF) == 0x0020:
            ser.Write("w,13,0\r\n".encode("utf-8"))
            ser.Write("w,15,0\r\n".encode("utf-8"))
        elif (cnt & 0x0FFF) == 0x0420:
            ser.Write("w,13,8000\r\n".encode("utf-8"))
            ser.Write("w,15,1\r\n".encode("utf-8"))
        elif (cnt & 0x0FFF) == 0x0820:
            ser.Write("w,13,0\r\n".encode("utf-8"))
            ser.Write("w,15,0\r\n".encode("utf-8"))
        elif (cnt & 0x0FFF) == 0x0C20:
            ser.Write("w,13,-8000\r\n".encode("utf-8"))
            ser.Write("w,15,1\r\n".encode("utf-8"))

        cnt += 1
        time.sleep(0.01)

    ser.Close()

if __name__ == "__main__":
    main()
