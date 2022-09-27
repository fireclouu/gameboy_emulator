/*
 * debug.cpp
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

#include <cstdint>
#include <istream>
#include <sstream>
#include <string>

#include "include/opcode.hpp"

Debug::Debug(Cpu *cpu, Mmu *mmu) {
  storeOpcode = storeIterate = storeFfwd = storePc = 0;
  debugDisable = false;
  this->cpu = cpu;
  this->mmu = mmu;
  break_n.breakCode = 0xFF;  // temporary break
}

void Debug::print() {
  int pc = cpu->cpuRegister.pc;
  int sp = cpu->cpuRegister.sp;
  printf("ITER: %lu\n", iterate);
  printf(
      "PC: %04X (%02X)  SP: %04X  BC: %04X  DE: %04X  HL: %04X  AF: %04X   "
      "%s\n",
      pc, mmu->readByte(pc), cpu->cpuRegister.sp, cpu->cpuRegister.reg_pair_bc,
      cpu->cpuRegister.reg_pair_de, cpu->cpuRegister.reg_pair_hl,
      cpu->cpuRegister.reg_pair_af,
      OP_INSTRUCTION[mmu->readByte(cpu->cpuRegister.pc)]);
  printf("MEMORY       STACK:\n");
  for (int x = 0; x < 4; x++) {
    if (pc + x < 0xFFFF)
      printf("[%04X: %02X]   ", pc + x, mmu->readByte(pc + x));
    if (sp + x < 0xFFFF)
      printf("[%04X: %02X]   ", sp + x, mmu->readByte(sp + x));
    printf("\n");
  }
}
void Debug::interact() {
  std::string convert;
  char x[100];
  char opt;
  std::cin.getline(x, 100);
  convert = x + 1;
  opt = x[0];

  std::stringstream ss;
  switch (opt) {
    case 'f':
      if (!(x[1] == '\0')) {
        break_n.breakCode = 0;
        break_n.ffwd = 1;
        storeFfwd = iterate + std::stoul(convert);
      }
      break;
    case 'c':
      break_n.breakCode = 0;
      break;
    case 'o': {
      if (!(x[1] == '\0')) {
        break_n.breakCode = 0;
        break_n.opcode = 1;
        ss << std::hex << convert;
        ss >> storeOpcode;
      }
    } break;
    case 'p':
      if (!(x[1] == '\0')) {
        break_n.breakCode = 0;
        break_n.pc = 1;
        ss << std::hex << convert;
        ss >> storePc;
      }
      break;
    case 'n':
      if (!(x[1] == '\0')) {
        break_n.breakCode = 0;
        break_n.next = 1;
        storeFfwd = iterate + std::stoul(convert);
      } else {
        break_n.breakCode = 0;
        break_n.next = 1;
        storeFfwd = 1;
      }
      break;
    case 'g':
      if (!(x[1] == '\0')) {
        break_n.breakCode = 0;
        break_n.step = 1;
      }
      break;
  }
}
void Debug::startDebug() {
  uint8_t opcode = mmu->readByte(cpu->cpuRegister.pc);
  opcodeTally[opcode]++;
  if (opcode == 0xCB) {
    uint8_t opcodeCb = mmu->readByte(cpu->cpuRegister.pc + 1);
    opcodeTallyCb[opcodeCb]++;
  }
  // forward, opc, pc
  if ((break_n.ffwd && storeFfwd < iterate) ||
      (break_n.opcode && opcode == storeOpcode) ||
      (break_n.pc && storePc == cpu->cpuRegister.pc) ||
      (break_n.next && storeFfwd < iterate) || (break_n.step)) {
    print();
    interact();
  }
  iterate++;
}
void Debug::endDebug() {
  printf("Program ended with %lu iterations!\n", iterate);
  printf("All used opcodes:\n");
  printf("Opcodes:\n");
  for (int a = 0; a < 0xFF; a++) {
    uint64_t tmp = opcodeTally[a];
    if (tmp) printf("%02X: %lu\n", a, tmp);
  }
  printf("-----------\n");
  printf("CB Opcodes:\n");
  for (int a = 0; a < 0xFF; a++) {
    uint64_t tmp = opcodeTallyCb[a];
    if (tmp) printf("%02X: %lu\n", a, tmp);
  }
  printf("debug end.");
}
