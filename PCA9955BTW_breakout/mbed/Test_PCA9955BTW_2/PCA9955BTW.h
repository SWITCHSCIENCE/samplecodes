/*************************************
**  PCA9955BTW Test 2               **
**                                  **
**  Created : 2016/01/26 12:07:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int PCA9955BTW_1_addr = 0xCA;      // define Device 1 I2C Address
const int PCA9955BTW_2_addr = 0xCE;      // define Device 2 I2C Address

/*---------------------------------------------------
    PCA9955BTW Register Address
*/
//  Control registers
#define MODE1                   0x00
#define MODE2                   0x01
#define LEDOUT0                 0x02
#define LEDOUT1                 0x03
#define LEDOUT2                 0x04
#define LEDOUT3                 0x05
#define GRPPWM                  0x06
#define GRPFREQ                 0x07
#define PPWM0                   0x08
#define PPWM1                   0x09
#define PPWM2                   0x0A
#define PPWM3                   0x0B
#define PPWM4                   0x0C
#define PPWM5                   0x0D
#define PPWM6                   0x0E
#define PPWM7                   0x0F
#define PPWM8                   0x10
#define PPWM9                   0x11
#define PPWM10                  0x12
#define PPWM11                  0x13
#define PPWM12                  0x14
#define PPWM13                  0x15
#define PPWM14                  0x16
#define PPWM15                  0x17
#define IREF0                   0x18
#define IREF1                   0x19
#define IREF2                   0x1A
#define IREF3                   0x1B
#define IREF4                   0x1C
#define IREF5                   0x1D
#define IREF6                   0x1E
#define IREF7                   0x1F
#define IREF8                   0x20
#define IREF9                   0x21
#define IREF10                  0x22
#define IREF11                  0x23
#define IREF12                  0x24
#define IREF13                  0x25
#define IREF14                  0x26
#define IREF15                  0x27
#define RAMP_RATE_GRP0          0x28
#define STEP_TIME_GRP0          0x29
#define HOLD_CNTL_GRP0          0x2A
#define IREF_GRP0               0x2B
#define RAMP_RATE_GRP1          0x2C
#define STEP_TIME_GRP1          0x2D
#define HOLD_CNTL_GRP1          0x2E
#define IREF_GRP1               0x2F
#define RAMP_RATE_GRP2          0x30
#define STEP_TIME_GRP2          0x31
#define HOLD_CNTL_GRP2          0x32
#define IREF_GRP2               0x33
#define RAMP_RATE_GRP3          0x34
#define STEP_TIME_GRP3          0x35
#define HOLD_CNTL_GRP3          0x36
#define IREF_GRP3               0x37
#define GRAD_MODE_SEL0          0x38
#define GRAD_MODE_SEL1          0x39
#define GRAD_GRP_SEL0           0x3A
#define GRAD_GRP_SEL1           0x3B
#define GRAD_GRP_SEL2           0x3C
#define GRAD_GRP_SEL3           0x3D
#define GRAD_CNTL               0x3E
#define OFFSET                  0X3F
#define SUBADR1                 0x40
#define SUBADR2                 0x41
#define SUBADR3                 0x42
#define ALLCALLADR              0x43
#define PWMALL                  0x44
#define IREFALL                 0x45
#define EFLAG0                  0x46
#define EFLAG1                  0x47
#define EFLAG2                  0x48
#define EFLAG3                  0x49
