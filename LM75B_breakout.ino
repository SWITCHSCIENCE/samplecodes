// LM75B_breakout
//
// Public domain 2014 Switch Science, Inc.
//
// Configuration
// OS fault queue programming : queue value = 1
// OS polarity selection      : OS active LOW
// OS operation mode          : OS comparator
// device oparation mode      : normal
 
 
#include <Wire.h>
#include <stdio.h>
#define LM75B_address 0x48          // A0=A1=A2=Low
#define temp_reg      0x00          //Temperture register
#define conf_reg      0x01          //Configuration register
#define thyst_reg     0x02          //Hysterisis register
#define tos_reg       0x03          //Overtemperature shutdown register
// 温度がtosに達するとLM75BのOSピンがLowになり、その後温度がthystを下回るとOSピンがHighに戻る。(OS active Low)
double tos = 30.0;                                            //割り込み発生温度（高） 0.5℃刻み  デフォルトでは80℃
double thyst = 28.0;                                          //割り込み発生温度（低） 0.5℃刻み　デフォルトでは75℃
signed int tos_data = (signed int)(tos / 0.5) << 7;           //レジスタ用に変換
signed int thyst_data = (signed int)(thyst / 0.5) << 7;       //レジスタ用に変換
int osPin = 2;                                                //LM75BのOSピンと接続するピン
 
void setup()
{   
  pinMode(osPin,INPUT_PULLUP);
  attachInterrupt(0,temp_interrupt,CHANGE);    //割り込み関数登録　2番ピンの状態変化で割り込み
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(LM75B_address);       //*****************************************
  Wire.write(tos_reg);                            
  Wire.write(tos_data >> 8);                      //tosの温度設定    
  Wire.write(tos_data);
  Wire.endTransmission();                      //*****************************************
  Wire.beginTransmission(LM75B_address);       //-----------------------------------------
  Wire.write(thyst_reg);
  Wire.write(thyst_data >> 8);                    //thystの温度設定
  Wire.write(thyst_data);
  Wire.endTransmission();                      //-----------------------------------------
  Wire.beginTransmission(LM75B_address);       //*****************************************
  Wire.write(temp_reg);                            //温度読み出しモードに設定
  Wire.endTransmission();                      //*****************************************
 
}
 
void loop()
{
  signed int temp_data = 0;                    //LM75Bの温度レジスタの値用変数
  double temp = 0.0;                           //温度用変数
  Wire.requestFrom(LM75B_address,2);           
  while(Wire.available()){
    temp_data |= (Wire.read() << 8);         //温度レジスタの上位8bit取得
    temp_data |= Wire.read();                //温度レジスタの下位8bit取得(有効3bit)
  }
 
  temp = (temp_data >> 5) * 0.125;             //レジスタの値を温度情報に変換
 
  Serial.println(temp);
  delay(100);
}
 
void temp_interrupt()                            //割り込み関数
{                                                
  if(digitalRead(osPin) == 0){                 //LM75Bの温度がtosの値を超えるとOSピンがLowになり
    Serial.print("TEMP : OVER ");
    Serial.print(tos);
    Serial.println(" deg");
  }
  else{                                        //LM75Bの温度がthystを下回るとOSピンがHighになる。
    Serial.print("TEMP : UNDER ");
    Serial.print(thyst);
    Serial.println(" deg");  
  }
}
