/*
│* main.hpp
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

#ifndef SRC_INCLUDE_CPU_HPP_
#define SRC_INCLUDE_CPU_HPP_
#define TITLE "GBEMU_V2"

#include <stdint.h>
#include "mmu.hpp"

struct CpuRegister {
  union {
    uint8_t all_reg[8];
    struct {
      union {
        uint16_t reg_pair_bc;
        struct {
          uint8_t reg_c, reg_b;
        };
      };
      union {
        uint16_t reg_pair_de;
        struct {
          uint8_t reg_e, reg_d;
        };
      };
      union {
        uint16_t reg_pair_hl;
        struct {
          uint8_t reg_l, reg_h;
        };
      };
      union {
        uint16_t reg_pair_af;
        struct {
          union {
            uint8_t reg_f;
            struct {
              uint8_t ZEROFILL : 4;
              uint8_t flag_c : 1, flag_h : 1, flag_n : 1, flag_z : 1;
            };
          };
          uint8_t reg_a;
        };
      };
    };
  };
  uint16_t sp;
  uint16_t pc;
  uint8_t *reg[8] = {&reg_b, &reg_c, &reg_d,  &reg_e,
                     &reg_h, &reg_l, nullptr, &reg_a};
  uint16_t *reg_pair[4] = {&reg_pair_bc, &reg_pair_de, &reg_pair_hl,
                           &sp};
};

class Cpu {
 private:
  Mmu *mmu;

 public:
  struct CpuRegister cpuRegister = {};
  explicit Cpu(Mmu *mmu);
  ~Cpu();
  void checkFlagH(uint8_t left, uint8_t right, bool isSubtraction);
  int decode(uint16_t opcodeAddr, uint8_t opcode);
  int decodeCb(uint16_t opcodeAddr, uint8_t opcode);
  void instructionStackPush(uint16_t addr_value);
  uint16_t instructionStackPop();
  void instructionAnd(uint8_t value);
  void instructionXor(uint8_t value);
  void instructionOr(uint8_t value);
  void instructionCp(uint8_t value);
  void instructionAdd(uint8_t value);
  void instructionSub(uint8_t value);
  void instructionInc(uint8_t* bytePtr);
  void instructionDec(uint8_t* bytePtr);
  void conditionalJpAdd(uint8_t flag, uint8_t expected, int8_t value);
  void conditionalJpAdd(uint8_t flag, uint8_t expected, uint8_t value);
  void conditionalJpA16(uint8_t flag, uint8_t expected, uint16_t value);
  void conditionalRet(uint8_t flag, uint8_t expected);
  void conditionalCall(uint16_t pc, uint8_t flag, uint8_t expected);
  bool programStop;
};
#endif  // SRC_INCLUDE_CPU_HPP_
