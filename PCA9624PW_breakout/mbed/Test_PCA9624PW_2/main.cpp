/*************************************
**  PCA9624PW.h x 2                 **
**                                  **
**  Created : 2015/09/18 17:23:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9624PW.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9624PW
*/
static unsigned char led_dat[16];

/*---------------------------------------------------
    PCA9624PW
*/
void InitLEDdriver();
unsigned char ReadLEDdata(int num);
void WriteLEDdata(int num);
void ClearLEDdata();
void AddLEDdata(unsigned short pat, unsigned char a);
void WriteLED();

/*---------------------------------------------------
    PCA9624PW
*/
void InitLEDdriver() {              // PCA9624PW Initialize
    char    cmd[3];

    // Mode 0 Set
    cmd[0] = MODE1;
    cmd[1] = 0x00;              // SLEEP = 0
    i2c.write(PCA9624PW_1_addr, cmd, 2);
    i2c.write(PCA9624PW_2_addr, cmd, 2);
    // Port Mode Configration
    cmd[0] = LEDOUT0 + 0x80;
    cmd[1] = 0xAA;              // LED3 ,2 ,1 ,0  PWM(=10)
    cmd[2] = 0xAA;              // LED7 ,6 ,5 ,4  PWM(=10)
    i2c.write(PCA9624PW_1_addr, cmd, 3);
    i2c.write(PCA9624PW_2_addr, cmd, 3);
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
    char            cmd[9];
    int             i;

    cmd[0] = PPWM0 + 0x80;
    for (i = 0; i < 8; i++) {
        cmd[i + 1] = (char)led_dat[i];
    }
    i2c.write(PCA9624PW_1_addr, cmd, 9);
    for (i = 0; i < 8; i++) {
        cmd[i + 1] = (char)led_dat[i + 8];
    }
    i2c.write(PCA9624PW_2_addr, cmd, 9);
}
/*---------------------------------------------------
    Main Routine
*/
int main() {
    int     cnt, i;

    InitLEDdriver();
    cnt = 0;

    while(1) {
        // 1bit ‚Ã‚Â“_“” -> Ž©“®Á“”
        if ((cnt & 0x010F) == 0x0000) WriteLEDdata(     ((cnt >> 4) & 0x000F) , 0xFF);
        if ((cnt & 0x010F) == 0x0100) WriteLEDdata(15 - ((cnt >> 4) & 0x000F) , 0xFF);
        for (i = 0; i < 16; i++) {
            if (ReadLEDdata(i) > 3)
                WriteLEDdata(i, ReadLEDdata(i) - 3);
            else
                WriteLEDdata(i, 0);
        }
        WriteLED();
        cnt++;
        wait(0.001);
    }
}
