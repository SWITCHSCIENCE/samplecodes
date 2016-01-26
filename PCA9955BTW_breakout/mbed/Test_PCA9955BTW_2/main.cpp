/*************************************
**  PCA9955BTW Test 2               **
**                                  **
**  Created : 2016/01/26 12:07:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "PCA9955BTW.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    PCA9955BTW
*/
static unsigned char led_dat[32];

/*---------------------------------------------------
    PCA9955BTW
*/
void InitLEDdriver();
unsigned char ReadLEDdata(int num);
void WriteLEDdata(int num);
void ClearLEDdata();
void AddLEDdata(unsigned long pat, unsigned char a);
void WriteLED();

/*---------------------------------------------------
    PCA9955BTW
*/
void InitLEDdriver() {              // PCA9955BTW Initialize
    char    cmd[7];
    int     i;

    // Mode 1/2 Set
    cmd[0] = MODE1 + 0x80;
    cmd[1] = 0x01;              // MODE1:SLEEP = 0
    cmd[2] = 0x05;              // MODE2:EXP_EN = 1
    i2c.write(PCA9955BTW_1_addr, cmd, 3);
    i2c.write(PCA9955BTW_2_addr, cmd, 3);
    // Current Set
    cmd[0] = IREFALL;
    cmd[1] = 0xFF;
    i2c.write(PCA9955BTW_1_addr, cmd, 2);
    i2c.write(PCA9955BTW_2_addr, cmd, 2);
    // Port Mode Configration
    cmd[0] = LEDOUT0 + 0x80;
    cmd[1] = 0xAA;              // LED3 ,2 ,1 ,0  PWM(=10)
    cmd[2] = 0xAA;              // LED7 ,6 ,5 ,4  PWM(=10)
    cmd[3] = 0xAA;              // LED11,10,9 ,8  PWM(=10)
    cmd[4] = 0xAA;              // LED15,14,13,12 PWM(=10)
    i2c.write(PCA9955BTW_1_addr, cmd, 5);
    i2c.write(PCA9955BTW_2_addr, cmd, 5);
//    // LED all clear
//    ClearLEDdata();
//    WriteLED();
}

unsigned char ReadLEDdata(int num) {   // Read LED data
    return led_dat[num & 0x1F];
}

void WriteLEDdata(int num, unsigned char dat) {   // Write LED data
    led_dat[num & 0x1F] = dat;
}

void ClearLEDdata() {
    int             i;

    for (i = 0; i < 32; i++) led_dat[i] = 0;
}

void AddLEDdata(unsigned long pat, unsigned char a) {    // add LED data
    int             i;

    for (i = 0; i < 32; i++) {
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
    i2c.write(PCA9955BTW_1_addr, cmd, 17);
    for (i = 0; i < 16; i++) {
        cmd[i + 1] = (char)led_dat[i + 16];
    }
    i2c.write(PCA9955BTW_2_addr, cmd, 17);
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
        if ((cnt & 0x0107) == 0x0000) WriteLEDdata(     ((cnt >> 3) & 0x001F) , 0xFF);
        if ((cnt & 0x0107) == 0x0100) WriteLEDdata(31 - ((cnt >> 3) & 0x001F) , 0xFF);
        for (i = 0; i < 32; i++) {
            if (ReadLEDdata(i) > 6)
                WriteLEDdata(i, ReadLEDdata(i) - 6);
            else
                WriteLEDdata(i, 0);
        }
        WriteLED();
        cnt++;
        wait(0.001);
    }
}
