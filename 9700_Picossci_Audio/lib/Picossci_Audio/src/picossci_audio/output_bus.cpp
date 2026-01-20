// #define GPIO_DEBUG_PIN 21

#include "output_bus.hpp"

#include <hardware/pll.h>
#include <hardware/gpio.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <math.h>
#include <string.h>

namespace ns_picossci_audio
{
//--------------------------------------------------------------------------------

/// @brief CPU動作周波数を取得する
/// @param fref リファレンス周波数 [Hz]
/// @return CPU動作周波数 [Hz]
static uint_fast32_t getCpuClock(uint_fast32_t fref = 12000000)
{
  uint_fast8_t refdiv = pll_sys_hw->cs & 0x3f;
  uint_fast16_t fbdiv = pll_sys_hw->fbdiv_int & 0x00000fff;
  uint_fast8_t postdiv1 = (pll_sys_hw->prim >> 16) & 0x07;
  uint_fast8_t postdiv2 = (pll_sys_hw->prim >> 12) & 0x07;
  return (fref / refdiv) * fbdiv / (postdiv1 * postdiv2);
}

static pio_hw_t* getPioFromIndex(uint_fast8_t index)
{
  switch (index) {
#ifdef pio0
  case 0: return pio0;
#endif
#ifdef pio1
  case 1: return pio1;
#endif
#ifdef pio2
  case 2: return pio2;
#endif
#ifdef pio3
  case 3: return pio3;
#endif
  default: return nullptr;
  }
}

//--------------------------------------------------------------------------------

bool output_pio_base_t::init(int pio_index) {
  _pio_index = pio_index;
  auto target_pio = getPioFromIndex(_pio_index);
  if (target_pio == nullptr) { return false; }
  auto sm = pio_claim_unused_sm(target_pio, true);
  if (sm < 0) { return false; }
  _sm_index = sm;
  _inited = true;
  return true;
}

bool output_pio_base_t::deinit(void) {
  if (!_inited) { return false; }
  auto target_pio = getPioFromIndex(_pio_index);
  pio_remove_program(target_pio, &_program, _pg_offset);
  pio_sm_unclaim(target_pio, _sm_index);
  _inited = false;
  return true;
}

void output_pio_base_t::start(void) {
  if (!_inited) { return; }
  auto target_pio = getPioFromIndex(_pio_index);
  pio_sm_set_enabled(target_pio, _sm_index, true);
}

void output_pio_base_t::stop(void)
{
  if (!_inited) { return; }
  auto target_pio = getPioFromIndex(_pio_index);
  pio_sm_set_enabled(target_pio, _sm_index, false);
}

uint32_t output_pio_base_t::getDreq(void) const
{
  if (!_inited) { return 0; }
  uint32_t sm = _sm_index;
  switch (_pio_index) {
#ifdef DREQ_PIO0_TX0
  case 0: return DREQ_PIO0_TX0 + sm;
#endif
#ifdef DREQ_PIO1_TX0
  case 1: return DREQ_PIO1_TX0 + sm;
#endif
#ifdef DREQ_PIO2_TX0
  case 2: return DREQ_PIO2_TX0 + sm;
#endif
#ifdef DREQ_PIO3_TX0
  case 3: return DREQ_PIO3_TX0 + sm;
#endif
  default: return sm;
  }
}

volatile void* output_pio_base_t::getWriteAddr(void) const
{
  if (!_inited) { return nullptr; }
  auto target_pio = getPioFromIndex(_pio_index);
  return target_pio ? &(target_pio->txf[_sm_index]) : nullptr;
}

//--------------------------------------------------------------------------------

bool output_pio_simple_t::init(const config_t& config)
{
  if (_inited) { return false; }
  if (config.pin_bits > 3) { return false; }
  if (!output_pio_base_t::init(config.pio_index)) {
    return false;
  }
  _config = config;

  /// 現在のCPUクロックを取得する
  uint32_t cpu_clock = getCpuClock();

  uint32_t best_div256 = 256;
  uint32_t best_cycle = 0;
  float best_frac = 65536.0f;
  /// 現在のCPU速度に基づいて、送信周波数に最も近くなるPIO動作分周比を求める
  for (uint32_t cycle = 0; cycle < 32; ++cycle) {
    float f_div = (float)cpu_clock / (config.freq_hz * (1 + cycle) / 256.0f);
    uint32_t div256 = roundf(f_div);
    if (div256 < 256) { break; }
    if (div256 > 65535) { div256 = 65535; }
    float frac = fabsf(f_div - (float)div256);
    if (frac < best_frac) {
      best_frac = frac;
      best_div256 = div256;
      best_cycle = cycle;
      if (frac < __FLT_EPSILON__) { break; }
    }
  }
  if (best_div256 < 256) { best_div256 = 1; }
  if (best_div256 > 65535) { best_div256 = 65535; }

  /// OSR (Output Shift Register) の内容をGPIOに出力するPIO命令の作成。
  /// 0x6000 は OUT命令。 0x100*cycleはウェイトサイクル数。下位5bit分は出力するピンの数。
  _pio_instruction = 0x6000 | (0x100 * best_cycle) | (1 << config.pin_bits);

  auto target_pio = getPioFromIndex(config.pio_index);

  /// pioに登録するプログラム (上記の出力命令のみをループする処理のため、命令長は 1)
  _program.instructions = &_pio_instruction;
  _program.length = 1;
  _program.origin = -1;

  _pg_offset = pio_add_program(target_pio, &_program);
  pio_sm_config c = pio_get_default_sm_config();

  /// 動作速度の設定 (整数成分と 1/256成分に分ける)
  sm_config_set_clkdiv_int_frac(&c, (uint8_t)(best_div256 >> 8), (uint8_t)best_div256);

  sm_config_set_sideset(&c, 0, false, false);
  sm_config_set_wrap(&c, _pg_offset, _pg_offset); /// 1命令でループする設定

  sm_config_set_fifo_join(&c, pio_fifo_join::PIO_FIFO_JOIN_TX);

  // autopullによるデータの読取りを8bit単位で動作するように設定しておく。
  // 左シフト設定にして最上位ビットから出力されるようにする
  sm_config_set_out_shift(&c, false, true, 8);

  /// 出力先GPIOの設定
  auto pin_count = 1u << config.pin_bits;
  sm_config_set_out_pins(&c, config.pin_num, pin_count);
  for (uint pidx = 0; pidx < pin_count; pidx++) {
    pio_gpio_init(target_pio, pidx + config.pin_num);
  }
  pio_sm_set_consecutive_pindirs(target_pio, _sm_index, config.pin_num, pin_count, true);

  /// PIO初期化
  pio_sm_init(target_pio, _sm_index, _pg_offset, &c);

  _inited = true;
  return true;
}

//--------------------------------------------------------------------------------

bool output_pio_i2s_t::init(const config_t& config)
{
  if (_inited) { return false; }
  if (config.bps < 8 || config.bps > 32) { return false; }
  if (!output_pio_base_t::init(config.pio_index)) {
    return false;
  }
  _config = config;

  /// 現在のCPUクロックを取得する
  uint32_t cpu_clock = getCpuClock();

  /// 現在のCPU速度に基づいて、送信周波数に最も近くなるPIO動作分周比を求める
  // ビットレート , I2SはLRチャンネルがあるのでbpsを2倍にする
  float f_div = (float)cpu_clock / (config.freq_hz * (_config.bps * 4) / 256.0f);
  uint32_t div256 = roundf(f_div);

  static constexpr const uint16_t _pio_instructions[] = {
    0xa822, 0x7001, 0x1841, 0x7001, 0xb822, 0x6001, 0x0845, 0x6001, };  // LSB justified
 // 0xa822, 0x6001, 0x0841, 0x7001, 0xb822, 0x7001, 0x1845, 0x6001, };  // Philips standard

  auto target_pio = getPioFromIndex(config.pio_index);

  /// pioに登録するプログラム
  _program.instructions = _pio_instructions;
  _program.length = sizeof(_pio_instructions) / sizeof(_pio_instructions[0]);
  _program.origin = -1;

  _pg_offset = pio_add_program(target_pio, &_program);
  pio_sm_config c = pio_get_default_sm_config();

  /// 動作速度の設定 (整数成分と 1/256成分に分ける)
  sm_config_set_clkdiv_int_frac(&c, (uint8_t)(div256 >> 8), (uint8_t)div256);

  // BCLKとLRCKの設定
  sm_config_set_sideset(&c, 2, false, false);
  sm_config_set_wrap(&c, _pg_offset, _pg_offset + _program.length - 1);

  sm_config_set_sideset_pins(&c, config.pin_bclk);
  // sm_config_set_out_shift(&c, false, true, (_config.bps <= 16) ? 2 * _config.bps : _config.bps);
  sm_config_set_out_shift(&c, false, true, 16);

  sm_config_set_fifo_join(&c, pio_fifo_join::PIO_FIFO_JOIN_TX);

  /// 出力先GPIOの設定
  pio_gpio_init(target_pio, config.pin_dout);
  pio_gpio_init(target_pio, config.pin_bclk);
  pio_gpio_init(target_pio, config.pin_bclk+1);
  auto pin_count = 1;
  sm_config_set_out_pins(&c, config.pin_dout, pin_count);
  pio_sm_set_consecutive_pindirs(target_pio, _sm_index, config.pin_dout, pin_count, true);
  pio_sm_set_consecutive_pindirs(target_pio, _sm_index, config.pin_bclk, 2, true); // BCLKとLRCKのピン番号は連続している必要がある
  /// PIO初期化
  pio_sm_init(target_pio, _sm_index, _pg_offset, &c);

  // Yレジスタにbps-2を設定することで、I2SのPhilipsフォーマットに合わせる
  pio_sm_exec(target_pio, _sm_index, pio_encode_set(pio_y, _config.bps - 2));

  _inited = true;
  return true;
}

//--------------------------------------------------------------------------------

/// ISR関数側からDMAインスタンスを特定するために、インスタンスのポインタを配列で管理している。
/// ISRハンドラ自体をインスタンス別に分け、インスタンスとハンドラを1対1で対応させるようにしている。
/// これによりISR内から操作対象のインスタンスを特定でき、複数インスタンスの同時使用を可能とした。
output_dma_continuous_t* _continuous_dma_used_instance[output_dma_continuous_t::continuous_dma_instance_max] = { nullptr, nullptr, nullptr, nullptr };

void output_dma_continuous_t::irq_inner(uint_fast8_t num)
{
#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 1);
#endif

  auto me = _continuous_dma_used_instance[num];
  auto dma_ch = me->_dma_chan_looping;
  dma_hw->ints0 = 1u << dma_ch;
  me->make_buffer((dma_hw->ch[dma_ch].read_addr >> 2) - 1);

#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 0);
#endif
}

/// DMAバッファ作成コールバック関数を呼び出し、DMAバッファを作成する
void output_dma_continuous_t::make_buffer(uint8_t target_index)
{
  uint_fast8_t loop_bit_mask = ((1 << _config.dma_loop_bits) - 1);
  target_index &= loop_bit_mask;
  uint_fast8_t loop_index = _loop_index;
  if (loop_index != target_index)
  {
    auto loop_base_addr = (uint8_t**)_dma_loop_addr_list;
    auto callback_function = _config.callback_function;
    auto callback_param = _config.callback_param;
    do
    {
      auto addr = callback_function(callback_param);
      /// コールバック関数がnullptrを返した場合は次回の割り込み時に再度コールバックを行う
      if (addr == nullptr) { return; }
      loop_base_addr[loop_index] = addr;
      loop_index = (loop_index + 1) & loop_bit_mask;
    } while (target_index != loop_index);
    _loop_index = loop_index;
  }
}

bool output_dma_continuous_t::init(const config_t& config)
{
  _inited = false;
  if (config.dma_loop_bits == 0 || config.callback_function == nullptr) { return false; }

#if defined ( GPIO_DEBUG_PIN )
#if __has_include (<FreeRTOS.h>)
gpio_init(GPIO_DEBUG_PIN);
#else
_gpio_init(GPIO_DEBUG_PIN);
#endif
gpio_set_dir(GPIO_DEBUG_PIN, GPIO_OUT);
#endif

  uint_fast8_t instance_index = 0;
  while (_continuous_dma_used_instance[instance_index] != nullptr && instance_index < continuous_dma_instance_max) { ++instance_index; }
  if (instance_index >= continuous_dma_instance_max) { return false; }

  _continuous_dma_used_instance[instance_index] = this;
  _config = config;
  int dma_chan_transfer = dma_claim_unused_channel(false);
  if (dma_chan_transfer < 0) { return false; }
  int dma_chan_looping  = dma_claim_unused_channel(false);
  if (dma_chan_looping < 0)
  {
    dma_channel_unclaim(dma_chan_transfer);
    return false;
  }
  uint8_t loop_bits = config.dma_loop_bits;

  _dma_loop_addr_list = (uintptr_t*)((uintptr_t)malloc(sizeof(uintptr_t) << (loop_bits + 1)) & ~((sizeof(uintptr_t) << loop_bits) - 1));

  _dma_chan_transfer = dma_chan_transfer;
  _dma_chan_looping  = dma_chan_looping;
  { // バッファを出力するためのDMAチャンネル設定
    auto dma_chan_config = dma_channel_get_default_config(dma_chan_transfer);
    channel_config_set_dreq(&dma_chan_config, config.dreq);
    channel_config_set_transfer_data_size(&dma_chan_config, (dma_channel_transfer_size)config.dma_transfer_size); // データを何Byte単位で読むか設定
    channel_config_set_bswap(&dma_chan_config, false); // バイトオーダーの変更は行わない
    channel_config_set_chain_to(&dma_chan_config, _dma_chan_looping); // ループ用のDMAにチェインする
    dma_chan_config.ctrl |= DMA_CH0_CTRL_TRIG_HIGH_PRIORITY_BITS;//   channel_config_set_high_priority(&dma_chan_config, true);
    dma_channel_configure(
        dma_chan_transfer,
        &dma_chan_config,
        config.write_addr,  // 出力先アドレスはconfigで指定されたアドレス
        nullptr,
        config.dma_length,  // 転送サイズはconfigで指定された長さ
        false // ここでは転送を開始しない
    );
  }

  { // バッファ出力を終えた際に、次のバッファに切り替えて再開させるためのDMAチャンネル設定
    // このDMAはリングバッファを用いて動作することで、データの生成タイミングにズレが生じてもデータが途絶えないようにする
    auto dma_chan_config = dma_channel_get_default_config(dma_chan_looping);
    channel_config_set_read_increment(&dma_chan_config, true); // 読出しアドレスは順次進める
    channel_config_set_chain_to(&dma_chan_config, dma_chan_transfer); // データ転送用のDMAにチェインする
    channel_config_set_transfer_data_size(&dma_chan_config, DMA_SIZE_32); // アドレス書き換えを目的としているので4Byte単位で読み込む
    channel_config_set_ring(&dma_chan_config, false, 2+loop_bits); // 4バイト単位でのDMAループを構成するのでloop_bitsに2を加算する
    dma_chan_config.ctrl |= DMA_CH0_CTRL_TRIG_HIGH_PRIORITY_BITS;//    channel_config_set_high_priority(&dma_chan_config, true);
    dma_channel_configure(
        dma_chan_looping,
        &dma_chan_config,
        &(dma_hw->ch[dma_chan_transfer].read_addr), // バッファ転送DMAのデータ読み出しアドレスに対して書き換えを行う設定。
        _dma_loop_addr_list,
        1,    // 転送サイズは1個で良い (アドレス情報を書き換えるだけなので一回で終了する)
        false // ここでは転送を開始しない
    );

    _loop_index = 0;

    static constexpr void(*irq_handlers[continuous_dma_instance_max])(void) = { irq_handler0, irq_handler1, irq_handler2, irq_handler3 };

    auto dma_irq = (instance_index & 1) ? DMA_IRQ_1 : DMA_IRQ_0;
    if (dma_irq == DMA_IRQ_0) {
      dma_channel_set_irq0_enabled(dma_chan_looping, true);
    } else {
      dma_channel_set_irq1_enabled(dma_chan_looping, true);
    }
    irq_add_shared_handler(dma_irq, irq_handlers[instance_index], PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(dma_irq, true);
  }

  _inited = true;
  return true;
}

void output_dma_continuous_t::start(void)
{
  if (!_inited) { return; }
  // 転送を始める前に最初のデータ要求をしておくことでDMA転送の初回のデータを準備する。
  make_buffer((uint8_t)-1);
  dma_channel_start(_dma_chan_looping);
}

void output_dma_continuous_t::stop(void)
{
  if (!_inited) { return; }
  dma_channel_abort(_dma_chan_transfer);
  dma_channel_abort(_dma_chan_looping);
}

//--------------------------------------------------------------------------------
bool output_bus_continuous_t::init(const config_t& config)
{
  if (_inited) { return false; }
  _started = false;
  if (config.dma_buf_count < 2) { return false; }
  _config = config;

  /// DMAバッファの作成時に各バッファ長さを4バイト単位にアライメントしておく。
  uint32_t aligned_len = (config.dma_buf_size + 3) & ~3;
  _aligned_length = aligned_len;
  _dma_buffer = (uint8_t*)(~3 & (uintptr_t)malloc(4 + aligned_len * config.dma_buf_count));
  if (_dma_buffer)
  {
    /// 出力DMAの初期化
    auto cfg_dma = _cont_dma.getConfig();

    /// 出力先PIOからDMA転送先の情報を取得してセットする
    cfg_dma.dreq = _config.output_pio->getDreq();
    cfg_dma.write_addr = _config.output_pio->getWriteAddr();

    /// 出力DMAに用意させるリングバッファの個数は最小の2個とする (1bit = 2個)
    cfg_dma.dma_loop_bits = 1;
    cfg_dma.dma_length = config.dma_buf_size >> config.dma_transfer_size;
    cfg_dma.dma_transfer_size = config.dma_transfer_size;

    /// DMAコールバックを指定
    cfg_dma.callback_function = continuous_output_dma_callback;
    cfg_dma.callback_param = this;

    if (_cont_dma.init(cfg_dma))
    {
      _inited = true;
      return true;
    }
    free(_dma_buffer);
    _dma_buffer = nullptr;
  }
  return false;
}

void output_bus_continuous_t::start(void)
{
  if (!_inited || _started) { return; }

  uint_fast8_t dma_index = 0;
  auto len = _aligned_length;
  /// 出力DMAリングバッファが2個分のバッファを使用しているので、初回のDMAデータ準備は2個分減らしておく。
  /// ※ ここで2個減らしておかないと、DMA転送の最中のバッファをコールバックに渡してしまう可能性が生じるため必ず減らしておく。
  uint_fast8_t loop_end = _config.dma_buf_count - 2;
  for (; dma_index < loop_end; ++dma_index) {
    _config.callback_function(_config.callback_param, (uintptr_t)&_dma_buffer[dma_index * len], len);
  }
  _dma_index = dma_index;
  _started = true;

  _config.output_pio->start();
  _cont_dma.start();
}

void output_bus_continuous_t::stop(void)
{
  if (!_inited || !_started) { return; }
  _started = false;

  _config.output_pio->stop();
  _cont_dma.stop();
}

uint8_t* output_bus_continuous_t::continuous_output_dma_callback(void* cb_param)
{
  auto me = (output_bus_continuous_t*)cb_param;
  uint_fast8_t idx = me->_dma_index;
  uint32_t len = me->_aligned_length;
  auto buffer = me->_dma_buffer;

  me->_config.callback_function(me->_config.callback_param, (uintptr_t)&buffer[idx * len], len);
  auto dma_count = me->_config.dma_buf_count;
  // 次のDMAバッファを設定する
  if (++idx >= dma_count) { idx = 0; }
  me->_dma_index = idx;
  // さらに次のDMAバッファを返却する
  if (++idx >= dma_count) { idx = 0; }
  auto res = &buffer[idx * len];
  return res;
}

//--------------------------------------------------------------------------------
} // namespace ns_picossci_audio
