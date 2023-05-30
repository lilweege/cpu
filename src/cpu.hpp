#pragma once

#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <bit>

template<uint32_t MemSize>
class Memory
{
public:
    template<typename T>
    T Read(uint32_t address) const
    {
        T t;
        memcpy((uint8_t*) &t, m_rawMemory + address, sizeof(T));
        return t;
    }

    template<typename T>
    void Write(uint32_t address, T value)
    {
        memcpy(m_rawMemory + address, (const uint8_t*) &value, sizeof(T));
    }

    void* Buffer() { return m_rawMemory; }

private:
    uint8_t m_rawMemory[MemSize];
};


class RegisterFile {
public:
    template<typename T=uint32_t>
    T Read(uint32_t x) const
    {
        assert(x < 32);
        if (x == 0) return 0;
        return static_cast<T>(m_registers[x]);
    }
    
    void Write(uint32_t x, auto value)
    {
        assert(x < 32);
        if (x != 0)
            m_registers[x] = static_cast<uint32_t>(value);
    }
private:
    uint32_t m_registers[32];
};


class CSRFile {
public:
    uint32_t Read(uint32_t x) const
    {
        assert(x < 4096);
        return m_registers[x];
    }
    
    void Write(uint32_t x, uint32_t value)
    {
        assert(x < 4096);
        m_registers[x] = value;
    }
private:
    uint32_t m_registers[4096];
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
    void InitializeFromELF(uint8_t* data, size_t size);
    bool Step();
public:
    uint32_t pc;
    uint32_t fcsr;
    RegisterFile intRegs;
    RegisterFile fltRegs;
    CSRFile csr;
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
