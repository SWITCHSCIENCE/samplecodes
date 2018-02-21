/*  Arduino define  */
#define ADCpin_AnalogUV   0
#define _LED              13

float uv, v;
int data;

void setup() {
  pinMode(_LED, OUTPUT);

  digitalWrite(_LED, LOW);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(_LED, LOW);

  data = analogRead(ADCpin_AnalogUV);
  v = (float)data / 1023 * 5.0f;
  uv = (float)(data - 200) * 0.04367f;
  
  Serial.print(uv, 2);
  Serial.print(" ");
  Serial.print(v, 3);
  Serial.print(" 0x");
  Serial.println(data, HEX);
  
  delay(100);
}

