#include <Arduino.h>

// PicossciNTSCライブラリ
#include <Picossci_NTSC.h>

// LovyanGFXライブラリ
#include <LovyanGFX.h>

static constexpr const size_t frame_buf_width = 320;
static constexpr const size_t frame_buf_height = 240;

static Picossci_NTSC picossci_ntsc;

// 描画先としてLovyanGFXのLGFX_Spriteを使用する
static LGFX_Sprite display;

static void callback_video(void*)
{
  for (;;) {
    // 描画先のY座標を取得
    int y = picossci_ntsc.video.getCurrentY();
    if (y < 0) { // 負の値の場合は空きがない。
      return;
    }
    auto fb = (uint8_t*)display.getBuffer();
    picossci_ntsc.video.writeScanLine(&fb[sizeof(uint16_t) * frame_buf_width * (y * frame_buf_height / 480)], frame_buf_width);
  }
}

void drawGradation(void)
{
  // 背景にグラデーションを描画する
  display.startWrite();
  display.setAddrWindow(0, 0, display.width(), display.height());
  for (int y = 0; y < display.height(); ++y) {
    for (int x = 0; x < display.width(); ++x) {
      display.writeColor(display.color888(x>>1, (x + y) >> 2, y>>1), 1);
    }
  }
  display.endWrite();
}

void setup(void)
{
  picossci_ntsc.setCpuClock(157500);

  display.setColorDepth(16);
  display.createSprite(frame_buf_width, frame_buf_height);

  int x_offset = 16;
  picossci_ntsc.video.setOffset(x_offset);
  picossci_ntsc.video.setScale(((720 - x_offset * 2) * 256) / frame_buf_width);
  picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_swap565);

  auto cfg = picossci_ntsc.video.getConfig();
  cfg.callback_function = callback_video;

  picossci_ntsc.video.init(cfg);
  picossci_ntsc.video.start();


  drawGradation();

  // 文字の描画方法には大きく分けて２通り、print 系の関数と drawString 系の関数があります。

  // drawString 関数では、第１引数で文字列を指定し、第２引数でX座標、第３引数でY座標を指定します。
  display.drawString("string!", 10, 10);

  // drawNumber 関数では、第１引数が数値になります。
  display.drawNumber(123, 100, 10);

  // drawFloat 関数では、第１引数が数値、第２引数が小数点以下の桁数になり、第３引数がX座標、第４引数がY座標になります。
  display.drawFloat(3.14, 2, 150, 10);

  // print 関数では、setCursor関数で指定した座標 (またはprint関数で最後に描画した文字の続き)に描画します。
  display.setCursor(10, 20);
  display.print("print!");

  // printf関数で、第２引数以降の内容を描画できます。(C言語のprintf準拠ですので文字列や浮動小数も描画できます)
  int value = 123;
  display.printf("test %d", value);

  // println関数で、文字列を描画後に改行できます。print("\n");と同じ効果です。
  display.println("println");

  // フォントを変更するには、setFont関数を使用します。
  // TFT_eSPIのsetTextFont関数と同じフォントは Font0 ～ Font8 になります。
  // ※ エディタの入力支援が使える場合、引数に&fonts::まで入力する事でフォント一覧が表示されます
  display.setFont(&fonts::Font4);
  display.println("TestFont4");

  // TFT_eSPIとの互換性のためにsetTextFont関数による番号でのフォント変更にも対応しています。
  // 引数に指定できる数字は 0, 2, 4, 6, 7, 8 です。(TFT_eSPI準拠です)
  // ※ ただし この方法は、他の番号のフォントも強制的にバイナリに含まれサイズが膨らむため、非推奨です。
  display.setTextFont(2);
  display.println("TestFont2");


  // setTextColorで色を変更できます。
  // １つ目の引数が文字色、２つ目の引数が背景色になります。
  display.setTextColor(0x00FFFFU, 0xFF0000U);
  display.print("CyanText RedBack");
  // ※ 同じ場所に文字を繰り返し描画し直したい場合、背景色を指定して重ね書きすることを推奨します。
  //    fillRect等で消去してから書き直すと、ちらつきが発生する可能性があります。


  // setTextColorで第１引数のみを指定し第２引数を省略した場合は、
  // 背景を塗り潰さず文字だけを描画します。
  display.setTextColor(0xFFFF00U);
  display.print("YellowText ClearBack");


  // Font6は時計用の文字のみが収録されています。
  display.setFont(&fonts::Font6);
  display.print("apm.:-0369");

  // Font7は７セグメント液晶風のフォントが収録されています。
  display.setFont(&fonts::Font7);
  display.print(".:-147");

  // Font8は数字のみが収録されています。
  display.setFont(&fonts::Font8);
  display.print(".:-258");


  delay(3000);
  drawGradation();

