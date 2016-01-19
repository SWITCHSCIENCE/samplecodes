/*************************************
**  SC16IS750.h                     **
**                                  **
**  Created : 2015/06/26 15:24:00   **
**  Author  : O.aoki                **
**************************************/


/*---------------------------------------------------
    Constant value
*/
const int addr = 0x98;      // define the I2C Address

/*---------------------------------------------------
    Register Address
*/
/* ---- General register set ---- */
#define THR     0x00
#define RHR     0x00
#define FCR     0x02
#define LCR     0x03
#define MCR     0x04

/* ---- Special register set ---- */
#define DLL     0x00
#define DLH     0x01

/* ---- Enhanced register set ---- */
#define EFR     0x02
