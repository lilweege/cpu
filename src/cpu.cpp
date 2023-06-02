#include "cpu.hpp"

#include "elf.h"
#include <cstdlib>

static int32_t SignExtend(uint32_t x, uint32_t n)
{
    assert(n > 0 && n < 32);
    bool isNegative = x & (1U << (n-1U));
    if (!isNegative)
        return static_cast<int32_t>(x);

    uint32_t extendedBitMask = ~((1U << n) - 1U);
    return static_cast<int32_t>(x | extendedBitMask);
}

FormattedInstruction FormatInstruction(RawInstruction ins)
{
    FormattedInstruction result;
    FormatInstruction(ins, result.buffer, sizeof(result.buffer));
    return result;
}

#define CSR_cycle          0xc00
#define CSR_cycleh         0xc80
#define CSR_dcsr           0x7b0
#define CSR_dpc            0x7b1
#define CSR_dscratch0      0x7b2
#define CSR_dscratch1      0x7b3
#define CSR_fcsr           0x003
#define CSR_fflags         0x001
#define CSR_frm            0x002
#define CSR_hcounteren     0x606
#define CSR_hedeleg        0x602
#define CSR_hgatp          0x680
#define CSR_hgeie          0x607
#define CSR_hgeip          0xe07
#define CSR_hideleg        0x603
#define CSR_hie            0x604
#define CSR_hip            0x644
#define CSR_hpmcounter3    0xc03
#define CSR_hpmcounter4    0xc04
#define CSR_hpmcounter5    0xc05
#define CSR_hpmcounter6    0xc06
#define CSR_hpmcounter7    0xc07
#define CSR_hpmcounter8    0xc08
#define CSR_hpmcounter9    0xc09
#define CSR_hpmcounter10   0xc0a
#define CSR_hpmcounter11   0xc0b
#define CSR_hpmcounter12   0xc0c
#define CSR_hpmcounter13   0xc0d
#define CSR_hpmcounter14   0xc0e
#define CSR_hpmcounter15   0xc0f
#define CSR_hpmcounter16   0xc10
#define CSR_hpmcounter17   0xc11
#define CSR_hpmcounter18   0xc12
#define CSR_hpmcounter19   0xc13
#define CSR_hpmcounter20   0xc14
#define CSR_hpmcounter21   0xc15
#define CSR_hpmcounter22   0xc16
#define CSR_hpmcounter23   0xc17
#define CSR_hpmcounter24   0xc18
#define CSR_hpmcounter25   0xc19
#define CSR_hpmcounter26   0xc1a
#define CSR_hpmcounter27   0xc1b
#define CSR_hpmcounter28   0xc1c
#define CSR_hpmcounter29   0xc1d
#define CSR_hpmcounter30   0xc1e
#define CSR_hpmcounter31   0xc1f
#define CSR_hpmcounter3h   0xc83
#define CSR_hpmcounter4h   0xc84
#define CSR_hpmcounter5h   0xc85
#define CSR_hpmcounter6h   0xc86
#define CSR_hpmcounter7h   0xc87
#define CSR_hpmcounter8h   0xc88
#define CSR_hpmcounter9h   0xc89
#define CSR_hpmcounter10h  0xc8a
#define CSR_hpmcounter11h  0xc8b
#define CSR_hpmcounter12h  0xc8c
#define CSR_hpmcounter13h  0xc8d
#define CSR_hpmcounter14h  0xc8e
#define CSR_hpmcounter15h  0xc8f
#define CSR_hpmcounter16h  0xc90
#define CSR_hpmcounter17h  0xc91
#define CSR_hpmcounter18h  0xc92
#define CSR_hpmcounter19h  0xc93
#define CSR_hpmcounter20h  0xc94
#define CSR_hpmcounter21h  0xc95
#define CSR_hpmcounter22h  0xc96
#define CSR_hpmcounter23h  0xc97
#define CSR_hpmcounter24h  0xc98
#define CSR_hpmcounter25h  0xc99
#define CSR_hpmcounter26h  0xc9a
#define CSR_hpmcounter27h  0xc9b
#define CSR_hpmcounter28h  0xc9c
#define CSR_hpmcounter29h  0xc9d
#define CSR_hpmcounter30h  0xc9e
#define CSR_hpmcounter31h  0xc9f
#define CSR_hstatus        0x600
#define CSR_htimedelta     0x605
#define CSR_htimedeltah    0x615
#define CSR_htinst         0x64a
#define CSR_htval          0x643
#define CSR_instret        0xc02
#define CSR_instreth       0xc82
#define CSR_marchid        0xf12
#define CSR_mbase          0x380
#define CSR_mbound         0x381
#define CSR_mcause         0x342
#define CSR_mcounteren     0x306
#define CSR_mcountinhibit  0x320
#define CSR_mcycle         0xb00
#define CSR_mcycleh        0xb80
#define CSR_mdbase         0x384
#define CSR_mdbound        0x385
#define CSR_medeleg        0x302
#define CSR_mepc           0x341
#define CSR_mhartid        0xf14
#define CSR_mhpmcounter3   0xb03
#define CSR_mhpmcounter4   0xb04
#define CSR_mhpmcounter5   0xb05
#define CSR_mhpmcounter6   0xb06
#define CSR_mhpmcounter7   0xb07
#define CSR_mhpmcounter8   0xb08
#define CSR_mhpmcounter9   0xb09
#define CSR_mhpmcounter10  0xb0a
#define CSR_mhpmcounter11  0xb0b
#define CSR_mhpmcounter12  0xb0c
#define CSR_mhpmcounter13  0xb0d
#define CSR_mhpmcounter14  0xb0e
#define CSR_mhpmcounter15  0xb0f
#define CSR_mhpmcounter16  0xb10
#define CSR_mhpmcounter17  0xb11
#define CSR_mhpmcounter18  0xb12
#define CSR_mhpmcounter19  0xb13
#define CSR_mhpmcounter20  0xb14
#define CSR_mhpmcounter21  0xb15
#define CSR_mhpmcounter22  0xb16
#define CSR_mhpmcounter23  0xb17
#define CSR_mhpmcounter24  0xb18
#define CSR_mhpmcounter25  0xb19
#define CSR_mhpmcounter26  0xb1a
#define CSR_mhpmcounter27  0xb1b
#define CSR_mhpmcounter28  0xb1c
#define CSR_mhpmcounter29  0xb1d
#define CSR_mhpmcounter30  0xb1e
#define CSR_mhpmcounter31  0xb1f
#define CSR_mhpmcounter3h  0xb83
#define CSR_mhpmcounter4h  0xb84
#define CSR_mhpmcounter5h  0xb85
#define CSR_mhpmcounter6h  0xb86
#define CSR_mhpmcounter7h  0xb87
#define CSR_mhpmcounter8h  0xb88
#define CSR_mhpmcounter9h  0xb89
#define CSR_mhpmcounter10h 0xb8a
#define CSR_mhpmcounter11h 0xb8b
#define CSR_mhpmcounter12h 0xb8c
#define CSR_mhpmcounter13h 0xb8d
#define CSR_mhpmcounter14h 0xb8e
#define CSR_mhpmcounter15h 0xb8f
#define CSR_mhpmcounter16h 0xb90
#define CSR_mhpmcounter17h 0xb91
#define CSR_mhpmcounter18h 0xb92
#define CSR_mhpmcounter19h 0xb93
#define CSR_mhpmcounter20h 0xb94
#define CSR_mhpmcounter21h 0xb95
#define CSR_mhpmcounter22h 0xb96
#define CSR_mhpmcounter23h 0xb97
#define CSR_mhpmcounter24h 0xb98
#define CSR_mhpmcounter25h 0xb99
#define CSR_mhpmcounter26h 0xb9a
#define CSR_mhpmcounter27h 0xb9b
#define CSR_mhpmcounter28h 0xb9c
#define CSR_mhpmcounter29h 0xb9d
#define CSR_mhpmcounter30h 0xb9e
#define CSR_mhpmcounter31h 0xb9f
#define CSR_mhpmevent3     0x323
#define CSR_mhpmevent4     0x324
#define CSR_mhpmevent5     0x325
#define CSR_mhpmevent6     0x326
#define CSR_mhpmevent7     0x327
#define CSR_mhpmevent8     0x328
#define CSR_mhpmevent9     0x329
#define CSR_mhpmevent10    0x32a
#define CSR_mhpmevent11    0x32b
#define CSR_mhpmevent12    0x32c
#define CSR_mhpmevent13    0x32d
#define CSR_mhpmevent14    0x32e
#define CSR_mhpmevent15    0x32f
#define CSR_mhpmevent16    0x330
#define CSR_mhpmevent17    0x331
#define CSR_mhpmevent18    0x332
#define CSR_mhpmevent19    0x333
#define CSR_mhpmevent20    0x334
#define CSR_mhpmevent21    0x335
#define CSR_mhpmevent22    0x336
#define CSR_mhpmevent23    0x337
#define CSR_mhpmevent24    0x338
#define CSR_mhpmevent25    0x339
#define CSR_mhpmevent26    0x33a
#define CSR_mhpmevent27    0x33b
#define CSR_mhpmevent28    0x33c
#define CSR_mhpmevent29    0x33d
#define CSR_mhpmevent30    0x33e
#define CSR_mhpmevent31    0x33f
#define CSR_mibase         0x382
#define CSR_mibound        0x383
#define CSR_mideleg        0x303
#define CSR_mie            0x304
#define CSR_mimpid         0xf13
#define CSR_minstret       0xb02
#define CSR_minstreth      0xb82
#define CSR_mip            0x344
#define CSR_misa           0x301
#define CSR_mscratch       0x340
#define CSR_mstatus        0x300
#define CSR_mstatush       0x310
#define CSR_mtinst         0x34a
#define CSR_mtval          0x343
#define CSR_mtval2         0x34b
#define CSR_mtvec          0x305
#define CSR_mvendorid      0xf11
#define CSR_pmpaddr0       0x3b0
#define CSR_pmpaddr1       0x3b1
#define CSR_pmpaddr2       0x3b2
#define CSR_pmpaddr3       0x3b3
#define CSR_pmpaddr4       0x3b4
#define CSR_pmpaddr5       0x3b5
#define CSR_pmpaddr6       0x3b6
#define CSR_pmpaddr7       0x3b7
#define CSR_pmpaddr8       0x3b8
#define CSR_pmpaddr9       0x3b9
#define CSR_pmpaddr10      0x3ba
#define CSR_pmpaddr11      0x3bb
#define CSR_pmpaddr12      0x3bc
#define CSR_pmpaddr13      0x3bd
#define CSR_pmpaddr14      0x3be
#define CSR_pmpaddr15      0x3bf
#define CSR_pmpaddr16      0x3c0
#define CSR_pmpaddr17      0x3c1
#define CSR_pmpaddr18      0x3c2
#define CSR_pmpaddr19      0x3c3
#define CSR_pmpaddr20      0x3c4
#define CSR_pmpaddr21      0x3c5
#define CSR_pmpaddr22      0x3c6
#define CSR_pmpaddr23      0x3c7
#define CSR_pmpaddr24      0x3c8
#define CSR_pmpaddr25      0x3c9
#define CSR_pmpaddr26      0x3ca
#define CSR_pmpaddr27      0x3cb
#define CSR_pmpaddr28      0x3cc
#define CSR_pmpaddr29      0x3cd
#define CSR_pmpaddr30      0x3ce
#define CSR_pmpaddr31      0x3cf
#define CSR_pmpaddr32      0x3d0
#define CSR_pmpaddr33      0x3d1
#define CSR_pmpaddr34      0x3d2
#define CSR_pmpaddr35      0x3d3
#define CSR_pmpaddr36      0x3d4
#define CSR_pmpaddr37      0x3d5
#define CSR_pmpaddr38      0x3d6
#define CSR_pmpaddr39      0x3d7
#define CSR_pmpaddr40      0x3d8
#define CSR_pmpaddr41      0x3d9
#define CSR_pmpaddr42      0x3da
#define CSR_pmpaddr43      0x3db
#define CSR_pmpaddr44      0x3dc
#define CSR_pmpaddr45      0x3dd
#define CSR_pmpaddr46      0x3de
#define CSR_pmpaddr47      0x3df
#define CSR_pmpaddr48      0x3e0
#define CSR_pmpaddr49      0x3e1
#define CSR_pmpaddr50      0x3e2
#define CSR_pmpaddr51      0x3e3
#define CSR_pmpaddr52      0x3e4
#define CSR_pmpaddr53      0x3e5
#define CSR_pmpaddr54      0x3e6
#define CSR_pmpaddr55      0x3e7
#define CSR_pmpaddr56      0x3e8
#define CSR_pmpaddr57      0x3e9
#define CSR_pmpaddr58      0x3ea
#define CSR_pmpaddr59      0x3eb
#define CSR_pmpaddr60      0x3ec
#define CSR_pmpaddr61      0x3ed
#define CSR_pmpaddr62      0x3ee
#define CSR_pmpaddr63      0x3ef
#define CSR_pmpcfg0        0x3a0
#define CSR_pmpcfg1        0x3a1
#define CSR_pmpcfg2        0x3a2
#define CSR_pmpcfg3        0x3a3
#define CSR_pmpcfg4        0x3a4
#define CSR_pmpcfg5        0x3a5
#define CSR_pmpcfg6        0x3a6
#define CSR_pmpcfg7        0x3a7
#define CSR_pmpcfg8        0x3a8
#define CSR_pmpcfg9        0x3a9
#define CSR_pmpcfg10       0x3aa
#define CSR_pmpcfg11       0x3ab
#define CSR_pmpcfg12       0x3ac
#define CSR_pmpcfg13       0x3ad
#define CSR_pmpcfg14       0x3ae
#define CSR_pmpcfg15       0x3af
#define CSR_satp           0x180
#define CSR_scause         0x142
#define CSR_scounteren     0x106
#define CSR_sedeleg        0x102
#define CSR_sepc           0x141
#define CSR_sideleg        0x103
#define CSR_sie            0x104
#define CSR_sip            0x144
#define CSR_sscratch       0x140
#define CSR_sstatus        0x100
#define CSR_stval          0x143
#define CSR_stvec          0x105
#define CSR_tdata1         0x7a1
#define CSR_tdata2         0x7a2
#define CSR_tdata3         0x7a3
#define CSR_time           0xc01
#define CSR_timeh          0xc81
#define CSR_tselect        0x7a0
#define CSR_ucause         0x042
#define CSR_uepc           0x041
#define CSR_uie            0x004
#define CSR_uip            0x044
#define CSR_uscratch       0x040
#define CSR_ustatus        0x000
#define CSR_utval          0x043
#define CSR_utvec          0x005
#define CSR_vsatp          0x280
#define CSR_vscause        0x242
#define CSR_vsepc          0x241
#define CSR_vsie           0x204
#define CSR_vsip           0x244
#define CSR_vsscratch      0x240
#define CSR_vsstatus       0x200
#define CSR_vstval         0x243
#define CSR_vstvec         0x205