// IPAフォントをコンバートした日本語フォントが４種類ｘ９サイズ = 36通りプリセットされています。
// 末尾の数字がサイズを表しており、8, 12, 16, 20, 24, 28, 32, 36, 40 が用意されています。
// fonts::lgfxJapanMincho_12      // 明朝体 サイズ12 固定幅フォント
// fonts::lgfxJapanMinchoP_16     // 明朝体 サイズ16 プロポーショナルフォント
// fonts::lgfxJapanGothic_20      // ゴシック体 サイズ20 固定幅フォント
// fonts::lgfxJapanGothicP_24     // ゴシック体 サイズ24 プロポーショナルフォント

// efontをコンバートした日本語・韓国語・中国語（簡体字・繁体字）フォントが各４種類ｘ５サイズ＝20通りプリセットされています。
// 数字がサイズを表しており、10, 12, 14, 16, 24 が用意されています。
// 末尾の文字は b= ボールド(太字) / i= イタリック(斜体) を表しています。
// fonts::efontJA_10              // 日本語 サイズ10
// fonts::efontCN_12_b            // 簡体字 サイズ12 ボールド
// fonts::efontTW_14_bi           // 繁体字 サイズ14 ボールドイタリック
// fonts::efontKR_16_i            // 韓国語 サイズ16 イタリック

  display.setCursor(0, 0);
  display.setFont(&fonts::lgfxJapanMincho_16);   display.print("明朝体 16 Hello World\nこんにちは世界\n");
//display.setFont(&fonts::lgfxJapanMinchoP_16);  display.print("明朝 P 16 Hello World\nこんにちは世界\n");
  display.setFont(&fonts::lgfxJapanGothic_16);   display.print("ゴシック体 16 Hello World\nこんにちは世界\n");
//display.setFont(&fonts::lgfxJapanGothicP_16);  display.print("ゴシック P 16 Hello World\nこんにちは世界\n");

