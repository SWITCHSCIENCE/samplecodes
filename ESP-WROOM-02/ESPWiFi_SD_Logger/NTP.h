////////////////////////////////////////////////////////////////////////////////
/*
  NTP.h
  original file: ESP8266WiFi/NTPClient.ino, public domain
  edited by Switch Science https://www.switch-science.com/
 */
////////////////////////////////////////////////////////////////////////////////
/*orginal header

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 updated for the ESP8266 12 Apr 2015
 by Ivan Grokhotkov

 This code is in the public domain.

*/
////////////////////////////////////////////////////////////////////////////////
#pragma once

// NTP time stamp is in the first 48 bytes of the message
const int NTP_PACKET_SIZE = 48;

class NTPClient
{
  public:
    NTPClient(){};

      /* Initialize */
    void udpSetup();

    // @param (*unixtime) set unixtime into this variavble
    // @return if get time from server failed, return false
    bool getTime(unsigned long *unixtime);

  private:
    // send an NTP request to the time server at the given address
    unsigned long sendNTPpacket(IPAddress&);

    unsigned int localPort = 2390;      // local port to listen for UDP packets

    /* Don't hardwire the IP address or we won't get the benefits of the pool.
    *  Lookup the IP address for the host name instead */
    //IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
    IPAddress timeServerIP; // time.nist.gov NTP server address


    uint8_t packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

    // A UDP instance to let us send and receive packets over UDP
    WiFiUDP udp;
};
