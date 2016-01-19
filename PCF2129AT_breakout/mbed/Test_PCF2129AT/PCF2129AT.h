/*************************************
**  PCF2129AT.h                     **
**                                  **
**  Created : 2015/07/14 11:31:00   **
**  Author  : O.aoki                **
**************************************/

/*---------------------------------------------------
    Constant value
*/
const int addr = 0xA2;          // define the I2C Address

/*---------------------------------------------------
    Register Address
*/
//  Control registers
#define Control1                0x00
#define Control2                0x01
#define Control3                0x02
//  Time and date registers
#define Seconds                 0x03
#define Minutes                 0x04
#define Hours                   0x05
#define Days                    0x06
#define Weekdays                0x07
#define Months                  0x08
#define Years                   0x09

/*---------------------------------------------------
    struct
*/
typedef struct
{
    char    s;
    char    m;
    char    h;
    char    d;
    char    wd;
    char    mm;
    char    y;
} dt_dat;
