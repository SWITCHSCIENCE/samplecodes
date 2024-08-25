#include <Arduino.h>

// PicossciNTSCライブラリ
#include <Picossci_NTSC.h>

// LovyanGFXライブラリ
#include <LovyanGFX.h>

static constexpr const size_t frame_buf_width = 320;
static constexpr const size_t frame_buf_height = 240;

static Picossci_NTSC picossci_ntsc;

// 描画先としてLovyanGFXのLGFX_Spriteを使用する
static LGFX_Sprite _sprites[2];
static size_t count = 0;


static void callback_video(void*)
{
  for (;;) {
    // 描画先のY座標を取得
    int y = picossci_ntsc.video.getCurrentY();
    if (y < 0) { // 負の値の場合は空きがない。
      return;
    }
    auto fb = (uint8_t*)(_sprites[~count & 1]).getBuffer();
    picossci_ntsc.video.writeScanLine(&fb[frame_buf_width * (y * frame_buf_height / 480)], frame_buf_width);
  }
}

extern "C" void set_clock(uint32_t freq_khz);


void setup(void)
{
  picossci_ntsc.setCpuClock(157500);

  for (std::uint32_t i = 0; i < 2; ++i)
  {
    _sprites[i].setColorDepth(8);
    _sprites[i].createSprite(frame_buf_width, frame_buf_height);
    _sprites[i].createPalette();
  }
  auto cfg = picossci_ntsc.video.getConfig();
  cfg.callback_function = callback_video;
  cfg.dma_buf_count = 16;
  int x_offset = 16;
  picossci_ntsc.video.setOffset(x_offset);
  picossci_ntsc.video.setScale(((720 - x_offset * 2) * 256) / frame_buf_width);
  picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_palette);

  picossci_ntsc.video.init(cfg);
  picossci_ntsc.video.start();
}

void loop(void)
{
  ++count;
  bool flip = count & 1;
  for (int i = 0; i < 256; ++i) {
    int r = ((i-count-1) & 0x7F)<<1;
    int g = ((i-count-1) & 0xFF);
    int b = ((i-count-1) & 0x3F)<<2;
    picossci_ntsc.video.setPalette(i, r << 16 | g << 8 | b);
  }
  _sprites[!flip].clearClipRect();
  _sprites[!flip].setColor(count);
  _sprites[!flip].fillRect(0,0,frame_buf_width,3);
  _sprites[!flip].fillRect(0,0,3,frame_buf_height);
  _sprites[!flip].fillRect(frame_buf_width-3,0,3,frame_buf_height);
  _sprites[!flip].fillRect(0,frame_buf_height-3,frame_buf_width,3);
  _sprites[!flip].setClipRect(3,3,frame_buf_width-6,frame_buf_height-6);
  _sprites[!flip].pushRotateZoom(&_sprites[flip], frame_buf_width>>1, (frame_buf_height>>1)+10, ((float)count)*.5, 0.9, 0.95);
}
