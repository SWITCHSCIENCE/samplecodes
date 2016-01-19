/*************************************
**  PCAL9555APW.h                   **
**                                  **
**  Created : 2015/07/10 15:41:00   **
**  Author  : O.aoki                **
**************************************/


/*---------------------------------------------------
    Constant value
*/
const int addr = 0x40;          // define the I2C Address

/*---------------------------------------------------
    Register Address
*/
#define InputPort0              0x00
#define InputPort1              0x01
#define OutputPort0             0x02
#define OutputPort1             0x03
#define PolarityInversionPort0  0x04
#define PolarityInversionPort1  0x05
#define Configuration0          0x06
#define Configuration1          0x07
#define InputLatchRegister0     0x44
