/*
  ESPrWiFiClock.ino
  ESP-WROOM-02でNTPサーバーから取得した時間をAQM1248A小型液晶に表示する
  http://mag.switch-science.com/

  require:
    Arduino Timer Library(http://playground.arduino.cc/Code/Time)
    Adafruit GFX Library (https://github.com/adafruit/Adafruit-GFX-Library)
    Switch Science AQM1248A Library(https://github.com/SWITCHSCIENCE/samplecodes/tree/master/AQM1248A_breakout/Arduino/AQM1248A_lib)
*/
#include "AQM1248A.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include <Ticker.h>

extern "C" {
#include <user_interface.h>
}

#define RS  16
#define CS  5
AQM1248A aqm = AQM1248A(CS,RS,4);

char ssid[] = " "; // SSID
char pass[] = " "; // PASSWORD


unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* ntpServerName = "ntp.nict.jp";
IPAddress timeServerIP; // time.nist.gov NTP server address
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;

#define UTC_TOKYO +9
unsigned long unixtime = 0;
char date_ymd[20];
char date_hms[20];
String weeks[8] = { "NaN", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" } ;

long timeLastConnected;

Ticker secCounter;

void setup()
{
    Serial.begin(9600);
    Serial.println("SSCI Wifi Clock");
    aqm.begin(); 
    aqm.showPic();

    delay(500);

    aqm.fillScreen(0);    aqm.showPic();

    aqm.setCursor(0,0);
    aqm.setTextColor(1);
    aqm.setTextSize(1);
    
    Serial.print("Connecting to ");
    aqm.print("Connecting to ");aqm.showPic();
    Serial.println(ssid);
    aqm.println("AP");aqm.showPic();
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    aqm.println("WiFi connected");aqm.showPic();
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    aqm.println(WiFi.localIP());aqm.showPic();

    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());

    delay(1000);
    aqm.fillScreen(0);    aqm.showPic();

    while(!getTime(&unixtime));
    secCounter.attach_ms(1000, incrementSecond);

    wifi_set_sleep_type(MODEM_SLEEP_T);

}

void loop()
{
    if(millis() - timeLastConnected > 1000*60*10) // 10分に一度更新
    {
        if(WiFi.status() != WL_CONNECTED)
        {
            log("connection failed");
        }else{
            /* 時間取得 */
            if(getTime(&unixtime))
            {
                time_t t = unixtime + (UTC_TOKYO * 60 * 60); // 日本標準時に調整
                sprintf(date_ymd, "%04d-%02d-%2d", year(t), month(t), day(t));
                sprintf(date_hms, "%02d:%02d:%02d", hour(t), minute(t), second(t));
                log("time updated");
            }else{
                log("time update failed");
            }
        }
        timeLastConnected = millis(); 
    }

    showClock();

    aqm.fillScreen(0);
}


void log(String s)
{
    Serial.print(date_ymd);
    Serial.print(" ");
    Serial.print(date_hms);
    Serial.print(": ");
    Serial.println(s);
}

/* unixtime を液晶に表示 */
void showClock()
{
    time_t t = unixtime + (UTC_TOKYO * 60 * 60); // 日本標準時に調整
    sprintf(date_ymd, "%04d-%02d-%2d", year(t), month(t), day(t));
    sprintf(date_hms, "%02d:%02d:%02d", hour(t), minute(t), second(t));

    /* アナログ時計を表示 */
    aqm.drawCircle(110,30,15,1);
    aqm.drawLine(110,30, 110 + 13 * cos( (second(t)/60.0 *2 -0.5)  *PI  ), 30 + 13 * sin( (second(t)/60.0 *2 -0.5)*PI) , 1 );
    aqm.drawLine(110,30, 110 + 8 * cos( (hour(t)/12.0 *2 -0.5)*PI ), 30 + 8 * sin( (hour(t)/12.0 *2 -0.5)*PI), 1 );
    aqm.drawLine(110,30, 110 + 15 * cos( (minute(t)/60.0 *2 -0.5)*PI ), 30 + 15 * sin( (minute(t)/60.0 *2 -0.5)*PI) , 1 );

    /* デジタル時計を表示 */
    aqm.setCursor(0,0);
    aqm.setTextColor(1);
    aqm.setTextSize(1);
    aqm.print(date_ymd);
    aqm.println(" " + weeks[weekday(t)]);
    aqm.setTextSize(2);
    aqm.println(date_hms);
    aqm.showPic();
}

/* 1秒ごとに実行される*/
void incrementSecond()
{
    unixtime++;
}

/* 時間取得 */
bool getTime(unsigned long *unixtime)
{
 //get a random server from the pool
 WiFi.hostByName(ntpServerName, timeServerIP);

 sendNTPpacket(timeServerIP); // send an NTP packet to a time server
 // wait to see if a reply is available
 for(unsigned int i = millis(); (millis() - i) < 1000; )
 {
    aqm.fillScreen(0);
    showClock();
    delay(1);
 }

 int cb = udp.parsePacket();
 if (!cb) {
   log("no packet yet");
   return false;
 }
 else {
   //Serial.print("packet received, length=");
   //Serial.println(cb);
   // We've received a packet, read the data from it
   udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

   //the timestamp starts at byte 40 of the received packet and is four bytes,
   // or two words, long. First, esxtract the two words:

   unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
   unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
   // combine the four bytes (two words) into a long integer
   // this is NTP time (seconds since Jan 1 1900):
   unsigned long secsSince1900 = highWord << 16 | lowWord;
   //Serial.print("Seconds since Jan 1 1900 = " );
   //Serial.println(secsSince1900);

   // now convert NTP time into everyday time:
   // Serial.print("Unix time = ");
   // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
   const unsigned long seventyYears = 2208988800UL;
   // subtract seventy years:
   unsigned long epoch = secsSince1900 - seventyYears;
   // print Unix time:
   // Serial.println(epoch);
   *unixtime = epoch;
   return true;
 }
}

/* パケット送信 */
void sendNTPpacket(IPAddress& address)
{
 log("sending NTP packet...");
 // set all bytes in the buffer to 0
 memset(packetBuffer, 0, NTP_PACKET_SIZE);
 // Initialize values needed to form NTP request
 // (see URL above for details on the packets)
 packetBuffer[0] = 0b11100011;   // LI, Version, Mode
 packetBuffer[1] = 0;     // Stratum, or type of clock
 packetBuffer[2] = 6;     // Polling Interval
 packetBuffer[3] = 0xEC;  // Peer Clock Precision
 // 8 bytes of zero for Root Delay & Root Dispersion
 packetBuffer[12]  = 49;
 packetBuffer[13]  = 0x4E;
 packetBuffer[14]  = 49;
 packetBuffer[15]  = 52;

 // all NTP fields have been given values, now
 // you can send a packet requesting a timestamp:
 udp.beginPacket(address, 123); //NTP requests are to port 123
 udp.write(packetBuffer, NTP_PACKET_SIZE);
 udp.endPacket();
}