#include <Wire.h>
#include "SSCI_BME280.h"

void SSCI_BME280::writeReg(uint8_t reg_address, uint8_t data)
{
  Wire.beginTransmission(_i2c_addr);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}

SSCI_BME280::SSCI_BME280() {

}

void SSCI_BME280::setMode(uint8_t i2c_addr, uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t bme280mode, uint8_t t_sb, uint8_t filter, uint8_t spi3w_en) {
  uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | bme280mode;
  uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
  uint8_t ctrl_hum_reg  = osrs_h;
  _i2c_addr = i2c_addr;
  writeReg(BME280_REG_ctrl_hum, ctrl_hum_reg);
  writeReg(BME280_REG_ctrl_meas, ctrl_meas_reg);
  writeReg(BME280_REG_config, config_reg);
}

void SSCI_BME280::readTrim()
{
  uint8_t data[33], i = 0;
  Wire.beginTransmission(_i2c_addr);
  Wire.write(BME280_REG_calib00);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_addr, 24);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }

  Wire.beginTransmission(_i2c_addr);
  Wire.write(BME280_REG_calib25);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_addr, 1);
  data[i] = Wire.read();
  i++;

  Wire.beginTransmission(_i2c_addr);
  Wire.write(BME280_REG_calib26);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_addr, 8);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  calibData.dig_T1 = (data[1] << 8) | data[0];
  calibData.dig_T2 = (data[3] << 8) | data[2];
  calibData.dig_T3 = (data[5] << 8) | data[4];
  calibData.dig_P1 = (data[7] << 8) | data[6];
  calibData.dig_P2 = (data[9] << 8) | data[8];
  calibData.dig_P3 = (data[11] << 8) | data[10];
  calibData.dig_P4 = (data[13] << 8) | data[12];
  calibData.dig_P5 = (data[15] << 8) | data[14];
  calibData.dig_P6 = (data[17] << 8) | data[16];
  calibData.dig_P7 = (data[19] << 8) | data[18];
  calibData.dig_P8 = (data[21] << 8) | data[20];
  calibData.dig_P9 = (data[23] << 8) | data[22];
  calibData.dig_H1 = data[24];
  calibData.dig_H2 = (data[26] << 8) | data[25];
  calibData.dig_H3 = data[27];
  calibData.dig_H4 = (data[28] << 4) | (0x0F & data[29]);
  calibData.dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
  calibData.dig_H6 = data[31];
}

void SSCI_BME280::readData(double *temp_act, double *press_act, double *hum_act)
{
  int i = 0;
  uint32_t data[8];
  unsigned long int hum_raw, temp_raw, press_raw;

  Wire.beginTransmission(_i2c_addr);
  Wire.write(BME280_REG_press_msb);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_addr, 8);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  press_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
  temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  hum_raw  = (data[6] << 8) | data[7];
  *temp_act = (double)calibration_T(temp_raw) / 100.0;
  *press_act = (double)calibration_P(press_raw) / 100.0;
  *hum_act = (double)calibration_H(hum_raw) / 1024.0;
}


signed long int SSCI_BME280::calibration_T(signed long int adc_T)
{

  signed long int var1, var2, T;
  var1 = ((((adc_T >> 3) - ((signed long int)calibData.dig_T1 << 1))) * ((signed long int)calibData.dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((signed long int)calibData.dig_T1)) * ((adc_T >> 4) - ((signed long int)calibData.dig_T1))) >> 12) * ((signed long int)calibData.dig_T3)) >> 14;

  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}
unsigned long int SSCI_BME280::calibration_P(signed long int adc_P)
{
  signed long int var1, var2;
  unsigned long int P;
  var1 = (((signed long int)t_fine) >> 1) - (signed long int)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((signed long int)calibData.dig_P6);
  var2 = var2 + ((var1 * ((signed long int)calibData.dig_P5)) << 1);
  var2 = (var2 >> 2) + (((signed long int)calibData.dig_P4) << 16);
  var1 = (((calibData.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((signed long int)calibData.dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((signed long int)calibData.dig_P1)) >> 15);
  if (var1 == 0)
  {
    return 0;
  }
  P = (((unsigned long int)(((signed long int)1048576) - adc_P) - (var2 >> 12))) * 3125;
  if (P < 0x80000000)
  {
    P = (P << 1) / ((unsigned long int) var1);
  }
  else
  {
    P = (P / (unsigned long int)var1) * 2;
  }
  var1 = (((signed long int)calibData.dig_P9) * ((signed long int)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
  var2 = (((signed long int)(P >> 2)) * ((signed long int)calibData.dig_P8)) >> 13;
  P = (unsigned long int)((signed long int)P + ((var1 + var2 + calibData.dig_P7) >> 4));
  return P;
}

unsigned long int SSCI_BME280::calibration_H(signed long int adc_H)
{
  signed long int v_x1;

  v_x1 = (t_fine - ((signed long int)76800));
  v_x1 = (((((adc_H << 14) - (((signed long int)calibData.dig_H4) << 20) - (((signed long int)calibData.dig_H5) * v_x1)) +
            ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)calibData.dig_H6)) >> 10) *
                (((v_x1 * ((signed long int)calibData.dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + ((signed long int)2097152)) *
                ((signed long int)calibData.dig_H2) + 8192) >> 14));
  v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)calibData.dig_H1)) >> 4));
  v_x1 = (v_x1 < 0 ? 0 : v_x1);
  v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
  return (unsigned long int)(v_x1 >> 12);
}
