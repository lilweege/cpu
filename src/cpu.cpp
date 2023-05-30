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

static const char* CSRName(uint32_t immediate)
{
    switch (immediate) {
        case 0xc00: return "cycle";
        case 0xc80: return "cycleh";
        case 0x7b0: return "dcsr";
        case 0x7b1: return "dpc";
        case 0x7b2: return "dscratch0";
        case 0x7b3: return "dscratch1";
        case 0x003: return "fcsr";
        case 0x001: return "fflags";
        case 0x002: return "frm";
        case 0x606: return "hcounteren";
        case 0x602: return "hedeleg";
        case 0x680: return "hgatp";
        case 0x607: return "hgeie";
        case 0xe07: return "hgeip";
        case 0x603: return "hideleg";
        case 0x604: return "hie";
        case 0x644: return "hip";
        case 0xc03: return "hpmcounter3";
        case 0xc04: return "hpmcounter4";
        case 0xc05: return "hpmcounter5";
        case 0xc06: return "hpmcounter6";
        case 0xc07: return "hpmcounter7";
        case 0xc08: return "hpmcounter8";
        case 0xc09: return "hpmcounter9";
        case 0xc0a: return "hpmcounter10";
        case 0xc0b: return "hpmcounter11";
        case 0xc0c: return "hpmcounter12";
        case 0xc0d: return "hpmcounter13";
        case 0xc0e: return "hpmcounter14";
        case 0xc0f: return "hpmcounter15";
        case 0xc10: return "hpmcounter16";
        case 0xc11: return "hpmcounter17";
        case 0xc12: return "hpmcounter18";
        case 0xc13: return "hpmcounter19";
        case 0xc14: return "hpmcounter20";
        case 0xc15: return "hpmcounter21";
        case 0xc16: return "hpmcounter22";
        case 0xc17: return "hpmcounter23";
        case 0xc18: return "hpmcounter24";
        case 0xc19: return "hpmcounter25";
        case 0xc1a: return "hpmcounter26";
        case 0xc1b: return "hpmcounter27";
        case 0xc1c: return "hpmcounter28";
        case 0xc1d: return "hpmcounter29";
        case 0xc1e: return "hpmcounter30";
        case 0xc1f: return "hpmcounter31";
        case 0xc83: return "hpmcounter3h";
        case 0xc84: return "hpmcounter4h";
        case 0xc85: return "hpmcounter5h";
        case 0xc86: return "hpmcounter6h";
        case 0xc87: return "hpmcounter7h";
        case 0xc88: return "hpmcounter8h";
        case 0xc89: return "hpmcounter9h";
        case 0xc8a: return "hpmcounter10h";
        case 0xc8b: return "hpmcounter11h";
        case 0xc8c: return "hpmcounter12h";
        case 0xc8d: return "hpmcounter13h";
        case 0xc8e: return "hpmcounter14h";
        case 0xc8f: return "hpmcounter15h";
        case 0xc90: return "hpmcounter16h";
        case 0xc91: return "hpmcounter17h";
        case 0xc92: return "hpmcounter18h";
        case 0xc93: return "hpmcounter19h";
        case 0xc94: return "hpmcounter20h";
        case 0xc95: return "hpmcounter21h";
        case 0xc96: return "hpmcounter22h";
        case 0xc97: return "hpmcounter23h";
        case 0xc98: return "hpmcounter24h";
        case 0xc99: return "hpmcounter25h";
        case 0xc9a: return "hpmcounter26h";
        case 0xc9b: return "hpmcounter27h";
        case 0xc9c: return "hpmcounter28h";
        case 0xc9d: return "hpmcounter29h";
        case 0xc9e: return "hpmcounter30h";
        case 0xc9f: return "hpmcounter31h";
        case 0x600: return "hstatus";
        case 0x605: return "htimedelta";
        case 0x615: return "htimedeltah";
        case 0x64a: return "htinst";
        case 0x643: return "htval";
        case 0xc02: return "instret";
        case 0xc82: return "instreth";
        case 0xf12: return "marchid";
        case 0x380: return "mbase";
        case 0x381: return "mbound";
        case 0x342: return "mcause";
        case 0x306: return "mcounteren";
        case 0x320: return "mcountinhibit";
        case 0xb00: return "mcycle";
        case 0xb80: return "mcycleh";
        case 0x384: return "mdbase";
        case 0x385: return "mdbound";
        case 0x302: return "medeleg";
        case 0x341: return "mepc";
        case 0xf14: return "mhartid";
        case 0xb03: return "mhpmcounter3";
        case 0xb04: return "mhpmcounter4";
        case 0xb05: return "mhpmcounter5";
        case 0xb06: return "mhpmcounter6";
        case 0xb07: return "mhpmcounter7";
        case 0xb08: return "mhpmcounter8";
        case 0xb09: return "mhpmcounter9";
        case 0xb0a: return "mhpmcounter10";
        case 0xb0b: return "mhpmcounter11";
        case 0xb0c: return "mhpmcounter12";
        case 0xb0d: return "mhpmcounter13";
        case 0xb0e: return "mhpmcounter14";
        case 0xb0f: return "mhpmcounter15";
        case 0xb10: return "mhpmcounter16";
        case 0xb11: return "mhpmcounter17";
        case 0xb12: return "mhpmcounter18";
        case 0xb13: return "mhpmcounter19";
        case 0xb14: return "mhpmcounter20";
        case 0xb15: return "mhpmcounter21";
        case 0xb16: return "mhpmcounter22";
        case 0xb17: return "mhpmcounter23";
        case 0xb18: return "mhpmcounter24";
        case 0xb19: return "mhpmcounter25";
        case 0xb1a: return "mhpmcounter26";
        case 0xb1b: return "mhpmcounter27";
        case 0xb1c: return "mhpmcounter28";
        case 0xb1d: return "mhpmcounter29";
        case 0xb1e: return "mhpmcounter30";
        case 0xb1f: return "mhpmcounter31";
        case 0xb83: return "mhpmcounter3h";
        case 0xb84: return "mhpmcounter4h";
        case 0xb85: return "mhpmcounter5h";
        case 0xb86: return "mhpmcounter6h";
        case 0xb87: return "mhpmcounter7h";
        case 0xb88: return "mhpmcounter8h";
        case 0xb89: return "mhpmcounter9h";
        case 0xb8a: return "mhpmcounter10h";
        case 0xb8b: return "mhpmcounter11h";
        case 0xb8c: return "mhpmcounter12h";
        case 0xb8d: return "mhpmcounter13h";
        case 0xb8e: return "mhpmcounter14h";
        case 0xb8f: return "mhpmcounter15h";
        case 0xb90: return "mhpmcounter16h";
        case 0xb91: return "mhpmcounter17h";
        case 0xb92: return "mhpmcounter18h";
        case 0xb93: return "mhpmcounter19h";
        case 0xb94: return "mhpmcounter20h";
        case 0xb95: return "mhpmcounter21h";
        case 0xb96: return "mhpmcounter22h";
        case 0xb97: return "mhpmcounter23h";
        case 0xb98: return "mhpmcounter24h";
        case 0xb99: return "mhpmcounter25h";
        case 0xb9a: return "mhpmcounter26h";
        case 0xb9b: return "mhpmcounter27h";
        case 0xb9c: return "mhpmcounter28h";
        case 0xb9d: return "mhpmcounter29h";
        case 0xb9e: return "mhpmcounter30h";
        case 0xb9f: return "mhpmcounter31h";
        case 0x323: return "mhpmevent3";
        case 0x324: return "mhpmevent4";
        case 0x325: return "mhpmevent5";
        case 0x326: return "mhpmevent6";
        case 0x327: return "mhpmevent7";
        case 0x328: return "mhpmevent8";
        case 0x329: return "mhpmevent9";
        case 0x32a: return "mhpmevent10";
        case 0x32b: return "mhpmevent11";
        case 0x32c: return "mhpmevent12";
        case 0x32d: return "mhpmevent13";
        case 0x32e: return "mhpmevent14";
        case 0x32f: return "mhpmevent15";
        case 0x330: return "mhpmevent16";
        case 0x331: return "mhpmevent17";
        case 0x332: return "mhpmevent18";
        case 0x333: return "mhpmevent19";
        case 0x334: return "mhpmevent20";
        case 0x335: return "mhpmevent21";
        case 0x336: return "mhpmevent22";
        case 0x337: return "mhpmevent23";
        case 0x338: return "mhpmevent24";
        case 0x339: return "mhpmevent25";
        case 0x33a: return "mhpmevent26";
        case 0x33b: return "mhpmevent27";
        case 0x33c: return "mhpmevent28";
        case 0x33d: return "mhpmevent29";
        case 0x33e: return "mhpmevent30";
        case 0x33f: return "mhpmevent31";
        case 0x382: return "mibase";
        case 0x383: return "mibound";
        case 0x303: return "mideleg";
        case 0x304: return "mie";
        case 0xf13: return "mimpid";
        case 0xb02: return "minstret";
        case 0xb82: return "minstreth";
        case 0x344: return "mip";
        case 0x301: return "misa";
        case 0x340: return "mscratch";
        case 0x300: return "mstatus";
        case 0x310: return "mstatush";
        case 0x34a: return "mtinst";
        case 0x343: return "mtval";
        case 0x34b: return "mtval2";
        case 0x305: return "mtvec";
        case 0xf11: return "mvendorid";
        case 0x3b0: return "pmpaddr0";
        case 0x3b1: return "pmpaddr1";
        case 0x3b2: return "pmpaddr2";
        case 0x3b3: return "pmpaddr3";
        case 0x3b4: return "pmpaddr4";
        case 0x3b5: return "pmpaddr5";
        case 0x3b6: return "pmpaddr6";
        case 0x3b7: return "pmpaddr7";
        case 0x3b8: return "pmpaddr8";
        case 0x3b9: return "pmpaddr9";
        case 0x3ba: return "pmpaddr10";
        case 0x3bb: return "pmpaddr11";
        case 0x3bc: return "pmpaddr12";
        case 0x3bd: return "pmpaddr13";
        case 0x3be: return "pmpaddr14";
        case 0x3bf: return "pmpaddr15";
        case 0x3c0: return "pmpaddr16";
        case 0x3c1: return "pmpaddr17";
        case 0x3c2: return "pmpaddr18";
        case 0x3c3: return "pmpaddr19";
        case 0x3c4: return "pmpaddr20";
        case 0x3c5: return "pmpaddr21";
        case 0x3c6: return "pmpaddr22";
        case 0x3c7: return "pmpaddr23";
        case 0x3c8: return "pmpaddr24";
        case 0x3c9: return "pmpaddr25";
        case 0x3ca: return "pmpaddr26";
        case 0x3cb: return "pmpaddr27";
        case 0x3cc: return "pmpaddr28";
        case 0x3cd: return "pmpaddr29";
        case 0x3ce: return "pmpaddr30";
        case 0x3cf: return "pmpaddr31";
        case 0x3d0: return "pmpaddr32";
        case 0x3d1: return "pmpaddr33";
        case 0x3d2: return "pmpaddr34";
        case 0x3d3: return "pmpaddr35";
        case 0x3d4: return "pmpaddr36";
        case 0x3d5: return "pmpaddr37";
        case 0x3d6: return "pmpaddr38";
        case 0x3d7: return "pmpaddr39";
        case 0x3d8: return "pmpaddr40";
        case 0x3d9: return "pmpaddr41";
        case 0x3da: return "pmpaddr42";
        case 0x3db: return "pmpaddr43";
        case 0x3dc: return "pmpaddr44";
        case 0x3dd: return "pmpaddr45";
        case 0x3de: return "pmpaddr46";
        case 0x3df: return "pmpaddr47";
        case 0x3e0: return "pmpaddr48";
        case 0x3e1: return "pmpaddr49";
        case 0x3e2: return "pmpaddr50";
        case 0x3e3: return "pmpaddr51";
        case 0x3e4: return "pmpaddr52";
        case 0x3e5: return "pmpaddr53";
        case 0x3e6: return "pmpaddr54";
        case 0x3e7: return "pmpaddr55";
        case 0x3e8: return "pmpaddr56";
        case 0x3e9: return "pmpaddr57";
        case 0x3ea: return "pmpaddr58";
        case 0x3eb: return "pmpaddr59";
        case 0x3ec: return "pmpaddr60";
        case 0x3ed: return "pmpaddr61";
        case 0x3ee: return "pmpaddr62";
        case 0x3ef: return "pmpaddr63";
        case 0x3a0: return "pmpcfg0";
        case 0x3a1: return "pmpcfg1";
        case 0x3a2: return "pmpcfg2";
        case 0x3a3: return "pmpcfg3";
        case 0x3a4: return "pmpcfg4";
        case 0x3a5: return "pmpcfg5";
        case 0x3a6: return "pmpcfg6";
        case 0x3a7: return "pmpcfg7";
        case 0x3a8: return "pmpcfg8";
        case 0x3a9: return "pmpcfg9";
        case 0x3aa: return "pmpcfg10";
        case 0x3ab: return "pmpcfg11";
        case 0x3ac: return "pmpcfg12";
        case 0x3ad: return "pmpcfg13";
        case 0x3ae: return "pmpcfg14";
        case 0x3af: return "pmpcfg15";
        case 0x180: return "satp";
        case 0x142: return "scause";
        case 0x106: return "scounteren";
        case 0x102: return "sedeleg";
        case 0x141: return "sepc";
        case 0x103: return "sideleg";
        case 0x104: return "sie";
        case 0x144: return "sip";
        case 0x140: return "sscratch";
        case 0x100: return "sstatus";
        case 0x143: return "stval";
        case 0x105: return "stvec";
        case 0x7a1: return "tdata1";
        case 0x7a2: return "tdata2";
        case 0x7a3: return "tdata3";
        case 0xc01: return "time";
        case 0xc81: return "timeh";
        case 0x7a0: return "tselect";
        case 0x042: return "ucause";
        case 0x041: return "uepc";
        case 0x004: return "uie";
        case 0x044: return "uip";
        case 0x040: return "uscratch";
        case 0x000: return "ustatus";
        case 0x043: return "utval";
        case 0x005: return "utvec";
        case 0x280: return "vsatp";
        case 0x242: return "vscause";
        case 0x241: return "vsepc";
        case 0x204: return "vsie";
        case 0x244: return "vsip";
        case 0x240: return "vsscratch";
        case 0x200: return "vsstatus";
        case 0x243: return "vstval";
        case 0x205: return "vstvec";
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
    fprintf(stderr, "Instruction 0x%08X unimplemented\n", instruction.value);
    assert(0);
    (void) instruction;
    return InstructionType::ILLEGAL;
}

static InstructionType DecodeQ1(RawInstruction instruction)
{
    fprintf(stderr, "Instruction 0x%08X unimplemented\n", instruction.value);
    assert(0);
    (void) instruction;
    return InstructionType::ILLEGAL;
}

static InstructionType DecodeQ2(RawInstruction instruction)
{
    fprintf(stderr, "Instruction 0x%08X unimplemented\n", instruction.value);
    assert(0);
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
    if (instruction.value == 0) return InstructionType::ILLEGAL;
    switch (instruction.value & 0b11) {
        case 0b00: return DecodeQ0(instruction);
        case 0b01: return DecodeQ1(instruction);
        case 0b10: return DecodeQ2(instruction);
        case 0b11: return DecodeQ3(instruction);
    }
    return InstructionType::ILLEGAL;
}

void CPU::Reset()
{
    pc = 0;
    fcsr = 0;
    for (uint32_t i = 0; i < 32; ++i)
        intRegs.Write(i, 0U);
    for (uint32_t i = 0; i < 32; ++i)
        fltRegs.Write(i, 0U);
    for (uint32_t i = 0; i < 4096; ++i)
        csr.Write(i, 0);
    memset(memory.Buffer(), 0, 1024*1024);
}

void CPU::InitializeFromELF(uint8_t* data, size_t size)
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
        exit(1);
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "ELF file is not 32-bit\n");
        exit(1);
    }

    if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "ELF file is not little-endian\n");
        exit(1);
    }

    // Rest of e_ident: version, OSABI, padding...

    if (header.e_type != ET_EXEC) {
        fprintf(stderr, "ELF file is not executable\n");
        exit(1);
    }

    if (header.e_machine != EM_RISCV) {
        fprintf(stderr, "ELF file targets wrong machine, expected RISC-V\n");
        exit(1);
    }

    if (header.e_version != EV_CURRENT) {
        fprintf(stderr, "ELF file has wrong version, expected 1\n");
        exit(1);
    }

    if (header.e_entry == 0) {
        fprintf(stderr, "ELF file does not specify entry point\n");
        exit(1);
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
            memcpy((uint8_t*)memory.Buffer() + pHeader.p_paddr, data + pHeader.p_offset, pHeader.p_filesz);
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
}


bool CPU::Step()
{
    RawInstruction ins{memory.Read<uint32_t>(pc)};
    InstructionType type = DecodeInstruction(ins);
    if (type == InstructionType::MRET) {
        // TODO: Actually do privilege stuff
        pc = csr.Read(0x341);
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