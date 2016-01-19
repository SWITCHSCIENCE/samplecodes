/*************************************
**  SC16IS750 test                  **
**                                  **
**  Created : 2015/06/26 12:45:00   **
**  Author  : O.aoki                **
**************************************/

#include "mbed.h"
#include "SC16IS750.h"

I2C i2c(p9, p10);               // sda, scl
Serial pc(USBTX, USBRX);        // tx, rx

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

/*---------------------------------------------------
    Serial
*/
void Serial_Init() {            // base CK = 12MHz
    char cmd[2];

// ボーレイト設定
    cmd[0] = LCR << 3;
    cmd[1] = 0x83;
    i2c.write(addr, cmd, 2);    // Divisor Latch Enable
    cmd[0] = DLL << 3;
    cmd[1] = 78;
    i2c.write(addr, cmd, 2);    // divisor latch LSB
    cmd[0] = DLH << 3;
    cmd[1] = 0;
    i2c.write(addr, cmd, 2);    // divisor latch MSB

// 通信フォーマット設定
    cmd[0] = LCR << 3;
    cmd[1] = 0x03;
    i2c.write(addr, cmd, 2);

// FIFO設定
    cmd[0] = FCR << 3;
    cmd[1] = 0x57;
    i2c.write(addr, cmd, 2);
}

/*---------------------------------------------------
    Main Routine
*/
int main() {
    char cmd[0x100], str[0x100];
    int  i, j;

    Serial_Init();

    while(1) {
        for (i = 0; i < 0x100; i++) str[i] = 0;
        pc.printf("\r\n  Input string >");
        pc.scanf("%s", str);
        str[0xff] = 0;
        j = strlen(str);
        pc.printf("\r\n  - Trans string = %s (%d) \r\n", str, j);
        cmd[0] = THR << 3;
        for (i = 0; i < j; i++) cmd[i + 1] = str[i];
        i2c.write(addr, cmd, j + 1);

        wait(0.2);
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

        cmd[0] = RHR << 3;
        i2c.write(addr, cmd, 1, true);
        i2c.read(addr, cmd, j);
        cmd[j] = 0;
        pc.printf("  - Receive string = %s (%d) \r\n", cmd, j);
    }
}