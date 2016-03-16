/*
  SDlogger.cpp
  read/write data from/into SD card
  original file: SD/Datalogger.ino, public domain
  edited by Switch Science https://www.switch-science.com/
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "SDlogger.h"

const int chipSelect = 15;

bool SDlogger::init()
{

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return false;
  }
  Serial.println("card initialized.");
  return true;
}

void SDlogger::recordData(String fileName, String dataStream)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataStream);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataStream);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file");
  }
}
