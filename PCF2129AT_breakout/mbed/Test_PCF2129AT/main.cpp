/*************************************
**  PCF2129AT Test                  **
**                                  **
**  Created : 2015/07/14 11:31:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCF2129AT.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCF2129AT
*/
void get_time(dt_dat *dt) {
    char    cmd[7];

    cmd[0] = Seconds;
    i2c.write(addr, cmd, 1);
    i2c.read(addr, cmd, 7);
    dt->s  = ((cmd[0] >> 4) * 10) + (cmd[0] & 0x0F);
    dt->m  = ((cmd[1] >> 4) * 10) + (cmd[1] & 0x0F);
    dt->h  = ((cmd[2] >> 4) * 10) + (cmd[2] & 0x0F);
    dt->d  = ((cmd[3] >> 4) * 10) + (cmd[3] & 0x0F);
    dt->wd = ((cmd[4] >> 4) * 10) + (cmd[4] & 0x0F);
    dt->mm = ((cmd[5] >> 4) * 10) + (cmd[5] & 0x0F);
    dt->y  = ((cmd[6] >> 4) * 10) + (cmd[6] & 0x0F);
}

void set_time(dt_dat *dt) {
    char    cmd[8];

    cmd[0] = Seconds;
    cmd[1] = ((dt->s  / 10) * 0x10) + (dt->s  % 10);
    cmd[2] = ((dt->m  / 10) * 0x10) + (dt->m  % 10);
    cmd[3] = ((dt->h  / 10) * 0x10) + (dt->h  % 10);
    cmd[4] = ((dt->d  / 10) * 0x10) + (dt->d  % 10);
    cmd[5] =                          (dt->wd % 10);
    cmd[6] = ((dt->mm / 10) * 0x10) + (dt->mm % 10);
    cmd[7] = ((dt->y  / 10) * 0x10) + (dt->y  % 10);
    i2c.write(addr, cmd, 8);
}

void Init_RTC() {              // PCF2129AT Initialize
    dt_dat  dt;

    dt.y  = 15;
    dt.mm = 07;
    dt.d  = 14;
    dt.wd = 02;
    dt.h  = 11;
    dt.m  = 31;
    dt.s  = 00;
    set_time(&dt);
}

/*---------------------------------------------------
    Main Routine
*/
int main() {
    dt_dat  dt;

    Init_RTC();
    myled1 = 0;
    myled2 = 0;
    myled3 = 0;
    myled4 = 0;
    pc.printf("\r\n\r\n");

    while(1) {
        myled4 = 0;
        myled1 = 1;
        wait(0.2);
        myled1 = 0;
        myled2 = 1;
        wait(0.2);
        myled2 = 0;
        myled3 = 1;
        wait(0.2);
        myled3 = 0;
        myled4 = 1;
        wait(0.2);
        get_time(&dt);
        pc.printf("%04d/%02d/%02d(%02d) %02d:%02d:%02d\r\n", 2000 + dt.y, dt.mm, dt.d, dt.wd, dt.h, dt.m, dt.s);
        wait(0.2);
    }
}
