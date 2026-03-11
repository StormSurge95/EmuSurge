#include "NES_CPU.h"
#include "../../core/Helpers.h"

#include <sstream>

NES_CPU::NES_CPU() {
    using a = NES_CPU;
    lookup = {
        /*0x00                           0x01                           0x02                           0x03                          0x04                           0x05                           0x06                           0x07                          0x08                           0x09                           0x0A                           0x0B                           0x0C                           0x0D                           0x0E                           0x0F*/
        { "BRK",&a::BRK,&a::IMM,7,2 }, { "ORA",&a::ORA,&a::IZX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZP0,3,2 }, { "ORA",&a::ORA,&a::ZP0,3,2 }, { "ASL",&a::ASL,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PHP",&a::PHP,&a::IMP,3,1 }, { "ORA",&a::ORA,&a::IMM,2,2 }, { "ASL",&a::ASL,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "NOP",&a::NOP,&a::ABS,4,3 }, { "ORA",&a::ORA,&a::ABS,4,3 }, { "ASL",&a::ASL,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x10                           0x11                           0x12                           0x13                          0x14                           0x15                           0x16                           0x17                          0x18                           0x19                           0x1A                           0x1B                           0x1C                           0x1D                           0x1E                           0x1F*/
        { "BPL",&a::BPL,&a::REL,2,2 }, { "ORA",&a::ORA,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "ORA",&a::ORA,&a::ZPX,4,2 }, { "ASL",&a::ASL,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLC",&a::CLC,&a::IMP,2,1 }, { "ORA",&a::ORA,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "ORA",&a::ORA,&a::ABX,4,3 }, { "ASL",&a::ASL,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x20                           0x21                           0x22                           0x23                          0x24                           0x25                           0x26                           0x27                          0x28                           0x29                           0x2A                           0x2B                           0x2C                           0x2D                           0x2E                           0x2F*/
        { "JSR",&a::JSR,&a::ABS,6,3 }, { "AND",&a::AND,&a::IZX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "BIT",&a::BIT,&a::ZP0,3,2 }, { "AND",&a::AND,&a::ZP0,3,2 }, { "ROL",&a::ROL,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PLP",&a::PLP,&a::IMP,4,1 }, { "AND",&a::AND,&a::IMM,2,2 }, { "ROL",&a::ROL,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "BIT",&a::BIT,&a::ABS,4,3 }, { "AND",&a::AND,&a::ABS,4,3 }, { "ROL",&a::ROL,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x30                           0x31                           0x32                           0x33                          0x34                           0x35                           0x36                           0x37                          0x38                           0x39                           0x3A                           0x3B                           0x3C                           0x3D                           0x3E                           0x3F*/
        { "BMI",&a::BMI,&a::REL,2,2 }, { "AND",&a::AND,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "AND",&a::AND,&a::ZPX,4,2 }, { "ROL",&a::ROL,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SEC",&a::SEC,&a::IMP,2,1 }, { "AND",&a::AND,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "AND",&a::AND,&a::ABX,4,3 }, { "ROL",&a::ROL,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x40                           0x41                           0x42                           0x43                          0x44                           0x45                           0x46                           0x47                          0x48                           0x49                           0x4A                           0x4B                           0x4C                           0x4D                           0x4E                           0x4F*/
        { "RTI",&a::RTI,&a::IMP,6,1 }, { "EOR",&a::EOR,&a::IZX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZP0,3,2 }, { "EOR",&a::EOR,&a::ZP0,3,2 }, { "LSR",&a::LSR,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PHA",&a::PHA,&a::IMP,3,1 }, { "EOR",&a::EOR,&a::IMM,2,2 }, { "LSR",&a::LSR,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "JMP",&a::JMP,&a::ABS,3,3 }, { "EOR",&a::EOR,&a::ABS,4,3 }, { "LSR",&a::LSR,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x50                           0x51                           0x52                           0x53                          0x54                           0x55                           0x56                           0x57                          0x58                           0x59                           0x5A                           0x5B                           0x5C                           0x5D                           0x5E                           0x5F*/
        { "BVC",&a::BVC,&a::REL,2,2 }, { "EOR",&a::EOR,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "EOR",&a::EOR,&a::ZPX,4,2 }, { "LSR",&a::LSR,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLI",&a::CLI,&a::IMP,2,1 }, { "EOR",&a::EOR,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "EOR",&a::EOR,&a::ABX,4,3 }, { "LSR",&a::LSR,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x60                           0x61                           0x62                           0x63                          0x64                           0x65                           0x66                           0x67                          0x68                           0x69                           0x6A                           0x6B                           0x6C                           0x6D                           0x6E                           0x6F*/
        { "RTS",&a::RTS,&a::IMP,6,1 }, { "ADC",&a::ADC,&a::IZX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZP0,3,2 }, { "ADC",&a::ADC,&a::ZP0,3,2 }, { "ROR",&a::ROR,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PLA",&a::PLA,&a::IMP,4,1 }, { "ADC",&a::ADC,&a::IMM,2,2 }, { "ROR",&a::ROR,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "JMP",&a::JMP,&a::IND,5,3 }, { "ADC",&a::ADC,&a::ABS,4,3 }, { "ROR",&a::ROR,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x70                           0x71                           0x72                           0x73                          0x74                           0x75                           0x76                           0x77                          0x78                           0x79                           0x7A                           0x7B                           0x7C                           0x7D                           0x7E                           0x7F*/
        { "BVS",&a::BVS,&a::REL,2,2 }, { "ADC",&a::ADC,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "ADC",&a::ADC,&a::ZPX,4,2 }, { "ROR",&a::ROR,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SEI",&a::SEI,&a::IMP,2,1 }, { "ADC",&a::ADC,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "ADC",&a::ADC,&a::ABX,4,3 }, { "ROR",&a::ROR,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x80                           0x81                           0x82                           0x83                          0x84                           0x85                           0x86                           0x87                          0x88                           0x89                           0x8A                           0x8B                           0x8C                           0x8D                           0x8E                           0x8F*/
        { "NOP",&a::NOP,&a::IMM,2,2 }, { "STA",&a::STA,&a::IZX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "STY",&a::STY,&a::ZP0,3,2 }, { "STA",&a::STA,&a::ZP0,3,2 }, { "STX",&a::STX,&a::ZP0,3,2 }, { "???",&a::XXX,&a::IMP,3,1 },{ "DEY",&a::DEY,&a::IMP,2,1 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "TXA",&a::TXA,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "STY",&a::STY,&a::ABS,4,3 }, { "STA",&a::STA,&a::ABS,4,3 }, { "STX",&a::STX,&a::ABS,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0x90                           0x91                           0x92                           0x93                          0x94                           0x95                           0x96                           0x97                          0x98                           0x99                           0x9A                           0x9B                           0x9C                           0x9D                           0x9E                           0x9F*/
        { "BCC",&a::BCC,&a::REL,2,2 }, { "STA",&a::STA,&a::IZY,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,6,1 },{ "STY",&a::STY,&a::ZPX,4,2 }, { "STA",&a::STA,&a::ZPX,4,2 }, { "STX",&a::STX,&a::ZPY,4,2 }, { "???",&a::XXX,&a::IMP,4,1 },{ "TYA",&a::TYA,&a::IMP,2,1 }, { "STA",&a::STA,&a::ABY,5,3 }, { "TXS",&a::TXS,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,5,1 }, { "NOP",&a::NOP,&a::ABX,5,3 }, { "STA",&a::STA,&a::ABX,5,3 }, { "???",&a::XXX,&a::IMP,5,1 }, { "???",&a::XXX,&a::IMP,5,1 },
        /*0xA0                           0xA1                           0xA2                           0xA3                          0xA4                           0xA5                           0xA6                           0xA7                          0xA8                           0xA9                           0xAA                           0xAB                           0xAC                           0xAD                           0xAE                           0xAF*/
        { "LDY",&a::LDY,&a::IMM,2,2 }, { "LDA",&a::LDA,&a::IZX,6,2 }, { "LDX",&a::LDX,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "LDY",&a::LDY,&a::ZP0,3,2 }, { "LDA",&a::LDA,&a::ZP0,3,2 }, { "LDX",&a::LDX,&a::ZP0,3,2 }, { "???",&a::XXX,&a::IMP,3,1 },{ "TAY",&a::TAY,&a::IMP,2,1 }, { "LDA",&a::LDA,&a::IMM,2,2 }, { "TAX",&a::TAX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "LDY",&a::LDY,&a::ABS,4,3 }, { "LDA",&a::LDA,&a::ABS,4,3 }, { "LDX",&a::LDX,&a::ABS,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0xB0                           0xB1                           0xB2                           0xB3                          0xB4                           0xB5                           0xB6                           0xB7                          0xB8                           0xB9                           0xBA                           0xBB                           0xBC                           0xBD                           0xBE                           0xBF*/
        { "BCS",&a::BCS,&a::REL,2,2 }, { "LDA",&a::LDA,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,5,1 },{ "LDY",&a::LDY,&a::ZPX,4,2 }, { "LDA",&a::LDA,&a::ZPX,4,2 }, { "LDX",&a::LDX,&a::ZPY,4,2 }, { "???",&a::XXX,&a::IMP,4,1 },{ "CLV",&a::CLV,&a::IMP,2,1 }, { "LDA",&a::LDA,&a::ABY,4,3 }, { "TSX",&a::TSX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,4,1 }, { "LDY",&a::LDY,&a::ABX,4,3 }, { "LDA",&a::LDA,&a::ABX,4,3 }, { "LDX",&a::LDX,&a::ABY,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0xC0                           0xC1                           0xC2                           0xC3                          0xC4                           0xC5                           0xC6                           0xC7                          0xC8                           0xC9                           0xCA                           0xCB                          0xCC                            0xCD                           0xCE                           0xCF*/
        { "CPY",&a::CPY,&a::IMM,2,2 }, { "CMP",&a::CMP,&a::IZX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,8,1 },{ "CPY",&a::CPY,&a::ZP0,3,2 }, { "CMP",&a::CMP,&a::ZP0,3,2 }, { "DEC",&a::DEC,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "INY",&a::INY,&a::IMP,2,1 }, { "CMP",&a::CMP,&a::IMM,2,2 }, { "DEX",&a::DEX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "CPY",&a::CPY,&a::ABS,4,3 }, { "CMP",&a::CMP,&a::ABS,4,3 }, { "DEC",&a::DEC,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0xD0                           0xD1                           0xD2                           0xD3                          0xD4                           0xD5                           0xD6                           0xD7                          0xD8                           0xD9                           0xDA                           0xDB                          0xDC                            0xDD                           0xDE                           0xDF*/
        { "BNE",&a::BNE,&a::REL,2,2 }, { "CMP",&a::CMP,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "CMP",&a::CMP,&a::ZPX,4,2 }, { "DEC",&a::DEC,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLD",&a::CLD,&a::IMP,2,1 }, { "CMP",&a::CMP,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "CMP",&a::CMP,&a::ABX,4,3 }, { "DEC",&a::DEC,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0xE0                           0xE1                           0xE2                           0xE3                          0xE4                           0xE5                           0xE6                           0xE7                          0xE8                           0xE9                           0xEA                           0xEB                          0xEC                            0xED                           0xEE                           0xEF*/
        { "CPX",&a::CPX,&a::IMM,2,2 }, { "SBC",&a::SBC,&a::IZX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,8,1 },{ "CPX",&a::CPX,&a::ZP0,3,2 }, { "SBC",&a::SBC,&a::ZP0,3,2 }, { "INC",&a::INC,&a::ZP0,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "INX",&a::INX,&a::IMP,2,1 }, { "SBC",&a::SBC,&a::IMM,2,2 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "CPX",&a::CPX,&a::ABS,4,3 }, { "SBC",&a::SBC,&a::ABS,4,3 }, { "INC",&a::INC,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0xF0                           0xF1                           0xF2                           0xF3                          0xF4                           0xF5                           0xF6                           0xF7                          0xF8                           0xF9                           0xFA                           0xFB                          0xFC                            0xFD                           0xFE                           0xFF*/
        { "BEQ",&a::BEQ,&a::REL,2,2 }, { "SBC",&a::SBC,&a::IZY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "SBC",&a::SBC,&a::ZPX,4,2 }, { "INC",&a::INC,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SED",&a::SED,&a::IMP,2,1 }, { "SBC",&a::SBC,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "SBC",&a::SBC,&a::ABX,4,3 }, { "INC",&a::INC,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 }
    };
}

uint8_t NES_CPU::read(uint16_t addr, bool readonly) {
    return bus->read(addr);
}

void NES_CPU::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

uint8_t NES_CPU::fetch() {
    if (!(lookup[opcode].addrmode == &NES_CPU::IMP))
        fetched = read(addrAbs);

    return fetched;
}

void NES_CPU::reset() {
    a = 0;
    x = 0;
    y = 0;

    sp = 0xFD;
    status = I | U;

    uint16_t lo = read(0xFFFC);
    uint16_t hi = read(0xFFFD);

    pc = (hi << 8) | lo;

    pc = 0xC000;

    addrRel = 0;
    addrAbs = 0;
    fetched = 0;

    cycles = 8;
}

void NES_CPU::clock() {
    if (cycles == 0) {
        if (traceStream) *traceStream << trace();
        opcode = read(pc++);
        Instruction& inst = lookup[opcode];

        cycles = inst.cycles;

        uint8_t e1 = (this->*inst.addrmode)();
        uint8_t e2 = (this->*inst.operate)();

        cycles += (e1 & e2);
    }

    cycles--;
}

std::string NES_CPU::disassembleInst(uint16_t addr) {
    uint16_t line_addr = addr;
    uint8_t oc = read(addr++, true);

    std::string sInst = std::string(lookup[oc].name) + " ";

    if (lookup[oc].addrmode == &NES_CPU::IMM) {
        uint8_t val = read(addr++, true);

        sInst += "#$" + hex(val, 2);
    } else if (lookup[oc].addrmode == &NES_CPU::ZP0) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2);
    } else if (lookup[oc].addrmode == &NES_CPU::ZPX) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2) + ",X";
    } else if (lookup[oc].addrmode == &NES_CPU::ZPY) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2) + ",Y";
    } else if (lookup[oc].addrmode == &NES_CPU::ABS) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4);
    } else if (lookup[oc].addrmode == &NES_CPU::ABX) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4) + ",X";
    } else if (lookup[oc].addrmode == &NES_CPU::ABY) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4) + ",Y";
    } else if (lookup[oc].addrmode == &NES_CPU::IND) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "($" + hex(val, 4) + ")";
    } else if (lookup[oc].addrmode == &NES_CPU::IZX) {
        uint8_t val = read(addr++, true);

        sInst += "($" + hex(val, 2) + ",X)";
    } else if (lookup[oc].addrmode == &NES_CPU::IZY) {
        uint8_t val = read(addr++, true);

        sInst += "($" + hex(val, 2) + "),Y";
    } else if (lookup[oc].addrmode == &NES_CPU::REL) {
        int8_t offset = read(addr++, true);
        uint16_t target = addr + offset;

        sInst += "$" + hex(target, 4);
    }

    return sInst;
}

std::string NES_CPU::formatInst() {
    std::stringstream ss;

    ss << "A:" << hex(a, 2) << " ";
    ss << "X:" << hex(x, 2) << " ";
    ss << "Y:" << hex(y, 2) << " ";
    ss << "P:" << hex(status, 2) << " ";
    ss << "SP:" << hex(sp, 2);

    return ss.str();
}

std::string NES_CPU::trace() {
    uint16_t PC = pc;

    uint8_t oc = read(pc);

    const Instruction& inst = lookup[oc];

    std::stringstream ss;

    // program counter
    ss << hex(PC, 4) << "  ";

    // raw opcode bytes
    uint8_t b1 = read(PC + 1, true);
    uint8_t b2 = read(PC + 2, true);

    ss << hex(oc, 2) << " ";

    if (inst.bytes >= 2)
        ss << hex(b1, 2) << " ";
    else
        ss << "   ";

    if (inst.bytes == 3)
        ss << hex(b2, 2);
    else
        ss << "  ";

    ss << "  ";

    ss << disassembleInst(PC);

    while (ss.str().size() < 48) {
        ss << " ";
    }

    ss << formatInst();

    ss << std::endl;

    return ss.str();
}

void NES_CPU::IRQ() {
    if (getFlag(I) == 0) {
        write(0x0100 + sp--, (pc >> 8) & 0x00FF);
        write(0x0100 + sp--, pc & 0x00FF);
        setFlag(B, 0);
        setFlag(U, 1);
        setFlag(I, 1);
        write(0x0100 + sp--, status);
        uint16_t lo = read(0xFFFE);
        uint16_t hi = read(0xFFFF);
        pc = (hi << 8) | lo;
        cycles = 7;
    }
}

void NES_CPU::NMI() {
    write(0x0100 + sp--, (pc >> 8) & 0x00FF);
    write(0x0100 + sp--, pc & 0x00FF);
    setFlag(B, 0);
    setFlag(U, 1);
    setFlag(I, 1);
    write(0x0100 + sp--, status);
    uint16_t lo = read(0xFFFA);
    uint16_t hi = read(0xFFFB);
    pc = (hi << 8) | lo;
    cycles = 8;
}