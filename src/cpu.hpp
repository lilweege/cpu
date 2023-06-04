#pragma once

#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <bit>


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


template<typename BufferType, uint32_t _Size>
struct MemoryBase
{
    constexpr static uint32_t Size = _Size;
#ifndef EMULATOR_STANDALONE
    bool didChange[Size];
#endif
    BufferType buffer[Size];
};


template<uint32_t Size>
struct Memory : public MemoryBase<uint8_t, Size>
{
    template<typename T>
    T Read(uint32_t address) const
    {
        assert(address + sizeof(T) <= this->Size);
        T t;
        memcpy((uint8_t*) &t, this->buffer + address, sizeof(T));
        return t;
    }

    template<typename T>
    void Write(uint32_t address, T value)
    {
        assert(address + sizeof(T) <= this->Size);
        memset(this->didChange + address, 1, sizeof(T));
        memcpy(this->buffer + address, (const uint8_t*) &value, sizeof(T));
    }
};


template<typename BufferType, uint32_t Size>
struct RegisterFile : public MemoryBase<BufferType, Size>
{
    template<typename T=BufferType>
    T Read(uint32_t x) const
    {
        assert(x < this->Size);
        return static_cast<T>(this->buffer[x]);
    }
    
    void Write(uint32_t x, auto value)
    {
        assert(x < this->Size);
        this->didChange[x] = true;
        this->buffer[x] = static_cast<BufferType>(value);
    }
};

using FloatRegisterFile = RegisterFile<float, 32>;
struct IntegerRegisterFile : public RegisterFile<uint32_t, 32>
{
    using Base = RegisterFile<uint32_t, 32>;

    template<typename T=uint32_t>
    T Read(uint32_t x) const { return (x == 0) ? 0 : Base::Read<T>(x); }

    void Write(uint32_t x, auto value) { if (x != 0) Base::Write(x, value); }
};

struct CSRFile : RegisterFile<uint32_t, 4096>
{
    using Base = RegisterFile<uint32_t, 4096>;

    uint32_t Read(uint32_t x) const
    {
        switch (x) {
            default:                return Base::Read(x);
            break; case CSR_frm:    return (Base::Read(CSR_fcsr) >> 5) & 0b111;
            break; case CSR_fflags: return Base::Read(CSR_fcsr) & 0b00011111;
            break; case CSR_fcsr:   return Base::Read(CSR_fcsr) & 0b11111111;
        }
    }

    void Write(uint32_t x, uint32_t value)
    {
        switch (x) {
            default:                Base::Write(x, value);
            break; case CSR_frm:    Base::Write(CSR_fcsr, (Base::Read(CSR_fcsr) & ~(0b111 << 5)) | ((value & 0b111) << 5));
            break; case CSR_fflags: Base::Write(CSR_fcsr, (Base::Read(CSR_fcsr) & ~(0b11111)) | (value & 0b11111));
        }
    }
};


enum class InstructionType : uint32_t
{
    ILLEGAL,

    // Privileged
    MRET,

    // I
    LUI,
    AUIPC,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    FENCE,
    ECALL,
    EBREAK,

    // Zifencei
    FENCE_I,

    // Zicsr
    CSRRW,
    CSRRS,
    CSRRC,
    CSRRWI,
    CSRRSI,
    CSRRCI,

    // M
    MUL,
    MULH,
    MULHSU,
    MULHU,
    DIV,
    DIVU,
    REM,
    REMU,

    // F
    FLW,
    FSW,
    FMADDS,
    FMSUBS,
    FNMSUBS,
    FNMADDS,
    FADDS,
    FSUBS,
    FMULS,
    FDIVS,
    FSQRTS,
    FSGNJS,
    FSGNJNS,
    FSGNJXS,
    FMINS,
    FMAXS,
    FCVTWS,
    FCVTWUS,
    FMVXW,
    FEQS,
    FLTS,
    FLES,
    FCLASSS,
    FCVTSW,
    FCVTSWU,
    FMVWX,

    COUNT,
};


struct RawInstruction
{
    struct R4_Type
    {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t funct2 : 2;
        uint32_t rs3 : 5;
    };

    struct R_Type
    {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t funct7 : 7;
    };

    struct I_Type
    {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t imm11_0 : 12;
    };

    struct S_Type
    {
        uint32_t opcode : 7;
        uint32_t imm4_0 : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t imm11_5 : 7;

        uint32_t imm() const { return (imm11_5 << 5) | imm4_0; }
    };

    struct B_Type
    {
        uint32_t opcode : 7;
        uint32_t imm11 : 1;
        uint32_t imm4_1 : 4;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t imm10_5 : 6;
        uint32_t imm12 : 1;

        uint32_t imm() const { return (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1); }
    };

    struct U_Type
    {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t imm31_12 : 20;
    };

    struct J_Type
    {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t imm19_12 : 8;
        uint32_t imm11 : 1;
        uint32_t imm10_1 : 10;
        uint32_t imm20 : 1;

        uint32_t imm() const { return (imm10_1 << 1) | (imm11 << 11) | (imm19_12 << 12) | (imm20 << 20); }
    };


    union
    {
        uint32_t value;
        R4_Type R4typ;
        R_Type Rtyp;
        I_Type Ityp;
        S_Type Styp;
        B_Type Btyp;
        U_Type Utyp;
        J_Type Jtyp;
    };

    RawInstruction(uint32_t _value) : value{_value} {}
    RawInstruction() = default;

};

static_assert(std::endian::native == std::endian::little, "Please compile on a little endian system :)");
static_assert(sizeof(RawInstruction) == 4);
static_assert(std::is_trivial_v<RawInstruction>);


enum class ParseELFResult : uint32_t
{
    Ok,
    WrongMagic,
    WrongClass,
    WrongData,
    WrongType,
    WrongMachine,
    WrongVersion,
    NoEntry,
};


struct CPU
{
public:
    void Reset();
    ParseELFResult InitializeFromELF(uint8_t* data, size_t size);
    bool Step();
public:
    uint32_t pc;
    IntegerRegisterFile intRegs;
    FloatRegisterFile fltRegs;
    CSRFile csr;
    Memory<1024*1024> memory;
};


struct FormattedInstruction
{
    char buffer[32];
};

const char* ParseELFResultMessage(ParseELFResult result);
const char* InstructionName(InstructionType type);
void FormatInstruction(RawInstruction ins, char* buffer, size_t buffsz);
FormattedInstruction FormatInstruction(RawInstruction ins);
InstructionType DecodeInstruction(RawInstruction instruction);
