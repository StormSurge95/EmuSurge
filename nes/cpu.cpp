#include "../common/format_utils.h"
#include "bus.h"
#include "cpu.h"

#include <cstdint>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

CPU::CPU() {
    using a = CPU;

    lookup = {
        /*0x00                           0x01                           0x02                           0x03                          0x04                           0x05                           0x06                           0x07                          0x08                           0x09                           0x0A                           0x0B                           0x0C                           0x0D                           0x0E                           0x0F*/    
        { "BRK",&a::BRK,&a::IMM,7,2 }, { "ORA",&a::ORA,&a::IIX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPI,3,2 }, { "ORA",&a::ORA,&a::ZPI,3,2 }, { "ASL",&a::ASL,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PHP",&a::PHP,&a::IMP,3,1 }, { "ORA",&a::ORA,&a::IMM,2,2 }, { "ASL",&a::ASL,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "NOP",&a::NOP,&a::ABS,4,3 }, { "ORA",&a::ORA,&a::ABS,4,3 }, { "ASL",&a::ASL,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x10                           0x11                           0x12                           0x13                          0x14                           0x15                           0x16                           0x17                          0x18                           0x19                           0x1A                           0x1B                           0x1C                           0x1D                           0x1E                           0x1F*/
        { "BPL",&a::BPL,&a::REL,2,2 }, { "ORA",&a::ORA,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "ORA",&a::ORA,&a::ZPX,4,2 }, { "ASL",&a::ASL,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLC",&a::CLC,&a::IMP,2,1 }, { "ORA",&a::ORA,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "ORA",&a::ORA,&a::ABX,4,3 }, { "ASL",&a::ASL,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x20                           0x21                           0x22                           0x23                          0x24                           0x25                           0x26                           0x27                          0x28                           0x29                           0x2A                           0x2B                           0x2C                           0x2D                           0x2E                           0x2F*/
        { "JSR",&a::JSR,&a::ABS,6,3 }, { "AND",&a::AND,&a::IIX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "BIT",&a::BIT,&a::ZPI,3,2 }, { "AND",&a::AND,&a::ZPI,3,2 }, { "ROL",&a::ROL,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PLP",&a::PLP,&a::IMP,4,1 }, { "AND",&a::AND,&a::IMM,2,2 }, { "ROL",&a::ROL,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "BIT",&a::BIT,&a::ABS,4,3 }, { "AND",&a::AND,&a::ABS,4,3 }, { "ROL",&a::ROL,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x30                           0x31                           0x32                           0x33                          0x34                           0x35                           0x36                           0x37                          0x38                           0x39                           0x3A                           0x3B                           0x3C                           0x3D                           0x3E                           0x3F*/
        { "BMI",&a::BMI,&a::REL,2,2 }, { "AND",&a::AND,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "AND",&a::AND,&a::ZPX,4,2 }, { "ROL",&a::ROL,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SEC",&a::SEC,&a::IMP,2,1 }, { "AND",&a::AND,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "AND",&a::AND,&a::ABX,4,3 }, { "ROL",&a::ROL,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x40                           0x41                           0x42                           0x43                          0x44                           0x45                           0x46                           0x47                          0x48                           0x49                           0x4A                           0x4B                           0x4C                           0x4D                           0x4E                           0x4F*/
        { "RTI",&a::RTI,&a::IMP,6,1 }, { "EOR",&a::EOR,&a::IIX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPI,3,2 }, { "EOR",&a::EOR,&a::ZPI,3,2 }, { "LSR",&a::LSR,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PHA",&a::PHA,&a::IMP,3,1 }, { "EOR",&a::EOR,&a::IMM,2,2 }, { "LSR",&a::LSR,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "JMP",&a::JMP,&a::ABS,3,3 }, { "EOR",&a::EOR,&a::ABS,4,3 }, { "LSR",&a::LSR,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x50                           0x51                           0x52                           0x53                          0x54                           0x55                           0x56                           0x57                          0x58                           0x59                           0x5A                           0x5B                           0x5C                           0x5D                           0x5E                           0x5F*/
        { "BVC",&a::BVC,&a::REL,2,2 }, { "EOR",&a::EOR,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "EOR",&a::EOR,&a::ZPX,4,2 }, { "LSR",&a::LSR,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLI",&a::CLI,&a::IMP,2,1 }, { "EOR",&a::EOR,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "EOR",&a::EOR,&a::ABX,4,3 }, { "LSR",&a::LSR,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x60                           0x61                           0x62                           0x63                          0x64                           0x65                           0x66                           0x67                          0x68                           0x69                           0x6A                           0x6B                           0x6C                           0x6D                           0x6E                           0x6F*/
        { "RTS",&a::RTS,&a::IMP,6,1 }, { "ADC",&a::ADC,&a::IIX,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPI,3,2 }, { "ADC",&a::ADC,&a::ZPI,3,2 }, { "ROR",&a::ROR,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "PLA",&a::PLA,&a::IMP,4,1 }, { "ADC",&a::ADC,&a::IMM,2,2 }, { "ROR",&a::ROR,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "JMP",&a::JMP,&a::IIY,5,3 }, { "ADC",&a::ADC,&a::ABS,4,3 }, { "ROR",&a::ROR,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0x70                           0x71                           0x72                           0x73                          0x74                           0x75                           0x76                           0x77                          0x78                           0x79                           0x7A                           0x7B                           0x7C                           0x7D                           0x7E                           0x7F*/
        { "BVS",&a::BVS,&a::REL,2,2 }, { "ADC",&a::ADC,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "ADC",&a::ADC,&a::ZPX,4,2 }, { "ROR",&a::ROR,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SEI",&a::SEI,&a::IMP,2,1 }, { "ADC",&a::ADC,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "ADC",&a::ADC,&a::ABX,4,3 }, { "ROR",&a::ROR,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0x80                           0x81                           0x82                           0x83                          0x84                           0x85                           0x86                           0x87                          0x88                           0x89                           0x8A                           0x8B                           0x8C                           0x8D                           0x8E                           0x8F*/
        { "NOP",&a::NOP,&a::IMM,2,2 }, { "STA",&a::STA,&a::IIX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "STY",&a::STY,&a::ZPI,3,2 }, { "STA",&a::STA,&a::ZPI,3,2 }, { "STX",&a::STX,&a::ZPI,3,2 }, { "???",&a::XXX,&a::IMP,3,1 },{ "DEY",&a::DEY,&a::IMP,2,1 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "TXA",&a::TXA,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "STY",&a::STY,&a::ABS,4,3 }, { "STA",&a::STA,&a::ABS,4,3 }, { "STX",&a::STX,&a::ABS,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0x90                           0x91                           0x92                           0x93                          0x94                           0x95                           0x96                           0x97                          0x98                           0x99                           0x9A                           0x9B                           0x9C                           0x9D                           0x9E                           0x9F*/
        { "BCC",&a::BCC,&a::REL,2,2 }, { "STA",&a::STA,&a::IIY,6,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,6,1 },{ "STY",&a::STY,&a::ZPX,4,2 }, { "STA",&a::STA,&a::ZPX,4,2 }, { "STX",&a::STX,&a::ZPY,4,2 }, { "???",&a::XXX,&a::IMP,4,1 },{ "TYA",&a::TYA,&a::IMP,2,1 }, { "STA",&a::STA,&a::ABY,5,3 }, { "TXS",&a::TXS,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,5,1 }, { "NOP",&a::NOP,&a::ABX,5,3 }, { "STA",&a::STA,&a::ABX,5,3 }, { "???",&a::XXX,&a::IMP,5,1 }, { "???",&a::XXX,&a::IMP,5,1 },
        /*0xA0                           0xA1                           0xA2                           0xA3                          0xA4                           0xA5                           0xA6                           0xA7                          0xA8                           0xA9                           0xAA                           0xAB                           0xAC                           0xAD                           0xAE                           0xAF*/
        { "LDY",&a::LDY,&a::IMM,2,2 }, { "LDA",&a::LDA,&a::IIX,6,2 }, { "LDX",&a::LDX,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "LDY",&a::LDY,&a::ZPI,3,2 }, { "LDA",&a::LDA,&a::ZPI,3,2 }, { "LDX",&a::LDX,&a::ZPI,3,2 }, { "???",&a::XXX,&a::IMP,3,1 },{ "TAY",&a::TAY,&a::IMP,2,1 }, { "LDA",&a::LDA,&a::IMM,2,2 }, { "TAX",&a::TAX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "LDY",&a::LDY,&a::ABS,4,3 }, { "LDA",&a::LDA,&a::ABS,4,3 }, { "LDX",&a::LDX,&a::ABS,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0xB0                           0xB1                           0xB2                           0xB3                          0xB4                           0xB5                           0xB6                           0xB7                          0xB8                           0xB9                           0xBA                           0xBB                           0xBC                           0xBD                           0xBE                           0xBF*/
        { "BCS",&a::BCS,&a::REL,2,2 }, { "LDA",&a::LDA,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,5,1 },{ "LDY",&a::LDY,&a::ZPX,4,2 }, { "LDA",&a::LDA,&a::ZPX,4,2 }, { "LDX",&a::LDX,&a::ZPY,4,2 }, { "???",&a::XXX,&a::IMP,4,1 },{ "CLV",&a::CLV,&a::IMP,2,1 }, { "LDA",&a::LDA,&a::ABY,4,3 }, { "TSX",&a::TSX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,4,1 }, { "LDY",&a::LDY,&a::ABX,4,3 }, { "LDA",&a::LDA,&a::ABX,4,3 }, { "LDX",&a::LDX,&a::ABY,4,3 }, { "???",&a::XXX,&a::IMP,4,1 },
        /*0xC0                           0xC1                           0xC2                           0xC3                          0xC4                           0xC5                           0xC6                           0xC7                          0xC8                           0xC9                           0xCA                           0xCB                          0xCC                            0xCD                           0xCE                           0xCF*/
        { "CPY",&a::CPY,&a::IMM,2,2 }, { "CMP",&a::CMP,&a::IIX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,8,1 },{ "CPY",&a::CPY,&a::ZPI,3,2 }, { "CMP",&a::CMP,&a::ZPI,3,2 }, { "DEC",&a::DEC,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "INY",&a::INY,&a::IMP,2,1 }, { "CMP",&a::CMP,&a::IMM,2,2 }, { "DEX",&a::DEX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "CPY",&a::CPY,&a::ABS,4,3 }, { "CMP",&a::CMP,&a::ABS,4,3 }, { "DEC",&a::DEC,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0xD0                           0xD1                           0xD2                           0xD3                          0xD4                           0xD5                           0xD6                           0xD7                          0xD8                           0xD9                           0xDA                           0xDB                          0xDC                            0xDD                           0xDE                           0xDF*/
        { "BNE",&a::BNE,&a::REL,2,2 }, { "CMP",&a::CMP,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "CMP",&a::CMP,&a::ZPX,4,2 }, { "DEC",&a::DEC,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "CLD",&a::CLD,&a::IMP,2,1 }, { "CMP",&a::CMP,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "CMP",&a::CMP,&a::ABX,4,3 }, { "DEC",&a::DEC,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 },
        /*0xE0                           0xE1                           0xE2                           0xE3                          0xE4                           0xE5                           0xE6                           0xE7                          0xE8                           0xE9                           0xEA                           0xEB                          0xEC                            0xED                           0xEE                           0xEF*/
        { "CPX",&a::CPX,&a::IMM,2,2 }, { "SBC",&a::SBC,&a::IIX,6,2 }, { "NOP",&a::NOP,&a::IMM,2,2 }, { "???",&a::XXX,&a::IMP,8,1 },{ "CPX",&a::CPX,&a::ZPI,3,2 }, { "SBC",&a::SBC,&a::ZPI,3,2 }, { "INC",&a::INC,&a::ZPI,5,2 }, { "???",&a::XXX,&a::IMP,5,1 },{ "INX",&a::INX,&a::IMP,2,1 }, { "SBC",&a::SBC,&a::IMM,2,2 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,2,1 }, { "CPX",&a::CPX,&a::ABS,4,3 }, { "SBC",&a::SBC,&a::ABS,4,3 }, { "INC",&a::INC,&a::ABS,6,3 }, { "???",&a::XXX,&a::IMP,6,1 },
        /*0xF0                           0xF1                           0xF2                           0xF3                          0xF4                           0xF5                           0xF6                           0xF7                          0xF8                           0xF9                           0xFA                           0xFB                          0xFC                            0xFD                           0xFE                           0xFF*/
        { "BEQ",&a::BEQ,&a::REL,2,2 }, { "SBC",&a::SBC,&a::IIY,5,2 }, { "???",&a::XXX,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,8,1 },{ "NOP",&a::NOP,&a::ZPX,4,2 }, { "SBC",&a::SBC,&a::ZPX,4,2 }, { "INC",&a::INC,&a::ZPX,6,2 }, { "???",&a::XXX,&a::IMP,6,1 },{ "SED",&a::SED,&a::IMP,2,1 }, { "SBC",&a::SBC,&a::ABY,4,3 }, { "NOP",&a::NOP,&a::IMP,2,1 }, { "???",&a::XXX,&a::IMP,7,1 }, { "NOP",&a::NOP,&a::ABX,4,3 }, { "SBC",&a::SBC,&a::ABX,4,3 }, { "INC",&a::INC,&a::ABX,7,3 }, { "???",&a::XXX,&a::IMP,7,1 }
    };


}

void CPU::connectBus(Bus* b) {
    bus = b;
    bus->cpu = this;
}

void CPU::reset() {
    a = 0;
    x = 0;
    y = 0;

    sp = 0xFD;
    status = I | U;

    uint16_t lo = read(0xFFFC);
    uint16_t hi = read(0xFFFD);

    pc = (hi << 8) | lo;

    pc = 0xC000;
}

void CPU::clock() {
    if (cycles == 0) {
        if (tStream) *tStream << trace() << std::endl;

        opcode = read(pc++);

        INSTRUCTION& inst = lookup[opcode];

        cycles = inst.cycles;

        uint8_t a1 = (this->*inst.addrmode)();
        uint8_t a2 = (this->*inst.operate)();

        cycles += (a1 & a2);
    }
    cycles--;
}

bool CPU::complete() const {
    return cycles == 0;
}

void CPU::attachTraceStream(std::ofstream* ofs) {
    tStream = ofs;
}

uint8_t CPU::read(uint16_t addr, bool readOnly) {
    return bus->cpuRead(addr);
}

void CPU::write(uint16_t addr, uint8_t data) {
    bus->cpuWrite(addr, data);
}

uint8_t CPU::getFlag(FLAGS f) const {
    return ((status & f) > 0) ? 1 : 0;
}

void CPU::setFlag(FLAGS f, bool v) {
    if (v)
        status |= f;
    else
        status &= ~f;
}

void CPU::push(uint8_t value) {
    write(0x0100 + sp, value);
    sp--;
}

uint8_t CPU::pop() {
    sp++;
    return read(0x0100 + sp);
}

uint8_t CPU::fetch() {
    if (!(lookup[opcode].addrmode == &CPU::IMP))
        fetched = read(addr_abs);

    return fetched;
}

uint16_t CPU::getPC() const {
    return pc;
}
#pragma region AddressingModes
uint8_t CPU::ABS() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;

    addr_abs = (hi << 8) | lo;

    return 0;
}
uint8_t CPU::ABX() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;

    addr_abs = (hi << 8) | lo;
    addr_abs += x;

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1;

    return 0;
}
uint8_t CPU::ABY() {
    uint16_t lo = read(pc);
    pc++;
    uint16_t hi = read(pc);
    pc++;

    addr_abs = (hi << 8) | lo;
    addr_abs += y;

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1;

    return 0;
}
uint8_t CPU::IIX() {
    uint16_t t = read(pc);
    pc++;

    uint16_t lo = read((uint16_t)(t + x) & 0x00FF);
    uint16_t hi = read((uint16_t)(t + x + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;

    return 0;
}
uint8_t CPU::IIY() {
    uint16_t t = read(pc);
    pc++;

    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;
    addr_abs += y;

    if ((addr_abs & 0xFF00) != (hi << 8))
        return 1;

    return 0;
}
uint8_t CPU::IMM() {
    addr_abs = pc++;
    return 0;
}
uint8_t CPU::IMP() {
    fetched = a;
    return 0;
}
uint8_t CPU::IND() {
    uint16_t ptr_lo = read(pc);
    pc++;
    uint16_t ptr_hi = read(pc);
    pc++;

    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF)
        addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr);
    else
        addr_abs = (read(ptr + 1) << 8) | read(ptr);

    return 0;
}
uint8_t CPU::REL() {
    addr_rel = read(pc);
    pc++;

    if (addr_rel & 0x80)
        addr_rel |= 0xFF00;

    return 0;
}
uint8_t CPU::ZPI() {
    addr_abs = read(pc);
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}
uint8_t CPU::ZPX() {
    addr_abs = (read(pc) + x);
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}
uint8_t CPU::ZPY() {
    addr_abs = (read(pc) + y);
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}
#pragma endregion
#pragma region Instructions
uint8_t CPU::ADC() {
    fetch();

    uint16_t t = (uint16_t)a + (uint16_t)fetched + (uint16_t)getFlag(C);

    setFlag(C, t > 255);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(V, (~((uint16_t)a ^ (uint16_t)fetched) &
                ((uint16_t)a ^ t)) & 0x80);
    setFlag(N, t & 0x80);

    a = t & 0xFF;

    return 1;
}
uint8_t CPU::AND() {
    fetch();

    a = a & fetched;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 1;
}
uint8_t CPU::ASL() {
    fetch();

    uint16_t t = (uint16_t)fetched << 1;

    setFlag(C, fetched & 0x80);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(N, t & 0x80);

    if (lookup[opcode].addrmode == &CPU::IMP)
        a = t & 0xFF;
    else
        write(addr_abs, t & 0xFF);

    return 0;
}
uint8_t CPU::BCC() {
    if (getFlag(C) == 0) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BCS() {
    if (getFlag(C) == 1) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BEQ() {
    if (getFlag(Z) == 1) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BIT() {
    fetch();

    uint8_t t = a & fetched;

    setFlag(Z, t == 0);
    setFlag(V, t & 0x40);
    setFlag(N, t & 0x80);

    return 0;
}
uint8_t CPU::BMI() {
    if (getFlag(N) == 1) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BNE() {
    if (getFlag(Z) == 0) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BPL() {
    if (getFlag(N) == 0) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BRK() {
    pc++;

    setFlag(I, 1);

    push((pc >> 8) & 0xFF);
    push(pc & 0xFF);

    setFlag(B, 1);
    push(status);
    setFlag(B, 0);

    uint16_t lo = read(0xFFFE);
    uint16_t hi = read(0xFFFF);

    pc = (hi << 8) | lo;

    return 0;
}
uint8_t CPU::BVC() {
    if (getFlag(V) == 1) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::BVS() {
    if (getFlag(V) == 0) {
        cycles++;

        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;
    }

    return 0;
}
uint8_t CPU::CLC() {
    setFlag(C, false);
    return 0;
}
uint8_t CPU::CLD() {
    setFlag(D, false);
    return 0;
}
uint8_t CPU::CLI() {
    setFlag(I, false);
    return 0;
}
uint8_t CPU::CLV() {
    setFlag(V, false);
    return 0;
}
uint8_t CPU::CMP() {
    fetch();

    uint16_t t = (uint16_t)a - (uint16_t)fetched;

    setFlag(C, a >= fetched);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(N, t & 0x80);

    return 1;
}
uint8_t CPU::CPX() {
    fetch();

    uint16_t t = (uint16_t)x - (uint16_t)fetched;

    setFlag(C, x >= fetched);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(N, t & 0x80);

    return 1;
}
uint8_t CPU::CPY() {
    fetch();

    uint16_t t = (uint16_t)y - (uint16_t)fetched;

    setFlag(C, y >= fetched);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(N, t & 0x80);

    return 1;
}
uint8_t CPU::DEC() {
    fetch();

    uint8_t t = fetched;
    t--;

    write(addr_abs, t);

    setFlag(Z, t == 0);
    setFlag(N, t & 0x80);

    return 0;
}
uint8_t CPU::DEX() {
    x--;

    setFlag(Z, x == 0);
    setFlag(N, x & 0x80);

    return 0;
}
uint8_t CPU::DEY() {
    y--;

    setFlag(Z, x == 0);
    setFlag(N, x & 0x80);

    return 0;
}
uint8_t CPU::EOR() {
    fetch();

    a = a ^ fetched;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 1;
}
uint8_t CPU::INC() {
    fetch();

    uint8_t t = fetched;
    t++;

    write(addr_abs, t);

    setFlag(Z, t == 0);
    setFlag(N, t & 0x80);

    return 0;
}
uint8_t CPU::INX() {
    x++;

    setFlag(Z, x == 0);
    setFlag(N, x & 0x80);

    return 0;
}
uint8_t CPU::INY() {
    y++;

    setFlag(Z, y == 0);
    setFlag(N, y & 0x80);

    return 0;
}
uint8_t CPU::JMP() {
    pc = addr_abs;
    return 0;
}
uint8_t CPU::JSR() {
    pc--;

    push((pc >> 8) & 0xFF);
    push(pc & 0xFF);

    pc = addr_abs;

    return 0;
}
uint8_t CPU::LDA() {
    fetch();
    a = fetched;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 1;
}
uint8_t CPU::LDX() {
    fetch();
    x = fetched;

    setFlag(Z, x == 0);
    setFlag(N, x & 0x80);

    return 1;
}
uint8_t CPU::LDY() {
    fetch();
    y = fetched;

    setFlag(Z, y == 0);
    setFlag(N, y & 0x80);

    return 1;
}
uint8_t CPU::LSR() {
    fetch();

    uint16_t t = (uint16_t)fetched >> 1;

    setFlag(C, fetched & 0x80);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(N, t & 0x80);

    if (lookup[opcode].addrmode == &CPU::IMP)
        a = t & 0xFF;
    else
        write(addr_abs, t & 0xFF);

    return 0;
}
uint8_t CPU::NOP() {
    return 0;
}
uint8_t CPU::ORA() {
    fetch();

    a = a | fetched;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 1;
}
uint8_t CPU::PHA() {
    push(a);
    return 0;
}
uint8_t CPU::PHP() {
    push(status | 0x10);
    return 0;
}
uint8_t CPU::PLA() {
    a = pop();

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 0;
}
uint8_t CPU::PLP() {
    status = pop();
    return 0;
}
uint8_t CPU::ROL() {
    fetch();

    uint8_t t = (fetched << 1) & getFlag(C);

    setFlag(C, fetched & 0x80);
    setFlag(Z, t == 0);
    setFlag(N, t & 0x80);

    if (lookup[opcode].addrmode == &CPU::IMP)
        a = t;
    else
        write(addr_abs, t);

    return 0;
}
uint8_t CPU::ROR() {
    fetch();

    uint8_t t = (fetched >> 1) & (getFlag(C) << 7);

    setFlag(C, fetched & 0x01);
    setFlag(Z, t == 0);
    setFlag(N, t & 0x80);

    if (lookup[opcode].addrmode == &CPU::IMP)
        a = t;
    else
        write(addr_abs, t);

    return 0;
}
uint8_t CPU::RTI() {
    status = pop();

    uint8_t lo = pop();
    uint8_t hi = pop();

    pc = (hi << 8) | lo;

    return 0;
}
uint8_t CPU::RTS() {
    uint16_t lo = pop();
    uint16_t hi = pop();

    pc = (hi << 8) | lo;
    pc++;

    return 0;
}
uint8_t CPU::SBC() {
    fetch();

    uint16_t t = (uint16_t)a + (uint16_t)~fetched + (uint16_t)getFlag(C);

    setFlag(C, t > 255);
    setFlag(Z, (t & 0xFF) == 0);
    setFlag(V, (~((uint16_t)a ^ (uint16_t)fetched) &
                ((uint16_t)a ^ t)) & 0x80);
    setFlag(N, t & 0x80);

    a = t & 0xFF;

    return 1;
}
uint8_t CPU::SEC() {
    setFlag(C, true);
    return 0;
}
uint8_t CPU::SED() {
    setFlag(D, true);
    return 0;
}
uint8_t CPU::SEI() {
    setFlag(I, true);
    return 0;
}
uint8_t CPU::STA() {
    write(addr_abs, a);

    return 0;
}
uint8_t CPU::STX() {
    write(addr_abs, x);

    return 0;
}
uint8_t CPU::STY() {
    write(addr_abs, y);

    return 0;
}
uint8_t CPU::TAX() {
    x = a;

    setFlag(Z, x == 0);
    setFlag(N, x & 0x80);

    return 0;
}
uint8_t CPU::TAY() {
    y = a;

    setFlag(Z, y == 0);
    setFlag(N, y & 0x80);

    return 0;
}
uint8_t CPU::TSX() {
    x = sp;

    setFlag(Z, y == 0);
    setFlag(N, y & 0x80);

    return 0;
}
uint8_t CPU::TXA() {
    a = x;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 0;
}
uint8_t CPU::TXS() {
    sp = x;

    return 0;
}
uint8_t CPU::TYA() {
    a = y;

    setFlag(Z, a == 0);
    setFlag(N, a & 0x80);

    return 0;
}

uint8_t CPU::XXX() {
    return 0;
}
#pragma endregion
std::string CPU::disassembleInstruction(uint16_t addr) {
    uint16_t line_addr = addr;
    uint8_t oc = read(addr++, true);

    std::string sInst = std::string(lookup[oc].name) + " ";

    if (lookup[oc].addrmode == &CPU::IMM) {
        uint8_t val = read(addr++, true);

        sInst += "#$" + hex(val, 2);
    }
    else if (lookup[oc].addrmode == &CPU::ZPI) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2);
    }
    else if (lookup[oc].addrmode == &CPU::ZPX) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2) + ",X";
    }
    else if (lookup[oc].addrmode == &CPU::ZPY) {
        uint8_t addr8 = read(addr++, true);

        sInst += "$" + hex(addr8, 2) + ",Y";
    }
    else if (lookup[oc].addrmode == &CPU::ABS) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4);
    }
    else if (lookup[oc].addrmode == &CPU::ABX) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4) + ",X";
    }
    else if (lookup[oc].addrmode == &CPU::ABY) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "$" + hex(val, 4) + ",Y";
    }
    else if (lookup[oc].addrmode == &CPU::IND) {
        uint16_t lo = read(addr++, true);
        uint16_t hi = read(addr++, true);
        uint16_t val = (hi << 8) | lo;

        sInst += "($" + hex(val, 4) + ")";
    }
    else if (lookup[oc].addrmode == &CPU::IIX) {
        uint8_t val = read(addr++, true);

        sInst += "($" + hex(val, 2) + ",X)";
    }
    else if (lookup[oc].addrmode == &CPU::IIY) {
        uint8_t val = read(addr++, true);

        sInst += "($" + hex(val, 2) + "),Y";
    }
    else if (lookup[oc].addrmode == &CPU::REL) {
        int8_t offset = read(addr++, true);
        uint16_t target = addr + offset;

        sInst += "$" + hex(target, 4);
    }

    return sInst;
}

std::string CPU::formatInstruction(uint16_t addr) {
    std::string line = disassembleInstruction(addr);

    line += " A:" + hex(a);
    line += " X:" + hex(a);
    line += " Y:" + hex(a);
    line += " P:" + hex(a);
    line += " SP:" + hex(a);

    return line;
}

std::string CPU::trace() {
    uint16_t PC = pc;

    uint8_t oc = read(pc);

    const INSTRUCTION& inst = lookup[oc];

    std::stringstream ss;

    // Program counter
    ss << hex(PC, 4) << "  ";

    // Raw opcode bytes
    uint8_t b1 = read(PC + 1, true);
    uint8_t b2 = read(PC + 2, true);

    ss << hex(oc, 2) << " ";

    if (inst.bytes >= 2)
        ss << hex(b1, 2) << " ";
    else
        ss << "  ";

    if (inst.bytes == 3)
        ss << hex(b2, 2);
    else
        ss << "  ";

    ss << "  ";

    // Instruction text
    ss << disassembleInstruction(PC);

    // Padding to match nestest alignment
    while (ss.str().size() < 48) {
        ss << " ";
    }

    // Registers
    ss << "A:" << hex(a, 2) << " ";
    ss << "X:" << hex(x, 2) << " ";
    ss << "Y:" << hex(y, 2) << " ";
    ss << "P:" << hex(status, 2) << " ";
    ss << "SP:" << hex(sp, 2);

    return ss.str();
}