static const char* CSRName(uint32_t immediate)
{
    switch (immediate) {
        case CSR_cycle: return "cycle";
        case CSR_cycleh: return "cycleh";
        case CSR_dcsr: return "dcsr";
        case CSR_dpc: return "dpc";
        case CSR_dscratch0: return "dscratch0";
        case CSR_dscratch1: return "dscratch1";
        case CSR_fcsr: return "fcsr";
        case CSR_fflags: return "fflags";
        case CSR_frm: return "frm";
        case CSR_hcounteren: return "hcounteren";
        case CSR_hedeleg: return "hedeleg";
        case CSR_hgatp: return "hgatp";
        case CSR_hgeie: return "hgeie";
        case CSR_hgeip: return "hgeip";
        case CSR_hideleg: return "hideleg";
        case CSR_hie: return "hie";
        case CSR_hip: return "hip";
        case CSR_hpmcounter3: return "hpmcounter3";
        case CSR_hpmcounter4: return "hpmcounter4";
        case CSR_hpmcounter5: return "hpmcounter5";
        case CSR_hpmcounter6: return "hpmcounter6";
        case CSR_hpmcounter7: return "hpmcounter7";
        case CSR_hpmcounter8: return "hpmcounter8";
        case CSR_hpmcounter9: return "hpmcounter9";
        case CSR_hpmcounter10: return "hpmcounter10";
        case CSR_hpmcounter11: return "hpmcounter11";
        case CSR_hpmcounter12: return "hpmcounter12";
        case CSR_hpmcounter13: return "hpmcounter13";
        case CSR_hpmcounter14: return "hpmcounter14";
        case CSR_hpmcounter15: return "hpmcounter15";
        case CSR_hpmcounter16: return "hpmcounter16";
        case CSR_hpmcounter17: return "hpmcounter17";
        case CSR_hpmcounter18: return "hpmcounter18";
        case CSR_hpmcounter19: return "hpmcounter19";
        case CSR_hpmcounter20: return "hpmcounter20";
        case CSR_hpmcounter21: return "hpmcounter21";
        case CSR_hpmcounter22: return "hpmcounter22";
        case CSR_hpmcounter23: return "hpmcounter23";
        case CSR_hpmcounter24: return "hpmcounter24";
        case CSR_hpmcounter25: return "hpmcounter25";
        case CSR_hpmcounter26: return "hpmcounter26";
        case CSR_hpmcounter27: return "hpmcounter27";
        case CSR_hpmcounter28: return "hpmcounter28";
        case CSR_hpmcounter29: return "hpmcounter29";
        case CSR_hpmcounter30: return "hpmcounter30";
        case CSR_hpmcounter31: return "hpmcounter31";
        case CSR_hpmcounter3h: return "hpmcounter3h";
        case CSR_hpmcounter4h: return "hpmcounter4h";
        case CSR_hpmcounter5h: return "hpmcounter5h";
        case CSR_hpmcounter6h: return "hpmcounter6h";
        case CSR_hpmcounter7h: return "hpmcounter7h";
        case CSR_hpmcounter8h: return "hpmcounter8h";
        case CSR_hpmcounter9h: return "hpmcounter9h";
        case CSR_hpmcounter10h: return "hpmcounter10h";
        case CSR_hpmcounter11h: return "hpmcounter11h";
        case CSR_hpmcounter12h: return "hpmcounter12h";
        case CSR_hpmcounter13h: return "hpmcounter13h";
        case CSR_hpmcounter14h: return "hpmcounter14h";
        case CSR_hpmcounter15h: return "hpmcounter15h";
        case CSR_hpmcounter16h: return "hpmcounter16h";
        case CSR_hpmcounter17h: return "hpmcounter17h";
        case CSR_hpmcounter18h: return "hpmcounter18h";
        case CSR_hpmcounter19h: return "hpmcounter19h";
        case CSR_hpmcounter20h: return "hpmcounter20h";
        case CSR_hpmcounter21h: return "hpmcounter21h";
        case CSR_hpmcounter22h: return "hpmcounter22h";
        case CSR_hpmcounter23h: return "hpmcounter23h";
        case CSR_hpmcounter24h: return "hpmcounter24h";
        case CSR_hpmcounter25h: return "hpmcounter25h";
        case CSR_hpmcounter26h: return "hpmcounter26h";
        case CSR_hpmcounter27h: return "hpmcounter27h";
        case CSR_hpmcounter28h: return "hpmcounter28h";
        case CSR_hpmcounter29h: return "hpmcounter29h";
        case CSR_hpmcounter30h: return "hpmcounter30h";
        case CSR_hpmcounter31h: return "hpmcounter31h";
        case CSR_hstatus: return "hstatus";
        case CSR_htimedelta: return "htimedelta";
        case CSR_htimedeltah: return "htimedeltah";
        case CSR_htinst: return "htinst";
        case CSR_htval: return "htval";
        case CSR_instret: return "instret";
        case CSR_instreth: return "instreth";
        case CSR_marchid: return "marchid";
        case CSR_mbase: return "mbase";
        case CSR_mbound: return "mbound";
        case CSR_mcause: return "mcause";
        case CSR_mcounteren: return "mcounteren";
        case CSR_mcountinhibit: return "mcountinhibit";
        case CSR_mcycle: return "mcycle";
        case CSR_mcycleh: return "mcycleh";
        case CSR_mdbase: return "mdbase";
        case CSR_mdbound: return "mdbound";
        case CSR_medeleg: return "medeleg";
        case CSR_mepc: return "mepc";
        case CSR_mhartid: return "mhartid";
        case CSR_mhpmcounter3: return "mhpmcounter3";
        case CSR_mhpmcounter4: return "mhpmcounter4";
        case CSR_mhpmcounter5: return "mhpmcounter5";
        case CSR_mhpmcounter6: return "mhpmcounter6";
        case CSR_mhpmcounter7: return "mhpmcounter7";
        case CSR_mhpmcounter8: return "mhpmcounter8";
        case CSR_mhpmcounter9: return "mhpmcounter9";
        case CSR_mhpmcounter10: return "mhpmcounter10";
        case CSR_mhpmcounter11: return "mhpmcounter11";
        case CSR_mhpmcounter12: return "mhpmcounter12";
        case CSR_mhpmcounter13: return "mhpmcounter13";
        case CSR_mhpmcounter14: return "mhpmcounter14";
        case CSR_mhpmcounter15: return "mhpmcounter15";
        case CSR_mhpmcounter16: return "mhpmcounter16";
        case CSR_mhpmcounter17: return "mhpmcounter17";
        case CSR_mhpmcounter18: return "mhpmcounter18";
        case CSR_mhpmcounter19: return "mhpmcounter19";
        case CSR_mhpmcounter20: return "mhpmcounter20";
        case CSR_mhpmcounter21: return "mhpmcounter21";
        case CSR_mhpmcounter22: return "mhpmcounter22";
        case CSR_mhpmcounter23: return "mhpmcounter23";
        case CSR_mhpmcounter24: return "mhpmcounter24";
        case CSR_mhpmcounter25: return "mhpmcounter25";
        case CSR_mhpmcounter26: return "mhpmcounter26";
        case CSR_mhpmcounter27: return "mhpmcounter27";
        case CSR_mhpmcounter28: return "mhpmcounter28";
        case CSR_mhpmcounter29: return "mhpmcounter29";
        case CSR_mhpmcounter30: return "mhpmcounter30";
        case CSR_mhpmcounter31: return "mhpmcounter31";
        case CSR_mhpmcounter3h: return "mhpmcounter3h";
        case CSR_mhpmcounter4h: return "mhpmcounter4h";
        case CSR_mhpmcounter5h: return "mhpmcounter5h";
        case CSR_mhpmcounter6h: return "mhpmcounter6h";
        case CSR_mhpmcounter7h: return "mhpmcounter7h";
        case CSR_mhpmcounter8h: return "mhpmcounter8h";
        case CSR_mhpmcounter9h: return "mhpmcounter9h";
        case CSR_mhpmcounter10h: return "mhpmcounter10h";
        case CSR_mhpmcounter11h: return "mhpmcounter11h";
        case CSR_mhpmcounter12h: return "mhpmcounter12h";
        case CSR_mhpmcounter13h: return "mhpmcounter13h";
        case CSR_mhpmcounter14h: return "mhpmcounter14h";
        case CSR_mhpmcounter15h: return "mhpmcounter15h";
        case CSR_mhpmcounter16h: return "mhpmcounter16h";
        case CSR_mhpmcounter17h: return "mhpmcounter17h";
        case CSR_mhpmcounter18h: return "mhpmcounter18h";
        case CSR_mhpmcounter19h: return "mhpmcounter19h";
        case CSR_mhpmcounter20h: return "mhpmcounter20h";
        case CSR_mhpmcounter21h: return "mhpmcounter21h";
        case CSR_mhpmcounter22h: return "mhpmcounter22h";
        case CSR_mhpmcounter23h: return "mhpmcounter23h";
        case CSR_mhpmcounter24h: return "mhpmcounter24h";
        case CSR_mhpmcounter25h: return "mhpmcounter25h";
        case CSR_mhpmcounter26h: return "mhpmcounter26h";
        case CSR_mhpmcounter27h: return "mhpmcounter27h";
        case CSR_mhpmcounter28h: return "mhpmcounter28h";
        case CSR_mhpmcounter29h: return "mhpmcounter29h";
        case CSR_mhpmcounter30h: return "mhpmcounter30h";
        case CSR_mhpmcounter31h: return "mhpmcounter31h";
        case CSR_mhpmevent3: return "mhpmevent3";
        case CSR_mhpmevent4: return "mhpmevent4";
        case CSR_mhpmevent5: return "mhpmevent5";
        case CSR_mhpmevent6: return "mhpmevent6";
        case CSR_mhpmevent7: return "mhpmevent7";
        case CSR_mhpmevent8: return "mhpmevent8";
        case CSR_mhpmevent9: return "mhpmevent9";
        case CSR_mhpmevent10: return "mhpmevent10";
        case CSR_mhpmevent11: return "mhpmevent11";
        case CSR_mhpmevent12: return "mhpmevent12";
        case CSR_mhpmevent13: return "mhpmevent13";
        case CSR_mhpmevent14: return "mhpmevent14";
        case CSR_mhpmevent15: return "mhpmevent15";
        case CSR_mhpmevent16: return "mhpmevent16";
        case CSR_mhpmevent17: return "mhpmevent17";
        case CSR_mhpmevent18: return "mhpmevent18";
        case CSR_mhpmevent19: return "mhpmevent19";
        case CSR_mhpmevent20: return "mhpmevent20";
        case CSR_mhpmevent21: return "mhpmevent21";
        case CSR_mhpmevent22: return "mhpmevent22";
        case CSR_mhpmevent23: return "mhpmevent23";
        case CSR_mhpmevent24: return "mhpmevent24";
        case CSR_mhpmevent25: return "mhpmevent25";
        case CSR_mhpmevent26: return "mhpmevent26";
        case CSR_mhpmevent27: return "mhpmevent27";
        case CSR_mhpmevent28: return "mhpmevent28";
        case CSR_mhpmevent29: return "mhpmevent29";
        case CSR_mhpmevent30: return "mhpmevent30";
        case CSR_mhpmevent31: return "mhpmevent31";
        case CSR_mibase: return "mibase";
        case CSR_mibound: return "mibound";
        case CSR_mideleg: return "mideleg";
        case CSR_mie: return "mie";
        case CSR_mimpid: return "mimpid";
        case CSR_minstret: return "minstret";
        case CSR_minstreth: return "minstreth";
        case CSR_mip: return "mip";
        case CSR_misa: return "misa";
        case CSR_mscratch: return "mscratch";
        case CSR_mstatus: return "mstatus";
        case CSR_mstatush: return "mstatush";
        case CSR_mtinst: return "mtinst";
        case CSR_mtval: return "mtval";
        case CSR_mtval2: return "mtval2";
        case CSR_mtvec: return "mtvec";
        case CSR_mvendorid: return "mvendorid";
        case CSR_pmpaddr0: return "pmpaddr0";
        case CSR_pmpaddr1: return "pmpaddr1";
        case CSR_pmpaddr2: return "pmpaddr2";
        case CSR_pmpaddr3: return "pmpaddr3";
        case CSR_pmpaddr4: return "pmpaddr4";
        case CSR_pmpaddr5: return "pmpaddr5";
        case CSR_pmpaddr6: return "pmpaddr6";
        case CSR_pmpaddr7: return "pmpaddr7";
        case CSR_pmpaddr8: return "pmpaddr8";
        case CSR_pmpaddr9: return "pmpaddr9";
        case CSR_pmpaddr10: return "pmpaddr10";
        case CSR_pmpaddr11: return "pmpaddr11";
        case CSR_pmpaddr12: return "pmpaddr12";
        case CSR_pmpaddr13: return "pmpaddr13";
        case CSR_pmpaddr14: return "pmpaddr14";
        case CSR_pmpaddr15: return "pmpaddr15";
        case CSR_pmpaddr16: return "pmpaddr16";
        case CSR_pmpaddr17: return "pmpaddr17";
        case CSR_pmpaddr18: return "pmpaddr18";
        case CSR_pmpaddr19: return "pmpaddr19";
        case CSR_pmpaddr20: return "pmpaddr20";
        case CSR_pmpaddr21: return "pmpaddr21";
        case CSR_pmpaddr22: return "pmpaddr22";
        case CSR_pmpaddr23: return "pmpaddr23";
        case CSR_pmpaddr24: return "pmpaddr24";
        case CSR_pmpaddr25: return "pmpaddr25";
        case CSR_pmpaddr26: return "pmpaddr26";
        case CSR_pmpaddr27: return "pmpaddr27";
        case CSR_pmpaddr28: return "pmpaddr28";
        case CSR_pmpaddr29: return "pmpaddr29";
        case CSR_pmpaddr30: return "pmpaddr30";
        case CSR_pmpaddr31: return "pmpaddr31";
        case CSR_pmpaddr32: return "pmpaddr32";
        case CSR_pmpaddr33: return "pmpaddr33";
        case CSR_pmpaddr34: return "pmpaddr34";
        case CSR_pmpaddr35: return "pmpaddr35";
        case CSR_pmpaddr36: return "pmpaddr36";
        case CSR_pmpaddr37: return "pmpaddr37";
        case CSR_pmpaddr38: return "pmpaddr38";
        case CSR_pmpaddr39: return "pmpaddr39";
        case CSR_pmpaddr40: return "pmpaddr40";
        case CSR_pmpaddr41: return "pmpaddr41";
        case CSR_pmpaddr42: return "pmpaddr42";
        case CSR_pmpaddr43: return "pmpaddr43";
        case CSR_pmpaddr44: return "pmpaddr44";
        case CSR_pmpaddr45: return "pmpaddr45";
        case CSR_pmpaddr46: return "pmpaddr46";
        case CSR_pmpaddr47: return "pmpaddr47";
        case CSR_pmpaddr48: return "pmpaddr48";
        case CSR_pmpaddr49: return "pmpaddr49";
        case CSR_pmpaddr50: return "pmpaddr50";
        case CSR_pmpaddr51: return "pmpaddr51";
        case CSR_pmpaddr52: return "pmpaddr52";
        case CSR_pmpaddr53: return "pmpaddr53";
        case CSR_pmpaddr54: return "pmpaddr54";
        case CSR_pmpaddr55: return "pmpaddr55";
        case CSR_pmpaddr56: return "pmpaddr56";
        case CSR_pmpaddr57: return "pmpaddr57";
        case CSR_pmpaddr58: return "pmpaddr58";
        case CSR_pmpaddr59: return "pmpaddr59";
        case CSR_pmpaddr60: return "pmpaddr60";
        case CSR_pmpaddr61: return "pmpaddr61";
        case CSR_pmpaddr62: return "pmpaddr62";
        case CSR_pmpaddr63: return "pmpaddr63";
        case CSR_pmpcfg0: return "pmpcfg0";
        case CSR_pmpcfg1: return "pmpcfg1";
        case CSR_pmpcfg2: return "pmpcfg2";
        case CSR_pmpcfg3: return "pmpcfg3";
        case CSR_pmpcfg4: return "pmpcfg4";
        case CSR_pmpcfg5: return "pmpcfg5";
        case CSR_pmpcfg6: return "pmpcfg6";
        case CSR_pmpcfg7: return "pmpcfg7";
        case CSR_pmpcfg8: return "pmpcfg8";
        case CSR_pmpcfg9: return "pmpcfg9";
        case CSR_pmpcfg10: return "pmpcfg10";
        case CSR_pmpcfg11: return "pmpcfg11";
        case CSR_pmpcfg12: return "pmpcfg12";
        case CSR_pmpcfg13: return "pmpcfg13";
        case CSR_pmpcfg14: return "pmpcfg14";
        case CSR_pmpcfg15: return "pmpcfg15";
        case CSR_satp: return "satp";
        case CSR_scause: return "scause";
        case CSR_scounteren: return "scounteren";
        case CSR_sedeleg: return "sedeleg";
        case CSR_sepc: return "sepc";
        case CSR_sideleg: return "sideleg";
        case CSR_sie: return "sie";
        case CSR_sip: return "sip";
        case CSR_sscratch: return "sscratch";
        case CSR_sstatus: return "sstatus";
        case CSR_stval: return "stval";
        case CSR_stvec: return "stvec";
        case CSR_tdata1: return "tdata1";
        case CSR_tdata2: return "tdata2";
        case CSR_tdata3: return "tdata3";
        case CSR_time: return "time";
        case CSR_timeh: return "timeh";
        case CSR_tselect: return "tselect";
        case CSR_ucause: return "ucause";
        case CSR_uepc: return "uepc";
        case CSR_uie: return "uie";
        case CSR_uip: return "uip";
        case CSR_uscratch: return "uscratch";
        case CSR_ustatus: return "ustatus";
        case CSR_utval: return "utval";
        case CSR_utvec: return "utvec";
        case CSR_vsatp: return "vsatp";
        case CSR_vscause: return "vscause";
        case CSR_vsepc: return "vsepc";
        case CSR_vsie: return "vsie";
        case CSR_vsip: return "vsip";
        case CSR_vsscratch: return "vsscratch";
        case CSR_vsstatus: return "vsstatus";
        case CSR_vstval: return "vstval";
        case CSR_vstvec: return "vstvec";
        default: return "";
    }
}

