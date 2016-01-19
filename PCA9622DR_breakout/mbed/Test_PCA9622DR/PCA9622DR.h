/*************************************
**  PCA9622DR.h                     **
**                                  **
**  Created : 2015/08/28 14:33:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int PCA9622DR_addr = 0xE0;        // define the I2C Address

/*---------------------------------------------------
    PCA9622DR Register Address
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
#define PPWM8                   0x0A
#define PPWM9                   0x0B
#define PPWM10                  0x0C
#define PPWM11                  0x0D
#define PPWM12                  0x0E
#define PPWM13                  0x0F
#define PPWM14                  0x10
#define PPWM15                  0x11
#define GRPPWM                  0x12
#define GRPFREQ                 0x13
#define LEDOUT0                 0x14
#define LEDOUT1                 0x15
#define LEDOUT2                 0x16
#define LEDOUT3                 0x17
#define SUBADR1                 0x18
#define SUBADR2                 0x19
#define SUBADR3                 0x1A
#define ALLCALLADR              0x1B
