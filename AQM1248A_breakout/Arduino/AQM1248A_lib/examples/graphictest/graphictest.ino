#include "Adafruit_GFX.h"
#include "AQM1248A.h"
#define PWR 8
#define RS  9
#define CS  10
AQM1248A aqm = AQM1248A(CS,RS,PWR);
void setup()
{
    Serial.begin(9600);
    aqm.begin(); // 初期化します
    // 初期画像
    aqm.showPic(); // バッファの内容を液晶に反映します(必須)
    delay(1000);
    aqm.fillScreen(0);
    aqm.showPic();
}

void loop()
{
    // 円の描画
    aqm.drawCircle(63,24,10,1);
    aqm.showPic();
    delay(1000);
    aqm.setCursor(0,0);
    aqm.setTextColor(1);
    aqm.setTextSize(1);
    aqm.println("circle");
    aqm.showPic();
    delay(1000);

    aqm.fillScreen(1);aqm.showPic();

    // 三角形の表示
    aqm.fillTriangle(43,47,73,47,63,0,0);
    aqm.showPic();
    delay(1000);
    aqm.setCursor(0,0);
    aqm.setTextColor(0);
    aqm.setTextSize(2);
    aqm.println("triangle");
    aqm.showPic();
    delay(1000);

    aqm.fillScreen(0);aqm.showPic();

    // 四角形の表示
    aqm.fillRoundRect(43,5,30,30,3,1);
    aqm.showPic();
    delay(1000);
    aqm.setCursor(0,0);
    aqm.setTextColor(1);
    aqm.setTextSize(3);
    aqm.println("rectangle");
    aqm.showPic();
    delay(1000);

    aqm.fillScreen(0);aqm.showPic();

    // sin波の表示
    for(int i=0;i<127;i++)
    {
        aqm.drawPixel(i,(int)(sin(i*0.2)*20) + 24,1);
        aqm.showPic();
    }
    delay(1000);
    aqm.fillScreen(0);aqm.showPic();


}
