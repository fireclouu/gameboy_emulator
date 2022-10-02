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

#include "cpu.hpp"
#include "mmu.hpp"

#define ROM_SIZE 0x8000

class Gameboy {
 public:
  bool halt;
  uint8_t romData[ROM_SIZE] = {};
  Gameboy();
  ~Gameboy();
  void setup(Cpu *cpu, Mmu *mmu);
};

#endif  // SRC_INCLUDE_GAMEBOY_HPP_
