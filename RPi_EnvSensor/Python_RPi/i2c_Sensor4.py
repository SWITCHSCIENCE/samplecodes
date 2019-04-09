#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import time
import RPi.GPIO as GPIO
import spidev
import smbus

bus_number  = 1
i2c_address = 0x76

bus = smbus.SMBus(bus_number)

digT = []
digP = []
digH = []

t_fine = 0.0

def writeReg(reg_address, data):
	bus.write_byte_data(i2c_address,reg_address,data)

def get_calib_param():
	calib = []
	
	for i in range (0x88,0x88+24):
		calib.append(bus.read_byte_data(i2c_address,i))
	calib.append(bus.read_byte_data(i2c_address,0xA1))
	for i in range (0xE1,0xE1+7):
		calib.append(bus.read_byte_data(i2c_address,i))

	digT.append((calib[1] << 8) | calib[0])
	digT.append((calib[3] << 8) | calib[2])
	digT.append((calib[5] << 8) | calib[4])
	digP.append((calib[7] << 8) | calib[6])
	digP.append((calib[9] << 8) | calib[8])
	digP.append((calib[11]<< 8) | calib[10])
	digP.append((calib[13]<< 8) | calib[12])
	digP.append((calib[15]<< 8) | calib[14])
	digP.append((calib[17]<< 8) | calib[16])
	digP.append((calib[19]<< 8) | calib[18])
	digP.append((calib[21]<< 8) | calib[20])
	digP.append((calib[23]<< 8) | calib[22])
	digH.append( calib[24] )
	digH.append((calib[26]<< 8) | calib[25])
	digH.append( calib[27] )
	digH.append((calib[28]<< 4) | (0x0F & calib[29]))
	digH.append((calib[30]<< 4) | ((calib[29] >> 4) & 0x0F))
	digH.append( calib[31] )
	
	for i in range(1,2):
		if digT[i] & 0x8000:
			digT[i] = (-digT[i] ^ 0xFFFF) + 1

	for i in range(1,8):
		if digP[i] & 0x8000:
			digP[i] = (-digP[i] ^ 0xFFFF) + 1

	for i in range(0,6):
		if digH[i] & 0x8000:
			digH[i] = (-digH[i] ^ 0xFFFF) + 1  

def readData():
	data = []
	for i in range (0xF7, 0xF7+8):
		data.append(bus.read_byte_data(i2c_address,i))
	pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4)
	temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4)
	hum_raw  = (data[6] << 8)  |  data[7]
	
	compensate_T(temp_raw)
	compensate_P(pres_raw)

def compensate_P(adc_P):
	global  t_fine
	pressure = 0.0
	
	v1 = (t_fine / 2.0) - 64000.0
	v2 = (((v1 / 4.0) * (v1 / 4.0)) / 2048) * digP[5]
	v2 = v2 + ((v1 * digP[4]) * 2.0)
	v2 = (v2 / 4.0) + (digP[3] * 65536.0)
	v1 = (((digP[2] * (((v1 / 4.0) * (v1 / 4.0)) / 8192)) / 8)  + ((digP[1] * v1) / 2.0)) / 262144
	v1 = ((32768 + v1) * digP[0]) / 32768
	
	if v1 == 0:
		return 0
	pressure = ((1048576 - adc_P) - (v2 / 4096)) * 3125
	if pressure < 0x80000000:
		pressure = (pressure * 2.0) / v1
	else:
		pressure = (pressure / v1) * 2
	v1 = (digP[8] * (((pressure / 8.0) * (pressure / 8.0)) / 8192.0)) / 4096
	v2 = ((pressure / 4.0) * digP[7]) / 8192.0
	pressure = pressure + ((v1 + v2 + digP[6]) / 16.0)  

	print "pressure : %7.2f hPa" % (pressure/100)

def compensate_T(adc_T):
	global t_fine
	v1 = (adc_T / 16384.0 - digT[0] / 1024.0) * digT[1]
	v2 = (adc_T / 131072.0 - digT[0] / 8192.0) * (adc_T / 131072.0 - digT[0] / 8192.0) * digT[2]
	t_fine = v1 + v2
	temperature = t_fine / 5120.0
	print "temp : %-6.2f ℃" % (temperature) 

def compensate_H(adc_H):
	global t_fine
	var_h = t_fine - 76800.0
	if var_h != 0:
		var_h = (adc_H - (digH[3] * 64.0 + digH[4]/16384.0 * var_h)) * (digH[1] / 65536.0 * (1.0 + digH[5] / 67108864.0 * var_h * (1.0 + digH[2] / 67108864.0 * var_h)))
	else:
		return 0
	var_h = var_h * (1.0 - digH[0] * var_h / 524288.0)
	if var_h > 100.0:
		var_h = 100.0
	elif var_h < 0.0:
		var_h = 0.0
	print "hum : %6.2f ％" % (var_h)

def setup():
	osrs_t = 1			#Temperature oversampling x 1
	osrs_p = 1			#Pressure oversampling x 1
	osrs_h = 1			#Humidity oversampling x 1
	mode   = 3			#Normal mode
	t_sb   = 5			#Tstandby 1000ms
	filter = 0			#Filter off
	spi3w_en = 0			#3-wire SPI Disable

	ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode
	config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en
	ctrl_hum_reg  = osrs_h

	writeReg(0xF2,ctrl_hum_reg)
	writeReg(0xF4,ctrl_meas_reg)
	writeReg(0xF5,config_reg)

def read_light():
	val = bus.read_byte_data(0x6A, 0xC0)
	return val

