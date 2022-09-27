/*
 * mmu.cpp
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

#include "include/mmu.hpp"

#include <stdint.h>

#include "include/cpu.hpp"

Mmu::Mmu(uint8_t* memoryMap, int romSize, bool* haltPtr) {
  this->memoryMap = memoryMap;
  this->romSize = romSize;
  this->halt = haltPtr;
}

void Mmu::getMemoryMap(uint8_t* memoryMap, int romSize) {
  this->memoryMap = memoryMap;
  this->romSize = romSize;
}
uint8_t Mmu::readByte(uint16_t addr) { return this->memoryMap[addr]; }
uint16_t Mmu::readShort(uint16_t addr) {
  return (memoryMap[addr + 1] << 8) + memoryMap[addr];
}
void Mmu::writeByte(uint16_t addr, uint8_t value) {
  if (addr < romSize) {
    std::printf("ERROR (W): 0x%04X: overwriting ROM file!\n", addr);
    (*halt) = true;
  } else {
    memoryMap[addr] = value;
  }
}
uint8_t* Mmu::getBytePtr(uint16_t addr) {
  uint8_t* ptr = nullptr;
  if (addr < romSize) {
    std::printf("INFO: 0x%04X: address might be written.\n", addr);
  }
  ptr = &memoryMap[addr];
  return ptr;
}
