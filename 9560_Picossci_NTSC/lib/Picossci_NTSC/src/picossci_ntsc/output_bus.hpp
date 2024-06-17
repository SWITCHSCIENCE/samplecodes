#ifndef PICOSSCI_NTSC_OUTPUT_BUS_RP2040_HPP_
#define PICOSSCI_NTSC_OUTPUT_BUS_RP2040_HPP_

#include <stdint.h>
#include <stddef.h>

#include <hardware/pio.h>

//--------------------------------------------------------------------------------

/// @brief PIOを使用してGPIOからデータ送信するクラス
/// 使用するGPIOの本数は 1, 2, 4, 8 から選択可能
/// このクラスの出力先をR2R-DACが構成されたピンにすることでアナログ出力ができる。
/// このクラスは単独では機能しないので、DMA転送クラスと組み合わせて使用する。
class simple_output_pio_t
{
public:
  struct config_t {
    // 送信周波数
    float freq_hz;

    struct {
      // 使用するPIO (0 or 1)
      uint8_t pio_index:1;
      // 使用するステートマシン (0 ~ 3)
      uint8_t sm_index:2;
      // 使用するピンの本数 (0=1本 / 1=2本 / 2=4本 / 3=8本)
      uint8_t pin_bits:2;

      uint8_t _reserved:3;
    };
    // 出力先GPIOピン番号
    int8_t pin_num;
  };

  const config_t& getConfig(void) const { return _config; }
  bool init(const config_t& config);
  void start(void);
  void stop(void);
  bool deinit(void);

  // Dreqの値を取得する。DMA転送クラスで使用する。
  uint32_t getDreq(void) const;

  // PIOのTX FIFOのアドレスを取得する。DMA転送クラスで使用する。
  volatile void* getWriteAddr(void) const;

protected:
  config_t _config;
  pio_program_t _program;
  uint _pg_offset;
  uint16_t _pio_instruction;
  bool _inited = false;
};

//--------------------------------------------------------------------------------

/// @brief DMAを使用して継続的にデータを転送し続けるクラス
/// DMAを2チャンネル使い交互に動作させることでCPUの介在なくリングバッファDMAを実現する。
/// コールバック関数にて送信するデータのポインタを渡すことができる。
/// データの出力先アドレスは固定。PIOのTX_FIFOアドレスへ出力する形で使用する。
class continuous_output_dma_t
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
  static void irq_handler0(void) { irq_inner(0); }
  static void irq_handler1(void) { irq_inner(1); }
  static void irq_handler2(void) { irq_inner(2); }
  static void irq_handler3(void) { irq_inner(3); }
};

//--------------------------------------------------------------------------------

/// 継続的にDMAを使用してGPIOからデータを送信し続けるクラス
/// simple_output_pio_t と continuous_output_dma_tを使用する
class continuous_output_bus_t
{
public:
  struct config_t {
    void* callback_param;
    bool (*callback_function)(void* cb_param, uintptr_t buffer, size_t len) = nullptr;
    // 送信周波数
    float freq_hz;
    // 1回あたりのDMA転送長
    uint32_t dma_length;
    // DMAバッファの数 (最低3)
    uint8_t dma_count;
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
  simple_output_pio_t _out_pio;
  continuous_output_dma_t _cont_dma;
  config_t _config;
  uint8_t _dma_index;
  bool _inited = false;
  bool _started = false;

  static uint8_t* continuous_output_dma_callback(void* cb_param);
};

#endif
