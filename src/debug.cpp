/* debug.cpp
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

#include "include/debug.hpp"
#include "include/main.hpp"
#include "include/opcode.hpp"

Debugger::Debugger(GB_Register*param_reg, uint8_t*memory) {
  addrRegister = param_reg;
  addrMemory = memory;
  newValueIterator = 0;
  isIterator = 1;
}
void Debugger::debugStepInteractive() {
  op_used[addrMemory[addrRegister->pc]]++;
  if (!printQuiet)
    debugPrintStep("");
  if (((addrMemory[addrRegister->pc] == opcodeVal) && isOpcode) ||
      (iterator >= newValueIterator && isIterator) ||
      (isPc && addrRegister->pc == pcVal)) {
    debugInteract();
  }
  iterator++;
}
void Debugger::dbgEnd(int param) {
  if (!mainSwitchEnable)
    return;
  printf("\nOPCODE TALLY: (%lu iterations)\n", iterator);
  for (int a = 0; a < 0xFF; a++) {
    if (op_used[a] == 0)
      continue;
    printf("0x%02X (%s):  %ld\n", a, OP_INSTRUCTION[a], op_used[a]);
  }
  printf("PROGRAM ENDED\n");
  exit(param);
}
void Debugger::debugInteract() {
  if (!mainSwitchEnable)
    return;
  int inputSize = 100;
  char input[100];
  std::stringstream ss;
  std::string inputToInt;

  for (int x = 0; x < inputSize; x++) {
    input[x] = '\0';
  }

  printf("Enter argument:\n");
  std::cin.getline(input, inputSize);
  printf("\n");
  uint64_t convertedValue;
  // convert to int
  inputToInt = (input + 1);
  if (input[0] == '\0')
    return;
  switch (input[0]) {
  case 'o':
    ss << std::hex << inputToInt;
    ss >> convertedValue;
    opcodeVal = convertedValue;
    isOpcode = true;
    isPc = false;
    isIterator = false;
    printQuiet = false;
    break;
  case 'i':
    convertedValue = input[1] != '\0' ? stoul(inputToInt) : 1;
    newValueIterator = iterator + convertedValue;
    isIterator = true;
    isOpcode = false;
    isPc = false;
    printQuiet = false;
    break;
  case 'f':
  case '\0':
    convertedValue = input[1] != '\0' ? stoul(inputToInt) : 1;
    newValueIterator = iterator + convertedValue;
    isIterator = true;
    isOpcode = false;
    isPc = false;
    printQuiet = true;
    break;
  case 'c':
    isIterator = isOpcode = isPc = false;
    printQuiet = true;
    break;
  case 'b':
    ss << std::hex << inputToInt;
    ss >> convertedValue;
    isPc = true;
    isIterator = isOpcode = false;
    pcVal = convertedValue;
    break;
  case 'q':
    exit(0);
    break;
  default:
    debugInteract();
  }
}
void Debugger::debugPrintStep(const std::string msg) {
  if (!mainSwitchEnable)
    return;
  printf("PC: %04X (0x%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  "
         "%s\n",
         addrRegister->pc, addrMemory[addrRegister->pc],
     addrRegister->reg_pair_af,
         addrRegister->reg_pair_bc,
     addrRegister->reg_pair_de,
     addrRegister->reg_pair_hl,
     addrRegister->sp,
         msg.c_str());
  printf("ITER: %lu INST: %s\n", iterator,
         OP_INSTRUCTION[addrMemory[addrRegister->pc]]);
  printf("MEMORY and STACK:\n");
  for (int x = 0; x < 4; x++) {
    int hold_pc = addrRegister->pc + x;
    int hold_sp = addrRegister->sp + x;
    if (hold_pc <= 0xFFFF) {
      printf("[ 0x%04X: 0x%02X ]   ", hold_pc, addrMemory[hold_pc]);
    }
    if (hold_sp <= 0xFFFF) {
      printf("[ 0x%04X: 0x%02X ]", hold_sp, addrMemory[hold_sp]);
    }
    printf("\n");
  }
  printf("\n");
}
void Debugger::print_memory(uint8_t*param_memory, int param_mem_size) {
  if (!mainSwitchEnable)
    return;
  for (int x = 0; x < param_mem_size; x++) {
    printf("%04X: 0x%02X   ", x, param_memory[x]);
    if (((x + 1) % 4) == 0)
      printf("\n");
  }
}

void Debugger::disable() {
  mainSwitchEnable = 0;
  isOpcode = isIterator = 0;
}
