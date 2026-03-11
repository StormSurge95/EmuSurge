#pragma once

#include "../../core/Bus.h"
#include "../../core/Device.h"
#include "../../core/Helpers.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class NES_CPU : public Device {
    public:
        NES_CPU();

        uint16_t pc = 0;

        inline void connectBus(Bus* b) { bus = b; }

        void reset();
        void clock();

        void IRQ();
        void NMI();

        inline void attachTraceStream(std::ofstream* ts) { traceStream = ts; }

    private:
        Bus* bus = nullptr;

        uint8_t a = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t sp = 0;

        uint8_t status = 0;

        uint8_t fetched = 0x00;
        uint16_t addrAbs = 0x0000;
        uint16_t addrRel = 0x0000;
        uint8_t opcode = 0x00;

        uint8_t cycles = 0;
        enum FLAGS {
            C = (1 << 0),
            Z = (1 << 1),
            I = (1 << 2),
            D = (1 << 3),
            B = (1 << 4),
            U = (1 << 5),
            V = (1 << 6),
            N = (1 << 7)
        };
        struct Instruction {
            std::string name;

            uint8_t(NES_CPU::* operate)(void) = nullptr;
            uint8_t(NES_CPU::* addrmode)(void) = nullptr;

            uint8_t cycles = 0;
            uint8_t bytes = 0;
        };
        uint8_t fetch();
        std::vector<Instruction> lookup;

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        inline uint8_t getFlag(FLAGS f) const { return ((status & f) > 0) ? 1 : 0; }
        inline void setFlag(FLAGS f, bool v) { status = (v ? status |= f : status &= ~f); }

        inline void push(uint8_t data) {
            write(0x0100 + sp, data);
            sp--;
        }
        inline uint8_t pop() {
            sp++;
            return read(0x0100 + sp);
        }

        #pragma region Addressing Modes
        inline uint8_t IMP() {
            fetched = a;
            return 0;
        }
        inline uint8_t IMM() {
            addrAbs = pc++;
            return 0;
        }
        inline uint8_t ZP0() {
            addrAbs = read(pc++) & 0x00FF;
            return 0;
        }
        inline uint8_t ZPX() {
            addrAbs = (read(pc++) + x) & 0x00FF;
            return 0;
        }
        inline uint8_t ZPY() {
            addrAbs = (read(pc++) + y) & 0x00FF;
            return 0;
        }
        inline uint8_t ABS() {
            uint16_t lo = read(pc++);
            uint16_t hi = read(pc++);
            addrAbs = (hi << 8) | lo;
            return 0;
        }
        inline uint8_t ABX() {
            uint16_t lo = read(pc++);
            uint16_t hi = read(pc++);
            addrAbs = ((hi << 8) | lo) + x;
            if ((addrAbs & 0xFF00) != (hi << 8))
                return 1;
            return 0;
        }
        inline uint8_t ABY() {
            uint16_t lo = read(pc++);
            uint16_t hi = read(pc++);
            addrAbs = ((hi << 8) | lo) + y;
            if ((addrAbs & 0xFF00) != (hi << 8))
                return 1;
            return 0;
        }
        inline uint8_t REL() {
            addrRel = read(pc++);
            if (addrRel & 0x80)
                addrRel |= 0xFF00;
            return 0;
        }
        inline uint8_t IND() {
            uint16_t plo = read(pc++);
            uint16_t phi = read(pc++);
            uint16_t ptr = (phi << 8) | plo;
            uint16_t lo = read(ptr);
            uint16_t hi = read(ptr + 1);
            if (plo == 0x00FF)
                hi = read(ptr & 0xFF00);
            addrAbs = (hi << 8) | lo;
            return 0;
        }
        inline uint8_t IZX() {
            uint16_t t = read(pc++);
            uint16_t lo = read((uint16_t)(t + x) & 0x00FF);
            uint16_t hi = read((uint16_t)(t + x + 1) & 0x00FF);
            addrAbs = (hi << 8) | lo;
            return 0;
        }
        inline uint8_t IZY() {
            uint16_t t = read(pc++);
            uint16_t lo = read(t & 0x00FF);
            uint16_t hi = read((t + 1) & 0x00FF);
            addrAbs = ((hi << 8) | lo) + y;
            if ((addrAbs & 0xFF00) != (hi << 8)) return 1;
            return 0;
        }
        #pragma endregion
        #pragma region Instructions
        #pragma region Access
        inline uint8_t LDA() {
            fetch();
            a = fetched;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 1;
        }
        inline uint8_t STA() {
            write(addrAbs, a);

            return 0;
        }
        inline uint8_t LDX() {
            fetch();
            x = fetched;

            setFlag(Z, x == 0);
            setFlag(N, x & 0x80);

            return 1;
        }
        inline uint8_t STX() {
            write(addrAbs, x);

            return 0;
        }
        inline uint8_t LDY() {
            fetch();
            y = fetched;

            setFlag(Z, y == 0);
            setFlag(N, y & 0x80);

            return 1;
        }
        inline uint8_t STY() {
            write(addrAbs, y);

            return 0;
        }
        #pragma endregion
        #pragma region Transfer
        inline uint8_t TAX() {
            x = a;

            setFlag(Z, x == 0);
            setFlag(N, x & 0x80);

            return 0;
        }
        inline uint8_t TXA() {
            a = x;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 0;
        }
        inline uint8_t TAY() {
            y = a;

            setFlag(Z, y == 0);
            setFlag(N, y & 0x80);

            return 0;
        }
        inline uint8_t TYA() {
            a = y;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 0;
        }
        #pragma endregion
        #pragma region Arithmetic
        inline uint8_t ADC() {
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
        inline uint8_t SBC() {
            fetch();

            int16_t t = (int16_t)a + (int16_t)~fetched + (int16_t)getFlag(C);

            setFlag(C, !(t < 0x00));
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(V, (t ^ a) & (t ^ ~fetched) & 0x80);
            setFlag(N, t & 0x80);

            a = t & 0xFF;

            return 1;
        }
        inline uint8_t INC() {
            fetch();

            uint8_t t = fetched;
            t++;

            write(addrAbs, t);

            setFlag(Z, t == 0);
            setFlag(N, t & 0x80);

            return 0;
        }
        inline uint8_t DEC() {
            fetch();

            uint8_t t = fetched;
            t--;

            write(addrAbs, t);

            setFlag(Z, t == 0);
            setFlag(N, t & 0x80);

            return 0;
        }
        inline uint8_t INX() {
            x++;

            setFlag(Z, x == 0);
            setFlag(N, x & 0x80);

            return 0;
        }
        inline uint8_t DEX() {
            x--;

            setFlag(Z, x == 0);
            setFlag(N, x & 0x80);

            return 0;
        }
        inline uint8_t INY() {
            y++;

            setFlag(Z, y == 0);
            setFlag(N, y & 0x80);

            return 0;
        }
        inline uint8_t DEY() {
            y--;

            setFlag(Z, y == 0);
            setFlag(N, y & 0x80);

            return 0;
        }
        #pragma endregion
        #pragma region Shift
        inline uint8_t ASL() {
            fetch();

            uint16_t t = (uint16_t)fetched << 1;

            setFlag(C, fetched & 0x80);
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(N, t & 0x80);

            if (lookup[opcode].addrmode == &NES_CPU::IMP)
                a = t & 0xFF;
            else
                write(addrAbs, t & 0xFF);

            return 0;
        }
        inline uint8_t LSR() {
            fetch();

            uint16_t t = (uint16_t)fetched >> 1;

            setFlag(C, fetched & 0x01);
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(N, t & 0x80);

            if (lookup[opcode].addrmode == &NES_CPU::IMP)
                a = t & 0xFF;
            else
                write(addrAbs, t & 0xFF);

            return 0;
        }
        inline uint8_t ROL() {
            fetch();

            uint8_t t = (fetched << 1) | getFlag(C);

            setFlag(C, fetched & 0x80);
            setFlag(Z, t == 0);
            setFlag(N, t & 0x80);

            if (lookup[opcode].addrmode == &NES_CPU::IMP)
                a = t;
            else
                write(addrAbs, t);

            return 0;
        }
        inline uint8_t ROR() {
            fetch();

            uint8_t t = (fetched >> 1) | (getFlag(C) << 7);

            setFlag(C, fetched & 0x01);
            setFlag(Z, t == 0);
            setFlag(N, t & 0x80);

            if (lookup[opcode].addrmode == &NES_CPU::IMP)
                a = t;
            else
                write(addrAbs, t);

            return 0;
        }
        #pragma endregion
        #pragma region Bitwise
        inline uint8_t AND() {
            fetch();

            a = a & fetched;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 1;
        }
        inline uint8_t ORA() {
            fetch();

            a = a | fetched;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 1;
        }
        inline uint8_t EOR() {
            fetch();

            a = a ^ fetched;

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 1;
        }
        inline uint8_t BIT() {
            fetch();

            uint8_t t = a & fetched;

            setFlag(Z, t == 0);
            setFlag(V, fetched & 0x40);
            setFlag(N, fetched & 0x80);

            return 0;
        }
        #pragma endregion
        #pragma region Compare
        inline uint8_t CMP() {
            fetch();

            uint16_t t = (uint16_t)a - (uint16_t)fetched;

            setFlag(C, a >= fetched);
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(N, t & 0x80);

            return 1;
        }
        inline uint8_t CPX() {
            fetch();

            uint16_t t = (uint16_t)x - (uint16_t)fetched;

            setFlag(C, x >= fetched);
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(N, t & 0x80);

            return 1;
        }
        inline uint8_t CPY() {
            fetch();

            uint16_t t = (uint16_t)y - (uint16_t)fetched;

            setFlag(C, y >= fetched);
            setFlag(Z, (t & 0xFF) == 0);
            setFlag(N, t & 0x80);

            return 1;
        }
        #pragma endregion
        #pragma region Branch
        inline uint8_t BCC() {
            if (getFlag(C) == 0) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BCS() {
            if (getFlag(C) == 1) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BEQ() {
            if (getFlag(Z) == 1) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BNE() {
            if (getFlag(Z) == 0) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BPL() {
            if (getFlag(N) == 0) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BMI() {
            if (getFlag(N) == 1) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BVC() {
            if (getFlag(V) == 0) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        inline uint8_t BVS() {
            if (getFlag(V) == 1) {
                cycles++;

                addrAbs = pc + addrRel;

                if ((addrAbs & 0xFF00) != (pc & 0xFF00))
                    cycles++;

                pc = addrAbs;
            }

            return 0;
        }
        #pragma endregion
        #pragma region Jump
        inline uint8_t JMP() {
            pc = addrAbs;
            return 0;
        }
        inline uint8_t JSR() {
            pc--;

            push((pc >> 8) & 0xFF);
            push(pc & 0xFF);

            pc = addrAbs;

            return 0;
        }
        inline uint8_t RTS() {
            uint16_t lo = pop();
            uint16_t hi = pop();

            pc = (hi << 8) | lo;
            pc++;

            return 0;
        }
        inline uint8_t BRK() {
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
        inline uint8_t RTI() {
            status = pop() | U;

            uint8_t lo = pop();
            uint8_t hi = pop();

            pc = (hi << 8) | lo;

            return 0;
        }
        #pragma endregion
        #pragma region Stack
        inline uint8_t PHA() {
            push(a);
            return 0;
        }
        inline uint8_t PLA() {
            a = pop();

            setFlag(Z, a == 0);
            setFlag(N, a & 0x80);

            return 0;
        }
        inline uint8_t PHP() {
            push(status | 0x10);
            return 0;
        }
        inline uint8_t PLP() {
            status = pop();
            status &= ~B;
            status |= U;
            return 0;
        }
        inline uint8_t TXS() {
            sp = x;

            return 0;
        }
        inline uint8_t TSX() {
            x = sp;

            setFlag(Z, x == 0);
            setFlag(N, x & 0x80);

            return 0;
        }
        #pragma endregion
        #pragma region Flags
        inline uint8_t CLC() {
            setFlag(C, false);
            return 0;
        }
        inline uint8_t SEC() {
            setFlag(C, true);
            return 0;
        }
        inline uint8_t CLI() {
            setFlag(I, false);
            return 0;
        }
        inline uint8_t SEI() {
            setFlag(I, true);
            return 0;
        }
        inline uint8_t CLD() {
            setFlag(D, false);
            return 0;
        }
        inline uint8_t SED() {
            setFlag(D, true);
            return 0;
        }
        inline uint8_t CLV() {
            setFlag(V, false);
            return 0;
        }
        #pragma endregion
        #pragma region Other
        inline uint8_t NOP() {
            return 0;
        }
        #pragma endregion
        inline uint8_t XXX() {
            std::string msg = "ILLEGAL OPCODE ATTEMPTED: " + hex(opcode, 2);
            throw std::exception(msg.c_str());
        }
#pragma endregion
        #pragma region Debugging
        std::ofstream* traceStream = nullptr;
        std::stringstream ss;

        std::string disassembleInst(uint16_t addr);
        std::string formatInst() const;
        std::string trace();
        #pragma endregion
};