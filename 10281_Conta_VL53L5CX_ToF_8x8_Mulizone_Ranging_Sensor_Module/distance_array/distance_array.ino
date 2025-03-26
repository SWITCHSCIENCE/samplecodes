#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>  //http://librarymanager/All#SparkFun_VL53L5CX

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;  // Result data class structure, 1356 byes of RAM

int imageResolution = 0;  //Used to pretty print output
int imageWidth = 0;       //Used to pretty print output

// const int pinSDA = 0;
// const int pinSCL = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  delay(1000);
  Serial.println("SwitchScience VL53L5CX Imager Example");

  // Wire.setSDA(pinSDA);
  // Wire.setSCL(pinSCL);
  // Wire.setPins(pinSDA, pinSCL);
  Wire.begin();  //This resets to 100kHz I2C
  Wire.setClock(1000000);

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (myImager.begin() == false) {
    Serial.println(F("Sensor not found - check your wiring. Freezing"));
    while (1)
      ;
  }

  myImager.setResolution(8 * 8);  //Enable all 64 pads

  imageResolution = myImager.getResolution();  //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution);          //Calculate printing width

  //Using 4x4, min frequency is 1Hz and max is 60Hz
  //Using 8x8, min frequency is 1Hz and max is 15Hz
  bool response = myImager.setRangingFrequency(15);
  if (response == true) {
    int frequency = myImager.getRangingFrequency();
    if (frequency > 0) {
      Serial.print("Ranging frequency set to ");
      Serial.print(frequency);
      Serial.println(" Hz.");
    } else
      Serial.println(F("Error recovering ranging frequency."));
  } else {
    Serial.println(F("Cannot set ranging frequency requested. Freezing..."));
    while (1)
      ;
  }

  myImager.startRanging();
}

void loop() {
  //Poll sensor for new data
  if (myImager.isDataReady() == true) {
    if (myImager.getRangingData(&measurementData))  //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0; y <= imageWidth * (imageWidth - 1); y += imageWidth) {
        for (int x = imageWidth - 1; x >= 0; x--) {
          Serial.print("\t");
          Serial.print(measurementData.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
  }

  delay(5);  //Small delay between polling
}
