#include "M5CoreInk.h"

#include <WiFi.h>
#include <time.h>

Ink_Sprite InkPageSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;

char timeStrbuff[64];

static const int JST = 3600 * 9;
static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};

RTC_SLOW_ATTR bool ntpDataFlag = false;

const char *ssid = "SSID";
const char *password = "PASS";
time_t t;
struct tm *tm;

int minX, minY, hourX, hourY;
int oldminX, oldminY, oldhourX, oldhourY;
int centerX = 100;
int centerY = 120;

void testDraw()
{
    drawLineInk(99, 0, 99, 0, 0);
    drawLineInk(99, 49, 99, 0, 0);
    drawLineInk(99, 99, 99, 0, 0);
    drawLineInk(99, 149, 99, 0, 0);
    drawLineInk(99, 199, 99, 0, 0);
    drawLineInk(99, 199, 99, 49, 0);
    drawLineInk(99, 199, 99, 99, 0);
    drawLineInk(99, 199, 99, 149, 0);
    drawLineInk(99, 199, 99, 199, 0);
    drawLineInk(99, 149, 99, 199, 0);
    drawLineInk(99, 99, 99, 199, 0);
    drawLineInk(99, 49, 99, 199, 0);
    drawLineInk(99, 0, 99, 199, 0);
    drawLineInk(99, 0, 99, 149, 0);
    drawLineInk(99, 0, 99, 99, 0);
    drawLineInk(99, 0, 99, 49, 0);
}

void setupTime()
{
    t = time(NULL);
    tm = localtime(&t);

    RTCtime.Hours = tm->tm_hour;
    RTCtime.Minutes = tm->tm_min;
    RTCtime.Seconds = tm->tm_sec;
    M5.rtc.SetTime(&RTCtime);

    RTCDate.Year = tm->tm_year + 1900;
    RTCDate.Month = tm->tm_mon + 1;
    RTCDate.Date = tm->tm_mday;
    M5.rtc.SetData(&RTCDate);
}

void drawLineInk(int x1, int x2, int y1, int y2, int C)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int e = 0;

    if (abs(dy) < abs(dx))
    {
        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
            dx = x2 - x1;
            dy = y2 - y1;
        }
        int y = y1;
        if (dy > 0)
        {
            for (int x = x1; x <= x2; ++x)
            {
                InkPageSprite.drawPix(x, y, C);
                e = e + 2 * dy;
                if (e >= dx)
                {
                    y++;
                    e = e - 2 * dx;
                }
            }
        }
        else
        {
            dy = abs(dy);
            for (int x = x1; x <= x2; ++x)
            {
                InkPageSprite.drawPix(x, y, C);
                e = e + 2 * dy;
                if (e >= dx)
                {
                    y--;
                    e = e - 2 * dx;
                }
            }
        }
    }
    else
    {
        if (y1 > y2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
            dx = x2 - x1;
            dy = y2 - y1;
        }
        int x = x1;
        if (dx > 0)
        {
            for (int y = y1; y <= y2; ++y)
            {
                InkPageSprite.drawPix(x, y, C);
                e = e + 2 * dx;
                if (e >= dy)
                {
                    x++;
                    e = e - 2 * dy;
                }
            }
        }
        else
        {
            dx = abs(dx);
            for (int y = y1; y <= y2; ++y)
            {
                InkPageSprite.drawPix(x, y, C);
                e = e + 2 * dx;
                if (e >= dy)
                {
                    x--;
                    e = e - 2 * dy;
                }
            }
        }
    }
}

void setTimePos()
{
    minX = (int)(sin(RTCtime.Minutes * 3.1415 / 30 + RTCtime.Seconds * 3.1415 / 1800) * 60) + centerX;
    minY = (int)(cos(RTCtime.Minutes * 3.1415 / 30 + RTCtime.Seconds * 3.1415 / 1800) * -60) + centerY;
    if (RTCtime.Hours > 12)
    {
        hourX = (int)(sin((RTCtime.Hours - 12) * 3.1415 / 6 + RTCtime.Minutes * 3.1415 / 360) * 30) + centerX;
        hourY = (int)(cos((RTCtime.Hours - 12) * 3.1415 / 6 + RTCtime.Minutes * 3.1415 / 360) * -30) + centerY;
    }
    else
    {
        hourX = (int)(sin(RTCtime.Hours * 3.1415 / 6 + RTCtime.Minutes * 3.1415 / 360) * 30) + centerX;
        hourY = (int)(cos(RTCtime.Hours * 3.1415 / 6 + RTCtime.Minutes * 3.1415 / 360) * -30) + centerY;
    }
}

void setup()
{
    M5.begin();
    Serial.print("\n\nStart\n");
    //  if (ntpDataFlag == false) {
    //    ntpDataFlag = true;
    if (!M5.M5Ink.isInit())
    {
        Serial.printf("Ink Init faild");
        while (1)
            delay(100);
    }
    M5.M5Ink.clear();
    delay(1000);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(500);
    }
    Serial.println();
    Serial.printf("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    delay(2000);
    setupTime();
    //  }
    //create ink refresh Sprite
    if (InkPageSprite.creatSprite(0, 0, 200, 200, true) != 0)
    {
        Serial.printf("Ink Sprite create faild");
    }
    delay(100);
    int oldminX = 0;
    int oldminY = 0;
    int oldhourX = 0;
    int oldhourY = 0;
}

void loop()
{
    M5.rtc.GetTime(&RTCtime);
    M5.rtc.GetData(&RTCDate);
    Serial.printf("%d/%02d/%02d %02d:%02d:%02d\n",
                  RTCDate.Year, RTCDate.Month, RTCDate.Date,
                  RTCtime.Hours, RTCtime.Minutes, RTCtime.Seconds);
    sprintf(timeStrbuff, "%d/%02d/%02d %02d:%02d:%02d",
            RTCDate.Year, RTCDate.Month, RTCDate.Date,
            RTCtime.Hours, RTCtime.Minutes, RTCtime.Seconds);

    InkPageSprite.drawString(20, 0, timeStrbuff);

    testDraw();

    drawLineInk(centerX, centerX, 40, 50, 0);
    drawLineInk(centerX, centerX, 199, 189, 0);
    drawLineInk(20, 30, centerY, centerY, 0);
    drawLineInk(180, 170, centerY, centerY, 0);

    setTimePos();
    drawLineInk(centerX, oldminX, centerY, oldminY, 1);
    drawLineInk(centerX, oldhourX, centerY, oldhourY, 1);
    drawLineInk(centerX, minX, centerY, minY, 0);
    drawLineInk(centerX, hourX, centerY, hourY, 0);

    oldminX = minX;
    oldminY = minY;
    oldhourX = hourX;
    oldhourY = hourY;

    InkPageSprite.pushSprite();
    //  Serial.printf("Go sleep\n");
    delay(60000);
    //  esp_sleep_enable_timer_wakeup(30 * 1000 * 1000);  // wakeup every 5secs
    //  esp_deep_sleep_start();
}
