#ifndef PICOSSCI_AUDIO_OUTPUT_BUS_RP2040_HPP_
#define PICOSSCI_AUDIO_OUTPUT_BUS_RP2040_HPP_

#include <stdint.h>
#include <stddef.h>

#include <hardware/pio.h>

namespace ns_picossci_audio
{
//--------------------------------------------------------------------------------
class output_pio_base_t
{
public:
  output_pio_base_t() : _pio_index(0), _sm_index(0), _inited(0) {}

  virtual ~output_pio_base_t() {}

  virtual bool init(int pio_index);

  virtual bool deinit(void);

  virtual void start(void);

  virtual void stop(void);

  // Dreqの値を取得する。DMA転送クラスで使用する。
  uint32_t getDreq(void) const;

  // PIOのTX FIFOのアドレスを取得する。DMA転送クラスで使用する。
  volatile void* getWriteAddr(void) const;

protected:
  pio_program_t _program;
  uint _pg_offset;

  struct {
    // 使用するPIO (0 or 1)
    uint8_t _pio_index:1;
    // 使用するステートマシン (0 ~ 3)
    uint8_t _sm_index:2;

    uint8_t _inited:1;

    uint8_t _reserved:4;
  };
};

//--------------------------------------------------------------------------------

/// @brief PIOを使用してGPIOからデータ送信するクラス
/// 使用するGPIOの本数は 1, 2, 4, 8 から選択可能
/// このクラスの出力先をR2R-DACが構成されたピンにすることでアナログ出力ができる。
/// このクラスは単独では機能しないので、DMA転送クラスと組み合わせて使用する。
class output_pio_simple_t : public output_pio_base_t
{
public:
  struct config_t {
    // 送信周波数
    float freq_hz;

    // 使用するPIO (0 or 1)
    uint8_t pio_index;

    // 使用するピンの本数 (0=1本 / 1=2本 / 2=4本 / 3=8本)
    uint8_t pin_bits:2;

    // 出力先GPIOピン番号
    int8_t pin_num;
  };

  const config_t& getConfig(void) const { return _config; }
  bool init(const config_t& config);

protected:
  config_t _config;
  uint16_t _pio_instruction;
};

//--------------------------------------------------------------------------------

/// @brief PIOを使用してGPIOからI2Sデータ送信するクラス
/// pin_bclkを+1した番号がlrckとして扱われる
/// このクラスは単独では機能しないので、DMA転送クラスと組み合わせて使用する。
class output_pio_i2s_t : public output_pio_base_t
{
public:
  struct config_t {
    // オーディオのサンプリングレート
    float freq_hz = 48000;
    // 使用するPIO (0 or 1)
    uint8_t pio_index;

    // 出力先GPIOピン番号
    int8_t pin_dout;
    // BCLKとLRCKのピン番号は連続している必要がある。BCLKの次のピンがLRCKとして扱われる
    int8_t pin_bclk;
    uint8_t bps = 16; // 8/16/24/32
  };

  const config_t& getConfig(void) const { return _config; }
  bool init(const config_t& config);

protected:
  config_t _config;
};

//--------------------------------------------------------------------------------

/// @brief DMAを使用して継続的にデータを転送し続けるクラス
/// DMAを2チャンネル使い交互に動作させることでCPUの介在なくリングバッファDMAを実現する。
/// コールバック関数にて送信するデータのポインタを渡すことができる。
/// データの出力先アドレスは固定。PIOのTX_FIFOアドレスへ出力する形で使用する。
class output_dma_continuous_t
{
public:
  struct config_t {
    /// DMA転送前にISRから呼ばれるコールバック関数
    /// 戻り値で送信するデータのポインタを返すこと。データの準備が間に合わなければnullptrを返す。
    /// なおnullptrを返却した場合は次回の割り込み時に複数回コールバックが呼ばれる。送信は止まらないので注意すること
    uint8_t* (*callback_function)(void* cb_param) = nullptr;
    /// コールバック関数の引数データ。
    void* callback_param = nullptr;
    // 書込み先のアドレス
    volatile void* write_addr;
    // 1回あたりのDMAの長さ
    uint32_t dma_length;
    // DMAループの数 (1u << dma_loop_bits)
    uint8_t dma_loop_bits;
    // DMA転送単位 (0=1Byte / 1=2Byte / 2=4Byte)
    uint8_t dma_transfer_size;
    // 使用する DREQ
    uint8_t dreq;
  };

  const config_t& getConfig(void) const { return _config; }
  bool init(const config_t& config);
  void start(void);
  void stop(void);

  // 継続DMAバスのインスタンスの最大数。
  static constexpr const uint_fast8_t continuous_dma_instance_max = 4;

protected:
  // DMA転送をループするためのバッファポインタ配列用 (aligned_allocを使用してアライメントされたメモリを確保する)
  uintptr_t* _dma_loop_addr_list;

  // データ転送用のDMAチャンネル
  int _dma_chan_transfer;

  // ループ用のDMAチャンネル
  int _dma_chan_looping;

  config_t _config;

  // 現在のループ位置
  uint8_t _loop_index;

  bool _inited = false;

  void make_buffer(uint8_t target_index);

  static void irq_inner(uint_fast8_t num);
  static inline void irq_handler0(void) __attribute__((always_inline)) { irq_inner(0); }
  static inline void irq_handler1(void) __attribute__((always_inline)) { irq_inner(1); }
  static inline void irq_handler2(void) __attribute__((always_inline)) { irq_inner(2); }
  static inline void irq_handler3(void) __attribute__((always_inline)) { irq_inner(3); }
};

//--------------------------------------------------------------------------------
/// 継続的にDMAを使用してGPIOからデータを送信し続けるクラス
class output_bus_continuous_t
{
public:
  struct config_t {
    output_pio_base_t* output_pio;

    void* callback_param;
    void (*callback_function)(void* cb_param, uintptr_t buffer, size_t len) = nullptr;
    // 送信周波数
    float freq_hz;
    // 1回あたりのDMA転送長
    uint32_t dma_buf_size;
    // DMAバッファの数 (最低3)
    uint8_t dma_buf_count;
    // DMA転送単位 (1 << dma_transfer_size)
    uint8_t dma_transfer_size;
    // 使用するPIO (0 or 1)
    uint8_t pio_index;
    // 出力先GPIO番号 (複数使用する場合は先頭の番号)
    int8_t pin_num;
    // 使用するGPIO本数 (0=1本 / 1=2本 / 2=4本 / 3=8本)
    uint8_t pin_bits;
  };

  const config_t& getConfig(void) const { return _config; }
  bool init(const config_t& config);
  void start(void);
  void stop(void);

protected:
  uint8_t* _dma_buffer = nullptr;
  uint32_t _aligned_length;
  output_dma_continuous_t _cont_dma;
  config_t _config;
  uint8_t _dma_index;
  bool _inited = false;
  bool _started = false;

  static uint8_t* continuous_output_dma_callback(void* cb_param);
};

//--------------------------------------------------------------------------------
} // namespace ns_picossci_audio

#endif