void FormatInstruction(RawInstruction ins, char* buffer, size_t buffsz)
{
    InstructionType type = DecodeInstruction(ins);
    switch (type) {
        default:
            snprintf(buffer, buffsz, "unimp");
            break;
        case InstructionType::ILLEGAL:
        case InstructionType::MRET:
        case InstructionType::FENCE:
        case InstructionType::FENCE_I: // TODO: Format fence correctly
        case InstructionType::ECALL:
        case InstructionType::EBREAK:
            snprintf(buffer, buffsz, "%s", InstructionName(type));
            break;
        // I-type
        case InstructionType::ADDI:
        case InstructionType::SLTI:
        case InstructionType::SLTIU:
        case InstructionType::ANDI:
        case InstructionType::ORI:
        case InstructionType::XORI:
        case InstructionType::JALR:
            snprintf(buffer, buffsz, "%s x%d, x%d, %d", InstructionName(type), ins.Ityp.rd, ins.Ityp.rs1, SignExtend(ins.Ityp.imm11_0, 12));
            break;
        // I-type (shift)
        case InstructionType::SLLI:
        case InstructionType::SRLI:
        case InstructionType::SRAI:
            snprintf(buffer, buffsz, "%s x%d, x%d, %d", InstructionName(type), ins.Ityp.rd, ins.Ityp.rs1, ins.Ityp.imm11_0 & 0b11111);
            break;
        // U-type
        case InstructionType::LUI:
        case InstructionType::AUIPC:
            snprintf(buffer, buffsz, "%s x%d, %d", InstructionName(type), ins.Utyp.rd, SignExtend(ins.Utyp.imm31_12, 20));
            break;
        // R-type
        case InstructionType::ADD:
        case InstructionType::SUB:
        case InstructionType::SLT:
        case InstructionType::SLTU:
        case InstructionType::AND:
        case InstructionType::OR:
        case InstructionType::XOR:
        case InstructionType::SLL:
        case InstructionType::SRL:
        case InstructionType::SRA:
            snprintf(buffer, buffsz, "%s x%d, x%d, x%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, ins.Rtyp.rs2);
            break;
        // J-type
        case InstructionType::JAL:
            snprintf(buffer, buffsz, "%s x%d, %d", InstructionName(type), ins.Jtyp.rd, SignExtend(ins.Jtyp.imm(), 21));
            break;
        // B-type
        case InstructionType::BEQ:
        case InstructionType::BNE:
        case InstructionType::BLT:
        case InstructionType::BLTU:
        case InstructionType::BGE:
        case InstructionType::BGEU:
            snprintf(buffer, buffsz, "%s x%d, x%d, %d", InstructionName(type), ins.Btyp.rs1, ins.Btyp.rs2, SignExtend(ins.Btyp.imm(), 13));
            break;
        // I-type (load)
        case InstructionType::LW:
        case InstructionType::LH:
        case InstructionType::LHU:
        case InstructionType::LB:
        case InstructionType::LBU:
            snprintf(buffer, buffsz, "%s x%d, %d(x%d)", InstructionName(type), ins.Ityp.rd, SignExtend(ins.Ityp.imm11_0, 12), ins.Ityp.rs1);
            break;
        // S-type
        case InstructionType::SW:
        case InstructionType::SH:
        case InstructionType::SB:
            snprintf(buffer, buffsz, "%s x%d, %d(x%d)", InstructionName(type), ins.Styp.rs2, SignExtend(ins.Styp.imm(), 12), ins.Ityp.rs1);
            break;
        case InstructionType::CSRRW:
        case InstructionType::CSRRS:
        case InstructionType::CSRRC:
        case InstructionType::CSRRWI:
        case InstructionType::CSRRSI:
        case InstructionType::CSRRCI: {
            const char* csrRepr = CSRName(ins.Ityp.imm11_0);
            if (csrRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s x%d, %s, x%d", InstructionName(type), ins.Ityp.rd, csrRepr, ins.Ityp.rs1);
            }
            else {
                snprintf(buffer, buffsz, "%s x%d, %d, x%d", InstructionName(type), ins.Ityp.rd, ins.Ityp.imm11_0, ins.Ityp.rs1);
            }
        } break;
    }

}

const char* InstructionName(InstructionType type)
{
    const char* names[] = {
        "illegal",
        "mret",
        "lui",
        "auipc",
        "jal",
        "jalr",
        "beq",
        "bne",
        "blt",
        "bge",
        "bltu",
        "bgeu",
        "lb",
        "lh",
        "lw",
        "lbu",
        "lhu",
        "sb",
        "sh",
        "sw",
        "addi",
        "slti",
        "sltiu",
        "xori",
        "ori",
        "andi",
        "slli",
        "srli",
        "srai",
        "add",
        "sub",
        "sll",
        "slt",
        "sltu",
        "xor",
        "srl",
        "sra",
        "or",
        "and",
        "fence",
        "ecall",
        "ebreak",
        "fence.i",
        "csrrw",
        "csrrs",
        "csrrc",
        "csrrwi",
        "csrrsi",
        "csrrci",
        "mul",
        "mulh",
        "mulhsu",
        "mulhu",
        "div",
        "divu",
        "rem",
        "remu",
        "flw",
        "fsw",
        "fmadd.s",
        "fmsub.s",
        "fnmsub.s",
        "fnmadd.s",
        "fadd.s",
        "fsub.s",
        "fmul.s",
        "fdiv.s",
        "fsqrt.s",
        "fsgnj.s",
        "fsgnjn.s",
        "fsgnjx.s",
        "fmin.s",
        "fmax.s",
        "fcvt.w.s",
        "fcvt.wu.s",
        "fmv.x.w",
        "feq.s",
        "flt.s",
        "fle.s",
        "fclass.s",
        "fcvt.s.w",
        "fcvt.s.wu",
        "fmv.w.x",
    };
    static_assert(static_cast<uint32_t>(InstructionType::COUNT) == sizeof(names) / sizeof(names[0]), "Exhaustive check of Instruction types failed");
    return names[static_cast<uint32_t>(type)];
}

static InstructionType DecodeQ0(RawInstruction instruction)
{
    fprintf(stderr, "DecodeQ0: Instruction 0x%08X unimplemented\n", instruction.value);
    (void) instruction;
    return InstructionType::ILLEGAL;
}

static InstructionType DecodeQ1(RawInstruction instruction)
{
    fprintf(stderr, "DecodeQ1: Instruction 0x%08X unimplemented\n", instruction.value);
    (void) instruction;
    return InstructionType::ILLEGAL;
}

static InstructionType DecodeQ2(RawInstruction instruction)
{
    fprintf(stderr, "DecodeQ2: Instruction 0x%08X unimplemented\n", instruction.value);
    (void) instruction;
    return InstructionType::ILLEGAL;
}

static InstructionType DecodeQ3(RawInstruction instruction)
{
    switch (instruction.value) {
        case 0b00110000001000000000000001110011: return InstructionType::MRET;
    }

    switch (instruction.Rtyp.opcode) {
        case 0b0110111: return InstructionType::LUI;
        case 0b0010111: return InstructionType::AUIPC;
        case 0b1101111: return InstructionType::JAL;
        case 0b1100111: return InstructionType::JALR;
        case 0b1100011: {
            switch (instruction.Btyp.funct3) {
                case 0b000: return InstructionType::BEQ;
                case 0b001: return InstructionType::BNE;
                case 0b100: return InstructionType::BLT;
                case 0b101: return InstructionType::BGE;
                case 0b110: return InstructionType::BLTU;
                case 0b111: return InstructionType::BGEU;
            }
        } break;
        case 0b0000011: {
            switch (instruction.Ityp.funct3) {
                case 0b000: return InstructionType::LB;
                case 0b001: return InstructionType::LH;
                case 0b010: return InstructionType::LW;
                case 0b100: return InstructionType::LBU;
                case 0b101: return InstructionType::LHU;
            }
        } break;
        case 0b0100011: {
            switch (instruction.Styp.funct3) {
                case 0b000: return InstructionType::SB;
                case 0b001: return InstructionType::SH;
                case 0b010: return InstructionType::SW;
            }
        } break;
        case 0b0010011: {
            switch (instruction.Ityp.funct3) {
                case 0b000: return InstructionType::ADDI;
                case 0b010: return InstructionType::SLTI;
                case 0b011: return InstructionType::SLTIU;
                case 0b100: return InstructionType::XORI;
                case 0b110: return InstructionType::ORI;
                case 0b111: return InstructionType::ANDI;
                case 0b001: return InstructionType::SLLI;
                case 0b101: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::SRLI;
                        case 0b0100000: return InstructionType::SRAI;
                    }
                } break;
            }
        } break;
        case 0b0110011: {
            switch (instruction.Rtyp.funct3) {
                case 0b000: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::ADD;
                        case 0b0100000: return InstructionType::SUB;
                        case 0b0000001: return InstructionType::MUL;
                    }
                } break;
                case 0b001: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::SLL;
                        case 0b0000001: return InstructionType::MULH;
                    }
                } break;
                case 0b010: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::SLT;
                        case 0b0000001: return InstructionType::MULHSU;
                    }
                } break;
                case 0b011: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::SLTU;
                        case 0b0000001: return InstructionType::MULHU;
                    }
                } break;
                case 0b100: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::XOR;
                        case 0b0000001: return InstructionType::DIV;
                    }
                } break;
                case 0b101: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::SRL;
                        case 0b0100000: return InstructionType::SRA;
                        case 0b0000001: return InstructionType::DIVU;
                    }
                } break;
                case 0b110: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::OR;
                        case 0b0000001: return InstructionType::REM;
                    }
                } break;
                case 0b111: {
                    switch (instruction.Rtyp.funct7) {
                        case 0b0000000: return InstructionType::AND;
                        case 0b0000001: return InstructionType::REMU;
                    }
                } break;
            }
        } break;
        case 0b0001111: {
            return InstructionType::FENCE;
        } break;
        case 0b1110011: {
            switch (instruction.Ityp.imm11_0) {
                case 0b000000000000: return InstructionType::ECALL;
                case 0b000000000001: return InstructionType::EBREAK;
                default: {
                    switch (instruction.Rtyp.funct3) {
                        case 0b001: return InstructionType::CSRRW;
                        case 0b010: return InstructionType::CSRRS;
                        case 0b011: return InstructionType::CSRRC;
                        case 0b101: return InstructionType::CSRRWI;
                        case 0b110: return InstructionType::CSRRSI;
                        case 0b111: return InstructionType::CSRRCI;
                    }
                } break;
            }
        } break;
        case 0b0000111: return InstructionType::FLW;
        case 0b0100111: return InstructionType::FSW;
        case 0b1000011: return InstructionType::FMADDS;
        case 0b1000111: return InstructionType::FMSUBS;
        case 0b1001011: return InstructionType::FNMSUBS;
        case 0b1001111: return InstructionType::FNMADDS;
        case 0b1010011: {
            switch (instruction.Rtyp.funct7) {
                case 0b0000000: return InstructionType::FADDS;
                case 0b0000100: return InstructionType::FSUBS;
                case 0b0001000: return InstructionType::FMULS;
                case 0b0001100: return InstructionType::FDIVS;
                case 0b0101100: return InstructionType::FSQRTS;
                case 0b0010000: {
                    switch (instruction.Rtyp.funct3) {
                        case 0b000: return InstructionType::FSGNJS;
                        case 0b001: return InstructionType::FSGNJNS;
                        case 0b010: return InstructionType::FSGNJXS;
                    }
                } break;
                case 0b0010100: {
                    switch (instruction.Rtyp.funct3) {
                        case 0b000: return InstructionType::FMINS;
                        case 0b001: return InstructionType::FMAXS;
                    }
                } break;
                case 0b1100000: {
                    switch (instruction.Rtyp.rs2) {
                        case 0b00000: return InstructionType::FCVTWS;
                        case 0b00001: return InstructionType::FCVTWUS;
                    }
                } break;
                case 0b1110000: {
                    switch (instruction.Rtyp.funct3) {
                        case 0b000: return InstructionType::FMVXW;
                        case 0b001: return InstructionType::FCLASSS;
                    }
                } break;
                case 0b1010000: {
                    switch (instruction.Rtyp.funct3) {
                        case 0b010: return InstructionType::FEQS;
                        case 0b001: return InstructionType::FLTS;
                        case 0b000: return InstructionType::FLES;
                    }
                } break;
                case 0b1101000: {
                    switch (instruction.Rtyp.rs2) {
                        case 0b00000: return InstructionType::FCVTSW;
                        case 0b00001: return InstructionType::FCVTSWU;
                    }
                } break;
                case 0b1111000: return InstructionType::FMVWX;
            }
        } break;
    }
    return InstructionType::ILLEGAL;
}

