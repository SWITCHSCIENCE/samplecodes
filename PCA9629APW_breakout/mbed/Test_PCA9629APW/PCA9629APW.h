/*************************************
**  PCA9629APW.h                    **
**                                  **
**  Created : 2015/09/25 12:23:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int PCA9629APW_addr = 0x44;        // define the I2C Slave 1 Address

/*---------------------------------------------------
    PCA9624PW Register Address
*/
//  Control registers
#define MODE                    0x00
#define WDTTOI                  0x01
#define WDTCNTL                 0x02
#define IO_CFG                  0x03
#define INTMODE                 0x04
#define MSK                     0x05
#define INTSTAT                 0x06
#define IP                      0x07
#define INT_MTR_ACT             0x08
#define EXTRASTEP0              0x09
#define EXTRASTEP1              0x0A
#define OP_CFG_PHS              0x0B
#define OP_STAT_TO              0x0C
#define RUCNTL                  0x0D
#define RDCNTL                  0x0E
#define PMA                     0x0F
#define LOOPDLY_CW              0x10
#define LOOPDLY_CCW             0x11
#define CWSCOUNTL               0x12
#define CWSCOUNTH               0x13
#define CCWSCOUNTL              0x14
#define CCWSCOUNTH              0x15
#define CWPWL                   0x16
#define CWPWH                   0x17
#define CCWPWL                  0x18
#define CCWPWH                  0x19
#define MCNTL                   0x1A
#define SUBADR1                 0x1B
#define SUBADR2                 0x1C
#define SUBADR3                 0x1D
#define ALLCALLADR              0x1E
#define STEPCOUNT0              0x1F
#define STEPCOUNT1              0x20
#define STEPCOUNT2              0x21
#define STEPCOUNT3              0x22