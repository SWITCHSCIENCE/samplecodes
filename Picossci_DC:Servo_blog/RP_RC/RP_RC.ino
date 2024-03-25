#define LED1 25

#define PWM0A 16
#define PWM0B 17
#define PWM1A 18
#define PWM1B 19

#define SPEED 128

#include "pio_usb.h"
#define HOST_PIN_DP   0   // Pin used as D+ for host, D- = D+ + 1

#include "Adafruit_TinyUSB.h"
static uint8_t const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };
Adafruit_USBH_Host USBHost;
tusb_desc_device_t desc_device;

//ascii文字コードとActionの対応付け
typedef enum {
    MOVE_FORWARD=119,//w
    MOVE_BACKWARD=120,//x
    STOP=115,//s
    TURN_RIGHT=100,//d
    TURN_LEFT=97//a
} Action;

void setup() {
  Serial1.begin(115200);
  Serial.println("~~~~RP_RemoteControl APP V1.0~~~~");

  pinMode(PWM0A,OUTPUT);
  pinMode(PWM0B,OUTPUT);
  pinMode(PWM1A,OUTPUT);
  pinMode(PWM1B,OUTPUT);

  pinMode(LED1,OUTPUT);
  digitalWrite(LED1,HIGH);

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_PIN_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);
  USBHost.begin(1);
}

void loop()
{
  USBHost.task();
}

//HIDデバイスが接続されたときに呼び出される
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  (void)desc_report;
  (void)desc_len;

  digitalWrite(LED1,LOW);
  // Interface protocol (hid_interface_protocol_enum_t)
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  char tempbuf[256];
  int count = sprintf(tempbuf, "[%04x:%04x][%u] HID Interface%u, Protocol = %s\r\n", vid, pid, dev_addr, instance, protocol_str[itf_protocol]);

  tud_cdc_write(tempbuf, count);
  tud_cdc_write_flush();


  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      tud_cdc_write_str("Error: cannot request report\r\n");
    }
  }
}

//HIDデバイスが外されたときに呼び出される
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  digitalWrite(LED1,HIGH);
  char tempbuf[256];
  int count = sprintf(tempbuf, "[%u] HID Interface%u is unmounted\r\n", dev_addr, instance);
  tud_cdc_write(tempbuf, count);
  tud_cdc_write_flush();
}

//keycodeからasciiの変換
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
  for(uint8_t i=0; i<6; i++)
  {
    if (report->keycode[i] == keycode)  return true;
  }

  return false;
}

//モーターの駆動処理
void drive_motor(Action action){
  Serial.println(action);
  switch(action){
    case MOVE_FORWARD:
      analogWrite(PWM0A,SPEED);digitalWrite(PWM0B,LOW);
      digitalWrite(PWM1A,LOW);analogWrite(PWM1B,SPEED);
      break;
    case MOVE_BACKWARD:
      digitalWrite(PWM0A,LOW);analogWrite(PWM0B,SPEED);
      analogWrite(PWM1A,SPEED);digitalWrite(PWM1B,LOW);
      break;   
    case TURN_LEFT:
      analogWrite(PWM0B,SPEED);digitalWrite(PWM0A,LOW);
      analogWrite(PWM1B,SPEED);digitalWrite(PWM1A,LOW);
      break;
    case TURN_RIGHT:
      analogWrite(PWM0A,SPEED);digitalWrite(PWM0B,LOW);
      analogWrite(PWM1A,SPEED);digitalWrite(PWM1B,LOW);
      break;
    case STOP:
      digitalWrite(PWM0A,HIGH);digitalWrite(PWM0B,HIGH);
      digitalWrite(PWM1A,HIGH);digitalWrite(PWM1B,HIGH);
      break;
    default:
      Serial.println("Invalid KeyCode");
  }
}


//キーボードの入力に対する処理
static void process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report)
{
  (void) dev_addr;
  static hid_keyboard_report_t prev_report = { 0, 0, {0} };
  bool flush = false;

  for(uint8_t i=0; i<6; i++)
  {
    uint8_t keycode = report->keycode[i];
    if ( keycode )
    {
      if ( find_key_in_report(&prev_report, keycode) )
      {
        // exist in previous report means the current key is holding
      }else
      {
        #ifdef KEYBOARD_COLEMAK
        uint8_t colemak_key_code = colemak[keycode];
        if (colemak_key_code != 0) keycode = colemak_key_code;
        #endif

        bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
        uint8_t ch = keycode2ascii[keycode][is_shift ? 1 : 0];

        if (ch)
        {
          if (ch == '\n') tud_cdc_write("\r", 1);
          tud_cdc_write(&ch, 1);
          drive_motor((Action)ch);
          flush = true;
        }
      }
    }
  }

  if (flush) tud_cdc_write_flush();
  prev_report = *report;
}

//HID入力がされたときのコールバック関数
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) len;
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if(itf_protocol == HID_ITF_PROTOCOL_KEYBOARD){
    process_kbd_report(dev_addr, (hid_keyboard_report_t const*) report );
  }

  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    tud_cdc_write_str("Error: cannot request report\r\n");
  }
}