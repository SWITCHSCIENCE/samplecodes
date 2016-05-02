// Scratch sensor board sketch for Arduino bootloader on Nekoboard2
// Switch-Science
// 2016/04/25

int analogPin[4] = { A0,A1,A2,A3 };  // select the input pin for the general analog 0-4
int lightPin = A4;   // select the input pin for the sensor
int micPin = A5;     // select the input pin for the mic
int sliderPin = A6;  // select the input pin for the slide volume
int buttonPin = 2;   // select the pin for the button

int sliderValue = 0; // variable to store the value coming from the slide volume
int lightValue = 0;  // variable to store the value coming from the light sensor
int micValue = 0;    // variable to store the value coming from the mic
int analogValue[4] = { 0,0,0,0};  // variable to store the value coming from the analog 0-4
const int firmware = 0x04; // Firmware virsion

int senddata[18]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

void setsenddata(int value,int channel) {
  int pos;
  byte highdata = 0;
  byte lowdata = 0;
  if (channel==15) {
    pos = 0;
  } else {
    pos = channel*2+2;
  }
  highdata = 0x80 | (channel<<3) | lowByte(value >>7);
  lowdata  = lowByte(0x7f & value);
  senddata[pos] = highdata;
  senddata[pos+1] = lowdata;
}

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(buttonPin,INPUT);
  digitalWrite(buttonPin,HIGH);  // プルアップのため
  Serial.begin(38400);
}

void loop() {
  int i;
 // if(1){
  if (Serial.available() > 0) {     // 受信したデータが存在したら
    int inByte = Serial.read();     // 受信データを読み込む
    if (inByte == 0x0001) {         // 送られてきたデータが0x01だったら
      for (int i=0; i<18; i++) {
        Serial.write(senddata[i]);  // 用意されているセンサーデータを送信する
      }
    }

    // read the value from the sensor:
    sliderValue = 1023 - analogRead(sliderPin);
    lightValue = analogRead(lightPin) - 15;
    if(lightValue < 0){
      lightValue = 0;
    }
    micValue = analogRead(micPin);

    for (i=0; i<4; i++) {
      analogValue[i] = analogRead(analogPin[i]);
    }

    // Make sending data
    for (i=0; i<16; i++) {
      switch(i) {
      case 0:
        // Ch0 Registance-D
        setsenddata(analogValue[3],0);
        break;
      case 1:
        // Ch1 Registance-C
        setsenddata(analogValue[2],1);
        break;
      case 2:
        // Ch2 Registance-B
        setsenddata(analogValue[1],2);
        break;
      case 3:
        // Ch3 Button
        if (digitalRead(buttonPin) == 1) {
          setsenddata(1023,3);
        } 
        else {
          setsenddata(0,3);
        } 
        break;
      case 4:
        // Ch4 Registance-A
        setsenddata(analogValue[0],4);
        break;
      case 5:
        // Ch5 Light
        setsenddata(lightValue,5);
        break;
      case 6:
        // Ch6 Sound
        setsenddata(micValue,6);
        break;
      case 7:
        // Ch7 Slider
        setsenddata(sliderValue,7);
        break;
      case 15:
        // Ch15 Firmware ID
        setsenddata(firmware,15);
        break;
      default:
        // Ch8-14 No operation
        break;
      }
    }
  }
}

