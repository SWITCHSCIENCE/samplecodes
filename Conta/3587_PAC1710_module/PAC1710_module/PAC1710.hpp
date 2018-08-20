#pragma once 

// PAC1710
namespace PAC1710{
    enum REG{
        CONFIG = 0x00,
        CONVRATE,
        ONSHOT,
        CHMASKREG,
        HLSTAT,
        LLSTAT,
        VSOCFG = 0x0A,
        C1VSCFG,
        C2VSCFG,
        C1SVH,
        C1SVL,
        C2SVH,
        C2SVL,
        C1VSOVH,
        C1VSOVL,
        C2VSOVH,
        C2VSOVL,
        C1PRH = 0x15,
        C1PRL,
        C2PRH,
        C2PRL,
        C1SVHL,
        C1SLHL,
        C2SVHL,
        C2SLHL,
        C1VSOVHL,
        C1VSOVLL,
        C2VSOVHL,
        C2VSOVLL,
        PID = 0xFD,
        MID,
        REV
    };
}