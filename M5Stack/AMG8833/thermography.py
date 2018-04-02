# MicroPython sample program for M5Stack with AMG8833

# M5Stack
# http://www.m5stack.com/
# Panasonic Grid-EYE AMG8833
# https://industrial.panasonic.com/jp/products/sensors/built-in-sensors/grid-eye

# Usage
# Save this code as main.py then restart.
# Button A : Start drawing grid matrix of thermal data.
# Button B : Caribration
# Button C : Stop drawing

from m5stack import lcd, buttonA, buttonB, buttonC
from machine import I2C
import gc
import ustruct

colors = (0x0000FF, 0x0008FF, 0x0010FF, 0x0018FF, 0x0020FF, 0x0028FF, 0x0030FF, 0x0038FF, 0x0040FF, 0x0048FF, 0x0050FF, 0x0058FF, 0x0060FF, 0x0068FF, 0x0070FF, 0x0078FF, 0x0080FF, 0x0088FF, 0x0090FF, 0x0098FF, 0x00A0FF, 0x00A8FF, 0x00B0FF, 0x00B8FF, 0x00C0FF, 0x00C8FF, 0x00D0FF, 0x00D8FF, 0x00E0FF, 0x00E8FF, 0x00F0FF, 0x00F8FF, 0x00FFFF, 0x00FFF7, 0x00FFEF, 0x00FFE7, 0x00FFDF, 0x00FFD7, 0x00FFCF, 0x00FFC7, 0x00FFBF, 0x00FFB7, 0x00FFAF, 0x00FFA7, 0x00FF9F, 0x00FF97, 0x00FF8F, 0x00FF87, 0x00FF7F, 0x00FF77, 0x00FF6F, 0x00FF67, 0x00FF5F, 0x00FF57, 0x00FF4F, 0x00FF47, 0x00FF3F, 0x00FF37, 0x00FF2F, 0x00FF27, 0x00FF1F, 0x00FF17, 0x00FF0F, 0x00FF07, 0x00FF00, 0x08FF00, 0x10FF00, 0x18FF00, 0x20FF00, 0x28FF00, 0x30FF00, 0x38FF00, 0x40FF00, 0x48FF00, 0x50FF00, 0x58FF00, 0x60FF00, 0x68FF00, 0x70FF00, 0x78FF00, 0x80FF00, 0x88FF00, 0x90FF00, 0x98FF00, 0xA0FF00, 0xA8FF00, 0xB0FF00, 0xB8FF00, 0xC0FF00, 0xC8FF00, 0xD0FF00, 0xD8FF00, 0xE0FF00, 0xE8FF00, 0xF0FF00, 0xF8FF00, 0xFFFF00, 0xFFF700, 0xFFEF00, 0xFFE700, 0xFFDF00, 0xFFD700, 0xFFCF00, 0xFFC700, 0xFFBF00, 0xFFB700, 0xFFAF00, 0xFFA700, 0xFF9F00, 0xFF9700, 0xFF8F00, 0xFF8700, 0xFF7F00, 0xFF7700, 0xFF6F00, 0xFF6700, 0xFF5F00, 0xFF5700, 0xFF4F00, 0xFF4700, 0xFF3F00, 0xFF3700, 0xFF2F00, 0xFF2700, 0xFF1F00, 0xFF1700, 0xFF0F00, 0xFF0700)

i2c_address = 0x69
register = 0x80

ison = False

offset = 0
rate = 1

i2c = I2C(freq=400000, sda=21, scl=22)

def getval(val):
  absval = (val & 0x7FF)
  if val & 0x800:
    return - float(0x800 - absval) * 0.25
  else:
    return float(absval) * 0.25

def minmax():
  global offset
  global rate
  reg = register
  min_temp = 1000
  max_temp = -1000
  for i in range(0, 64):
    val = ustruct.unpack('<h', i2c.readfrom_mem(i2c_address, reg, 2))[0]
    tmp = getval(val)
    if tmp < min_temp:
      min_temp = tmp
    if max_temp < tmp:
      max_temp = tmp
    reg += 2

  diff = max_temp - min_temp
  # add some margin
  diff *= 1.4
  rate = len(colors) / diff
  offset = min_temp * 0.8

  lcd.clear()
  lcd.print('min:    ' + '{:.2f}'.format(min_temp), 0, 0)
  lcd.print('max:    ' + '{:.2f}'.format(max_temp), 0, 10)
  lcd.print('rate:   ' + '{:.2f}'.format(rate), 0, 20)
  lcd.print('offset: ' + '{:.2f}'.format(offset), 0, 30)

def graph():
  global offset
  global rate
  reg = register
  for ic in range(0, 8):
    for ir in range(0, 8):
      val = ustruct.unpack('<h', i2c.readfrom_mem(i2c_address, reg, 2))[0]
      tmp = (getval(val) - offset) * rate
      if tmp < 0:
        tmp = 0
      if 127 < tmp:
        tmp = 127
      lcd.rect((8-ic) * 30 + 40, ir * 30, 30, 30, 0, colors[int(tmp)])
      reg += 2

def loop():
  global ison
  while True:
    if buttonA.isPressed():
      ison = True
    if buttonB.isPressed():
      minmax()
    if buttonC.isPressed():
      ison = False
    if ison:
      graph()

loop()