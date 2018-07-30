
/*
  Read the temperature pixels from the MLX90640 IR array
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14769

  This example initializes the MLX90640 and outputs the 768 temperature values
  from the 768 pixels.

  This example will work with a Teensy 3.1 and above. The MLX90640 requires some
  hefty calculations and larger arrays. You will need a microcontroller with 20,000
  bytes or more of RAM.

  This relies on the driver written by Melexis and can be found at:
  https://github.com/melexis/mlx90640-library

  Hardware Connections:
  Connect the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  to the Qwiic board
  Connect the male pins to the Teensy. The pinouts can be found here: https://www.pjrc.com/teensy/pinout.html
  Open the serial monitor at 9600 baud to see the output
*/
#include <M5Stack.h>
#include <Wire.h>
#include "colors.h"

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

static float mlx90640To[768];
paramsMLX90640 mlx90640;
//Returns true if the MLX90640 is detected on the I2C bus

enum THERMO_MODE {RAINBOW, GRAYSCALE};
THERMO_MODE thermo_mode = RAINBOW;

boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

void setup()
{
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  M5.begin();
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);

  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  M5.Lcd.println("MLX90640 IR Array Example");

  if (isConnected() == false)
  {
    M5.Lcd.println("MLX90640 not detected at default I2C addres. Please check wiring. Freezing.");
    while (1);
  }
  M5.Lcd.println("MLX90640 online!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    M5.Lcd.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    M5.Lcd.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array

  
  delay(1000);
  M5.Lcd.fillScreen(TFT_BLACK);
}

void loop()
{
  // check view mode
  if(M5.BtnA.wasPressed()) thermo_mode = RAINBOW;
  if(M5.BtnC.wasPressed()) thermo_mode = GRAYSCALE;
  
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }


//  M5.Lcd.fillScreen(TFT_BLACK);
  float maxT =0.0, minT = 100.0;
  for(int i=0;i<768;i++) {
    if(maxT < mlx90640To[i]) maxT = mlx90640To[i];
    if(minT > mlx90640To[i]) minT = mlx90640To[i];
  }
  switch(thermo_mode) {
    case RAINBOW:
      // draw canvas
      for (int h = 0 ; h <24 ; h++)
      {
        for(int v = 0; v < 32; v++) {
          float t = mlx90640To[32*h + v];
          float mapped = (t - minT)/(maxT-minT); // 正規化
          int r,g,b;
          int id = int(mapped*100);
          M5.Lcd.fillRect(240 - h*7, v*7, 7, 7, colors[id] ); // vertial, horizontal, size_v, size_h
        }
      }
      // color pallet
      for(int i=0; i<100; i++){
        M5.Lcd.fillRect( 250, 10 + 2*i, 30, 2, colors[100-i]);
      }

      break;
    case GRAYSCALE:
      // draw canvas
      for (int h = 0 ; h <24 ; h++)
      {
        for(int v = 0; v < 32; v++) {
          float t = mlx90640To[32*h + v];
          float mapped = (t - minT)/(maxT-minT); // 正規化
          int r,g,b;
          int id = int(mapped*255);
          M5.Lcd.fillRect(240 - h*7, v*7, 7, 7, M5.Lcd.color565(id,id,id) ); // vertial, horizontal, size_v, size_h
        }
      }
      // color pallet
      for(int i=0; i<100; i++){
        int co = int((100-i)/100.0*255);
        M5.Lcd.fillRect( 250, 10 + 2*i, 30, 2, M5.Lcd.color565(co,co,co));
      }

      break;
      
  }

  // some information
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(0,10);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("MLX90640\nDEMO with\nM5Stack");

  M5.Lcd.print("\n\nMFT2018\nSwitch Science");

  M5.Lcd.setCursor(0,230);
  M5.Lcd.print("Button A: Rainbow C: Grayscale");

  // show tempreture
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillRect( 290, 10, 30, 30, 0x0000);
  M5.Lcd.setCursor(290, 10);
  M5.Lcd.print(maxT);

  M5.Lcd.setCursor(290, 210);
  M5.Lcd.fillRect( 290,210, 30, 30, 0x0000);
  M5.Lcd.print(minT);
  
  delay(10);
  M5.update();
}



