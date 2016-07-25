/* GT20L16J1Y用サンプルプログラム
 * Arduinoのシリアルモニタから送信されたSJISの文字を
 * ROMから読みだしてシリアルモニタに返す
 * 半角カナまわりはデータにはあるがROM内の順とJISX規格での順が
 * 一致しないため未実装
 *
 * memo:9/30 00:00
 * memo:10/7 18:30
 * memo:2016/07/26 07:30
 */

#include <SPI.h>
unsigned char matrixdata32[32]; //16×16用表示データ
unsigned char matrixdata16[16]; //16×8用表示データ

// アドレス 0x828F:全角小文字o は大きさと位置が違うため自前で用意
// アドレス 0x3FCD0: サンセリフ半角小文字oを利用するのも、大きさが1dot大きいので同上
const unsigned char matrixdata32_o[32] = 
{
B00000000,
B00000000,
B00000000,
B00000000,
B11000000,
B00100000,
B00010000,
B00010000,
B00010000,
B00100000,
B11000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000011,
B00000100,
B00001000,
B00001000,
B00001000,
B00000100,
B00000011,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
};

void setup (void)
{
  pinMode (SS, OUTPUT);
  Serial.begin(115200);
  /*SPI通信の設定*/
  SPI.begin ();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  Serial.println("input word which you want to show");
  Serial.print("port:");
  Serial.print(SS);
  Serial.println("has set as chip select pin");
}

void loop (void)
{
  if (Serial.available() > 0)
  {
    /*Arduinoのシリアルモニタから送られてくる日本語がSJISであることに注意*/
    delayMicroseconds(200);//最低でも2文字受信したい
    uint8_t msbdata = Serial.read();//1バイト目 //10/07
    /*SJISの1バイトコードか否か*/
    if ( (msbdata < 0x80) || ((0xA0 < msbdata) && (msbdata <= 0xdF)) )
    {
      showSJIS1byte(msbdata);
    } else
    {
      uint8_t lsbdata = Serial.read();//2バイト目 //10/07
      uint16_t data =  ((msbdata << 8) + lsbdata); //2
      showSJIS2byte(data);
    }
  }
}

/*1byteのSJISを表示する*/
void showSJIS1byte(uint8_t code)
{
  readFontASCII(code);
  sendDotsToSerial16();
}

/*2byteのSJISを表示する*/
//showSJIS2byte(SJIS文字コード)
void showSJIS2byte(unsigned short code)
{ 
  if(code == 0x828F)
  {
    for(int i=0;i<32;i++)
    {
      matrixdata32[i] = matrixdata32_o[i];
    }
    sendDotsToSerial32();
  }else{
    /*Arduinoのシリアルで日本語はSJIS送信なのでSJIS->JISx0208変換をする*/
    Serial.print("SJIS, 0x"); Serial.print(code, HEX); Serial.print("\t");
    uint8_t c1 = ((code & 0xff00) >> 8);
    uint8_t c2 = (code & 0xFF);
    if (c1 >= 0xe0)
    {
      c1 = c1 - 0x40;
    }
    if (c2 >= 0x80)
    {
      c2 = c2 - 1;
    }
    if (c2 >= 0x9e)
    {
      c1 = (c1 - 0x70) * 2;
      c2 = c2 - 0x7d;
    } else
    {
      c1 = ((c1 - 0x70) * 2) - 1;
      c2 = c2 - 0x1f;
    }
    /*読み出し*/
    readFontJIS(c1, c2);
    /*表示*/
    sendDotsToSerial32();
  }
}

