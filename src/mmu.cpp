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

#include <algorithm>
#include <cstring>

Mmu::Mmu() {}

uint8_t Mmu::readByte(uint16_t addr) {
  currentTCycle += 4;
  uint8_t memoryByte = 0;
  uint16_t addrSection = (addr & 0xF000);
  switch (addrSection) {
    //  ROM Bank 00 (16kB)
    case 0x0000:
    case 0x1000:
    case 0x2000:
    case 0x3000:
      memoryByte = rom[addr & 0x7FFF];
      break;
    //  ROM Bank 01-NN (16kB)
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
      memoryByte = rom[addr & 0x7FFF];
      break;
      //  Video RAM (8kB)
    case 0x8000:
    case 0x9000:
      memoryByte = vram[addr & (VRAM_SIZE - 1)];
      break;
      //  External RAM (8kB)
    case 0xA000:
    case 0xB000:
      memoryByte = eram[addr & (ERAM_SIZE - 1)];
      break;
      //  Work RAM (4kB)
    case 0xC000:
      memoryByte = wram[addr & (WRAM_SIZE - 1)];
      break;
      //  Work RAM (4kB)
    case 0xD000:
      memoryByte = wram[addr & (WRAM_SIZE - 1)];
      break;
    case 0xE000:
    case 0xF000: {
      addrSection = (addr & 0x0F00);
      switch (addrSection) {
        // Echo RAM
        case 0x0000:
        case 0x0100:
        case 0x0200:
        case 0x0300:
        case 0x0400:
        case 0x0500:
        case 0x0600:
        case 0x0700:
        case 0x0800:
        case 0x0900:
        case 0x0A00:
        case 0x0B00:
        case 0x0C00:
        case 0x0D00:
          memoryByte = wram[addr & (WRAM_SIZE - 1)];
          break;
        // Sprite Attribute (OAM)
        case 0x0E00:
          if (addr < 0xFEA0) {
            memoryByte = oam[addr & (OAM_SIZE - 1)];
          } else {
            // Unusable map
            memoryByte = 0;
          }
          break;
        case 0x0F00:
          if (addr < 0xFF80) {
            // IO todo
            memoryByte = iomap[addr & (IOMAP_SIZE - 1)];
          } else {
            uint8_t hramAddr = (addr & 0xFF) - 0x80;
            memoryByte = hram[hramAddr];
          }
          break;
      }
    } break;
  }
  return memoryByte;
}
void Mmu::writeByte(uint16_t addr, uint8_t value) {
  currentTCycle += 4;
  uint16_t addrSection = (addr & 0xF000);
  switch (addrSection) {
    //  ROM Bank 00 (16kB)
    case 0x0000:
    case 0x1000:
    case 0x2000:
    case 0x3000:
      break;
    //  ROM Bank 01-NN (16kB)
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
      break;
      //  Video RAM (8kB)
    case 0x8000:
    case 0x9000:
      vram[addr & (VRAM_SIZE - 1)] = value;
      break;
      //  External RAM (8kB)
    case 0xA000:
    case 0xB000:
      eram[addr & (ERAM_SIZE - 1)] = value;
      break;
      //  Work RAM (4kB)
    case 0xC000:
      wram[addr & (WRAM_SIZE - 1)] = value;
      break;
      //  Work RAM (4kB)
    case 0xD000:
      wram[addr & (WRAM_SIZE - 1)] = value;
      break;
    case 0xE000:
    case 0xF000: {
      addrSection = (addr & 0x0F00);
      switch (addrSection) {
        // Echo RAM
        case 0x0000:
        case 0x0100:
        case 0x0200:
        case 0x0300:
        case 0x0400:
        case 0x0500:
        case 0x0600:
        case 0x0700:
        case 0x0800:
        case 0x0900:
        case 0x0A00:
        case 0x0B00:
        case 0x0C00:
        case 0x0D00:
          wram[addr & (WRAM_SIZE - 1)] = value;
          break;
        // Sprite Attribute (OAM)
        case 0x0E00:
          if (addr < 0xFEA0) {
            oam[addr & (OAM_SIZE - 1)] = value;
          } else {
            // Unusable map
          }
          break;
        case 0x0F00:
          if (addr < 0xFF80) {
            // IO todo
            iomap[addr & (IOMAP_SIZE - 1)] = value;
          } else {
            uint8_t hramAddr = (addr & 0xFF) - 0x80;
            hram[hramAddr] = value;
          }
          break;
      }
    } break;
  }
}
uint16_t Mmu::readShort(uint16_t addr) {
  return (readByte(addr + 1) << 8) + readByte(addr);
}

void Mmu::setCurrentTCycle(uint32_t* currentTCycle) {
  this->currentTCycle = currentTCycle;
}

void Mmu::setRom(uint8_t romData[ROM_SIZE]) {
    std::copy(romData, romData + ROM_SIZE, this->rom);
}
