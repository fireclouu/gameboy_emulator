/*
 * main.cpp
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

#include "include/main.hpp"

#include "include/debug.hpp"

Debug *debug;
Gameboy *gameboy;
void signalHandler(int signal) { gameboy->halt = true; }

int main(int argc, char **argv) {
  printf("%s\n", TITLE);
  gameboy = new Gameboy();
  Cpu *cpu = new Cpu();
  Mmu *mmu = new Mmu();
  Host *host = new Host(argc, argv, gameboy);
  host->loadFileOnArgument();
  gameboy->setup(cpu, mmu);

  // Debug
  debug = new Debug(gameboy, cpu, mmu);
  signal(SIGINT, signalHandler);


  printf("PROGRAM START\n-----\n");

  cpu->cpuRegister.pc = 0x0100;
  cpu->cpuRegister.sp = 0xFFFE;
  cpu->cpuRegister.reg_a = 0x11;
  cpu->cpuRegister.reg_f = 0x80;
  cpu->cpuRegister.reg_b = 0x00;
  cpu->cpuRegister.reg_c = 0x00;
  cpu->cpuRegister.reg_pair_de = 0xFF56;
  cpu->cpuRegister.reg_pair_hl = 0x000D;

  uint64_t tCycle = 0;
  const uint32_t MAX_T_CYCLE = 4194304;
  int cycle = 0;
  while (!gameboy->halt) {
    uint16_t pc;
    uint8_t opcode;

    // blarggs test - serial output
    if (mmu->readByte(0xff02) == 0x81) {
      char c = mmu->readByte(0xff01);
      printf("%c", c);
      mmu->writeByte(0xff02, 0);
    }
    debug->startDebug();
    pc = cpu->cpuRegister.pc;
    opcode = mmu->readByte(pc);
    cpu->cpuRegister.pc += OP_BYTES[opcode];
    // initial timing
    // lcd 4.194 MHz
    // 16.74ms / cycle
    cycle = cpu->decode(pc, opcode);

    tCycle += cycle;
    
    if (tCycle > MAX_T_CYCLE) tCycle = 0;

  }
  debug->endDebug();
  printf("\n-----\nPROGRAM END\n");
  return 0;
}
