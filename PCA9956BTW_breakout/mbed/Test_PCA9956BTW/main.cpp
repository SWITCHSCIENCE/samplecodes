/*************************************
**  PCA9956BTW Test                 **
**                                  **
**  Created : 2016/01/26 12:55:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9956BTW.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9956BTW
*/
static unsigned char led_dat[24];

/*---------------------------------------------------
    PCA9956BTW
*/
void InitLEDdriver();
unsigned char ReadLEDdata(int num);
void WriteLEDdata(int num);
void ClearLEDdata();
void AddLEDdata(unsigned int pat, unsigned char a);
void WriteLED();

/*---------------------------------------------------
    PCA9956BTW
*/
void InitLEDdriver() {              // PCA9626BTW Initialize
    char    cmd[7];

    // Mode 1/2 Set
    cmd[0] = MODE1 + 0x80;
    cmd[1] = 0x01;              // MODE1:SLEEP = 0
    cmd[2] = 0x05;              // MODE2:EXP_EN = 1
    i2c.write(PCA9956BTW_addr, cmd, 3);
    // Current Set
    cmd[0] = IREFALL;
    cmd[1] = 0xFF;
    i2c.write(PCA9956BTW_addr, cmd, 2);
    // Port Mode Configration
    cmd[0] = LEDOUT0 + 0x80;
    cmd[1] = 0xAA;              // LED3 ,2 ,1 ,0  PWM(=10)
    cmd[2] = 0xAA;              // LED7 ,6 ,5 ,4  PWM(=10)
    cmd[3] = 0xAA;              // LED11,10,9 ,8  PWM(=10)
    cmd[4] = 0xAA;              // LED15,14,13,12 PWM(=10)
    i2c.write(PCA9956BTW_addr, cmd, 5);
    // LED all clear
//    ClearLEDdata();
//    WriteLED();
}

unsigned char ReadLEDdata(int num) {   // Read LED data
    int     i;

    i = num & 0x0000001F;
    if (i >= 24) i = 0;
    return led_dat[i];
}

void WriteLEDdata(int num, unsigned char dat) {   // Write LED data
    int     i;

    i = num & 0x0000001F;
    if (i >= 24) i = 0;
    led_dat[i] = dat;
}

void ClearLEDdata() {
    int             i;

    for (i = 0; i < 24; i++) led_dat[i] = 0;
}

void AddLEDdata(unsigned int pat, unsigned char a) {    // add LED data
    int             i;

    for (i = 0; i < 24; i++) {
        if ((pat & (0x00000001 << i)) == (0x00000001 << i)) led_dat[i] = (led_dat[i] + a) & 0x00FF;
    }
}

void WriteLED() {   // Write to LED
    char            cmd[25];
    int             i;

    cmd[0] = PPWM0 + 0x80;
    for (i = 0; i < 24; i++) {
        cmd[i + 1] = (char)led_dat[i];
    }
    i2c.write(PCA9956BTW_addr, cmd, 25);
}
/*---------------------------------------------------
    Main Routine
*/
int main() {
    int     i;

    InitLEDdriver();
    myled1 = 0;
    myled2 = 0;
    myled3 = 0;
    myled4 = 0;

    while(1) {
        // 1bit Ç√Ç¬ì_ìî
        for (i = 0; i < 24; i++) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x00000001 << i, 4);
                if (ReadLEDdata(i) > 0xFB) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.005);
            }
        }
        // 1bit Ç√Ç¬è¡ìî
        for (i = 0; i < 24; i++) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x00000001 << i, 0xFC);
                if (ReadLEDdata(i) < 4) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.005);
            }
        }
    }
}
