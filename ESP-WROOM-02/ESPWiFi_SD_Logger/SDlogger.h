////////////////////////////////////////////////////////////////////////////////
/*
  SDlogger.h
  read/write data from/into SD card
  original file: SD/Datalogger.ino, public domain
  edited by Switch Science https://www.switch-science.com/
 */
////////////////////////////////////////////////////////////////////////////////
/* original header
   SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
  * analog sensors on analog ins 0, 1, and 2
  * SD card attached to SPI bus as follows:
  ** MOSI - pin 11
  ** MISO - pin 12
  ** CLK - pin 13
  ** CS - pin 4

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.
*/
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDlogger
{
    public:
        SDlogger()
        {}
        // @return false:Initialize failed
        bool init();

        // @param (filename)    name of file
        // @param (dataStream)  data witch will be written
        void recordData(String fileName, String dataStream);

    private:
};
