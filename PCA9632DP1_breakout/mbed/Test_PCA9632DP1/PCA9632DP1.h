/*************************************
**  PCA9632DP1.h                    **
**                                  **
**  Created : 2015/07/31 16:59:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int addr = 0xC4;          // define the I2C Address

/*---------------------------------------------------
    Register Address
*/
//  Control registers
#define MODE1                   0x00
#define MODE2                   0x01
#define PPWM0                   0x02
#define PPWM1                   0x03
#define PPWM2                   0x04
#define PPWM3                   0x05
#define GRPPWM                  0x06
#define GRPFREQ                 0x07
#define LEDOUT                  0x08
#define SUBADR1                 0x09
#define SUBADR2                 0x0A
#define SUBADR3                 0x0B
#define ALLCALLADR              0x0C
