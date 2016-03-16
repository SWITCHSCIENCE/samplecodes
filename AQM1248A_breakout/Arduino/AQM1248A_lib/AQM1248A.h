#include "Arduino.h"
#include <SPI.h>
#include "Adafruit_GFX.h"

class AQM1248A : public Adafruit_GFX
{
    public:
        AQM1248A(int8_t cs, int8_t rs, int8_t pwr);
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void begin();
        void showPic();
    private:
        void lcdData(uint8_t data);
        void lcdCmd(uint8_t cmd);
        int8_t _CS,_PWR,_RS;
        void initLCD();
};
