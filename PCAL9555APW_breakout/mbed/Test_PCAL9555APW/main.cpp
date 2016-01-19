/*************************************
**  PCAL9555APW test                **
**                                  **
**  Created : 2015/07/10 15:38:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCAL9555APW.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCAL9555APW
*/
void Init_GPIO() {              // PCAL9555APW Initialize
    char cmd[2];

    // Port 0 Configration
    cmd[0] = Configuration0;
    cmd[1] = 0x00;              // all output
    i2c.write(addr, cmd, 2);
    // Port 1 Configration
    cmd[0] = Configuration1;
    cmd[1] = 0xff;              // all input
    i2c.write(addr, cmd, 2);
    // Port 1 Polarity Inversion
    cmd[0] = PolarityInversionPort1;
    cmd[1] = 0xff;              // all Inversion
    i2c.write(addr, cmd, 2);
}

/*---------------------------------------------------
    Main Routine
*/
int main() {
    char cmd[2];

    Init_GPIO();
    myled1 = 0;
    myled2 = 0;
    myled3 = 0;
    myled4 = 0;

    while(1) {
        myled4 = 0;
        myled1 = 1;
        cmd[0] = OutputPort0;
        cmd[1] = 0xfe;
        i2c.write(addr, cmd, 2);
        wait(0.2);
        myled1 = 0;
        myled2 = 1;
        cmd[1] = 0xfd;
        i2c.write(addr, cmd, 2);
        wait(0.2);
        myled2 = 0;
        myled3 = 1;
        cmd[1] = 0xfb;
        i2c.write(addr, cmd, 2);
        wait(0.2);
        myled3 = 0;
        myled4 = 1;
        cmd[1] = 0xf7;
        i2c.write(addr, cmd, 2);
        wait(0.2);
        cmd[0] = InputPort1;
        i2c.write(addr, cmd, 1, true);
        i2c.read(addr, cmd, 1);
        pc.printf("\r\n  Input - %02x", (unsigned char)cmd[0]);
        wait(0.2);
    }
}
