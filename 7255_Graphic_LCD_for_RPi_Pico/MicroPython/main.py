from machine import Pin, SPI
import time

LCD_CLK_PIN = const(6)
LCD_MOSI_PIN = const(7)
LCD_MISO_PIN = const(4)
LCD_CS_PIN = const(5)
LCD_RST_PIN = const(10)
LCD_RS_PIN = const(9)

spi = SPI(0)
spi = SPI(0,
          baudrate=20000000,
          polarity=1,
          phase=1,
          sck=Pin(LCD_CLK_PIN),
          miso=Pin(LCD_MISO_PIN),
          mosi=Pin(LCD_MOSI_PIN),
        )
print(spi)  # test

cs = Pin(LCD_CS_PIN, Pin.OUT)
rst = Pin(LCD_RST_PIN, Pin.OUT)
dc = Pin(LCD_RS_PIN, Pin.OUT)

led1 = Pin(16, Pin.OUT)
led2 = Pin( 8, Pin.OUT)
led3 = Pin(25, Pin.OUT)

sw1 = Pin( 0, Pin.IN)
sw2 = Pin( 1, Pin.IN)
sw3 = Pin( 2, Pin.IN)
sw4 = Pin( 3, Pin.IN)
sw5 = Pin( 14, Pin.IN)
sw6 = Pin( 13, Pin.IN)
sw7 = Pin( 15, Pin.IN)
sw8 = Pin( 11, Pin.IN)
sw9 = Pin( 12, Pin.IN)

rst.value(0)
cs.value(1)
dc.value(0)
time.sleep(0.05)

rst.value(1)
time.sleep(0.05)

