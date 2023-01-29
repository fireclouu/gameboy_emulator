/*
│* mmu.hpp
│* Copyright (C) 2022 fireclouu
│*
│* This program is free software: you can redistribute it and/or modify
│* it under the terms of the GNU General Public License as published by
│* the Free Software Foundation, either version 3 of the License, or
│* (at your option) any later version.
│*
│* This program is distributed in the hope that it will be useful,
│* but WITHOUT ANY WARRANTY; without even the implied warranty of
│* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
│* GNU General Public License for more details.
│*
│* You should have received a copy of the GNU General Public License
│* along with this program. If not, see <http://www.gnu.org/licenses/>.
│*/

#ifndef SRC_INCLUDE_MMU_HPP_
#define SRC_INCLUDE_MMU_HPP_

#define ROM_SIZE 0x8000
#define VRAM_SIZE 0x2000
#define ERAM_SIZE 0x2000
#define WRAM_SIZE 0x2000
#define OAM_SIZE 0x00A0
#define IOMAP_SIZE 0x0080
#define HRAM_SIZE 0x007F

#include <stdint.h>

class Mmu {
 private:
  uint32_t *currentTCycle;
  uint8_t *romData;
  uint8_t vram[VRAM_SIZE] = {};
  uint8_t eram[ERAM_SIZE] = {};
  uint8_t wram[WRAM_SIZE] = {};
  uint8_t oam[OAM_SIZE] = {};
  uint8_t iomap[IOMAP_SIZE] = {};
  uint8_t hram[HRAM_SIZE] = {};

 public:
  Mmu(uint8_t *romData);
  ~Mmu();
  void writeByte(uint16_t addr, uint8_t value);
  uint8_t readByte(uint16_t addr);
  uint16_t readShort(uint16_t addr);
  void setRom(uint8_t romData[ROM_SIZE]);
};

#endif  // SRC_INCLUDE_MMU_HPP_
