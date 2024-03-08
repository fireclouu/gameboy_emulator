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
#include "include/host.hpp"
#include <cstdio>
#include <cstring>

void runTest(Host* host, uint8_t* romData) {
  const string dirPathTestsIndividual = "gb-test-roms/cpu_instrs/individual/";
  const string testpaths[] = {
    "01-special.gb",
    "02-interrupts.gb",
    "03-op sp,hl.gb",
    "04-op r,imm.gb",
    "05-op rp.gb",
    "06-ld r,r.gb",
    "07-jr,jp,call,ret,rst.gb",
    "08-misc instrs.gb",
    "09-op r,r.gb",
    "10-bit ops.gb",
    "11-op a,(hl).gb",
  };

  host = new Host("");

  for (int i = 0; i < 11; i++) {
    string testRomFilePath = dirPathTestsIndividual + testpaths[i];
    if (!host->loadFile(testRomFilePath)) continue;
    romData = host->getRomData();

    // init modules
    Cpu *cpu = new Cpu();
    Mmu *mmu = new Mmu(romData);

    // init system
    Gameboy *gameboy = new Gameboy(cpu, mmu);
    gameboy->start();
  }

  exit(0);
}
int main(int argc, char **argv) {
  Host *host = NULL;
  uint8_t *romData = NULL;

  // user input

  if (argc == 1) exit(1);
  while ((++argv)[0]) {
    if (argv[0][0] == '-') {
      char option = argv[0][1];
      string argument = argv[1] != NULL ? argv[1] : "";

      printf("%s", argument.c_str());
      switch(option) {
        case 'i':
          if (argument.empty()) {
            printf("-%c: No file path provided.\n", option);
            exit(1);
          } else {
            host = new Host(argument);
          }
          break;

        case 't':
          runTest(host, romData);
          break;

        default:
          printf("-%c: Unknown option.\n", option);
          exit(1);
      }
    }
  }

  if (host->loadFileOnArgument()) {
    romData = host->getRomData();
    // init modules
    Cpu *cpu = new Cpu();
    Mmu *mmu = new Mmu(romData);
    // // init system
    Gameboy *gameboy = new Gameboy(cpu, mmu);
    gameboy->start();
  }

  return 0;
}
