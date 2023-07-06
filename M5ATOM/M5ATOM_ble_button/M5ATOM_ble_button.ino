// ESP32でキーボードショートカットを作ってしまおう
// https://qiita.com/poruruba/items/eff3fedb1d4a63cbe08d

#include <M5Atom.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

#define BLE_PASSKEY 123456

uint8_t keycode = 0x1e;  // '1'

/*
   BLEデバイス処理
*/
BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

bool connected = false;

class MyCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      connected = true;
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(true);
      Serial.println("Connected");
    }

    void onDisconnect(BLEServer* pServer) {
      connected = false;
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(false);
      Serial.println("Disconnected");
    }
};

// ペアリング処理用
class MySecurity : public BLESecurityCallbacks {
    bool onConfirmPIN(uint32_t pin) {
      return false;
    }

    uint32_t onPassKeyRequest() {
      Serial.println("ONPassKeyRequest");
      return BLE_PASSKEY;
    }

    void onPassKeyNotify(uint32_t pass_key) {
      // ペアリング時のPINの表示
      Serial.println("onPassKeyNotify number");
      Serial.println(pass_key);
    }

    bool onSecurityRequest() {
      Serial.println("onSecurityRequest");
      return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
      Serial.println("onAuthenticationComplete");
      if (cmpl.success) {
        // ペアリング完了
        Serial.println("auth success");
      } else {
        // ペアリング失敗
        Serial.println("auth failed");
      }
    }
};

// BLEデバイスの起動
void taskServer(void*) {
  BLEDevice::init("1-Click-ATOM");

  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
  BLEDevice::setSecurityCallbacks(new MySecurity());

  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(1);    // <-- input REPORTID from report map
  output = hid->outputReport(1);  // <-- output REPORTID from report map

  std::string name = "SwitchScience, Inc.";
  hid->manufacturer()->setValue(name);

  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x02);

  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setKeySize(16);

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  uint32_t passkey = BLE_PASSKEY;
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));

  const uint8_t report[] = {
    USAGE_PAGE(1), 0x01,  // Generic Desktop Ctrls
    USAGE(1), 0x06,       // Keyboard
    COLLECTION(1), 0x01,  // Application
    REPORT_ID(1), 0x01,   //   Report ID (1)
    USAGE_PAGE(1), 0x07,  //   Kbrd/Keypad
    USAGE_MINIMUM(1), 0xE0,
    USAGE_MAXIMUM(1), 0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1), 0x01,  //   1 byte (Modifier)
    REPORT_COUNT(1), 0x08,
    HIDINPUT(1), 0x02,      //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1), 0x01,  //   1 byte (Reserved)
    REPORT_SIZE(1), 0x08,
    HIDINPUT(1), 0x01,      //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1), 0x06,  //   6 bytes (Keys)
    REPORT_SIZE(1), 0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,  //   101 keys
    USAGE_MINIMUM(1), 0x00,
    USAGE_MAXIMUM(1), 0x65,
    HIDINPUT(1), 0x00,      //   Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1), 0x05,  //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1), 0x01,
    USAGE_PAGE(1), 0x08,     //   LEDs
    USAGE_MINIMUM(1), 0x01,  //   Num Lock
    USAGE_MAXIMUM(1), 0x05,  //   Kana
    HIDOUTPUT(1), 0x02,      //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
    REPORT_COUNT(1), 0x01,   //   3 bits (Padding)
    REPORT_SIZE(1), 0x03,
    HIDOUTPUT(1), 0x01,  //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
    END_COLLECTION(0)
  };

  hid->reportMap((uint8_t*)report, sizeof(report));
  hid->startServices();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  hid->setBatteryLevel(7);

  //  Serial.println("Advertising started!");
  delay(portMAX_DELAY);
};

enum KEY_MODIFIER_MASK {
  KEY_MASK_CTRL = 0x01,
  KEY_MASK_SHIFT = 0x02,
  KEY_MASK_ALT = 0x04,
  KEY_MASK_WIN = 0x08
};

void sendKeys(uint8_t mod, uint8_t* keys, uint8_t num_key = 1) {
  uint8_t msg[] = { mod, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  for (int i = 0; i < num_key; i++)
    msg[2 + i] = keys[i];

  input->setValue(msg, sizeof(msg));
  input->notify();

  uint8_t msg1[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  input->setValue(msg1, sizeof(msg1));
  input->notify();
}

void setup() {
  setCpuFrequencyMhz(80);

  M5.begin(true, false, true);
  delay(1000);

  Serial.begin(115200);
  Serial.println("Starting KeyShortcut!");

  // BLEデバイスの起動処理の開始
  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);

}

void loop() {
  M5.update();
  // if (M5.Btn.wasReleased()) {
  //   if (connected) {
  //     Serial.println("Btn released");
  //     uint8_t keys[] = { 0x50 /* LEFT Arrow */ };
  //     sendKeys(KEY_MASK_CTRL | KEY_MASK_WIN, keys, 1);
  //   }
  // }
  if (connected) {

    uint8_t msg1[] = { 0x00, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 };
    input->setValue(msg1, sizeof(msg1));
    input->notify();
    delay(100);
    uint8_t msg2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    input->setValue(msg2, sizeof(msg2));
    input->notify();
    delay(850);
    keycode += 1;
    if (keycode > 0x27) {
      keycode = 0x1e;
    }


    //    if (M5.Btn.wasPressed()) {
    //      uint8_t msg[] = { 0x00, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //      input->setValue(msg, sizeof(msg));
    //      input->notify();
    //    }
    //    if (M5.Btn.wasReleased()) {
    //      uint8_t msg[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //      input->setValue(msg, sizeof(msg));
    //      input->notify();
    //    }
    //  }
  }
  delay(50);
}
