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

#include "include/gameboy.hpp"

Gameboy::Gameboy() {
  ime = false;
  halt = false;
  this->mmu = NULL;
  this->cpu = NULL;
}

Gameboy::~Gameboy() {}

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

void Gameboy::setup(Cpu *cpu, Mmu *mmu) {
  this->mmu = mmu;
  this->cpu = cpu;
  cpu->setMmu(mmu);
  cpu->setHalt(&halt);
  mmu->setCurrentTCycle(&cpu->currentTCycle);
  mmu->setRom(this->romData);
}
