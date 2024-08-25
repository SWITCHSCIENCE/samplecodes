#include <Arduino.h>

// PicossciNTSCライブラリ
#include <Picossci_NTSC.h>

// LovyanGFXライブラリ
#include <LovyanGFX.h>

static constexpr const size_t frame_buf_width = 360;
static constexpr const size_t frame_buf_height = 240;

static Picossci_NTSC picossci_ntsc;

// 描画先としてLovyanGFXのLGFX_Spriteを2個使用する
static LGFX_Sprite _sprites[2];

volatile std::uint32_t _draw_count;

// バッファに空きが出来たときに呼び出されるコールバック関数
static void callback_video(void*)
{
  for (;;) {
    // 描画先のY座標を取得
    int y = picossci_ntsc.video.getCurrentY();
    if (y < 0) { // 負の値の場合は空きがない。
      return;
    }
    auto fb = (uint8_t*)(_sprites[~_draw_count & 1]).getBuffer();
    picossci_ntsc.video.writeScanLine(&fb[frame_buf_width * (y * frame_buf_height / 480)], frame_buf_width);
  }
}

struct ball_info_t {
  int32_t x;
  int32_t y;
  int32_t dx;
  int32_t dy;
  int32_t r;
  int32_t m;
  uint32_t color;
};

static constexpr std::uint32_t SHIFTSIZE = 8;
static constexpr std::uint32_t BALL_MAX = 256;

static ball_info_t _balls[2][BALL_MAX];
static std::uint32_t _ball_count = 0, _fps = 0;
static std::uint32_t ball_count = 0;
static std::uint32_t sec, psec;
static std::uint32_t fps = 0, frame_count = 0;

static std::uint32_t _width;
static std::uint32_t _height;

volatile bool _is_running;
volatile std::uint32_t _loop_count;

static void drawfunc(void)
{
  ball_info_t *balls;
  ball_info_t *a;
  LGFX_Sprite *sprite;

  auto width  = _sprites[0].width();
  auto height = _sprites[0].height();

  std::size_t flip = _draw_count & 1;
  balls = &_balls[flip][0];

  sprite = &(_sprites[flip]);
  sprite->clear();

  for (int32_t i = 8; i < width; i += 16) {
    sprite->drawFastVLine(i, 0, height, 0x1F);
  }
  for (int32_t i = 8; i < height; i += 16) {
    sprite->drawFastHLine(0, i, width, 0x1F);
  }
  for (std::uint32_t i = 0; i < _ball_count; i++) {
    a = &balls[i];
    sprite->fillCircle( a->x >> SHIFTSIZE
                      , a->y >> SHIFTSIZE
                      , a->r >> SHIFTSIZE
                      , a->color);
  }

  sprite->setCursor(16,16);
  sprite->setTextColor(TFT_BLACK);
  sprite->printf("obj:%d fps:%d", _ball_count, _fps);
  sprite->setCursor(15,15);
  sprite->setTextColor(TFT_WHITE);
  sprite->printf("obj:%d fps:%d", _ball_count, _fps);
  // diffDraw(&_sprites[flip], &_sprites[!flip]);
  ++_draw_count;
}

