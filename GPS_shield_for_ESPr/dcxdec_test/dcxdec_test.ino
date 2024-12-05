#include "QZSSDCX.h"

// 1文字の16進数を数値に変換するヘルパー関数
uint8_t hex_char_to_value(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return 0;  // 無効な入力に対するデフォルト値
}

byte l1s_msgbuf[64];

// 16進文字列をバイト配列に変換
byte *str2byte(const char *mst) {
  size_t hex_length = strlen(mst);
  size_t byte_array_size = hex_length / 2;
  for (size_t i = 0; i < byte_array_size; i++) {
    // 上位4ビット（1文字目）と下位4ビット（2文字目）を計算
    l1s_msgbuf[i] = (hex_char_to_value(mst[2 * i]) << 4)
                    | hex_char_to_value(mst[2 * i + 1]);
  }
  return l1s_msgbuf;
}

void test(const char *msg) {
  DCXDecoder dec;
  Serial.println(msg);
  dec.decode(str2byte(msg));
  dec.printSummary(Serial, dec.r);
  Serial.println("----");
  dec.printAll(Serial, dec.r);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;  //ターミナルを開くまでまつ

  delay(3000);

  // J Alert
  test("C6B0600DE2BF88E9200000000000000000001FFFFFFFFFFFC0000011A1C84ED4");
  test("9AB0600DE21188EA208000000000000000001FFFFFFFFFFFC00000138C7533B1");
  test("C6B0600DE21188EA208100000000000000001FFFFFFFFFFFC00000113F4B1B94");
  test("9AB0600DE21188EB208200000000000000001FFFFFFFFFFFC000001339BF5A31");
  test("C6B0600DE21188EB208300000000000000001FFFFFFFFFFFC0000010AE78CA54");
  test("53B0600DE21188EC208500000000000000001FFFFFFFFFFFC00000104D84DFE6");
  test("9AB0600DE21188ED208600000000000000001FFFFFFFFFFFC0000012233F80B1");
  test("C6B0600DE21188ED208700000000000000001FFFFFFFFFFFC00000139C985E94");
  test("C6B0600DE2E208EE20880000000000000000000000000000400000130C9DA654");
  test("9AB0600DE2E208EF2088000000000000000000000000001F800000113C0237B1");
  test("C6B0600DE2E208EF20880000000000000000000000000FE000000012A3C82914");
  test("9AB0600DE2E208F02088000000000000000000000003F00000000010905F49F1");
  test("C6B0600DE2E208F0208800000000000000000000003C0000000000121BF87454");
  test("9AB0600DE2E208F12088000000000000000000000FC000000000001015A2A271");
  test("C6B0600DE2E208F1208800000000000000000001F000000000000011B864C094");
  test("9AB0600DE2E208F220880000000000000000001E000000000000001386CA21B1");
  test("9AB0600DE2E208F320880000000000000000100000000000000000130FA51EF1");

  // L Alert
  test("C6B0440DE10208ADE00000000000000000000113400000000000001395E136D4");
  test("9AB0440DE10408ADE000000000000000000001134000000000000011940F6AF1");
  test("C6B0440DE12288B5E00000000000000000000CCB4000000000000013B3960CD4");
  test("C6B2E40DE12888B7E00000000000000000000CCB40000000000000114F8BC354");
  test("C6B0440DE13F08BCE00000000000000000001A770000000000000010286BB654");
  test("53B0440DE14708BEE00000000000000000001A77000000000000001358AC5F66");
  test("9AB0440DE16188C5E00000000000000000002E1840000000000000122F1985F1");
  test("53B0440DE16B88C7E00000000000000000002E184000000000000012EFDE17A6");
  test("53B0440DE17E08CCE000BD36E47A359984BA011340000000000000106031CF66");
  test("9AB0600DE1A488D6E000B2C1E361A5190C460CCB4000000000000011B0C18CB1");
  test("9AB0600DE1DD88E4E000A545DACBB58D907E2E184000000000000012A0AA9B71");
  test("C6B2E40DE1B488DAE000B2C1E361A5190C460CCB4000000000000013C7201F14");
}

void loop() {
  // put your main code here, to run repeatedly:
}