class AQM1248A:
	def __init__(self):
		self.spi = spidev.SpiDev()
		self.spi.open(0,0)
		self.spi.max_speed_hz = 16000000
		self.write_cmd(0xAE)	# Display = OFF
		self.write_cmd(0xA0)	# ADC = normal
		self.write_cmd(0xC8)	# Common output = revers
		self.write_cmd(0xA3)	# LCD bias = 1/7
		# Inner Reg. ON
		self.write_cmd(0x2C)
		time.sleep(2)
		self.write_cmd(0x2E)
		time.sleep(2)
		self.write_cmd(0x2F)
		# Contrast set
		self.write_cmd(0x23)	# Vo voltage regulator internal resistor ratio set
		self.write_cmd(0x81)	# Electronic volume mode set
		self.write_cmd(0x1C)	# Electronic volume register set
		# Display set
		self.write_cmd(0xA4)	# Display all point ON/OFF = normal
		self.write_cmd(0x40)	# Display start line = 0
		self.write_cmd(0xA6)	# Display normal/revers = normal
		self.write_cmd(0xAF)	# Display = ON
	def reset(self):
		GPIO.output( 5, GPIO.LOW)	# RESET = 0
		time.sleep(0.1)
		GPIO.output( 5, GPIO.HIGH)	# RESET = 1
		time.sleep(0.1)
	def write_cmd(self, cmd):
		GPIO.output(25, GPIO.LOW)	# RS = 0
		self.spi.xfer2([cmd])
	def write_data(self, data):
		GPIO.output(25, GPIO.HIGH)	# RS = 1
		self.spi.xfer2([data])
	def write(self, cmd):
		if len(cmd) == 0:
			return
		GPIO.output(25, GPIO.LOW)	# RS = 0
		self.spi.xfer2([cmd[0]])
		GPIO.output(25, GPIO.HIGH)	# RS = 1
		self.spi.xfer2(list(cmd[1:]))


GPIO.setmode(GPIO.BCM)
GPIO.setup(24, GPIO.IN)		# SW 1
GPIO.setup(23, GPIO.IN)		# SW 2
GPIO.setup(18, GPIO.IN)		# SW 3
GPIO.setup(22, GPIO.OUT)	# LED 1
GPIO.setup(27, GPIO.OUT)	# LED 2
GPIO.setup(17, GPIO.OUT)	# LED 3
GPIO.setup( 4, GPIO.OUT)	# IrLED
GPIO.setup(25, GPIO.OUT)	# LCD - RS
GPIO.setup( 5, GPIO.OUT)	# LCD - RESET

setup()
get_calib_param()

lcd = AQM1248A()
pict = ((	  0,  0,  0,  0,128,192,192,224,224,224,224,224,224,224,224,224,
		224, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,224,
		224,224,224,224,224,224,224,192,192,  0,  0,  0,  0,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	),
	(	  0,  0,  0,  0,255,255,255,255,255,255,255,255,255,255,255, 31,
		  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,224,248,254,255,255,
		255,255,255,255,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
		  0,  0, 60,126,126,231,231,199,199,206,142,140,  0,  0,  3, 63,
		255,252,  0,  0,240,254, 30, 62,248,240,  0,  0,254,255, 15,  0,
		  0,255,255,255,  0,  7,  7,  7,  7,255,255,255,  7,  7,  7,  7,
		  0,248,252,254, 15,  7,  7,  7,  7, 15, 30, 28,  0,  0,  0,255,
		255,255,224,224,224,224,255,255,255,  0,  0,  0,  0,  0,  0,  0	),
	(	  0,  0,  0,  0,255,255,255,255,255,255,255,255,191,135,128,128,
		128,128,128,128,128,128,  0,  0,  0,  0,  7,  7,  7,  7,  7,135,
		199,231,247,251,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,
		  0,  0, 12, 28, 60, 56, 48, 48, 49, 57, 63, 31, 15,  0,  0,  0,
		  7, 63, 62, 62, 63,  7,  0,  0,  7, 63, 62, 62, 63,  7,  0,  0,
		  0, 63, 63, 63,  0,  0,  0,  0,  0, 63, 63, 63,  0,  0,  0,  0,
		  0,  3, 15, 31, 28, 56, 56, 56, 56, 28, 30, 14,  0,  0,  0, 63,
		 63, 63,  0,  0,  0,  0, 63, 63, 63,  0,  0,  0,  0,  0,  0,  0	))

if __name__ == '__main__':


	try:
		while True:
			if GPIO.input(24) == 0:
				GPIO.output(22, GPIO.LOW)
			else:
				GPIO.output(22, GPIO.HIGH)

			if GPIO.input(23) == 0:
				GPIO.output(27, GPIO.LOW)
			else:
				GPIO.output(27, GPIO.HIGH)

			if GPIO.input(18) == 0:
				GPIO.output( 4, GPIO.HIGH)
			else:
				GPIO.output( 4, GPIO.LOW)

			readData()

			ldat = read_light()
			print ("  light : {}\n".format(ldat))

			GPIO.output(17, GPIO.LOW)
			for y in range(0, 6):
				lcd.write_cmd(0xB0 + y)
				lcd.write_cmd(0x10)
				lcd.write_cmd(0x00)
				for x in range(0,128):
					lcd.write_data(pict[y % 3][x])
			time.sleep(0.2)
			GPIO.output(17, GPIO.HIGH)
			for y in range(0, 6):
				lcd.write_cmd(0xB0 + y)
				lcd.write_cmd(0x10)
				lcd.write_cmd(0x00)
				for x in range(0,128):
					lcd.write_data(pict[y % 3][x] ^ 0xFF)
			time.sleep(0.2)
	except KeyboardInterrupt:
		print("\nExit.")
		GPIO.cleanup()

