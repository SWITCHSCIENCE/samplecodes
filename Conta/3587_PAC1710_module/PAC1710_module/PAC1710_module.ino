#include <Wire.h>
#include "PAC1710.hpp"

#define DEVID PAC1710::ADDR::OPEN // Resistor OPEN (N.C.) at ADDR_SEL pin
const int VSHUNT_mOHM = 10;

/** I2C sending data to device
 */
void datasend(int id,int reg,int *data,int datasize) {
    Wire.beginTransmission(id);
    Wire.write(reg);
    for(int i=0;i<datasize;i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

/** I2C reading data from device
 */
int dataread(int id,int reg,int *data,int datasize) {
    Wire.beginTransmission(id);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(id, datasize, false);
    int i=0;
    while((i<datasize) && Wire.available()) {
        data[i] = Wire.read();
        i++;
    }
    return Wire.endTransmission(true);
}


uint16_t getID() {
    int id[2] = {0};
    dataread(DEVID, PAC1710::REG::PID, id, 2);
    return (id[0] << 8) | id[1];
}

void setup() {
    Wire.begin();
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    Serial.print("Reading PID and MID... ");
    Serial.println(getID(), HEX); // expected "585D" in PAC1710

    // Initialize PAC1710 here...
    // Default sample rate 80ms => Denominator: 2047
    // Default sample range => +-80 mV

}

void loop() {
    uint16_t ch1Vsense[2] = {0};
    uint16_t ch1Vsource[2] = {0};

    dataread(DEVID, PAC1710::REG::C1_SVRES_H, ch1Vsense, 2); // CHANNEL 1 VSENSE RESULT REGISTER 
    dataread(DEVID, PAC1710::REG::C1_VVRES_H, ch1Vsource, 2); // CHANNEL 1 VSOURCE RESULT REGISTER 

    float Ibus_mA = ( (int16_t(ch1Vsense[0] << 8 | (ch1Vsense[1])) >>4) * 3.9082);
    // 3.9082 is magic value in default denominator 2047, Rsenes 10mOhm, Measure range += 80mV
    // see "4.4 Current Measurement"
    // FSC = 8A

    float measuredVsource = (int16_t((ch1Vsource[0] << 3) | (ch1Vsource[1] >> 5) ) * 19.531); 
    // 19.531 is magic value in default denominator 2047
    // see "4.5 Voltage Measurement"

    Serial.print("\Ibus: ");
    Serial.print(Ibus_mA);
    Serial.print(" mA,");

    Serial.print("\tVsource: ");
    Serial.print(measuredVsource);
    Serial.println(" mV");

    delay(100);
}