InstructionType DecodeInstruction(RawInstruction instruction)
{
    switch (instruction.value & 0b11) {
        case 0b00: return InstructionType::ILLEGAL; // return DecodeQ0(instruction);
        case 0b01: return InstructionType::ILLEGAL; // return DecodeQ1(instruction);
        case 0b10: return InstructionType::ILLEGAL; // return DecodeQ2(instruction);
        case 0b11: return DecodeQ3(instruction);
    }
    return InstructionType::ILLEGAL;
}

void CPU::Reset()
{
    pc = 0;
    memset(&intRegs, 0, sizeof(intRegs));
    memset(&fltRegs, 0, sizeof(fltRegs));
    memset(&csr, 0, sizeof(csr));
    memset(memory.buffer, 0, memory.MemSize);
    memset(memory.didChange, false, memory.MemSize);
}

bool CPU::InitializeFromELF(uint8_t* data, size_t size)
{
    // ELF Header
    assert(sizeof(Elf32_Ehdr) < size);
    Elf32_Ehdr header;
    memcpy(&header, data, sizeof(header));

    if (header.e_ident[EI_MAG0] != ELFMAG0 ||
        header.e_ident[EI_MAG1] != ELFMAG1 ||
        header.e_ident[EI_MAG2] != ELFMAG2 ||
        header.e_ident[EI_MAG3] != ELFMAG3)
    {
        fprintf(stderr, "Invalid ELF file\n");
        return false;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "ELF file is not 32-bit\n");
        return false;
    }

    if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "ELF file is not little-endian\n");
        return false;
    }

    // Rest of e_ident: version, OSABI, padding...

    if (header.e_type != ET_EXEC) {
        fprintf(stderr, "ELF file is not executable\n");
        return false;
    }

    if (header.e_machine != EM_RISCV) {
        fprintf(stderr, "ELF file targets wrong machine, expected RISC-V\n");
        return false;
    }

    if (header.e_version != EV_CURRENT) {
        fprintf(stderr, "ELF file has wrong version, expected 1\n");
        return false;
    }

    if (header.e_entry == 0) {
        fprintf(stderr, "ELF file does not specify entry point\n");
        return false;
    }
    pc = header.e_entry & ~0x80000000;

    assert(header.e_phoff == sizeof(Elf32_Ehdr));
    size_t programHeaderOffset = header.e_phoff;
    size_t sectionHeaderOffset = header.e_shoff;

    assert(header.e_ehsize == sizeof(Elf32_Ehdr));
    assert(header.e_phentsize == sizeof(Elf32_Phdr));
    assert(header.e_shentsize == sizeof(Elf32_Shdr));

    size_t numProgramHeaders = header.e_phnum;
    size_t numSectionHeaders = header.e_shnum;

    size_t sectionHeadersStartIdx = header.e_shstrndx;


    // Program Headers
    size_t programHeadersSize = numProgramHeaders * sizeof(Elf32_Phdr);
    Elf32_Phdr* programHeaders = (Elf32_Phdr*) malloc(programHeadersSize);
    assert(programHeaders != NULL && "malloc failed - buy more RAM");

    assert(programHeaderOffset + programHeadersSize <= size);
    memcpy(programHeaders, data + programHeaderOffset, programHeadersSize);
    for (size_t i = 0; i < numProgramHeaders; ++i) {
        Elf32_Phdr pHeader = programHeaders[i];
        if (pHeader.p_type == PT_LOAD) {
            assert(pHeader.p_paddr == pHeader.p_vaddr); // Not always true, but simpler
            assert(pHeader.p_filesz == pHeader.p_memsz);
            pHeader.p_paddr &= ~0x80000000;
            // TODO: Respect flags
            memcpy(memory.buffer + pHeader.p_paddr, data + pHeader.p_offset, pHeader.p_filesz);
        }
    }

    free(programHeaders);

