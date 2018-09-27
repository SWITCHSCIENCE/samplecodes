/* INA260 test app*/

#include "mbed.h"
#include "INA260.hpp"


// Host PC Communication channels
Serial pc(USBTX, USBRX); // tx, rx
//i2c setup
I2C i2c(PB_9,PB_8);
INA260 VCmonitor(i2c); 
DigitalIn ALT(PA_8); //Alert pin
DigitalIn button(PC_13); //button
int main() { 

    double V,C,P;
    int count = 1;  
    // Sets 4 samples average and sampling time for voltage and current to 8.244ms
    VCmonitor.setConfig(0x0600 | 0x01C0 | 0x0038 | 0x0007); //INA260_CONFIG_AVGRANGE_64|INA260_CONFIG_BVOLTAGETIME_8244US|INA260_CONFIG_SCURRENTTIME_8244US|INA260_CONFIG_MODE_SANDBVOLT_CONTINUOUS
    pc.printf("INA260 TEST!\n");
    pc.printf(""__DATE__" , "__TIME__"\n");
    pc.printf("%d Config register\n",0x0600 | 0x01C0 | 0x0038 | 0x0007); //prints the COnfig reg value to PC COM port
    VCmonitor.setAlert(0x8001); //set current value as alert, latch alert pin
    VCmonitor.setLim(0x0); //set limit to 10mA (10/1.25)

    wait_ms(3000);
while(1)
{
         //get ina260 settings
         if((VCmonitor.getVoltage(&V) == 0) && (VCmonitor.getCurrent(&C) == 0) && (VCmonitor.getPower(&P) == 0))
         {
         pc.printf("%d,  V:%1.3fV ,C:%2.2fmA ,P:%2.1fmW\n",count,V,C,P);
         }
         count++;
         if(ALT==1)
         {
         pc.printf("Overcurrent!!!!");             
         }
         if (button == 0)
         {
             VCmonitor.readAlert(); //read Alert register to clear latched alert
         }
         wait_ms(1000);
}   
}