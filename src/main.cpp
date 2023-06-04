#include "cpu.hpp"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>



CPU cpu;
std::vector<std::pair<uint32_t, FormattedInstruction>> readableObjectCode;


static std::vector<uint8_t> ReadEntireFile(std::string_view filename)
{
    std::ifstream input(filename.data(), std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    return buffer;
}

static void TestDecode()
{
    { RawInstruction ins{0x00001a37}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LUI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lui x20, 0x1
    { RawInstruction ins{0x800002b7}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LUI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lui x5, 0x80000
    { RawInstruction ins{0x212120b7}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LUI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lui x1, 0x21212
    { RawInstruction ins{0xffffe517}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::AUIPC); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // auipc x10, 0xffffe
    { RawInstruction ins{0xfffff797}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::AUIPC); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // auipc x15, 0xfffff
    { RawInstruction ins{0xfe1ff06f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::JAL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // jal x0, 800029ec
    { RawInstruction ins{0x0000006f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::JAL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // jal x0, 80002258
    { RawInstruction ins{0xf89ff06f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::JAL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // jal x0, 800027ac
    { RawInstruction ins{0x00008067}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::JALR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // jalr x0, 0(x1)
    { RawInstruction ins{0x000f0067}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::JALR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // jalr x0, 0(x30)
    { RawInstruction ins{0x10e78463}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BEQ); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // beq x15, x14, 800024b8
    { RawInstruction ins{0x00050a63}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BEQ); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // beq x10, x0, 80002538
    { RawInstruction ins{0x1b5a0463}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BEQ); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // beq x20, x21, 80002a10
    { RawInstruction ins{0xfe5210e3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BNE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bne x4, x5, 800001f4
    { RawInstruction ins{0x00e79a63}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BNE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bne x15, x14, 80002184
    { RawInstruction ins{0x25df1863}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BNE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bne x30, x29, 80002f90
    { RawInstruction ins{0x1220c063}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BLT); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // blt x1, x2, 800003c4
    { RawInstruction ins{0x00054863}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BLT); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // blt x10, x0, 800000c4
    { RawInstruction ins{0xfe20cee3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BLT); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // blt x1, x2, 80000158
    { RawInstruction ins{0x000f5463}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BGE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bge x30, x0, 8000003c
    { RawInstruction ins{0x0020d663}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BGE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bge x1, x2, 80002b88
    { RawInstruction ins{0x0620d463}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BGE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bge x1, x2, 80002f04
    { RawInstruction ins{0xfec7ece3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BLTU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bltu x15, x12, 800020a8
    { RawInstruction ins{0x0020e663}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BLTU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bltu x1, x2, 80002b18
    { RawInstruction ins{0x00f5f463}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BGEU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bgeu x11, x15, 80002290
    { RawInstruction ins{0xfec572e3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::BGEU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // bgeu x10, x12, 80002084
    { RawInstruction ins{0x02008283}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lb x5, 32(x1)
    { RawInstruction ins{0x00708283}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lb x5, 7(x1)
    { RawInstruction ins{0x00108f03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lb x30, 1(x1)
    { RawInstruction ins{0x00411f03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lh x30, 4(x2)
    { RawInstruction ins{0x00611f03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lh x30, 6(x2)
    { RawInstruction ins{0x00811f03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lh x30, 8(x2)
    { RawInstruction ins{0x02052403}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lw x8, 32(x10)
    { RawInstruction ins{0x03452683}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lw x13, 52(x10)
    { RawInstruction ins{0x0006a703}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lw x14, 0(x13)
    { RawInstruction ins{0x0006c783}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LBU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lbu x15, 0(x13)
    { RawInstruction ins{0x0006c703}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LBU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lbu x14, 0(x13)
    { RawInstruction ins{0x0007c683}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LBU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lbu x13, 0(x15)
    { RawInstruction ins{0x0060df03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LHU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lhu x30, 6(x1)
    { RawInstruction ins{0xffe0df03}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LHU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lhu x30, -2(x1)
    { RawInstruction ins{0x0002d303}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::LHU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // lhu x6, 0(x5)
    { RawInstruction ins{0x00e78023}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sb x14, 0(x15)
    { RawInstruction ins{0x001101a3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sb x1, 3(x2)
    { RawInstruction ins{0xfee78fa3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sb x14, -1(x15)
    { RawInstruction ins{0xfe209d23}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sh x2, -6(x1)
    { RawInstruction ins{0x00111223}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sh x1, 4(x2)
    { RawInstruction ins{0x00111523}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sh x1, 10(x2)
    { RawInstruction ins{0x05612c23}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sw x22, 88(x2)
    { RawInstruction ins{0x01b12e23}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sw x27, 28(x2)
    { RawInstruction ins{0x01052223}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sw x16, 4(x10)
    { RawInstruction ins{0x00200793}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ADDI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // addi x15, x0, 2
    { RawInstruction ins{0x00000013}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ADDI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // addi x0, x0, 0
    { RawInstruction ins{0x0010af13}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slti x30, x1, 1
    { RawInstruction ins{0x7ff0af13}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slti x30, x1, 2047
    { RawInstruction ins{0x8000af13}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slti x30, x1, -2048
    { RawInstruction ins{0x0017b613}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTIU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sltiu x12, x15, 1
    { RawInstruction ins{0xfff0bf13}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTIU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sltiu x30, x1, -1
    { RawInstruction ins{0x999ac093}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::XORI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // xori x1, x21, -1639
    { RawInstruction ins{0xfff6c693}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::XORI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // xori x13, x13, -1
    { RawInstruction ins{0x00156513}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ORI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // ori x10, x10, 1
    { RawInstruction ins{0x04076713}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ORI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // ori x14, x14, 64
    { RawInstruction ins{0x5391e193}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ORI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // ori x3, x3, 1337
    { RawInstruction ins{0xff867693}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ANDI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // andi x13, x12, -8
    { RawInstruction ins{0x08077693}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ANDI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // andi x13, x14, 128
    { RawInstruction ins{0x04077693}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ANDI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // andi x13, x14, 64
    { RawInstruction ins{0x00381813}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLLI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slli x16, x16, 0x3
    { RawInstruction ins{0x01059793}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLLI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slli x15, x11, 0x10
    { RawInstruction ins{0x03079793}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLLI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slli x15, x15, 0x30
    { RawInstruction ins{0x00c7d793}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRLI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // srli x15, x15, 0xc
    { RawInstruction ins{0x41f55893}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRAI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // srai x17, x10, 0x1f
    { RawInstruction ins{0x40e0df13}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRAI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // srai x30, x1, 0xe
    { RawInstruction ins{0x00c58633}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ADD); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // add x12, x11, x12
    { RawInstruction ins{0x00d506b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ADD); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // add x13, x10, x13
    { RawInstruction ins{0x00a70533}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ADD); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // add x10, x14, x10
    { RawInstruction ins{0x40b50533}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SUB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sub x10, x10, x11
    { RawInstruction ins{0x40e78533}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SUB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sub x10, x15, x14
    { RawInstruction ins{0x41060633}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SUB); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sub x12, x12, x16
    { RawInstruction ins{0x00209f33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sll x30, x1, x2
    { RawInstruction ins{0x0020af33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLT); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // slt x30, x1, x2
    { RawInstruction ins{0x0020bf33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sltu x30, x1, x2
    { RawInstruction ins{0x000030b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SLTU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sltu x1, x0, x0
    { RawInstruction ins{0x00f647b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::XOR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // xor x15, x12, x15
    { RawInstruction ins{0x0020cf33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::XOR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // xor x30, x1, x2
    { RawInstruction ins{0x0020c133}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::XOR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // xor x2, x1, x2
    { RawInstruction ins{0x0020d0b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // srl x1, x1, x2
    { RawInstruction ins{0x0020df33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // srl x30, x1, x2
    { RawInstruction ins{0x4020df33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRA); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sra x30, x1, x2
    { RawInstruction ins{0x400050b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRA); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sra x1, x0, x0
    { RawInstruction ins{0x4020d133}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::SRA); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // sra x2, x1, x2
    { RawInstruction ins{0x00b7e5b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::OR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // or x11, x15, x11
    { RawInstruction ins{0x00f665b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::OR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // or x11, x12, x15
    { RawInstruction ins{0x00b7e7b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::OR); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // or x15, x15, x11
    { RawInstruction ins{0x00d57533}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::AND); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // and x10, x10, x13
    { RawInstruction ins{0x00b7f733}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::AND); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // and x14, x15, x11
    { RawInstruction ins{0x00c7f733}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::AND); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // and x14, x15, x12
    { RawInstruction ins{0x0310000f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FENCE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fence rw, w
    { RawInstruction ins{0x0820000f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FENCE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fence i, r
    { RawInstruction ins{0x0ff0000f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FENCE); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fence iorw, iorw
    { RawInstruction ins{0x00000073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::ECALL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // ecall
    { RawInstruction ins{0x10569073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrw x0, stvec, x13
    { RawInstruction ins{0x18079073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrw x0, satp, x15
    { RawInstruction ins{0x10551073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrw x0, stvec, x10
    { RawInstruction ins{0x1007a073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrs x0, sstatus, x15
    { RawInstruction ins{0x1006a073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrs x0, sstatus, x13
    { RawInstruction ins{0x1004b073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRC); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrc x0, sstatus, x9
    { RawInstruction ins{0x100db073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRC); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrc x0, sstatus, x27
    { RawInstruction ins{0x1006b073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRC); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrc x0, sstatus, x13
    { RawInstruction ins{0x14005073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRWI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrwi x0, sscratch, 0
    { RawInstruction ins{0x10016073}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRSI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrsi x0, sstatus, 2
    { RawInstruction ins{0x100176f3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRCI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrci x13, sstatus, 2
    { RawInstruction ins{0x10017773}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::CSRRCI); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // csrrci x14, sstatus, 2
    { RawInstruction ins{0x021080b3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MUL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mul x1, x1, x1
    { RawInstruction ins{0x02208f33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MUL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mul x30, x1, x2
    { RawInstruction ins{0x02208133}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MUL); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mul x2, x1, x2
    { RawInstruction ins{0x02209133}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MULH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mulh x2, x1, x2
    { RawInstruction ins{0x02209f33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MULH); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mulh x30, x1, x2
    { RawInstruction ins{0x0220a133}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MULHSU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mulhsu x2, x1, x2
    { RawInstruction ins{0x0220af33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MULHSU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mulhsu x30, x1, x2
    { RawInstruction ins{0x0220bf33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::MULHU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // mulhu x30, x1, x2
    { RawInstruction ins{0x0220cf33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::DIV); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // div x30, x1, x2
    { RawInstruction ins{0x0220df33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::DIVU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // divu x30, x1, x2
    { RawInstruction ins{0x0220ef33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::REM); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // rem x30, x1, x2
    { RawInstruction ins{0x0220ff33}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::REMU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // remu x30, x1, x2
    { RawInstruction ins{0x0004a787}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flw fa5, 0(s1)
    { RawInstruction ins{0x1e872687}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flw fa3, 488(a4)
    { RawInstruction ins{0x1e472707}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flw fa4, 484(a4)
    { RawInstruction ins{0x00aa2027}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsw fa0, 0(s4)
    { RawInstruction ins{0x00f4a027}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsw fa5, 0(s1)
    { RawInstruction ins{0x00fba827}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsw fa5, 16(s7)
    { RawInstruction ins{0xd19b1543}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmadd.s fa0, fs6, fs9, fs10, rtz
    { RawInstruction ins{0x114f8bc3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmadd.s fs7, ft11, fs4, ft2, rne
    { RawInstruction ins{0x08cf53c3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmadd.s ft7, ft10, fa2, ft1, unknown
    { RawInstruction ins{0x3166dd47}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmsub.s fs10, fa3, fs6, ft6, unknown
    { RawInstruction ins{0x50077347}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmsub.s ft6, fa4, ft0, fa0
    { RawInstruction ins{0xb903e1c7}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmsub.s ft3, ft7, fa6, fs7, unknown
    { RawInstruction ins{0xc9cd48cb}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmsub.s fa7, fs10, ft8, fs9, rmm
    { RawInstruction ins{0xa1ee44cb}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmsub.s fs1, ft8, ft10, fs4, rmm
    { RawInstruction ins{0xf8db734b}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmsub.s ft6, fs6, fa3, ft11
    { RawInstruction ins{0x19613e4f}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmadd.s ft8, ft2, fs6, ft3, rup
    { RawInstruction ins{0xc944cfcf}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmadd.s ft11, fs1, fs4, fs9, rmm
    { RawInstruction ins{0x191506cf}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FNMADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fnmadd.s fa3, fa0, fa7, ft3, rne
    { RawInstruction ins{0x0127f553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fadd.s fa0, fa5, fs2
    { RawInstruction ins{0x01257553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fadd.s fa0, fa0, fs2
    { RawInstruction ins{0x0135f9d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FADDS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fadd.s fs3, fa1, fs3
    { RawInstruction ins{0x0897f7d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsub.s fa5, fa5, fs1
    { RawInstruction ins{0x0957f7d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsub.s fa5, fa5, fs5
    { RawInstruction ins{0x0935f753}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSUBS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsub.s fa4, fa1, fs3
    { RawInstruction ins{0x10f97953}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMULS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmul.s fs2, fs2, fa5
    { RawInstruction ins{0x1187f7d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMULS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmul.s fa5, fa5, fs8
    { RawInstruction ins{0x116b7553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMULS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmul.s fa0, fs6, fs6
    { RawInstruction ins{0x1947f553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FDIVS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fdiv.s fa0, fa5, fs4
    { RawInstruction ins{0x18a7f553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FDIVS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fdiv.s fa0, fa5, fa0
    { RawInstruction ins{0x18f777d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FDIVS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fdiv.s fa5, fa4, fa5
    { RawInstruction ins{0x58057553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSQRTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsqrt.s fa0, fa0
    { RawInstruction ins{0x580e35d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSQRTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsqrt.s fa1, ft8, rup
    { RawInstruction ins{0x5808c0d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSQRTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsqrt.s ft1, fa7, rmm
    { RawInstruction ins{0x21ca0ed3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnj.s ft9, fs4, ft8
    { RawInstruction ins{0x20d103d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnj.s ft7, ft2, fa3
    { RawInstruction ins{0x209c0d53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnj.s fs10, fs8, fs1
    { RawInstruction ins{0x21dd1b53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJNS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjn.s fs6, fs10, ft9
    { RawInstruction ins{0x20971153}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJNS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjn.s ft2, fa4, fs1
    { RawInstruction ins{0x211d1953}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJNS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjn.s fs2, fs10, fa7
    { RawInstruction ins{0x20eb2153}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjx.s ft2, fs6, fa4
    { RawInstruction ins{0x219fa7d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjx.s fa5, ft11, fs9
    { RawInstruction ins{0x215baad3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FSGNJXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fsgnjx.s fs5, fs7, fs5
    { RawInstruction ins{0x286b82d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMINS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmin.s ft5, fs7, ft6
    { RawInstruction ins{0x29ac88d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMINS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmin.s fa7, fs9, fs10
    { RawInstruction ins{0x29728c53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMINS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmin.s fs8, ft5, fs7
    { RawInstruction ins{0x29441153}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMAXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmax.s ft2, fs0, fs4
    { RawInstruction ins{0x29689fd3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMAXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmax.s ft11, fa7, fs6
    { RawInstruction ins{0x286a1fd3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMAXS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmax.s ft11, fs4, ft6
    { RawInstruction ins{0xc0056553}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.w.s a0, fa0, unknown
    { RawInstruction ins{0xc006fad3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.w.s s5, fa3
    { RawInstruction ins{0xc00fa8d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.w.s a7, ft11, rdn
    { RawInstruction ins{0xc014cb53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWUS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.wu.s s6, fs1, rmm
    { RawInstruction ins{0xc01698d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWUS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.wu.s a7, fa3, rtz
    { RawInstruction ins{0xc01e5dd3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTWUS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.wu.s s11, ft8, unknown
    { RawInstruction ins{0xe00482d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVXW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.x.w t0, fs1
    { RawInstruction ins{0xe00d86d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVXW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.x.w a3, fs11
    { RawInstruction ins{0xe0088053}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVXW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.x.w zero, fa7
    { RawInstruction ins{0xa0742153}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FEQS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // feq.s sp, fs0, ft7
    { RawInstruction ins{0xa0a0a153}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FEQS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // feq.s sp, ft1, fa0
    { RawInstruction ins{0xa1aba853}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FEQS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // feq.s a6, fs7, fs10
    { RawInstruction ins{0xa0651953}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flt.s s2, fa0, ft6
    { RawInstruction ins{0xa0ab9f53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flt.s t5, fs7, fa0
    { RawInstruction ins{0xa19595d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLTS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // flt.s a1, fa1, fs9
    { RawInstruction ins{0xa1ff8d53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLES); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fle.s s10, ft11, ft11
    { RawInstruction ins{0xa0f40653}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLES); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fle.s a2, fs0, fa5
    { RawInstruction ins{0xa1ab0c53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FLES); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fle.s s8, fs6, fs10
    { RawInstruction ins{0xe00a1e53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCLASSS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fclass.s t3, fs4
    { RawInstruction ins{0xe00f1c53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCLASSS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fclass.s s8, ft10
    { RawInstruction ins{0xe00e9d53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCLASSS); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fclass.s s10, ft9
    { RawInstruction ins{0xd009d7d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.w fa5, s3, unknown
    { RawInstruction ins{0xd001a953}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.w fs2, gp, rdn
    { RawInstruction ins{0xd00507d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSW); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.w fa5, a0, rne
    { RawInstruction ins{0xd01c27d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSWU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.wu fa5, s8, rdn
    { RawInstruction ins{0xd019edd3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSWU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.wu fs11, s3, unknown
    { RawInstruction ins{0xd012c3d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FCVTSWU); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fcvt.s.wu ft7, t0, rmm
    { RawInstruction ins{0xf0000e53}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVWX); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.w.x ft8, zero
    { RawInstruction ins{0xf0098053}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVWX); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.w.x ft0, s3
    { RawInstruction ins{0xf00081d3}; InstructionType type = DecodeInstruction(ins); assert(type == InstructionType::FMVWX); FormattedInstruction out = FormatInstruction(ins); printf("%s\n", out.buffer); } // fmv.w.x ft3, ra
}


static void TestISA()
{
    const char* testNames[] = {
        "riscv-tests/isa/rv32ui-p-add",
        "riscv-tests/isa/rv32ui-p-addi",
        "riscv-tests/isa/rv32ui-p-and",
        "riscv-tests/isa/rv32ui-p-andi",
        "riscv-tests/isa/rv32ui-p-auipc",
        "riscv-tests/isa/rv32ui-p-beq",
        "riscv-tests/isa/rv32ui-p-bge",
        "riscv-tests/isa/rv32ui-p-bgeu",
        "riscv-tests/isa/rv32ui-p-blt",
        "riscv-tests/isa/rv32ui-p-bltu",
        "riscv-tests/isa/rv32ui-p-bne",
        "riscv-tests/isa/rv32ui-p-fence_i",
        "riscv-tests/isa/rv32ui-p-jal",
        "riscv-tests/isa/rv32ui-p-jalr",
        "riscv-tests/isa/rv32ui-p-lb",
        "riscv-tests/isa/rv32ui-p-lbu",
        "riscv-tests/isa/rv32ui-p-lh",
        "riscv-tests/isa/rv32ui-p-lhu",
        "riscv-tests/isa/rv32ui-p-lui",
        "riscv-tests/isa/rv32ui-p-lw",
        "riscv-tests/isa/rv32ui-p-ma_data",
        "riscv-tests/isa/rv32ui-p-or",
        "riscv-tests/isa/rv32ui-p-ori",
        "riscv-tests/isa/rv32ui-p-sb",
        "riscv-tests/isa/rv32ui-p-sh",
        "riscv-tests/isa/rv32ui-p-simple",
        "riscv-tests/isa/rv32ui-p-sll",
        "riscv-tests/isa/rv32ui-p-slli",
        "riscv-tests/isa/rv32ui-p-slt",
        "riscv-tests/isa/rv32ui-p-slti",
        "riscv-tests/isa/rv32ui-p-sltiu",
        "riscv-tests/isa/rv32ui-p-sltu",
        "riscv-tests/isa/rv32ui-p-sra",
        "riscv-tests/isa/rv32ui-p-srai",
        "riscv-tests/isa/rv32ui-p-srl",
        "riscv-tests/isa/rv32ui-p-srli",
        "riscv-tests/isa/rv32ui-p-sub",
        "riscv-tests/isa/rv32ui-p-sw",
        "riscv-tests/isa/rv32ui-p-xor",
        "riscv-tests/isa/rv32ui-p-xori",
        "riscv-tests/isa/rv32um-p-mul",
        "riscv-tests/isa/rv32um-p-mulh",
        "riscv-tests/isa/rv32um-p-mulhu",
        "riscv-tests/isa/rv32um-p-mulhsu",
        "riscv-tests/isa/rv32um-p-rem",
        "riscv-tests/isa/rv32um-p-remu",
        "riscv-tests/isa/rv32um-p-divu",
        "riscv-tests/isa/rv32um-p-div",
        "riscv-tests/isa/rv32uf-p-fadd",
        "riscv-tests/isa/rv32uf-p-recoding",
        "riscv-tests/isa/rv32uf-p-move",
        "riscv-tests/isa/rv32uf-p-ldst",
        "riscv-tests/isa/rv32uf-p-fmin",
        "riscv-tests/isa/rv32uf-p-fmadd",
        "riscv-tests/isa/rv32uf-p-fcvt_w",
        "riscv-tests/isa/rv32uf-p-fcvt",
        "riscv-tests/isa/rv32uf-p-fcmp",
        "riscv-tests/isa/rv32uf-p-fclass",
        "riscv-tests/isa/rv32uf-p-fdiv",
    };
    int numFailed = 0;
    size_t numTests = sizeof(testNames) / sizeof(testNames[0]);
    for (size_t i = 0; i < numTests; ++i) {
        const char* testName = testNames[i];
        auto buffer = ReadEntireFile(testName);
        auto parseResult = cpu.InitializeFromELF(buffer.data(), buffer.size());
        assert(parseResult == ParseELFResult::Ok);

        while (cpu.Step());

        uint32_t result = cpu.intRegs.Read(10);
        printf("Test %s: ", testName);
        if (result != 0) {
            printf("FAILED (%d)\n", result >> 1);
            ++numFailed;
        }
        else {
            printf("PASSED\n");
        }
    }
    assert(numFailed == 0);
}


// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
// #include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "portable-file-dialogs.h"

struct Texture
{
    int width, height;
    ImTextureID id;
};

typedef void (*ButtonPressCallback)();

struct Button
{
    ButtonPressCallback callbackFunc;
    Texture texture;
};

// Simple helper function to load an image into a OpenGL texture with common settings
Texture LoadTextureFromFile(const char* filename)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    IM_ASSERT(image_data != NULL);

    // Create a OpenGL texture identifier
    GLuint image_texture = 0;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return Texture{
        .width = image_width,
        .height = image_height,
        .id = (ImTextureID)(intptr_t)image_texture,
    };
}



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


static void FileOpenButtonPressed()
{
    auto selectedFiles = pfd::open_file::open_file("Open a file").result();
    if (!selectedFiles.empty()) {
        auto buffer = ReadEntireFile(selectedFiles[0]);
        ParseELFResult result = cpu.InitializeFromELF(buffer.data(), buffer.size());
        if (result != ParseELFResult::Ok) {
            pfd::message("Invalid ELF file", ParseELFResultMessage(result),
                pfd::choice::ok, pfd::icon::error);
        }
    }
    readableObjectCode.clear();
    readableObjectCode.reserve(cpu.memory.Size);
    for (uint32_t i = 0; i+4 <= cpu.memory.Size; i += 4) {
        uint32_t word = cpu.memory.Read<uint32_t>(i);
        if (DecodeInstruction(word) != InstructionType::ILLEGAL)
            readableObjectCode.push_back({i, FormatInstruction(word)});
    }
}

static void DebugStartButtonPressed()
{
    printf("DebugStartButtonPressed!\n");
}

static void DebugStopButtonPressed()
{
    printf("DebugStopButtonPressed!\n");
}

static void DebugRestartButtonPressed()
{
    printf("DebugRestartButtonPressed!\n");
}

static void DebugStepOverButtonPressed()
{
    memset(cpu.intRegs.didChange, false, cpu.intRegs.Size);
    memset(cpu.fltRegs.didChange, false, cpu.fltRegs.Size);
    memset(cpu.memory.didChange, false, cpu.memory.Size);
    cpu.Step();
}

static void DebugStepIntoButtonPressed()
{
    printf("DebugStepIntoButtonPressed!\n");
}

static void DebugStepOutButtonPressed()
{
    printf("DebugStepOutButtonPressed!\n");
}

static bool MemoryHighlightFn(const ImU8* data, size_t off)
{
    return cpu.memory.didChange[off] || ((off & ~0b11) == cpu.pc);
}
    

int main(int, char**)
{
    // TestDecode();
    // TestISA();
    // return 0;


    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "cpu", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Load images
    Button buttons[] = {
        { .callbackFunc = FileOpenButtonPressed,      .texture = LoadTextureFromFile("./images/folder-opened.png")   },
        { .callbackFunc = DebugStartButtonPressed,    .texture = LoadTextureFromFile("./images/debug-start.png")     },
        { .callbackFunc = DebugStopButtonPressed,     .texture = LoadTextureFromFile("./images/debug-stop.png")      },
        { .callbackFunc = DebugRestartButtonPressed,  .texture = LoadTextureFromFile("./images/debug-restart.png")   },
        { .callbackFunc = DebugStepOverButtonPressed, .texture = LoadTextureFromFile("./images/debug-step-over.png") },
        { .callbackFunc = DebugStepIntoButtonPressed, .texture = LoadTextureFromFile("./images/debug-step-into.png") },
        { .callbackFunc = DebugStepOutButtonPressed,  .texture = LoadTextureFromFile("./images/debug-step-out.png")  },
    };
    size_t numButtons = sizeof(buttons) / sizeof(buttons[0]);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    

    ImU32 highlightColor = IM_COL32(255, 0, 0, 255);
    MemoryEditor memEdit;
    memEdit.ReadOnly = true;
    memEdit.HighlightFn = MemoryHighlightFn;
    memEdit.HighlightColor = highlightColor;

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        {

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground)) {
                ImGuiID dockspaceID = ImGui::GetID("DockSpace");
                ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            }
            ImGui::End();
            ImGui::PopStyleVar(3);


            if (ImGui::Begin("Buttons")) {
                for (size_t i = 0; i < numButtons; ++i) {
                    const Button& btn = buttons[i];
                    if (ImGui::ImageButton(btn.texture.id, ImVec2(32, 32))) {
                        btn.callbackFunc();
                    }
                    ImGui::SameLine();
                }
            }
            ImGui::End();

            if (ImGui::Begin("Code")) {
                for (const auto& [addr, instruction] : readableObjectCode) {
                    bool isCurrentInstruction = addr == cpu.pc;
                    if (isCurrentInstruction) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                    ImGui::Text("%08X: %s", addr, instruction.buffer);
                    if (isCurrentInstruction) ImGui::PopStyleColor();
                }
            }
            ImGui::End();
            
            if (ImGui::Begin("Registers")) {
                for (uint32_t i = 0; i < cpu.intRegs.Size; ++i) {
                    {
                        uint32_t x = cpu.intRegs.Read(i);
                        bool didChange = cpu.intRegs.didChange[i];
                        if (didChange) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                        int length = snprintf(NULL, 0, "%d", x);
                        ImGui::Text("%*sx%u: %02X %02X %02X %02X  (%d)%*s",
                            i < 10, "", i, (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, (x >> 0) & 0xFF, x, 15-length, "");
                        if (didChange) ImGui::PopStyleColor();
                    }
                    ImGui::SameLine();
                    {
                        float y = cpu.fltRegs.Read(i);
                        uint32_t x = std::bit_cast<uint32_t>(y);
                        bool didChange = cpu.fltRegs.didChange[i];
                        if (didChange) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                        ImGui::Text("%*sx%u: %02X %02X %02X %02X  (%f)\n",
                            i < 10, "", i, (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, (x >> 0) & 0xFF, y);
                        if (didChange) ImGui::PopStyleColor();
                    }
                }
                
                ImGui::Text("pc: %02X %02X %02X %02X  (%d)\n",
                    (cpu.pc >> 24) & 0xFF,
                    (cpu.pc >> 16) & 0xFF,
                    (cpu.pc >> 8) & 0xFF,
                    (cpu.pc >> 0) & 0xFF, cpu.pc);
                uint32_t fcsr = cpu.csr.Read(CSR_fcsr);
                ImGui::Text("       frm  NV DZ OF UF NX");
                ImGui::Text("fcsr:  %d%d%d  %d  %d  %d  %d  %d",
                    (fcsr >> 7) & 1, (fcsr >> 6) & 1, (fcsr >> 5) & 1, (fcsr >> 4) & 1, (fcsr >> 3) & 1, (fcsr >> 2) & 1, (fcsr >> 1) & 1, (fcsr >> 0) & 1);
            }
            ImGui::End();

            memEdit.DrawWindow("Memory", cpu.memory.buffer, cpu.memory.Size);
            // ...
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImVec4 c = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        glClearColor(c.x * c.w, c.y * c.w, c.z * c.w, c.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
