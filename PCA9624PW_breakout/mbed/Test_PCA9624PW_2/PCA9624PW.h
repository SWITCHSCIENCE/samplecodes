/*************************************
**  PCA9624PW.h                     **
**                                  **
**  Created : 2015/09/18 17:23:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int PCA9624PW_1_addr = 0xC0;        // define the I2C Slave 1 Address
const int PCA9624PW_2_addr = 0xC2;        // define the I2C Slave 2 Address

/*---------------------------------------------------
    PCA9624PW Register Address
*/
//  Control registers
#define MODE1                   0x00
#define MODE2                   0x01
#define PPWM0                   0x02
#define PPWM1                   0x03
#define PPWM2                   0x04
#define PPWM3                   0x05
#define PPWM4                   0x06
#define PPWM5                   0x07
#define PPWM6                   0x08
#define PPWM7                   0x09
#define GRPPWM                  0x0A
#define GRPFREQ                 0x0B
#define LEDOUT0                 0x0C
#define LEDOUT1                 0x0D
#define SUBADR1                 0x0E
#define SUBADR2                 0x0F
#define SUBADR3                 0x10
#define ALLCALLADR              0x11
