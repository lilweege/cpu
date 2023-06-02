#pragma once

#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <bit>

template<uint32_t _MemSize>
class Memory
{
public:
    constexpr static uint32_t MemSize = _MemSize;
public:
    template<typename T>
    T Read(uint32_t address) const
    {
        assert(address + sizeof(T) <= MemSize);
        T t;
        memcpy((uint8_t*) &t, buffer + address, sizeof(T));
        return t;
    }

    template<typename T>
    void Write(uint32_t address, T value)
    {
        assert(address + sizeof(T) <= MemSize);
        memset(didChange + address, 1, sizeof(T));
        memcpy(buffer + address, (const uint8_t*) &value, sizeof(T));
    }

    bool didChange[MemSize];
    uint8_t buffer[MemSize];
};

template <uint32_t _NumRegs>
class RegisterFile
{
public:
    constexpr static uint32_t NumRegs = _NumRegs;
public:
    template<typename T=uint32_t>
    T Read(uint32_t x) const
    {
        assert(x < NumRegs);
        if (x == 0) return 0;
        return static_cast<T>(registers[x]);
    }
    
    void Write(uint32_t x, auto value)
    {
        assert(x < NumRegs);
        if (x != 0) {
            didChange[x] = true;
            registers[x] = static_cast<uint32_t>(value);
        }
    }

    bool didChange[NumRegs];
    uint32_t registers[NumRegs];
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



struct CPU
{
public:
    void Reset();
    bool InitializeFromELF(uint8_t* data, size_t size);
    bool Step();
public:
    uint32_t pc;
    RegisterFile<32> intRegs;
    RegisterFile<32> fltRegs;
    RegisterFile<4096> csr;
    Memory<1024*1024> memory;
};


struct FormattedInstruction
{
    char buffer[32];
};

const char* InstructionName(InstructionType type);
void FormatInstruction(RawInstruction ins, char* buffer, size_t buffsz);
FormattedInstruction FormatInstruction(RawInstruction ins);
InstructionType DecodeInstruction(RawInstruction instruction);
