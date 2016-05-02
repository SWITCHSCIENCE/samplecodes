//**************************************************
//      TJA1040 CAN sample プログラム
//              ２台のＴＪＡ1040でのループバックテスト
//                          2016/4/27
//
//**************************************************
#include "mbed.h"

Ticker ticker;                              //タイマー割り込みレジスタ
DigitalOut led1(LED1);                      //ＬＥＤレジスタ
DigitalOut led2(LED2);                      //ＬＥＤレジスタ


                 
CAN can1(p9, p10);                          //ＣＡＮ1のポート設定
CAN can2(p30, p29);                         //ＣＡＮ2のポート設定
char counter = 0;                           //通信回数のカウンター

int frquency(1000);                         //CANの通信速度ｋHz

void send()
{
    printf("send()\n");                     //デバッグ用ターミナルソフトでの表示メッセージ
    if(can1.write(CANMessage(1337, &counter, 1))) {         //データー送信
        printf("wloop()\n");
        counter++;
        printf("Message sent: %d\n", counter);              //送信済みメッセージ表示
        

    }
    led1 = !led1;                           //送信後LED表示
}

int main()
{
    printf("main()\n");
    ticker.attach(&send, 1);                //1秒の割り込み
    CANMessage msg;                         
    while(1) {
        printf("loop()\n");
        if(can2.read(msg)) {                //データー受信
            printf("Message received: %d\n", msg.data[0]);     //受信データー表示
            led2 = !led2;                                   //LED受信表示
        }
        wait(0.2);
    }
}
