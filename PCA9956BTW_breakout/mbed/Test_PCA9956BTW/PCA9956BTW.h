/*************************************
**  PCA9956BTW Test                 **
**                                  **
**  Created : 2016/01/26 12:55:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int PCA9956BTW_addr = 0x7E;        // define the I2C Address

/*---------------------------------------------------
    PCA9626B Register Address
*/
//  Control registers
#define MODE1                   0x00
#define MODE2                   0x01
#define LEDOUT0                 0x02
#define LEDOUT1                 0x03
#define LEDOUT2                 0x04
#define LEDOUT3                 0x05
#define LEDOUT4                 0x06
#define LEDOUT5                 0x07
#define GRPPWM                  0x08
#define GRPFREQ                 0x09
#define PPWM0                   0x0A
#define PPWM1                   0x0B
#define PPWM2                   0x0C
#define PPWM3                   0x0D
#define PPWM4                   0x0E
#define PPWM5                   0x0F
#define PPWM6                   0x10
#define PPWM7                   0x11
#define PPWM8                   0x12
#define PPWM9                   0x13
#define PPWM10                  0x14
#define PPWM11                  0x15
#define PPWM12                  0x16
#define PPWM13                  0x17
#define PPWM14                  0x18
#define PPWM15                  0x19
#define PPWM16                  0x1A
#define PPWM17                  0x1B
#define PPWM18                  0x1C
#define PPWM19                  0x1D
#define PPWM20                  0x1E
#define PPWM21                  0x1F
#define PPWM22                  0x20
#define PPWM23                  0x21
#define IREF0                   0x22
#define IREF1                   0x23
#define IREF2                   0x24
#define IREF3                   0x25
#define IREF4                   0x26
#define IREF5                   0x27
#define IREF6                   0x28
#define IREF7                   0x29
#define IREF8                   0x2A
#define IREF9                   0x2B
#define IREF10                  0x2C
#define IREF11                  0x2D
#define IREF12                  0x2E
#define IREF13                  0x2F
#define IREF14                  0x30
#define IREF15                  0x31
#define IREF16                  0x32
#define IREF17                  0x33
#define IREF18                  0x34
#define IREF19                  0x35
#define IREF20                  0x36
#define IREF21                  0x37
#define IREF22                  0x38
#define IREF23                  0x39
#define OFFSET                  0X3A
#define SUBADR1                 0x3B
#define SUBADR2                 0x3C
#define SUBADR3                 0x3D
#define ALLCALLADR              0x3E
#define PWMALL                  0x3F
#define IREFALL                 0x40
#define EFLAG0                  0x41
#define EFLAG1                  0x42
#define EFLAG2                  0x43
#define EFLAG3                  0x44
#define EFLAG4                  0x45
#define EFLAG5                  0x46