// ※ やまねこ氏の [日本語フォントサブセットジェネレーター](https://github.com/yamamaya/lgfxFontSubsetGenerator)
//    を使用することで、必要な文字だけを含む小サイズのフォントデータを作成できます。


  delay(3000);
  drawGradation();


  // LovyanGFXでは AdafruitGFX フォントも setFont 関数で使用できます。
  // (TFT_eSPIとの互換性のために setFreeFont関数も用意しています)
  display.setFont(&fonts::FreeSerif9pt7b);


  // 右揃え や 中央揃え で描画したい場合は、setTextDatum 関数で基準位置を指定します。
  // 縦方向が top、middle、baseline、bottomの4通り、横方向が left、center、rightの3通りです。
  // 縦と横の指定を組み合わせた12通りの中から指定します。
  display.setTextDatum( textdatum_t::top_left        );
  display.setTextDatum( textdatum_t::top_center      );
  display.setTextDatum( textdatum_t::top_right       );
  display.setTextDatum( textdatum_t::middle_left     );
  display.setTextDatum( textdatum_t::middle_center   );
  display.setTextDatum( textdatum_t::middle_right    );
  display.setTextDatum( textdatum_t::baseline_left   );
  display.setTextDatum( textdatum_t::baseline_center );
  display.setTextDatum( textdatum_t::baseline_right  );
  display.setTextDatum( textdatum_t::bottom_left     );
  display.setTextDatum( textdatum_t::bottom_center   );
  display.setTextDatum( textdatum_t::bottom_right    );
  // ※  "textdatum_t::" は省略可能です
  // ※ print系関数には縦方向の指定のみ効果があり、横方向の指定は効果がありません。

  // 右下揃え
  display.setTextDatum( bottom_right );
  display.drawString("bottom_right",  display.width() / 2,  display.height() / 2);

  // 左下揃え
  display.setTextDatum( bottom_left );
  display.drawString("bottom_left",  display.width() / 2,  display.height() / 2);

  // 右上揃え
  display.setTextDatum( top_right );
  display.drawString("top_right",  display.width() / 2,  display.height() / 2);

  // 左上揃え
  display.setTextDatum( top_left );
  display.drawString("top_left",  display.width() / 2,  display.height() / 2);


  // 基準座標に中心線を描画
  display.drawFastVLine(display.width() / 2, 0, display.height(), 0xFFFFFFU);
  display.drawFastHLine(0, display.height() / 2, display.width(), 0xFFFFFFU);


  delay(3000);
  drawGradation();

  display.setFont(&Font2);
  display.setCursor(0, 0);


  display.drawRect(8, 8, display.width() - 16, display.height() - 16, 0xFFFFFFU);

  // setClipRect関数で描画する範囲を限定できます。指定した範囲外には描画されなくなります。
  // ※ テキスト系のみならず、すべての描画関数に影響します。
  display.setClipRect(10, 10, display.width() - 20, display.height() - 20);


  // setTextSize 関数で 文字の拡大率を指定します。
  // 第１引数で横方向の倍率、第２引数で縦方向の倍率を指定します。
  // 第２引数を省略した場合は、第１引数の倍率が縦と横の両方に反映されます。
  display.setTextSize(2.7, 4);
  display.println("Size 2.7 x 4");

  display.setTextSize(2.5);
  display.println("Size 2.5 x 2.5");

  display.setTextSize(1.5, 2);
  display.println("Size 1.5 x 2");

  delay(1000);

  display.setTextColor(0xFFFFFFU, 0);
  for (float i = 0; i < 30; i += 0.01) {
    display.setTextSize(sin(i)+1.1, cos(i)+1.1);
    display.drawString("size test", 10, 10);
  }

  display.setTextSize(1);

  // setTextWrap 関数で、print 関数が画面端(描画範囲端)に到達した時の折り返し動作を指定します。
  // 第１引数をtrueにすると、右端到達後に左端へ移動します。
  // 第２引数をtrueにすると、下端到達後に上端へ移動します。(省略時:false)
  display.setTextWrap(false);
  display.println("setTextWrap(false) testing... long long long long string wrap test string ");
  // false指定時は位置調整されず、描画範囲外にはみ出した部分は描画されません。

  display.setTextWrap(true);
  display.setTextColor(0xFFFF00U, 0);
  display.println("setTextWrap(true) testing... long long long long string wrap test string ");
  // true指定時は描画範囲内に収まるよう座標を自動調整します。

  delay(1000);

  // 第２引数にtrue指定時は、画面下端に到達すると続きを上端から描画します。
  display.setTextColor(0xFFFFFFU, 0);
  display.setTextWrap(true, true);
  display.println("setTextWrap(true, true) testing...");
  for (int i = 0; i < 100; ++i) {
    display.printf("wrap test %03d ", i);
    delay(50);
  }


  drawGradation();

  // setTextScroll 関数で、画面下端に到達した時のスクロール動作を指定します。
  // setScrollRect 関数でスクロールする矩形範囲を指定します。(未指定時は画面全体がスクロールします)
  // ※ スクロール機能は、LCDが画素読出しに対応している必要があります。
  display.setTextScroll(true);

  // 第１～第４引数で X Y Width Height の矩形範囲を指定し、第５引数でスクロール後の色を指定します。第５引数は省略可(省略時は変更なし)
  display.setScrollRect(10, 10, display.width() - 20, display.height() - 20, 0x00001FU);

  for (int i = 0; i < 50; ++i) {
    display.printf("scroll test %d \n", i);
  }


  // setClipRectの範囲指定を解除します。
  display.clearClipRect();

  // setScrollRectの範囲指定を解除します。
  display.clearScrollRect();


  display.setTextSize(1);
  display.setTextColor(0xFFFFFFU, 0);


  // setTextPadding 関数で、drawString 系関数で背景塗り潰し時の最小幅を指定できます。
  display.setTextPadding(100);


  drawGradation();
}

