#include <Wire.h>
#include "PAC1710.hpp"

#define DEVID 0x18 // Resistor OPEN (N.C.) at ADDR_SEL pin

//送信用関数(デバイスアドレス,レジスタアドレス,&送信用データ,送信データ数)
void datasend(int id,int reg,int *data,int datasize) {
    Wire.beginTransmission(id);
    Wire.write(reg);
    for(int i=0;i<datasize;i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

//受信用関数(デバイスアドレス,レジスタアドレス,&受信用データ,受信データ数)
void dataread(int id,int reg,int *data,int datasize) {
    Wire.beginTransmission(id);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(id, datasize, false);
    for(int i=0;i<datasize;i++) {
        data[i] = Wire.read();
    }
    Serial.print(Wire.endTransmission(true),HEX);
}

uint16_t getID() {
    int pid,mid;
    dataread(DEVID, 0xFD, &pid, 1);
    dataread(DEVID, 0xFE, &mid, 1);
    return (pid << 8) && mid;
}

void setup() {
    Wire.begin();
    Serial.begin(115200);
    Serial.print("Reading PID and MID... ");
    Serial.print(getID(), HEX);
    Serial.println("DONE");
    
}

void loop() {
    
}