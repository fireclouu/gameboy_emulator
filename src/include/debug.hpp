/*
│* debug.hpp
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

#ifndef SRC_INCLUDE_DEBUG_HPP_
#define SRC_INCLUDE_DEBUG_HPP_
#include <cstdint>
#include <iostream>

#include "cpu.hpp"
#include "mmu.hpp"

class Debug {
 private:
  Cpu *cpu;
  Mmu *mmu;

 public:
  union {
      uint8_t breakCode;
      struct {
          uint8_t pc:1, opcode:1, ffwd:1, step:1, next:1, iterate:1;
      };
  } break_n;
  uint16_t storeOpcode;
  uint64_t storeIterate, storeFfwd;
  uint16_t storePc;
      uint64_t iterate = 0;
  uint8_t opcodeTally[0xFF] = {};
  uint8_t opcodeTallyCb[0xFF] = {};
  int debugDisable;
  explicit Debug(Cpu *cpu, Mmu *mmu);
  void print();
  void interact();
  void startDebug();
  void endDebug();
};
#endif  // SRC_INCLUDE_DEBUG_HPP_
