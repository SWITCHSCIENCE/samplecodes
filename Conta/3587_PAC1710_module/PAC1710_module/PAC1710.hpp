#pragma once 

// PAC1_710
namespace PAC1710{
    enum ADDR{
        R430 = 0x48,
        R560,
        R750,
        R1270,
        R0,
        R100,
        R180,
        R300,
        R1600 = 0x28,
        R2000,
        R2700,
        R3600,
        R5600,
        R9100,
        R20000,
        OPEN = 0x18
    };
    enum REG{
        CONFIG = 0x00,
        CONVRATE,
        ONESHOT,
        CHMASKREG,
        HLSTAT,
        LLSTAT,
        VSO_CFG = 0x0A,
        C1_VSAMP_CFG,
        C2_VSAMP_CFG,
        C1_SVRES_H = 0x0D,
        C1_SVRES_L,
        C2_SVRES_H,
        C2_SVRES_L,
        C1_VVRES_H = 0x11,
        C1_VVRES_L,
        C2_VVRES_H,
        C2_VVRES_L,
        C1_PR_H = 0x15,
        C1_PR_L,
        C2_PR_H,
        C2_PR_L,
        C1_SVLIM_H = 0x19,
        C2_SVLIM_H,
        C1_SVLIM_L,
        C2_SVLIM_L,
        C1_VVLIM_H = 0x1D,
        C2_VVLIM_H,
        C1_VVLIM_L,
        C2_VVLIM_L,
        PID = 0xFD,
        MID,
        REV
    };
}