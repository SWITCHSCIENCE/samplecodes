#include <Arduino.h>

// Picossci Audio ライブラリ
#include <Picossci_Audio.h>

static Picossci_Audio picossci_audio;

// 波形データ構造体
struct Waveform {
  const int16_t* data;
  size_t length;
  const char* name;
};

// 三角波 (64サンプル, 48kHzで約750Hz)
static constexpr int16_t kWaveTriangle[] = {
  -16384, -15360, -14336, -13312, -12288, -11264, -10240,  -9216,
   -8192,  -7168,  -6144,  -5120,  -4096,  -3072,  -2048,  -1024,
       0,   1024,   2048,   3072,   4096,   5120,   6144,   7168,
    8192,   9216,  10240,  11264,  12288,  13312,  14336,  15360,
   16384,  15360,  14336,  13312,  12288,  11264,  10240,   9216,
    8192,   7168,   6144,   5120,   4096,   3072,   2048,   1024,
       0,  -1024,  -2048,  -3072,  -4096,  -5120,  -6144,  -7168,
   -8192,  -9216, -10240, -11264, -12288, -13312, -14336, -15360
};

// 矩形波 (64サンプル)
static constexpr int16_t kWaveSquare[] = {
   16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
   16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
   16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
   16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
  -16384, -16384, -16384, -16384, -16384, -16384, -16384, -16384,
  -16384, -16384, -16384, -16384, -16384, -16384, -16384, -16384,
  -16384, -16384, -16384, -16384, -16384, -16384, -16384, -16384,
  -16384, -16384, -16384, -16384, -16384, -16384, -16384, -16384
};

// 正弦波 (64サンプル)
static constexpr int16_t kWaveSine[] = {
       0,   1606,   3196,   4756,   6270,   7723,   9102,  10394,
   11585,  12665,  13623,  14449,  15137,  15679,  16069,  16305,
   16384,  16305,  16069,  15679,  15137,  14449,  13623,  12665,
   11585,  10394,   9102,   7723,   6270,   4756,   3196,   1606,
       0,  -1606,  -3196,  -4756,  -6270,  -7723,  -9102, -10394,
  -11585, -12665, -13623, -14449, -15137, -15679, -16069, -16305,
  -16384, -16305, -16069, -15679, -15137, -14449, -13623, -12665,
  -11585, -10394,  -9102,  -7723,  -6270,  -4756,  -3196,  -1606
};

// 波形リスト
static constexpr Waveform kWaveforms[] = {
  { kWaveTriangle, sizeof(kWaveTriangle) / sizeof(kWaveTriangle[0]), "Triangle" },
  { kWaveSquare,   sizeof(kWaveSquare) / sizeof(kWaveSquare[0]),     "Square" },
  { kWaveSine,     sizeof(kWaveSine) / sizeof(kWaveSine[0]),         "Sine" },
};
static constexpr size_t kWaveformCount = sizeof(kWaveforms) / sizeof(kWaveforms[0]);

// 現在の波形インデックス
static uint8_t g_waveform_index = 0;
// 現在のボリューム (0x00=最大, 0x3F=最小)
static uint8_t g_volume = 0x20;

void setup(void)
{
  Serial.begin(115200);

  // Picossci Audio の初期化 (スイッチも自動的に初期化される)
  auto cfg = picossci_audio.getConfig();
  cfg.volume = g_volume;

  if (!picossci_audio.init(cfg)) {
    Serial.println("Picossci Audio init failed.");
  } else {
    Serial.println("Picossci Audio init OK.");
    Serial.println("Controls:");
    Serial.println("  SW1-1 (GPIO16): Volume Down");
    Serial.println("  SW1-T (GPIO17): Switch Waveform");
    Serial.println("  SW1-2 (GPIO18): Volume Up");
  }

  picossci_audio.start();

  Serial.printf("Waveform: %s\n", kWaveforms[g_waveform_index].name);
  Serial.printf("Volume: 0x%02X\n", g_volume);
}

void loop(void)
{
  static size_t wave_idx = 0;

  // スイッチ状態を更新
  picossci_audio.updateSwitches();

  // SW1-1: ボリュームダウン (値を増やすと音量が下がる)
  if (picossci_audio.sw[Picossci_Audio::SW_1].wasPressed()) {
    if (g_volume < 0x3F) {
      g_volume++;
      picossci_audio.setVolume(g_volume);
      Serial.printf("Volume: 0x%02X (down)\n", g_volume);
    }
  }

  // SW1-2: ボリュームアップ (値を減らすと音量が上がる)
  if (picossci_audio.sw[Picossci_Audio::SW_2].wasPressed()) {
    if (g_volume > 0x00) {
      g_volume--;
      picossci_audio.setVolume(g_volume);
      Serial.printf("Volume: 0x%02X (up)\n", g_volume);
    }
  }

  // SW1-T: 波形切替
  if (picossci_audio.sw[Picossci_Audio::SW_T].wasPressed()) {
    g_waveform_index = (g_waveform_index + 1) % kWaveformCount;
    wave_idx = 0;  // 波形を先頭から再生
    Serial.printf("Waveform: %s\n", kWaveforms[g_waveform_index].name);
  }

  // 音声データを生成して出力
  int remain = (picossci_audio.availableForWrite() >> 1) & ~1;
  if (remain == 0) {
    delay(1);
    return;
  }

  if (remain > 128) {
    remain = 128;
  }

  const Waveform& wave = kWaveforms[g_waveform_index];
  int16_t buffer[128];

  for (int i = 0; i < remain; i += 2) {
    int16_t sample = wave.data[wave_idx];
    wave_idx++;
    if (wave_idx >= wave.length) {
      wave_idx = 0;
    }

    // 左右同じ音を出力 (モノラル)
    buffer[i + 0] = sample;
    buffer[i + 1] = sample;
  }

  picossci_audio.write(buffer, remain << 1);
}

