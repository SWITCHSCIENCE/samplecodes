/*************************************
**  PCA9629APW.h                    **
**                                  **
**  Created : 2015/09/25 12:23:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9629APW.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9629APW
*/
void InitMotorDriver();

/*---------------------------------------------------
    PCA9624PW
*/
void InitMotorDriver() {                // PCA9622DR Initialize
    char    cmd[2];

    // Mode Set
    cmd[0] = MODE;
    cmd[1] = 0x00;                      // Normal
    i2c.write(PCA9629APW_addr, cmd, 2);
    // Phase ìÆçÏê›íË
    cmd[0] = OP_CFG_PHS;
    cmd[1] = (0x2 << 6) + 0x10;         // 1Phase,OUT[3:0] => motor drive
    i2c.write(PCA9629APW_addr, cmd, 2);
}

/*---------------------------------------------------
    Main Routine
*/
int main() {
    char            cmd[4];
    unsigned long   dum;

    InitMotorDriver();

    while(1) {

        myled3 = 1;

        cmd[0] = CWPWL + 0x80;              // Auto Increment
        cmd[1] = 0xFF;                      // 98.304ms [24us * 8191(0x1FFF)]
        cmd[2] = (0x03 << 5) + 0x1F;        // Prescaler = 3
        i2c.write(PCA9629APW_addr, cmd, 3);

        cmd[0] = CCWPWL + 0x80;             // Auto Increment
        cmd[1] = 0x00;                      // 98.304ms [24us * 4096(0x1000)]
        cmd[2] = (0x03 << 5) + 0x10;        // Prescaler = 3
        i2c.write(PCA9629APW_addr, cmd, 3);

        cmd[0] = CWSCOUNTL + 0x80;          // Auto Increment
        cmd[1] = 0x20;                      // 32 steps
        cmd[2] = 0x00;
        i2c.write(PCA9629APW_addr, cmd, 3);

        cmd[0] = CCWSCOUNTL + 0x80;         // Auto Increment
        cmd[1] = 0x40;                      // 64 steps
        cmd[2] = 0x00;
        i2c.write(PCA9629APW_addr, cmd, 3);

        cmd[0] = PMA;
        cmd[1] = 0x02;                      // One time
        i2c.write(PCA9629APW_addr, cmd, 2);

        cmd[0] = MCNTL;
        cmd[1] = 0x80 + 0x02;               // start , CW -> CCW 
        i2c.write(PCA9629APW_addr, cmd, 2);

        myled3 = 0;

        while (1) {
            wait(0.5);
            cmd[0] = 0x80 + STEPCOUNT0;
            i2c.write(PCA9629APW_addr, cmd, 1, true);
            i2c.read(PCA9629APW_addr, cmd, 4);
            dum = cmd[0] + (cmd[1] << 8) + (cmd[2] << 16) + (cmd[3] << 24);
            if (dum == 0) break;
        }

//        myled3 = 0;
    }   
}