void drawNumberTest(const lgfx::IFont* font)
{
  display.setFont(font);

  display.fillScreen(0x0000FF);

  display.setColor(0xFFFF00U);
  display.drawFastVLine( 80, 0, 240);
  display.drawFastVLine(160, 0, 240);
  display.drawFastVLine(240, 0, 240);
  display.drawFastHLine(0,  45, 320);
  display.drawFastHLine(0,  95, 320);
  display.drawFastHLine(0, 145, 320);
  display.drawFastHLine(0, 195, 320);

  for (int i = 0; i < 200; ++i) {
    display.setTextDatum( textdatum_t::bottom_right    );     display.drawNumber(i,  80,  45);
    display.setTextDatum( textdatum_t::bottom_center   );     display.drawNumber(i, 160,  45);
    display.setTextDatum( textdatum_t::bottom_left     );     display.drawNumber(i, 240,  45);
    display.setTextDatum( textdatum_t::baseline_right  );     display.drawNumber(i,  80,  95);
    display.setTextDatum( textdatum_t::baseline_center );     display.drawNumber(i, 160,  95);
    display.setTextDatum( textdatum_t::baseline_left   );     display.drawNumber(i, 240,  95);
    display.setTextDatum( textdatum_t::middle_right    );     display.drawNumber(i,  80, 145);
    display.setTextDatum( textdatum_t::middle_center   );     display.drawNumber(i, 160, 145);
    display.setTextDatum( textdatum_t::middle_left     );     display.drawNumber(i, 240, 145);
    display.setTextDatum( textdatum_t::top_right       );     display.drawNumber(i,  80, 195);
    display.setTextDatum( textdatum_t::top_center      );     display.drawNumber(i, 160, 195);
    display.setTextDatum( textdatum_t::top_left        );     display.drawNumber(i, 240, 195);
  }
}

void loop(void)
{
// ※ 名前が"Free"で始まるフォントは 9pt 12pt 18pt 24ptの４種類があります。
  drawNumberTest( &fonts::Font0                   );
  drawNumberTest( &fonts::Font2                   );
  drawNumberTest( &fonts::Font4                   );
  drawNumberTest( &fonts::Font6                   );
  drawNumberTest( &fonts::Font7                   );
  drawNumberTest( &fonts::Font8                   );
  drawNumberTest( &fonts::TomThumb                );
  drawNumberTest( &fonts::FreeMono9pt7b           );
  drawNumberTest( &fonts::FreeMonoBold9pt7b       );
  drawNumberTest( &fonts::FreeMonoOblique9pt7b    );
  drawNumberTest( &fonts::FreeMonoBoldOblique9pt7b);
  drawNumberTest( &fonts::FreeSans9pt7b           );
  drawNumberTest( &fonts::FreeSansBold9pt7b       );
  drawNumberTest( &fonts::FreeSansOblique9pt7b    );
  drawNumberTest( &fonts::FreeSansBoldOblique9pt7b);
  drawNumberTest( &fonts::FreeSerif9pt7b          );
  drawNumberTest( &fonts::FreeSerifBold9pt7b      );
  drawNumberTest( &fonts::FreeSerifItalic9pt7b    );
  drawNumberTest( &fonts::FreeSerifBoldItalic9pt7b);
  drawNumberTest( &fonts::Orbitron_Light_24       );
  drawNumberTest( &fonts::Roboto_Thin_24          );
  drawNumberTest( &fonts::Satisfy_24              );
  drawNumberTest( &fonts::Yellowtail_32           );
}