static void mainfunc(void)
{
  static constexpr float e = 0.999; // Coefficient of friction

  sec = lgfx::millis() / 1000;
  if (psec != sec) {
    psec = sec;
    fps = frame_count;
    frame_count = 0;

    if (++ball_count >= BALL_MAX) { ball_count = 1; }
    auto a = &_balls[_loop_count & 1][ball_count - 1];
    a->color = lgfx::color888(100+(rand()%155), 100+(rand()%155), 100+(rand()%155));
    a->x = 0;
    a->y = 0;
    a->dx = (rand() & (3 << SHIFTSIZE)) + 1;
    a->dy = (rand() & (3 << SHIFTSIZE)) + 1;
    a->r = (4 + (ball_count & 0x07)) << SHIFTSIZE;
    a->m =  4 + (ball_count & 0x07);
  }

  frame_count++;
  _loop_count++;

  ball_info_t *a, *b, *balls;
  int32_t rr, len, vx2vy2;
  float vx, vy, distance, t;

  size_t f = _loop_count & 1;
  balls = a = &_balls[f][0];
  b = &_balls[!f][0];
  memcpy(a, b, sizeof(ball_info_t) * ball_count);

  for (int i = 0; i != ball_count; i++) {
    a = &balls[i];
//  a->dy += 4; // gravity

    a->x += a->dx;
    if (a->x < a->r) {
      a->x = a->r;
      if (a->dx < 0) a->dx = - a->dx*e;
    } else if (a->x >= _width - a->r) {
      a->x = _width - a->r -1;
      if (a->dx > 0) a->dx = - a->dx*e;
    }
    a->y += a->dy;
    if (a->y < a->r) {
      a->y = a->r;
      if (a->dy < 0) a->dy = - a->dy*e;
    } else if (a->y >= _height - a->r) {
      a->y = _height - a->r -1;
      if (a->dy > 0) a->dy = - a->dy*e;
    }
    for (int j = i + 1; j != ball_count; j++) {
      b = &balls[j];

      rr = a->r + b->r;
      vx = a->x - b->x;
      if (abs(vx) > rr) continue;
      vy = a->y - b->y;
      if (abs(vy) > rr) continue;

      len = sqrt(vx * vx + vy * vy);
      if (len >= rr) continue;
      if (len == 0.0) continue;
      distance = (rr - len) >> 1;
      vx *= distance / len;
      vy *= distance / len;

      a->x += vx;
      b->x -= vx;
      vx = b->x - a->x;

      a->y += vy;
      b->y -= vy;
      vy = b->y - a->y;

      vx2vy2 = vx * vx + vy * vy;

      t = -(vx * a->dx + vy * a->dy) / vx2vy2;
      float arx = a->dx + vx * t;
      float ary = a->dy + vy * t;

      t = -(-vy * a->dx + vx * a->dy) / vx2vy2;
      float amx = a->dx - vy * t;
      float amy = a->dy + vx * t;

      t = -(vx * b->dx + vy * b->dy) / vx2vy2;
      float brx = b->dx + vx * t;
      float bry = b->dy + vy * t;

      t = -(-vy * b->dx + vx * b->dy) / vx2vy2;
      float bmx = b->dx - vy * t;
      float bmy = b->dy + vx * t;

      float adx = (a->m * amx + b->m * bmx + bmx * e * b->m - amx * e * b->m) / (a->m + b->m);
      float bdx = - e * (bmx - amx) + adx;
      float ady = (a->m * amy + b->m * bmy + bmy * e * b->m - amy * e * b->m) / (a->m + b->m);
      float bdy = - e * (bmy - amy) + ady;

      a->dx = roundf(adx + arx);
      a->dy = roundf(ady + ary);
      b->dx = roundf(bdx + brx);
      b->dy = roundf(bdy + bry);
    }
  }

  _fps = fps;
  _ball_count = ball_count;
}

void setup(void)
{
  picossci_ntsc.setCpuClock(157500);

  for (std::uint32_t i = 0; i < 2; ++i)
  {
    _sprites[i].setTextSize(2);
    _sprites[i].setColorDepth(8);
    _sprites[i].createSprite(frame_buf_width, frame_buf_height);
  }

  int x_offset = 0;
  picossci_ntsc.video.setOffset(x_offset);
  picossci_ntsc.video.setScale(((720 - x_offset * 2) * 256) / frame_buf_width);
  picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb332);

  auto cfg = picossci_ntsc.video.getConfig();
  cfg.callback_function = callback_video;
  cfg.dma_buf_count = 8;

  picossci_ntsc.video.init(cfg);
  picossci_ntsc.video.start();

  _width = frame_buf_width << SHIFTSIZE;
  _height = frame_buf_height << SHIFTSIZE;

  for (std::uint32_t i = 0; i < ball_count; ++i)
  {
    auto a = &_balls[_loop_count & 1][i];
    a->color = lgfx::color888(100+(rand()%155), 100+(rand()%155), 100+(rand()%155));
    a->x = 0;
    a->y = 0;
    a->dx = (rand() & (3 << SHIFTSIZE)) + 1;
    a->dy = (rand() & (3 << SHIFTSIZE)) + 1;
    a->r = (4 + (i & 0x07)) << SHIFTSIZE;
    a->m =  4 + (i & 0x07);
  }

  _is_running = true;
  _draw_count = 0;
  _loop_count = 0;
}

void loop(void)
{
  mainfunc();

  drawfunc();
}
