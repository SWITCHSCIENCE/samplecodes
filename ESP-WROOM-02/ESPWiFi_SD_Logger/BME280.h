/*
  BME280.h
  original file:https://github.com/SWITCHSCIENCE/BME280

  using BME280 module as i2c slave device
 */
#pragma once

class BME280
{
  public:
    // set i2c address
    // 0x77 SDO high
    // 0x76 SDO low
    BME280(int address=0x77):
      _address(address){}

    /* Initialize */
    void init();

    // @param (*temp)  write Temperature value into this variable
    // @param (*press) write Pressure value into this variable
    // @param (*hum)   write Humidity value into this variable
    void getSensorData(double *temp, double *press, double *hum);

  private:
    int _address;
    void readTrim();
    void writeReg(uint8_t reg_address, uint8_t data);
    void readData();
    signed long int calibration_T(signed long int adc_T);
    unsigned long int calibration_P(signed long int adc_P);
    unsigned long int calibration_H(signed long int adc_H);

    unsigned long int hum_raw,temp_raw,pres_raw;
    signed long int t_fine;

    uint16_t dig_T1;
     int16_t dig_T2;
     int16_t dig_T3;
    uint16_t dig_P1;
     int16_t dig_P2;
     int16_t dig_P3;
     int16_t dig_P4;
     int16_t dig_P5;
     int16_t dig_P6;
     int16_t dig_P7;
     int16_t dig_P8;
     int16_t dig_P9;
     int8_t  dig_H1;
     int16_t dig_H2;
     int8_t  dig_H3;
     int16_t dig_H4;
     int16_t dig_H5;
     int8_t  dig_H6;
};
