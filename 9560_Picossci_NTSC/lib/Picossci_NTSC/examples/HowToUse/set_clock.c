/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <hardware/platform_defs.h>

#ifndef USB_CLK_KHZ
#define USB_CLK_KHZ _u(48000)
#endif

#ifndef PLL_COMMON_REFDIV
#define PLL_COMMON_REFDIV 1
#endif

#ifndef XOSC_KHZ
#define XOSC_KHZ _u(12000)
#endif

#ifndef PICO_PLL_VCO_MIN_FREQ_KHZ
#ifndef PICO_PLL_VCO_MIN_FREQ_MHZ
#define PICO_PLL_VCO_MIN_FREQ_KHZ (750 * KHZ)
#else
#define PICO_PLL_VCO_MIN_FREQ_KHZ (PICO_PLL_VCO_MIN_FREQ_MHZ * KHZ)
#endif
#endif

#ifndef PICO_PLL_VCO_MAX_FREQ_KHZ
#ifndef PICO_PLL_VCO_MAX_FREQ_MHZ
#define PICO_PLL_VCO_MAX_FREQ_KHZ (1600 * KHZ)
#else
#define PICO_PLL_VCO_MAX_FREQ_KHZ (PICO_PLL_VCO_MAX_FREQ_MHZ * KHZ)
#endif
#endif

static bool calc_sys_clock_khz(uint32_t freq_khz, uint *vco_out, uint *postdiv1_out, uint *postdiv2_out) {
    uint reference_freq_khz = XOSC_KHZ / PLL_COMMON_REFDIV;

    uint result = UINT32_MAX;
    for (uint fbdiv = 320; fbdiv >= 16; fbdiv--) {
        uint vco_khz = fbdiv * reference_freq_khz;
        if (vco_khz < PICO_PLL_VCO_MIN_FREQ_KHZ  || vco_khz > PICO_PLL_VCO_MAX_FREQ_KHZ) continue;
        for (uint postdiv1 = 7; postdiv1 >= 1; postdiv1--) {
            for (uint postdiv2 = postdiv1; postdiv2 >= 1; postdiv2--) {
                uint out = vco_khz / (postdiv1 * postdiv2);
                uint diff = out < freq_khz ? (freq_khz - out) : (out - freq_khz);
                if (diff < result && !(vco_khz % (postdiv1 * postdiv2))) {
                    result = diff;
                    *vco_out = vco_khz * KHZ;
                    *postdiv1_out = postdiv1;
                    *postdiv2_out = postdiv2;
                    if (out == freq_khz) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void set_clock(uint32_t freq_khz) {
    uint vco_freq=0, post_div1=0, post_div2=0;
    calc_sys_clock_khz(freq_khz, &vco_freq, &post_div1, &post_div2);

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    USB_CLK_KHZ * KHZ,
                    USB_CLK_KHZ * KHZ);

    pll_init(pll_sys, PLL_COMMON_REFDIV, vco_freq, post_div1, post_div2);
    uint32_t freq = vco_freq / (post_div1 * post_div2);

    // Configure clocks
    // CLK_REF is the XOSC source
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0, // No aux mux
                    XOSC_KHZ * KHZ,
                    XOSC_KHZ * KHZ);

    // CLK SYS = PLL SYS (usually) 125MHz / 1 = 125MHz
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    freq, freq);

#if PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    freq, freq);
#else
    clock_configure(clk_peri,
                    0, // Only AUX mux on ADC
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    USB_CLK_KHZ * KHZ,
                    USB_CLK_KHZ * KHZ);
#endif
}
