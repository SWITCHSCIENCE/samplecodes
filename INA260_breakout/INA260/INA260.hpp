/**
 *
 * DESCRIPTION
 * INA260 - Bi-Directional 15A CURRENT/POWER MONITOR with I2C

 *
 * INA260 Class.
 */
class INA260 {
    public:
        /**
         * Constructor.
         *
         * @param i2c_ instance of I2C.
         * @param addr_ I2C slave address. 8bit
         * @param freq_ I2C frequency.
         */
        INA260(I2C &i2c_,int addr_ = 0x80,int freq_ = 100000);
        /**
         * Check INA260 exist.
         *
         * @param none
         * @return 0:NOT EXIST / !0:EXIST
         */
        int isExist(void);
        
        /**
         * INA260 raw level read.
         *
         * @param pointer_addr INA260 pointer address.
         * @param val_ read value.
         * @return 0:Read OK / !0:Read NG
         */
        int rawRead(char pointer_addr,unsigned short *val_);
        
        /**
         * INA260 raw level write.
         *
         * @param pointer_addr INA260 pointer address.
         * @param val_ write value.
         * @return 0:Read OK / !0:Read NG
         */
        int rawWrite(char pointer_addr,unsigned short val_);
        
        /**
         * Get voltage.
         *
         * @param V_ read value;
         * @return 0:Read OK / !0:Read NG
         */
        int getVoltage(double *V_);
        /**
         * Get current.
         *
         * @param I_ read value;
         * @return 0:Read OK / !0:Read NG
         */
        int getCurrent(double *I_);
        
        /**
         * Get current.
         *
         * @param I_ read value;
         * @return 0:Read OK / !0:Read NG
         */
         
        int getPower(double *P_);
        /**
         * Set configuration.
         *
         * @param val write value;
         * @return 0:Read OK / !0:Read NG
         */
        int setConfig(unsigned short val);
        /**
         * Set Alert Mask.
         *
         * @param val write value;
         * @return 0:Read OK / !0:Read NG
         */
        int setAlert(unsigned short val1);
        
        /**
         * Read Alert.
         *
         * @param val write value;
         * @return 0:Read OK / !0:Read NG
         */
        void readAlert(void);
        /**
         * Set Alert limits.
         *
         * @param val write value;
         * @return 0:Read OK / !0:Read NG
         */
        int setLim(unsigned short val2);

    private:
        I2C &i2c;
        int i2c_addr;
        int freq;
};

