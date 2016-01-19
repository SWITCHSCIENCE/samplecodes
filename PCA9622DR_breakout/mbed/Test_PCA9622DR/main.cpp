/*************************************
**  PCA9622DR Test                  **
**                                  **
**  Created : 2015/08/28 14:33:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9622DR.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9622DR
*/
static unsigned char led_dat[16];

/*---------------------------------------------------
    PCA9622DR
*/
void InitLEDdriver();
unsigned char ReadLEDdata(int num);
void WriteLEDdata(int num);
void ClearLEDdata();
void AddLEDdata(unsigned short pat, unsigned char a);
void WriteLED();

/*---------------------------------------------------
    PCA9622DR
*/
void InitLEDdriver() {              // PCA9622DR Initialize
    char    cmd[5];

    // Mode 0 Set
    cmd[0] = MODE1;
    cmd[1] = 0x00;              // SLEEP = 0
    i2c.write(PCA9622DR_addr, cmd, 2);
    // Port Mode Configration
    cmd[0] = LEDOUT0 + 0x80;
    cmd[1] = 0xAA;              // LED3 ,2 ,1 ,0  PWM(=10)
    cmd[2] = 0xAA;              // LED7 ,6 ,5 ,4  PWM(=10)
    cmd[3] = 0xAA;              // LED11,10,9 ,8  PWM(=10)
    cmd[4] = 0xAA;              // LED15,14,13,12 PWM(=10)
    i2c.write(PCA9622DR_addr, cmd, 5);
    // LED all clear
    ClearLEDdata();
    WriteLED();
}

unsigned char ReadLEDdata(int num) {   // Read LED data
    return led_dat[num & 0x0F];
}

void WriteLEDdata(int num, unsigned char dat) {   // Write LED data
    led_dat[num & 0x0F] = dat;
}

void ClearLEDdata() {
    int             i;

    for (i = 0; i < 16; i++) led_dat[i] = 0;
}

void AddLEDdata(unsigned short pat, unsigned char a) {    // add LED data
    int             i;

    for (i = 0; i < 16; i++) {
        if ((pat & (0x0001 << i)) == (0x0001 << i)) led_dat[i] = (led_dat[i] + a) & 0x00FF;
    }
}

void WriteLED() {   // Write to LED
    char            cmd[17];
    int             i;

    cmd[0] = PPWM0 + 0x80;
    for (i = 0; i < 16; i++) {
        cmd[i + 1] = (char)led_dat[i];
    }
    i2c.write(PCA9622DR_addr, cmd, 17);
}
/*---------------------------------------------------
    Main Routine
*/
int main() {
    int     stat, i;

    InitLEDdriver();
    myled1 = 0;
    myled2 = 0;
    myled3 = 0;
    myled4 = 0;
    stat = 0;
 
    while(1) {
        // 1bit づつ点灯
        for (i = 0; i < 16; i++) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x0001 << i, 4);
                if (ReadLEDdata(i) > 0xFB) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.001);
            }
        }
        // 1bit づつ消灯
        for (i = 0; i < 16; i++) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x0001 << i, 0xFC);
                if (ReadLEDdata(i) < 4) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.001);
            }
        }
        wait(0.25);
        // 4bit づつ点灯（1,2,3,4）
        for (i = 0; i < 16; i += 4) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x000F << i, 2);
                if (ReadLEDdata(i) > 0xFD) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.001);
            }
        }
        // 4bit づつ消灯（1,2,3,4）
        for (i = 0; i < 16; i += 4) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x000F << i, 0xFE);
                if (ReadLEDdata(i) < 2) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.001);
            }
        }
        wait(0.25);
        // 4bit づつ点灯（1,5,9,13）
        for (i = 0; i < 4; i++) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x1111 << i, 2);
                if (ReadLEDdata(i) > 0xFD) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.001);
            }
        }
        // 4bit づつ消灯（1,5,9,13）
        for (i = 0; i < 4; i++) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x1111 << i, 0xFE);
                if (ReadLEDdata(i) < 2) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.001);
            }
        }
        wait(0.25);
        // 4bit づつ点灯（13,14,15,16）
        for (i = 12; i >= 0; i -= 4) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x000F << i, 2);
                if (ReadLEDdata(i) > 0xFD) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.001);
            }
        }
        // 4bit づつ消灯（13,14,15,16）
        for (i = 12; i >= 0; i -= 4) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x000F << i, 0xFE);
                if (ReadLEDdata(i) < 2) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.001);
            }
        }
        wait(0.25);
        // 4bit づつ点灯（4,8,12,16）
        for (i = 3; i >= 0; i--) {
            while (ReadLEDdata(i) < 0xFF) {
                AddLEDdata(0x1111 << i, 2);
                if (ReadLEDdata(i) > 0xFD) WriteLEDdata(i, 0xFF);
                WriteLED();
                wait(0.001);
            }
        }
        // 4bit づつ消灯（4,8,12,16）
        for (i = 3; i >= 0; i--) {
            while (ReadLEDdata(i) > 0) {
                AddLEDdata(0x1111 << i, 0xFE);
                if (ReadLEDdata(i) < 2) WriteLEDdata(i, 0);
                WriteLED();
                wait(0.001);
            }
        }
        wait(0.25);
        // 市松１点灯
        while (ReadLEDdata(0) < 0xFF) {
            AddLEDdata(0xA5A5, 1);
            WriteLED();
            wait(0.001);
        }
        wait(0.25);
        // 市松１消灯
        while (ReadLEDdata(0) > 0) {
            AddLEDdata(0xA5A5, 0xFF);
            WriteLED();
            wait(0.001);
        }
        wait(0.25);
        // 市松２点灯
        while (ReadLEDdata(1) < 0xFF) {
            AddLEDdata(0x5A5A, 1);
            WriteLED();
            wait(0.001);
        }
        wait(0.25);
        // 市松２消灯
        while (ReadLEDdata(1) > 0) {
            AddLEDdata(0x5A5A, 0xFF);
            WriteLED();
            wait(0.001);
        }
        wait(0.25);
        // 全点灯
        while (ReadLEDdata(0) < 0xFF) {
            AddLEDdata(0xFFFF, 1);
            WriteLED();
            wait(0.001);
        }
        wait(0.25);
        // 全消灯
        while (ReadLEDdata(0) > 0) {
            AddLEDdata(0xFFFF, 0xFF);
            WriteLED();
            wait(0.001);
        }
        wait(0.5);
    }
}
