/*
  ESPrIR_sender.ino
  ESPr IR が受信した赤外線信号を記録・シリアルモニタに表示する
  Shop: https://www.switch-science.com/
  Blog: http://mag.switch-science.com/
*/
#define N 2000 // 最大5000程度が目安 in ESP-WROOM-02
#define IRPIN 5
class pinData
{
    public: long sampledTime;
    public: uint8_t data;
};

long timesss;
int lastPinState = LOW;
int nowPinState = LOW;

void printDataList(pinData *dataList)
{
}

void setup()
{
    Serial.begin(115200);
    pinMode(IRPIN, INPUT);
    Serial.println("\n start");
    lastPinState = digitalRead(IRPIN);
    timesss = micros();
}

void loop()
{
    nowPinState = digitalRead(IRPIN);
    if( (lastPinState != nowPinState) && (nowPinState == HIGH) ) // 信号の立ち上がりを検知したら
    {
        /* データの配列を用意 */
        pinData *dataList ;
        dataList = new pinData[N];
        int index = 0;

        /* 最初のデータを記録 */
        dataList[index].sampledTime = micros();
        dataList[index].data = nowPinState;
        lastPinState = nowPinState;

        index++;

        /* 最大1000msの間記録を続ける */
        for(timesss = micros(); micros()-timesss < 1000000; )
        {
            nowPinState = digitalRead(IRPIN);
            if( lastPinState != nowPinState) // 信号の立ち上がり or 立ち下がりを検知したら
            {
                dataList[index].sampledTime = micros();
                dataList[index].data = nowPinState;
                index++;
                if(index>N) break;
            }
        lastPinState = nowPinState;
        }
        Serial.print("unsigned int dumpedData[");
        Serial.print(index-1);
        Serial.println("] = {");
        for(int i=1; i<index;i++)
        {
 
            Serial.print(dataList[i].sampledTime - dataList[0].sampledTime);
            Serial.print("\t");            
            Serial.println(dataList[i].data);
            
            delay(1); // WDT対策
        }
        Serial.println("\n};");
        delete [] dataList;
    }
    lastPinState = nowPinState;

}

