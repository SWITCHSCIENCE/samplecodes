/*
  Rapiro_Genuino101_BLE.ino
  Genuino101を使ってBLE経由でRapiroを制御してみる
  http://mag.switch-science.com/
 */

#include <CurieBLE.h>

#define LEDpin 13

BLEPeripheral blePeripheral;
// NordicのUART用のUUID https://devzone.nordicsemi.com/documentation/nrf51/6.0.0/s110/html/a00066.html
// Nordic UART Service service 1
BLEService RapiroService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

// Nordic UART Service service 1 tx
BLEUnsignedCharCharacteristic characteristic1("6E400002-B5A3-F393-E0A9-E50E24DCCA9E",BLEWriteWithoutResponse);

// Nordic UART Service service 1 rx
BLEUnsignedCharCharacteristic characteristic2("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLENotify);

// Nordic UART Service service 1 rev
BLEUnsignedCharCharacteristic characteristic_rev("1E948DF1-4831-94BA-754C-3E5000003D71", BLERead);

void setup()
{
    Serial.begin(57600);

    Serial.println("BLE start");

    blePeripheral.setLocalName("Rapiro");
    blePeripheral.setAdvertisedServiceUuid(RapiroService.uuid());

    blePeripheral.addAttribute(RapiroService);
    blePeripheral.addAttribute(characteristic1);
    blePeripheral.addAttribute(characteristic2);

    // 接続時・切断時
    blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // characteristic1に値が書き込まれた時
    characteristic1.setEventHandler(BLEWritten, onWrittenTX);

    // 初期化
    characteristic1.setValue(0);
    characteristic2.setValue(0);

    blePeripheral.begin();
}

void loop()
{
    blePeripheral.poll();
}

/* characteristic1に値が書き込まれた時 */
void onWrittenTX(BLECentral& central, BLECharacteristic & characteristic)
{
  digitalWrite(LEDpin, LOW);
  Serial.print("Characteristic event, written: ");
  unsigned char _value = characteristic1.value();
  Serial.print(_value);
  Serial.print(" , length:");
  Serial.println(characteristic1.valueLength());

  switch(_value)
  {
      case 'F':
        Serial.println("#M1");
        characteristic2.setValue(1);
        break;
      case 'S':
        Serial.println("#M0");
        characteristic2.setValue(0);
        break;
      case 'L':
        Serial.println("#M3");
        characteristic2.setValue(3);
        break;
      case 'R':
        Serial.println("#M4");
        characteristic2.setValue(4);
        break;
      case '1':
        Serial.println("#M5");
        characteristic2.setValue(5);
      case '2':
        Serial.println("#M6");
        characteristic2.setValue(6);
        break;
      case '3':
        Serial.println("#M7");
        characteristic2.setValue(7);
      case '4':
        Serial.println("#M8");
        characteristic2.setValue(8);
        break;
      case '5':
        Serial.println("#M9");
        characteristic2.setValue(9);
      default:
        Serial.println("#M0");
        characteristic2.setValue(0);
        break;
  }
  digitalWrite(LEDpin, HIGH);
}

/* 接続時 */
void blePeripheralConnectHandler(BLECentral& central)
{
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

/* 切断時 */
void blePeripheralDisconnectHandler(BLECentral& central)
{
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}
