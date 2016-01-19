/*************************************
**  PCA9632DP1 test                 **
**                                  **
**  Created : 2015/07/31 16:59:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9632DP1.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9632DP1
*/
void Init_LED_Driver() {              // PCA9632DP1 Initialize
    char cmd[2];

    // Port 0 Configration
    cmd[0] = MODE1;
    cmd[1] = 0x00;              // SLEEP = 0
    i2c.write(addr, cmd, 2);
    // Port 1 Configration
    cmd[0] = LEDOUT;
    cmd[1] = 0xAA;              // LED3,2,1,0 PWM(=10)
    i2c.write(addr, cmd, 2);
}

/*---------------------------------------------------
    Main Routine
*/
int main() {
    char    cmd[5], sub[5];
    int     cnt, i;

    Init_LED_Driver();
    myled1 = 0;
    myled2 = 0;
    myled3 = 0;
    myled4 = 0;
    cnt = 0;
    cmd[0] = PPWM0 + 0x80;
    cmd[1] = 0x60;
    cmd[2] = 0x40;
    cmd[3] = 0x20;
    cmd[4] = 0x00;
    for (i = 0; i < 5; i++) sub[i] = 1;
    i2c.write(addr, cmd, 5);

    while(1) {
        wait(0.002);
        cmd[0] = PPWM0 + 0x80;
        for (i = 1; i < 5; i++) {
            cmd[i] += sub[i];
            if (cmd[i] == 0x00) sub[i] = 1;
            if (cmd[i] == 0x7F) sub[i] = -1;
        }
        i2c.write(addr, cmd, 5);
    }
}
