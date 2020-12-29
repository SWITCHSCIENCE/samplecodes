/*********************************************************************
  This is sample program for BT relay board by Switch Science, Inc.
  This program is a modified version of Adafruit nRF52 uart sample program.
  MIT licence.

  This is an example for our nRF52 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

// BT-Relay Hardware
#define BTN 4
#define Relay 5

bool relayStatus;
bool buttonStatus;

void setup()
{
  pinMode(Relay, OUTPUT);
  pinMode(BTN, INPUT);
  digitalWrite(Relay, LOW);
  relayStatus = LOW;
  buttonStatus = digitalRead(BTN);

  Serial.begin(115200);

#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
  while ( !Serial ) yield();
#endif

  Serial.println("Switch Science BlueTooth Relay board");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  // Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("BT Relay board");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Switch Science, Inc.");
  bledis.setModel("BT Relay board");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop()
{
  // Forward data from HW Serial to BLEUART
  while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }

  // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    uint8_t ch;
    ch = (uint8_t) bleuart.read();
    Serial.write(ch);
    if (ch == '1') {
      // Relay ON
      relayStatus = HIGH;
      digitalWrite(Relay, relayStatus);
    } else if (ch == '0') {
      // Relay OFF
      relayStatus = LOW;
      digitalWrite(Relay, relayStatus);
    } else if ((ch == 'T') || (ch == 't') || (ch == '2')) {
      // Relay Toggle
      relayStatus = !relayStatus;
      digitalWrite(Relay, relayStatus);
      Serial.print(relayStatus, HEX);
    } else if ((ch == 's') || (ch == '8')) {
      // Return relay status 
      if (relayStatus) {
        bleuart.write( "On\n", 3 );
        Serial.write("On");
      } else {
        bleuart.write( "Off\n", 4 );
        Serial.write("Off");
      }
    } else if ((ch == 'S') || (ch == '9')) {
      // Return relay status 
      if (relayStatus) {
        bleuart.write( "1", 1 );
        Serial.write("1");
      } else {
        bleuart.write( "0", 1 );
        Serial.write("0");
      }
    }
  }
  bool button = digitalRead(BTN);
  if ((button == HIGH) && (buttonStatus == LOW)) {
    relayStatus = LOW;
    digitalWrite(Relay, relayStatus);
    bleuart.write( "Off\n", 4 );
    Serial.write("Off\n");
    buttonStatus = HIGH;
  }
  if ((button == LOW) && (buttonStatus == HIGH)) {
    relayStatus = HIGH;
    digitalWrite(Relay, relayStatus);
    bleuart.write( "On\n", 4 );
    Serial.write("On\n");
    buttonStatus = LOW;
  }
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle connection where this event happens
   @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
