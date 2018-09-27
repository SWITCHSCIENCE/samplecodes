/**
 * @section DESCRIPTION
 * INA260 - Bi-Directional 15A CURRENT/POWER MONITOR with I2C
 *
 */
#include "mbed.h"
#include "INA260.hpp"

INA260::INA260(I2C &i2c_,int addr_,int freq_) : i2c(i2c_),i2c_addr(addr_),freq(freq_)
{
    i2c.frequency(freq);
}

int INA260::isExist(void)
{
    char p_addr = 0;   //Select Configuration Register.
    i2c.frequency(freq);
    if(i2c.write(i2c_addr | 0,&p_addr,sizeof(p_addr)) == 0){
        return 1;
    }
    return 0;
}

int INA260::rawWrite(char pointer_addr,unsigned short val_)
{
    char val[3];
    val[0] = pointer_addr;
    val[1] = static_cast<char>((val_ >> 8) & 0x00ff);
    val[2] = static_cast<char>(val_ & 0x00ff);
    i2c.frequency(freq);
    if(i2c.write(i2c_addr | 0,val,sizeof(val)) == 0){
        return 0;
    }
    return 1;
}

int INA260::rawRead(char pointer_addr,unsigned short *val_)
{
    char p_addr = pointer_addr;
    char val[2];
    i2c.frequency(freq);
    if(i2c.write(i2c_addr | 0,&p_addr,sizeof(p_addr)) == 0){
        if(i2c.read(i2c_addr | 0x01,val,sizeof(val)) == 0){
            *val_ = static_cast<unsigned short>(val[0]);
            *val_ = (*val_ << 8) | static_cast<unsigned short>(val[1]);
            return 0;
        }
    }
    return 1;
}

int INA260::getVoltage(double *V_)
{
    unsigned short val;
    if(rawRead(0x02,&val) == 0){
        *V_ = static_cast<double>(val) * 0.00125;
        return 0;
    }
    return 1;
}

int INA260::getCurrent(double *I_)
{
    unsigned short val;
    if(rawRead(0x01,&val) == 0){ //current register 0X1
        char *s_p = reinterpret_cast<char *>(&val);
        short d_s;
        char *d_p = reinterpret_cast<char *>(&d_s);
        *(d_p + 0) = *(s_p + 0);
        *(d_p + 1) = *(s_p + 1);
        *I_ = static_cast<double>(d_s)  * 1.25;
        return 0;
    }
    return 1;
}


int INA260::getPower(double *P_)
{
    unsigned short val;
    if(rawRead(0x03,&val) == 0){ //power register 0x3
        char *s_p = reinterpret_cast<char *>(&val);
        short d_s;
        char *d_p = reinterpret_cast<char *>(&d_s);
        *(d_p + 0) = *(s_p + 0);
        *(d_p + 1) = *(s_p + 1);
        *P_ = static_cast<double>(d_s)  * 10;
        return 0;
    }
    return 1;
}

int INA260::setConfig(unsigned short val)
{
    return rawWrite(0x00,val);  //config register 0x00
}

int INA260::setAlert(unsigned short val1)
{
    return rawWrite(0x06,val1);  //config register 0x00
}

int INA260::setLim(unsigned short val2)
{
    return rawWrite(0x07,val2);  //config register 0x00
}

void INA260::readAlert(void)
{
    unsigned short val4;
    rawRead(0x06,&val4);
    
 }   
