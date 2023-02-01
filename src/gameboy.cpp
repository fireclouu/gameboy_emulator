/*
 * gameboy.cpp
 * Copyright (C) 2022 fireclouu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/gameboy.hpp"
#include <cstdint>

// isMessagePassed
const char PASSED[] = {0x50, 0x61, 0x73, 0x73, 0x65, 0x64, 0x0a}; // PASSED\n
int passedCount = 0;
bool isPassed = false;
// get first line message
std::string buildMessage = "";
bool isInitialMessageFetched = false;
// islooping
uint16_t lastPc = 0;
uint8_t lastInstruction = 0;

Gameboy::Gameboy(Cpu *cpu, Mmu *mmu) {
    halt = false;
    ime = false;

    this->cpu = cpu;
    this->mmu = mmu;
    cpu->setMmu(mmu);
    cpu->setHalt(&halt);
}

bool isMessagePassed(char msg) {
    // check if program is building PASSED string
    // and if it is, create custom message
    char passed = PASSED[passedCount];
    if (!isPassed) {
        if (msg == passed) {
            passedCount++;
            if (sizeof(PASSED) == passedCount)
                return true;
        } else {
            passedCount = 0;
        }
    }
    return false;
}

void fetchInitialMessage(char msg) {
    if (msg != 0x0a && !isInitialMessageFetched) {
        buildMessage += msg;
    } else {
        if (!isInitialMessageFetched) {
            printf("TEST: %-40s", buildMessage.c_str());
            isInitialMessageFetched = true;
        }
    }
}

// check if pc is the same as pevious pc
// and instruction fetched is the same as previous one
bool isLooping(Cpu *cpu, Mmu *mmu) {
    if (cpu->cpuRegister.pc == lastPc && (mmu->readByte(cpu->cpuRegister.pc) == lastInstruction) && lastInstruction == 0x18) {
        return true;
    }
    lastPc = cpu->cpuRegister.pc;
    lastInstruction = mmu->readByte(lastPc);
    return false;
}

void Gameboy::handleInterrupt(uint16_t pc) {
    if (ime) {
        uint8_t const IF = mmu->readByte(INTERRUPT_FLAG);
        uint8_t const IE = mmu->readByte(INTERRUPT_ENABLE);
        if (IE & IF) {
            // VBLANK
            cpu->instructionStackPush(pc + 1);
            cpu->cpuRegister.pc = INT_VBLANK;
        }
    }
}

// for blaarg test suite
void testSerialOutput(Mmu *mmu) {
    if (mmu->readByte(0xff02) == 0x81) {
        char c = mmu->readByte(0xff01);
        // printf("%c", c);
        fetchInitialMessage(c);
        isPassed = isMessagePassed(c);
        mmu->writeByte(0xff02, 0);
    }
}

void testAutomation(Cpu *cpu, Mmu *mmu, bool *halt) {
    testSerialOutput(mmu);
    // check if looping endlessly
    if (isLooping(cpu, mmu)) {
        std::string msg = isPassed ? "OK!" : "FAIL!";
        printf("%s\n", msg.c_str());
        *halt = true;
    }
}

void Gameboy::start() {
    // isMessagePassed
    passedCount = 0;
    isPassed = false;
    // get first line message
    buildMessage = "";
    isInitialMessageFetched = false;
    // islooping
    lastPc = 0;
    lastInstruction = 0;
    // debugger attach
    Debug *debug = NULL;
    // debug = new Debug(cpu, mmu); // remove comment if needs to debug
    // initial setup
    cpu->cpuRegister.pc = 0x0100;
    cpu->cpuRegister.sp = 0xFFFE;
    cpu->cpuRegister.reg_a = 0x11;
    cpu->cpuRegister.reg_f = 0x80;
    cpu->cpuRegister.reg_b = 0x00;
    cpu->cpuRegister.reg_c = 0x00;
    cpu->cpuRegister.reg_pair_de = 0xFF56;
    cpu->cpuRegister.reg_pair_hl = 0x000D;
    uint8_t clk_div = 0;
    uint8_t old_clk_div = 0;
    uint8_t div_value = 0;
    uint16_t tac_clk_frq = 0;
    uint16_t tima_clk = 0;
    while (!this->halt) {
        // debug
        if (debug != NULL) {
            debug->startDebug();
        }
        // serial automation
        testAutomation(cpu, mmu, &this->halt);
        // pre-fetch
        uint8_t tma_value = mmu->readByte(0xFF06);
        // decode
        uint16_t pc = cpu->cpuRegister.pc;
        uint8_t opcode = mmu->readByte(pc);
        uint8_t tick = cpu->decode(opcode);
        if (tick == 0) {
            printf("Clock returned 0!\n");
            break;
        }
        // 0xff04
        old_clk_div = clk_div;
        clk_div += tick;
        if (old_clk_div > clk_div) {
            div_value = mmu->readByte(0xFF04);
            mmu->writeDiv(div_value + 1);
            uint8_t new_div_value = mmu->readByte(0xFF04);
            if ((div_value == 0) && (new_div_value == 1)) {
                uint16_t tima_value = mmu->readByte(0xFF05);
                mmu->writeByte(0xFF05, tima_value + 1);
            }
        }
        // 0xff07
        uint8_t tac_value = mmu->readByte(0xFF07);
        bool tac_timer_enable = (tac_value & 0x04) != 0 ? true : false;
        uint8_t tac_clk_mode = (tac_value & 0x03);
        if (tac_timer_enable) {
            switch (tac_clk_mode) {
                case 0:
                    tac_clk_frq = 1024;
                    break;
                case 1:
                    tac_clk_frq = 16;
                    break;
                case 2:
                    tac_clk_frq = 64;
                    break;
                case 3:
                    tac_clk_frq = 256;
                    break;
            }
            tima_clk += tick;
            if (tima_clk > tac_clk_frq) {
                uint16_t tima_value = mmu->readByte(0xFF05);
                if ((tima_value + 1) > 0xFF) {
                    mmu->writeByte(0xFF05, tma_value);
                } else {
                    mmu->writeByte(0xFF05, tima_value + 1);
                }
                tima_clk -= tac_clk_frq;
            }
        }
    }
    if (debug != NULL) {
        debug->endDebug();
    }
}

Gameboy::~Gameboy() {}
