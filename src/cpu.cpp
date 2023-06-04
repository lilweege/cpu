#include "cpu.hpp"

#include "elf.h"
#include <cstdlib>
#include <cmath>
#include <cfenv>
#include <climits>
#include <bit>
#include "helpers.hpp"

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

static const char* RMName(uint32_t rm)
{
    switch (rm) {
        case 0b000: return "rne"; // Round to Nearest, ties to Even
        case 0b001: return "rtz"; // Round towards Zero
        case 0b010: return "rdn"; // Round Down (towards −∞)
        case 0b011: return "rup"; // Round Up (towards +∞)
        case 0b100: return "rmm"; // Round to Nearest, ties to Max Magnitude
        case 0b111: return "";
        // case 0b111: return "dyn"; // In instruction’s rm field, selects dynamic rounding mode;
        case 0b101:
        case 0b110:
        default: return "unknown";
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
        case InstructionType::MUL:
        case InstructionType::MULH:
        case InstructionType::MULHSU:
        case InstructionType::MULHU:
        case InstructionType::DIV:
        case InstructionType::DIVU:
        case InstructionType::REM:
        case InstructionType::REMU:
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
        // I-type (float)
        case InstructionType::FLW:
            snprintf(buffer, buffsz, "%s f%d, %d(x%d)", InstructionName(type), ins.Ityp.rd, SignExtend(ins.Ityp.imm11_0, 12), ins.Ityp.rs1);
            break;
        // S-type (float)
        case InstructionType::FSW:
            snprintf(buffer, buffsz, "%s f%d, %d(x%d)", InstructionName(type), ins.Styp.rs2, SignExtend(ins.Styp.imm(), 12), ins.Ityp.rs1);
            break;
        // R-type (float)
        case InstructionType::FSGNJS:
        case InstructionType::FSGNJNS:
        case InstructionType::FSGNJXS:
        case InstructionType::FMINS:
        case InstructionType::FMAXS:
        case InstructionType::FEQS:
        case InstructionType::FLTS:
        case InstructionType::FLES:
            snprintf(buffer, buffsz, "%s f%d, f%d, f%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, ins.Rtyp.rs2);
            break;
        // R4-type (f, f, f, f, rm)
        case InstructionType::FMADDS:
        case InstructionType::FMSUBS:
        case InstructionType::FNMSUBS:
        case InstructionType::FNMADDS: {
            const char* rmRepr = RMName(ins.R4typ.funct3);
            if (rmRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s f%d, f%d, f%d, f%d, %s", InstructionName(type), ins.R4typ.rd, ins.R4typ.rs1, ins.R4typ.rs2, ins.R4typ.rs3, rmRepr);
            }
            else {
                snprintf(buffer, buffsz, "%s f%d, f%d, f%d, f%d", InstructionName(type), ins.R4typ.rd, ins.R4typ.rs1, ins.R4typ.rs2, ins.R4typ.rs3);
            }
        } break;
        // R-type (f, f, f, rm)
        case InstructionType::FADDS:
        case InstructionType::FSUBS:
        case InstructionType::FMULS:
        case InstructionType::FDIVS: {
            const char* rmRepr = RMName(ins.Rtyp.funct3);
            if (rmRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s f%d, f%d, f%d, %s", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, ins.Rtyp.rs2, rmRepr);
            }
            else {
                snprintf(buffer, buffsz, "%s f%d, f%d, f%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, ins.Rtyp.rs2);
            }
        } break;
        // R-type (f, f, rm)
        case InstructionType::FSQRTS: {
            const char* rmRepr = RMName(ins.Rtyp.funct3);
            if (rmRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s f%d, f%d, %s", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, rmRepr);
            }
            else {
                snprintf(buffer, buffsz, "%s f%d, f%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1);
            }
        } break;
        // R-type (x, f, rm)
        case InstructionType::FCVTWS:
        case InstructionType::FCVTWUS: {
            const char* rmRepr = RMName(ins.Rtyp.funct3);
            if (rmRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s x%d, f%d, %s", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, rmRepr);
            }
            else {
                snprintf(buffer, buffsz, "%s x%d, f%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1);
            }
        } break;
        // R-type (f, x, rm)
        case InstructionType::FCVTSW:
        case InstructionType::FCVTSWU: {
            const char* rmRepr = RMName(ins.Rtyp.funct3);
            if (rmRepr[0] != '\0') {
                snprintf(buffer, buffsz, "%s f%d, x%d, %s", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1, rmRepr);
            }
            else {
                snprintf(buffer, buffsz, "%s f%d, x%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1);
            }
        } break;
        // R-type (x, f)
        case InstructionType::FCLASSS:
        case InstructionType::FMVXW:
            snprintf(buffer, buffsz, "%s x%d, f%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1);
            break;
        // R-type (f, x)
        case InstructionType::FMVWX:
            snprintf(buffer, buffsz, "%s f%d, x%d", InstructionName(type), ins.Rtyp.rd, ins.Rtyp.rs1);
            break;
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
            switch (instruction.Rtyp.funct3) {
                case 0b000: {
                    switch (instruction.Ityp.imm11_0) {
                        case 0b000000000000: return InstructionType::ECALL;
                        case 0b000000000001: return InstructionType::EBREAK;
                    }
                } break;
                case 0b001: return InstructionType::CSRRW;
                case 0b010: return InstructionType::CSRRS;
                case 0b011: return InstructionType::CSRRC;
                case 0b101: return InstructionType::CSRRWI;
                case 0b110: return InstructionType::CSRRSI;
                case 0b111: return InstructionType::CSRRCI;
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
    memset(&memory, 0, sizeof(memory));
}

const char* ParseELFResultMessage(ParseELFResult result)
{
    switch (result) {
        case ParseELFResult::Ok: return "Success";
        case ParseELFResult::WrongMagic: return "Missing ELF header";
        case ParseELFResult::WrongClass: return "ELF file is not 32-bit";
        case ParseELFResult::WrongData: return "ELF file is not little-endian";
        case ParseELFResult::WrongType: return "ELF file is not executable";
        case ParseELFResult::WrongMachine: return "ELF file targets wrong machine, expected RISC-V";
        case ParseELFResult::WrongVersion: return "ELF file has wrong version, expected 1";
        case ParseELFResult::NoEntry: return "ELF file does not specify entry point";
    }
    return "";
}

ParseELFResult CPU::InitializeFromELF(uint8_t* data, size_t size)
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
        return ParseELFResult::WrongMagic;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS32) {
        return ParseELFResult::WrongClass;
    }

    if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
        return ParseELFResult::WrongData;
    }

    // Rest of e_ident: version, OSABI, padding...

    if (header.e_type != ET_EXEC) {
        return ParseELFResult::WrongType;
    }

    if (header.e_machine != EM_RISCV) {
        return ParseELFResult::WrongMachine;
    }

    if (header.e_version != EV_CURRENT) {
        return ParseELFResult::WrongVersion;
    }

    if (header.e_entry == 0) {
        return ParseELFResult::NoEntry;
    }

    // Parsed successfully...
    Reset();
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

    (void) numSectionHeaders;
    (void) sectionHeadersStartIdx;
    (void) sectionHeaderOffset;
#if 0 // These are unimportant for execution, only for linking
    // Section Headers
    size_t sectionHeadersSize = numSectionHeaders * sizeof(Elf32_Shdr);
    Elf32_Shdr* sectionHeaders = (Elf32_Shdr*) malloc(sectionHeadersSize);
    assert(sectionHeaders != NULL && "malloc failed - buy more RAM");

    assert(sectionHeaderOffset + sectionHeadersSize <= size);
    memcpy(sectionHeaders, data + sectionHeaderOffset, sectionHeadersSize);

    assert(sectionHeadersStartIdx != SHN_UNDEF);
    Elf32_Shdr shstrtab = sectionHeaders[sectionHeadersStartIdx];
    assert(shstrtab.sh_type == SHT_STRTAB);

    for (size_t i = 0; i < numSectionHeaders; ++i) {
        Elf32_Shdr sHeader = sectionHeaders[i];
        if (sHeader.sh_name != SHN_UNDEF) {
            const char* sectionName = (char*) data + shstrtab.sh_offset + sHeader.sh_name;
            printf("%zu: %s\n", i, sectionName);
        }

        // ...
    }

    free(sectionHeaders);
#endif

    return ParseELFResult::Ok;
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
        default: fprintf(stderr, "%s unimplemented", InstructionName(type)); return false;
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
        break; case InstructionType::MUL:   intRegs.Write(ins.Rtyp.rd, ( int32_t)((( int64_t)intRegs.Read< int32_t>(ins.Rtyp.rs1) * ( int64_t)intRegs.Read< int32_t>(ins.Rtyp.rs2))));
        break; case InstructionType::MULH:  intRegs.Write(ins.Rtyp.rd, ( int32_t)((( int64_t)intRegs.Read< int32_t>(ins.Rtyp.rs1) * ( int64_t)intRegs.Read< int32_t>(ins.Rtyp.rs2)) >> 32UL));
        break; case InstructionType::MULHSU:intRegs.Write(ins.Rtyp.rd, ( int32_t)((( int64_t)intRegs.Read< int32_t>(ins.Rtyp.rs1) * (uint64_t)intRegs.Read<uint32_t>(ins.Rtyp.rs2)) >> 32UL));
        break; case InstructionType::MULHU: intRegs.Write(ins.Rtyp.rd, ( int32_t)(((uint64_t)intRegs.Read<uint32_t>(ins.Rtyp.rs1) * (uint64_t)intRegs.Read<uint32_t>(ins.Rtyp.rs2)) >> 32UL));
        break; case InstructionType::DIVU: {
            uint64_t divisor = intRegs.Read<uint32_t>(ins.Rtyp.rs2);
            uint64_t dividend = intRegs.Read<uint32_t>(ins.Rtyp.rs1);
            uint32_t quotient = (divisor == 0) ? 0xFFFFFFFFUL : (uint32_t) (dividend / divisor);
            intRegs.Write(ins.Rtyp.rd, quotient);
        }
        break; case InstructionType::DIV: {
            int64_t divisor = intRegs.Read< int32_t>(ins.Rtyp.rs2);
            int64_t dividend = intRegs.Read< int32_t>(ins.Rtyp.rs1);
            int32_t quotient =  (divisor == 0) ? -1L : (int32_t) (dividend / divisor);
            intRegs.Write(ins.Rtyp.rd, quotient);
        }
        break; case InstructionType::REM: {
            int64_t divisor = intRegs.Read< int32_t>(ins.Rtyp.rs2);
            int64_t dividend = intRegs.Read< int32_t>(ins.Rtyp.rs1);
            int32_t remainder = (int32_t) ((divisor == 0) ? dividend : dividend % divisor);
            intRegs.Write(ins.Rtyp.rd, remainder);
        }
        break; case InstructionType::REMU:  {
            uint64_t divisor = intRegs.Read<uint32_t>(ins.Rtyp.rs2);
            uint64_t dividend = intRegs.Read<uint32_t>(ins.Rtyp.rs1);
            uint32_t remainder = (uint32_t) ((divisor == 0) ? dividend : dividend % divisor);
            intRegs.Write(ins.Rtyp.rd, remainder);
        }
        break; case InstructionType::FLW:     fltRegs.Write(ins.Ityp.rd, memory.Read<float>(intRegs.Read<uint32_t>(ins.Ityp.rs1) + SignExtend(ins.Ityp.imm11_0, 12)));
        break; case InstructionType::FSW:     memory.Write(intRegs.Read<uint32_t>(ins.Styp.rs1) + SignExtend(ins.Styp.imm(), 12), fltRegs.Read(ins.Styp.rs2));
        break; case InstructionType::FMADDS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = (fltRegs.Read(ins.R4typ.rs1) * fltRegs.Read(ins.R4typ.rs2)) + fltRegs.Read(ins.R4typ.rs3);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FMSUBS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = (fltRegs.Read(ins.R4typ.rs1) * fltRegs.Read(ins.R4typ.rs2)) - fltRegs.Read(ins.R4typ.rs3);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FNMSUBS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = -(fltRegs.Read(ins.R4typ.rs1) * fltRegs.Read(ins.R4typ.rs2)) + fltRegs.Read(ins.R4typ.rs3);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FNMADDS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = -(fltRegs.Read(ins.R4typ.rs1) * fltRegs.Read(ins.R4typ.rs2)) - fltRegs.Read(ins.R4typ.rs3);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FADDS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1) + fltRegs.Read(ins.Rtyp.rs2);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FSUBS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1) - fltRegs.Read(ins.Rtyp.rs2);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FMULS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1) * fltRegs.Read(ins.Rtyp.rs2);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FDIVS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1) / fltRegs.Read(ins.Rtyp.rs2);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FSQRTS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = sqrtf(fltRegs.Read(ins.Rtyp.rs1));
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            if (std::isnan(x)) x = bit_cast<float>(0x7FC00000U);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FSGNJS:  {
            uint32_t rs1_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs1));
            uint32_t rs2_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs2));
            fltRegs.Write(ins.Rtyp.rd, bit_cast<float>((rs2_u32 & 0x80000000) | (rs1_u32 & ~0x80000000)));
        }
        break; case InstructionType::FSGNJNS: {
            uint32_t rs1_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs1));
            uint32_t rs2_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs2));
            fltRegs.Write(ins.Rtyp.rd, bit_cast<float>((~rs2_u32 & 0x80000000) | (rs1_u32 & ~0x80000000)));
        }
        break; case InstructionType::FSGNJXS: {
            uint32_t rs1_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs1));
            uint32_t rs2_u32 = bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs2));
            fltRegs.Write(ins.Rtyp.rd, bit_cast<float>((rs2_u32 & 0x80000000) ^ rs1_u32));
        }
        break; case InstructionType::FMINS: {
            feclearexcept(FE_ALL_EXCEPT);
            float a = fltRegs.Read(ins.Rtyp.rs1);
            float b = fltRegs.Read(ins.Rtyp.rs2);
            float x = fminf(a, b);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            if (std::isnan(a)) x = b;
            if (std::isnan(b)) x = a;
            if (std::isnan(x)) {
                x = bit_cast<float>(0x7FC00000U);
            }
            if (a == 0.0f && b == 0.0f && bit_cast<uint32_t>(a) != bit_cast<uint32_t>(b)) {
                x = (bit_cast<uint32_t>(a) == bit_cast<uint32_t>(0.0f)) ? b : a;
            }
            csr.Write(CSR_fflags, flags);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FMAXS: {
            feclearexcept(FE_ALL_EXCEPT);
            float a = fltRegs.Read(ins.Rtyp.rs1);
            float b = fltRegs.Read(ins.Rtyp.rs2);
            float x = fmaxf(a, b);
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            // This matters because the bit patterns of NAN are implementation defined
            if (std::isnan(a)) x = b;
            if (std::isnan(b)) x = a;
            if (std::isnan(x)) {
                x = bit_cast<float>(0x7FC00000U);
            }
            // This matters because fminf(-0.0f, 0.0f) is implementation defined
            if (a == 0.0f && b == 0.0f && bit_cast<uint32_t>(a) != bit_cast<uint32_t>(b)) {
                x = (bit_cast<uint32_t>(a) == bit_cast<uint32_t>(0.0f)) ? a : b;
            }
            csr.Write(CSR_fflags, flags);
            fltRegs.Write(ins.Rtyp.rd, x);
        }
        break; case InstructionType::FCVTWS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1);
            int32_t y = 0;
            uint32_t flags = 0;
            if (std::isnan(x) || x > static_cast<float>(INT_MAX)) {
                flags |= 0b10000;
                y = INT_MAX;
            }
            else if (x < static_cast<float>(INT_MIN)) {
                flags |= 0b10000;
                y = INT_MIN;
            }
            else y = static_cast<int32_t>(x);
            flags |= (bool(fetestexcept(FE_INEXACT))   << 0)
                  |  (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                  |  (bool(fetestexcept(FE_OVERFLOW))  << 2)
                  |  (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                  |  (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            intRegs.Write(ins.Rtyp.rd, y);
        }
        break; case InstructionType::FCVTWUS: {
            feclearexcept(FE_ALL_EXCEPT);
            float x = fltRegs.Read(ins.Rtyp.rs1);
            uint32_t y = 0;
            uint32_t flags = 0;
            if (std::isnan(x) || x > static_cast<float>(UINT_MAX))  {
                flags |= 0b10000;
                y = UINT_MAX;
            }
            else if (x <= -1.0f)  {
                flags |= 0b10000;
                y = 0;
            }
            else y = static_cast<uint32_t>(x);
            flags |= (bool(fetestexcept(FE_INEXACT))   << 0)
                  |  (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                  |  (bool(fetestexcept(FE_OVERFLOW))  << 2)
                  |  (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                  |  (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
            intRegs.Write(ins.Rtyp.rd, y);
        }
        break; case InstructionType::FMVXW:   intRegs.Write(ins.Rtyp.rd, bit_cast<uint32_t>(fltRegs.Read(ins.Rtyp.rs1)));
        break; case InstructionType::FEQS: {
            feclearexcept(FE_ALL_EXCEPT);
            intRegs.Write(ins.Rtyp.rd, fltRegs.Read(ins.Rtyp.rs1) == fltRegs.Read(ins.Rtyp.rs2));
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
        }
        break; case InstructionType::FLTS: {
            feclearexcept(FE_ALL_EXCEPT);
            intRegs.Write(ins.Rtyp.rd, fltRegs.Read(ins.Rtyp.rs1) <  fltRegs.Read(ins.Rtyp.rs2));
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
        }
        break; case InstructionType::FLES: {
            feclearexcept(FE_ALL_EXCEPT);
            intRegs.Write(ins.Rtyp.rd, fltRegs.Read(ins.Rtyp.rs1) <= fltRegs.Read(ins.Rtyp.rs2));
            uint32_t flags = (bool(fetestexcept(FE_INEXACT))   << 0)
                           | (bool(fetestexcept(FE_UNDERFLOW)) << 1)
                           | (bool(fetestexcept(FE_OVERFLOW))  << 2)
                           | (bool(fetestexcept(FE_DIVBYZERO)) << 3)
                           | (bool(fetestexcept(FE_INVALID))   << 4);
            csr.Write(CSR_fflags, flags);
        }
        break; case InstructionType::FCLASSS: {
            float x = fltRegs.Read(ins.Rtyp.rs1);
            int cls = std::fpclassify(x);
            uint32_t result = 0;
            if (cls == FP_INFINITE) {
                result = (x < 0.0f) ? 0 : 7;
            }
            else if (cls == FP_NAN) {
                result = (bit_cast<uint32_t>(x) == 0x7FC00000U) ? 9 : 8;
            }
            else if (cls == FP_NORMAL) {
                result = (x < 0.0f) ? 1 : 6;
            }
            else if (cls == FP_SUBNORMAL) {
                result = (x < 0.0f) ? 2 : 5;
            }
            else if (cls == FP_ZERO) {
                result = (copysignf(1.0f, x) < 0) ? 3 : 4;
            }
            intRegs.Write(ins.Rtyp.rd, 1U << result);
        }
        break; case InstructionType::FCVTSW:  fltRegs.Write(ins.Rtyp.rd, (float)intRegs.Read< int32_t>(ins.Rtyp.rs1));
        break; case InstructionType::FCVTSWU: fltRegs.Write(ins.Rtyp.rd, (float)intRegs.Read<uint32_t>(ins.Rtyp.rs1));
        break; case InstructionType::FMVWX:   fltRegs.Write(ins.Rtyp.rd, bit_cast<float>(intRegs.Read<uint32_t>(ins.Rtyp.rs1)));
        break;
    }
    return true;
}