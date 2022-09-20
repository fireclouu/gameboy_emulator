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

#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_
#define TITLE "GBEMU_V2"
#include <fstream>
#include <iostream>
#include <string>

struct GB_Register {
  union {
    uint8_t all_reg[8];
    struct {
      union {
        uint16_t reg_pair_bc;
        struct {
          uint8_t reg_c;
          uint8_t reg_b;
        };
      };
      union {
        uint16_t reg_pair_de;
        struct {
          uint8_t reg_e;
          uint8_t reg_d;
        };
      };
      union {
        uint16_t reg_pair_hl;
        struct {
          uint8_t reg_l;
          uint8_t reg_h;
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
};

#endif  // SRC_MAIN_HPP_
