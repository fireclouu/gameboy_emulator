/*
│* gameboy.hpp
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

#ifndef SRC_INCLUDE_GAMEBOY_HPP_
#define SRC_INCLUDE_GAMEBOY_HPP_
#include <cstdint>
#include <string>

#include "cpu.hpp"
#include "mmu.hpp"
#include "opcode.hpp"

#define ROM_SIZE 0x8000

class Gameboy
{
private:
  enum IO_ADDR
  {
    INT_VBLANK = 0x40,
    INT_LCDSTAT = 0x48,
    INT_TIMER = 0x50,
    INT_SERIAL = 0x58,
    INT_JOYPAD = 0x60,
    INTERRUPT_FLAG = 0xFF0F,
    INTERRUPT_ENABLE = 0xFFFF,
  };

  bool halt;
  bool ime;
  Cpu *cpu;
  Mmu *mmu;

public:
  Gameboy(Cpu *cpu, Mmu *mmu, uint8_t *romData);
  ~Gameboy();
  uint8_t *romData;

  void handleInterrupt(uint16_t pc);
  void setRom(uint8_t *romData);
  void start();
};

#endif // SRC_INCLUDE_GAMEBOY_HPP_