/*漢字ROMとやりとり*/
//readFontJIS(JIS上位8bit,JIS下位8bit);
void readFontJIS(uint8_t c1, uint8_t c2)
{
  /*jisx変換後の表示*/
  Serial.print("jisx up8 = 0x"); Serial.print(c1, HEX); Serial.print("\t");
  Serial.print("jisx down8 = 0x"); Serial.print(c2, HEX); Serial.print("\t");
  /*jisxの区点を求める*/
  uint32_t MSB = c1 - 0x20;//区
  uint32_t LSB = c2 - 0x20;//点
  /*JISの句点番号で分類*/
  uint32_t Address = 0;
  Serial.print("MSB = d"); Serial.print(MSB, DEC); Serial.print("\t");
  Serial.print("LSB = d"); Serial.print(LSB, DEC); Serial.print("\t");
  /*各種記号・英数字・かな(一部機種依存につき注意,㍍などWindowsと互換性なし)*/
  if (MSB >= 1 && MSB <= 15 && LSB >= 1 && LSB <= 94)
  {
    Address = ( (MSB - 1) * 94 + (LSB - 1)) * 32;
  }
  /*第一水準*/
  if (MSB >= 16 && MSB <= 47 && LSB >= 1 && LSB <= 94)
  {
    Address = ( (MSB - 16) * 94 + (LSB - 1)) * 32 + 43584;
  }
  /*第二水準*/
  if (MSB >= 48 && MSB <= 84 && LSB >= 1 && LSB <= 94)
  {
    Address = ((MSB - 48) * 94 + (LSB - 1)) * 32 + 138464;
  }
  //
  /*GT20L16J1Y内部では1区と同等の内容が収録されている*/
  if (MSB == 85 && LSB >= 0x01 && LSB <= 94)
  {
    Address = ((MSB - 85) * 94 + (LSB - 1)) * 32 + 246944;
  }
  /*GT20L16J1Y内部では2区、3区と同等の内容が収録されている*/
  if (MSB >= 88 && MSB <= 89 && LSB >= 1 && LSB <= 94)
  {
    Address = ((MSB - 88) * 94 + (LSB - 1)) * 32 + 249952;
  }
  /*漢字ROMにデータを送信*/
  digitalWrite(SS, HIGH);
  Serial.print("Address = "); Serial.println(Address, HEX);
  digitalWrite(SS, LOW);  //通信開始
  SPI.transfer(0x03);
  SPI.transfer(Address >> 16  & 0xff);
  SPI.transfer(Address >> 8   & 0xff);
  SPI.transfer(Address        & 0xff);
  /*漢字ROMからデータを受信*/
  for (int i = 0; i < 32; i++)
  {
    matrixdata32[i] = SPI.transfer(0x00);
  }
  digitalWrite(SS, HIGH); //通信終了
}//spireadfont

/*漢字ROMとやりとり*/
//readFontASCII(ASCIIコード);
void readFontASCII(uint8_t ASCIICODE)
{
  Serial.print("ASCII,0x");Serial.print(ASCIICODE, HEX); //10/07
  uint32_t Address = 0;
  /*ASCII文字*/
  if (ASCIICODE >= 0x20 && ASCIICODE <= 0x7F)
  {
    Address = ( ASCIICODE - 0x20) * 16 + 255968;
  }
  /*漢字ROMにデータを送信*/
  digitalWrite(SS, HIGH);
  Serial.print("  Address = "); Serial.println(Address, HEX);
  digitalWrite(SS, LOW);  //通信開始
  SPI.transfer(0x03);
  SPI.transfer(Address >> 16  & 0xff);
  SPI.transfer(Address >> 8   & 0xff);
  SPI.transfer(Address        & 0xff);
  /*漢字ROMからデータを受信*/
  for (int i = 0; i < 16; i++)
  {
    matrixdata16[i] = SPI.transfer(0x00);
  }
  digitalWrite(SS, HIGH); //通信終了

}

/*シリアルモニタへ16*16のデータを表示する*/
void sendDotsToSerial32()
{
  /*上半分*/
  for (int i = 0; i < 8; i++)
  {
    for (int b = 0; b < 16; b++)
    {
      char byteDigit = (1 << i);
      if (matrixdata32[b] & byteDigit)
      {
        Serial.write("XX");
      } else
      {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  /*下半分*/
  for (int i = 0; i < 8; i++)
  {
    for (int b = 16; b < 32 ; b++)
    {
      char byteDigit = (1 << i);
      if (matrixdata32[b] & byteDigit)
      {
        Serial.write("XX");
      } else
      {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  Serial.println();
}//sendDataToSerial16


/*シリアルモニタへ16*8のデータを表示する*/
void sendDotsToSerial16()
{
  /*上半分*/
  for (int i = 0; i < 8; i++)
  {
    for (int b = 0; b < 8; b++)
    {
      char byteDigit = (1 << i);
      if (matrixdata16[b] & byteDigit)
      {
        Serial.write("XX");
      } else
      {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  /*下半分*/
  for (int i = 0; i < 8; i++)
  {
    for (int b = 8; b < 16; b++)
    {
      char byteDigit = (1 << i);
      if (matrixdata16[b] & byteDigit)
      {
        Serial.write("XX");
      } else
      {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  Serial.println();
}//sendDataToSerial32