pict = ((  0,  0,  0,  0,128,192,192,224,224,224,224,224,224,224,224,224,
         224, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,224,
         224,224,224,224,224,224,224,192,192,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
        (  0,  0,  0,  0,255,255,255,255,255,255,255,255,255,255,255, 31,
           3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,224,248,254,255,255,
         255,255,255,255,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
           0,  0, 60,126,126,231,231,199,199,206,142,140,  0,  0,  3, 63,
         255,252,  0,  0,240,254, 30, 62,248,240,  0,  0,254,255, 15,  0,
           0,255,255,255,  0,  7,  7,  7,  7,255,255,255,  7,  7,  7,  7,
           0,248,252,254, 15,  7,  7,  7,  7, 15, 30, 28,  0,  0,  0,255,
         255,255,224,224,224,224,255,255,255,  0,  0,  0,  0,  0,  0,  0),
        (  0,  0,  0,  0,255,255,255,255,255,255,255,255,191,135,128,128,
         128,128,128,128,128,128,  0,  0,  0,  0,  7,  7,  7,  7,  7,135,
         199,231,247,251,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
           0,  0, 12, 28, 60, 56, 48, 48, 49, 57, 63, 31, 15,  0,  0,  0,
           7, 63, 62, 62, 63,  7,  0,  0,  7, 63, 62, 62, 63,  7,  0,  0,
           0, 63, 63, 63,  0,  0,  0,  0,  0, 63, 63, 63,  0,  0,  0,  0,
           0,  3, 15, 31, 28, 56, 56, 56, 56, 28, 30, 14,  0,  0,  0, 63,
          63, 63,  0,  0,  0,  0, 63, 63, 63,  0,  0,  0,  0,  0,  0,  0),
        (  0,  0,  0,  0,255,255,255,255,255,255,255,255,255,255,255,255,
         255,255,127, 31,  3,  0,  0,128,192,224,240,248,252,254,255,255,
         255,255,255,255,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
           0,  0,224,176, 24,  8,  8,  8,  8, 16, 48,  0,  0,  0,192, 48,
          16,  8,  8,  8, 24, 16, 96,  0,  0,  0,248,  0,  0,  0,248,  8,
           8,  8,  8,  8,  8,  8,  0,  0,  0,248, 16, 96,192,  0,  0,  0,
           0,  0,248,  0,  0,  0,192, 48, 16,  8,  8,  8, 24, 16, 96,  0,
           0,248,  8,  8,  8,  8,  8,  8,  8,  0,  0,  0,  0,  0,  0,  0),
        (  0,  0,  0,  0,255,255,255,255,255,255,255,255,255,255,255,255,
         239,227,240,248,252,254,255,255,255,255,255,255,255,255,255,255,
         255,255,255,255,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
           0,  0, 96,193,129,  3,  2,  2,  6,132,204,120,  0, 15,112,192,
         128,  0,  0,  0,  0,192, 96,  0,  0,  0,255,  0,  0,  0,255,  2,
           2,  2,  2,  2,  2,  2,  0,  0,  0,255,  0,  0,  0,  3, 12, 24,
          96,192,255,  0,  0, 15,112,192,128,  0,  0,  0,  0,192, 96,  0,
           0,255,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0),
        (  0,  0,  0,  0,  1,  3,  3,  7,  7,  7,  7,  7,  7,  7,  7,  7,
           7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
           7,  7,  7,  7,  7,  7,  7,  3,  3,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
           1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1,  1,
           1,  1,  1,  1,  1,  1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
           0,  0,  1,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,
           0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0)
        )

# =====================================================================
#   AQM1248A
# =====================================================================
def write_cmd(cd):
    command = bytearray(1)
    command[0] = cd
    dc.value(0)
    cs.value(0)
    spi.write(command)
    cs.value(1)

def write_data(dt):
    dat = bytearray(1)
    dat[0] = dt
    dc.value(1)
    cs.value(0)
    spi.write(dat)
    cs.value(1)

def init_LCD():
    rst.value(0)
    cs.value(1)
    dc.value(0)
    time.sleep(0.1)
    rst.value(1)

    write_cmd(0xAE)  # display = OFF
    write_cmd(0xA0)  # ADC = normal
    write_cmd(0xC8)  # Common output = reverse
    write_cmd(0xA3)  # bias = 1/7

    write_cmd(0x2C)  # power control 1
    time.sleep(0.05)
    write_cmd(0x2E)  # power control 2
    time.sleep(0.05)
    write_cmd(0x2F)  # power control 3
    time.sleep(0.05)

    write_cmd(0x23)  # Vo voltage resistor ratio set
    write_cmd(0x81)  # Electronic volume mode set
    write_cmd(0x1C)  # Electronic volume value set

    write_cmd(0xA4)  # display all point = nomal
    write_cmd(0x40)  # display start line = 0
    write_cmd(0xA6)  # display nomal/reverse = nomal
    write_cmd(0xAF)  # display = on
 
def select_page(page):
    write_cmd(0b10110000 | (page & 0b00001111))

def select_column(col):
    write_cmd(0b00010000 | ((col >> 4) & 0b00001111) )
    write_cmd(0b00000000 | (col & 0b00001111) )

def set_start_line(line):
    write_cmd(0b01000000 | (line & 0b00111111))

def screen_reverse(scr):
    write_cmd(0b10100110 | (scr & 0b00000001))

def screen_clear():
    for p in range(8):
        select_page(p)
        select_column(0)
        for i in range(128):
            write_data(0)

# =====================================================================
#   main
# =====================================================================

init_LCD()

select_page(0)
select_column(0)
set_start_line(0)
screen_clear()

# for page in range(6):
#     select_page(page)
#     select_column(0)
#     for col in range(128):
#         write_data(pict[page][col])

# -----------------------------------------------------------------------------------
cnt = 0
psw = 0
dsw = 0
page = 0
vled1 = 0
vled2 = 0

while True:
    psw = 0
    if sw1.value() == 0:
        psw = psw + 0b000000001
    if sw2.value() == 0:
        psw = psw + 0b000000010
    if sw3.value() == 0:
        psw = psw + 0b000000100
    if sw4.value() == 0:
        psw = psw + 0b000001000
    if sw5.value() == 0:
        psw = psw + 0b000010000
    if sw6.value() == 0:
        psw = psw + 0b000100000
    if sw7.value() == 0:
        psw = psw + 0b001000000
    if sw8.value() == 0:
        psw = psw + 0b010000000
    if sw9.value() == 0:
        psw = psw + 0b100000000
#     if psw != dsw:
#         print('hex : {:03X}'.format(psw))
    if (psw & 0x000f) != 0:
        vled2 = vled2 ^ 1
        led2.value(vled2)
    if (psw & 0x00f0) != 0:
        vled1 = vled1 ^ 1
        led1.value(vled1)
    dsw = psw

    if (cnt & 0x7F) == 0x7F:
        screen_clear()
    elif (cnt & 0x0F) == 0:
        page = (cnt & 0x70) >> 4
        if page < 6:
            select_page(page)
            select_column(0)
            for col in range(128):
                write_data(pict[page][col])

#     led1.value(cnt % 2)
#     led2.value(1 - (cnt % 2))
    led3.value(cnt % 2)

    cnt += 1
    time.sleep(0.1)