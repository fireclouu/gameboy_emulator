/*
 * debug.hpp
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

#ifndef SRC_INCLUDE_DEBUG_HPP_
#define SRC_INCLUDE_DEBUG_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include "main.hpp"
#include "opcode.hpp"

class Debugger {
  private:
    bool mainSwitchEnable = 1, isOpcode = 0, isIterator = 1, isPc = 0,
         printQuiet = 0;
    uint8_t opcodeVal = 0;
    uint16_t pcVal = 0;
    uint64_t iterator = 0, newValueIterator = 0;
    GB_Register *addrRegister;
    uint8_t *addrMemory;
    uint64_t op_used[0xFF] = {};

    void debugInteract();
    void debugPrintStep(std::string msg);

	public:
    Debugger(GB_Register *param_reg, uint8_t *memory);
    void print_memory(uint8_t *param_memory, int param_mem_size);
    void debugStepInteractive();
    void dbgEnd(int param);
    void disable();
};
#endif  // SRC_INCLUDE_DEBUG_HPP_