#if 0 // These are unimportant for execution, only for linking
    // Section Headers
    size_t sectionHeadersSize = numSectionHeaders * sizeof(Elf32_Shdr);
    Elf32_Shdr* sectionHeaders = (Elf32_Shdr*) malloc(sectionHeadersSize);
    assert(sectionHeaders != NULL && "malloc failed - buy more RAM");

    assert(sectionHeaderOffset + sectionHeadersSize <= size);
    memcpy(sectionHeaders, data + sectionHeaderOffset, sectionHeadersSize);

    // ...

    free(sectionHeaders);
#endif

    return true;
}


bool CPU::Step()
{
    RawInstruction ins{memory.Read<uint32_t>(pc)};
    InstructionType type = DecodeInstruction(ins);
    if (type == InstructionType::MRET) {
        // TODO: Actually do privilege stuff
        pc = csr.Read(CSR_mepc);
        return true;
    }
    uint32_t oldPc = pc;
    pc += 4;
    switch (type) {
        default: printf("%s unimplemented", InstructionName(type)); exit(1);
        break; case InstructionType::ADDI:  intRegs.Write(ins.Ityp.rd, intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::SLTI:  intRegs.Write(ins.Ityp.rd, intRegs.Read< int32_t>(ins.Ityp.rs1) < SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::SLTIU: intRegs.Write(ins.Ityp.rd, intRegs.Read<uint32_t>(ins.Ityp.rs1) < (uint32_t)SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::ANDI:  intRegs.Write(ins.Ityp.rd, intRegs.Read< int32_t>(ins.Ityp.rs1) & SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::ORI:   intRegs.Write(ins.Ityp.rd, intRegs.Read< int32_t>(ins.Ityp.rs1) | SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::XORI:  intRegs.Write(ins.Ityp.rd, intRegs.Read< int32_t>(ins.Ityp.rs1) ^ SignExtend(ins.Ityp.imm11_0, 12));
        break; case InstructionType::SLLI:  intRegs.Write(ins.Ityp.rd, intRegs.Read<uint32_t>(ins.Ityp.rs1) << (ins.Ityp.imm11_0 & 0b11111));
        break; case InstructionType::SRLI:  intRegs.Write(ins.Ityp.rd, intRegs.Read<uint32_t>(ins.Ityp.rs1) >> (ins.Ityp.imm11_0 & 0b11111));
        break; case InstructionType::SRAI:  intRegs.Write(ins.Ityp.rd, intRegs.Read< int32_t>(ins.Ityp.rs1) >> (ins.Ityp.imm11_0 & 0b11111));
        break; case InstructionType::LUI:   intRegs.Write(ins.Utyp.rd, ins.Utyp.imm31_12 << 12);
        break; case InstructionType::AUIPC: intRegs.Write(ins.Utyp.rd, oldPc + ((uint32_t)ins.Utyp.imm31_12 << 12));
        break; case InstructionType::ADD:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) + intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::SUB:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) - intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::SLT:   intRegs.Write(ins.Rtyp.rd, intRegs.Read< int32_t>(ins.Rtyp.rs1) < intRegs.Read< int32_t>(ins.Rtyp.rs2));
        break; case InstructionType::SLTU:  intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) < intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::AND:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) & intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::OR:    intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) | intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::XOR:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) ^ intRegs.Read<uint32_t>(ins.Rtyp.rs2));
        break; case InstructionType::SLL:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) << (intRegs.Read<uint32_t>(ins.Rtyp.rs2) & 0b11111));
        break; case InstructionType::SRL:   intRegs.Write(ins.Rtyp.rd, intRegs.Read<uint32_t>(ins.Rtyp.rs1) >> (intRegs.Read<uint32_t>(ins.Rtyp.rs2) & 0b11111));
        break; case InstructionType::SRA:   intRegs.Write(ins.Rtyp.rd, intRegs.Read< int32_t>(ins.Rtyp.rs1) >> (intRegs.Read<uint32_t>(ins.Rtyp.rs2) & 0b11111));
        break; case InstructionType::JAL: {
            int32_t offset = SignExtend(ins.Jtyp.imm(), 21);
            intRegs.Write(ins.Rtyp.rd, oldPc + 4);
            pc = oldPc + offset;
        }
        break; case InstructionType::JALR: {
            // Should base address be unsigned?
            int32_t offset = SignExtend(ins.Ityp.imm11_0, 12);
            int32_t target = (intRegs.Read<int32_t>(ins.Ityp.rs1) + offset) & ~0b1;
            intRegs.Write(ins.Rtyp.rd, oldPc + 4);
            pc = target;
        }
        break; case InstructionType::BEQ:   if (intRegs.Read<uint32_t>(ins.Btyp.rs1) == intRegs.Read<uint32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::BNE:   if (intRegs.Read<uint32_t>(ins.Btyp.rs1) != intRegs.Read<uint32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::BLT:   if (intRegs.Read< int32_t>(ins.Btyp.rs1) <  intRegs.Read< int32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::BLTU:  if (intRegs.Read<uint32_t>(ins.Btyp.rs1) <  intRegs.Read<uint32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::BGE:   if (intRegs.Read< int32_t>(ins.Btyp.rs1) >= intRegs.Read< int32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::BGEU:  if (intRegs.Read<uint32_t>(ins.Btyp.rs1) >= intRegs.Read<uint32_t>(ins.Btyp.rs2)) pc = pc + SignExtend(ins.Btyp.imm(), 13) - 4;
        break; case InstructionType::LW:    intRegs.Write(ins.Ityp.rd, memory.Read< int32_t>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::LH:    intRegs.Write(ins.Ityp.rd, memory.Read< int16_t>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::LHU:   intRegs.Write(ins.Ityp.rd, memory.Read<uint16_t>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::LB:    intRegs.Write(ins.Ityp.rd, memory.Read<  int8_t>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::LBU:   intRegs.Write(ins.Ityp.rd, memory.Read< uint8_t>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::SW:    memory.Write(intRegs.Read<uint32_t>(ins.Styp.rs1) + SignExtend(ins.Styp.imm(), 12), intRegs.Read<uint32_t>(ins.Styp.rs2));
        break; case InstructionType::SH:    memory.Write(intRegs.Read<uint32_t>(ins.Styp.rs1) + SignExtend(ins.Styp.imm(), 12), intRegs.Read<uint16_t>(ins.Styp.rs2));
        break; case InstructionType::SB:    memory.Write(intRegs.Read<uint32_t>(ins.Styp.rs1) + SignExtend(ins.Styp.imm(), 12), intRegs.Read< uint8_t>(ins.Styp.rs2));
        break; case InstructionType::FENCE: // Do nothing
        break; case InstructionType::FENCE_I:
        break; case InstructionType::ECALL:  return false;
        break; case InstructionType::EBREAK: return false;
        break; case InstructionType::CSRRW: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            uint32_t oldRs1 = intRegs.Read(ins.Ityp.rs1);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, oldRs1);
        }
        break; case InstructionType::CSRRS: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            uint32_t oldRs1 = intRegs.Read(ins.Ityp.rs1);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, oldCsr | oldRs1);
        }
        break; case InstructionType::CSRRC: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            uint32_t oldRs1 = intRegs.Read(ins.Ityp.rs1);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, oldCsr & ~oldRs1);
        }
        break; case InstructionType::CSRRWI: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, ins.Ityp.rs1);
        }
        break; case InstructionType::CSRRSI: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, oldCsr | ins.Ityp.rs1);
        }
        break; case InstructionType::CSRRCI: {
            uint32_t oldCsr = csr.Read(ins.Ityp.imm11_0);
            intRegs.Write(ins.Ityp.rd, oldCsr);
            csr.Write(ins.Ityp.imm11_0, oldCsr & ~ins.Ityp.rs1);
        }
    }
    return true;
}