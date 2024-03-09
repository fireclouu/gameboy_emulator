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
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

void runCpuIndividualTests(Host* host, uint8_t* romData) {
  const string PATH_DIR_TEST_CPU_INDIVIDUAL = "gb-test-roms/cpu_instrs/individual/";
  set<fs::path> sortedByName;
  host = new Host();

  for (auto & entry : fs::directory_iterator(PATH_DIR_TEST_CPU_INDIVIDUAL)) {
    sortedByName.insert(entry.path());
  }

  for (auto & fileName : sortedByName) {
    string testRomFilePath = fileName;
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
  if (argc == 1) {
    runCpuIndividualTests
  (host, romData);
  };

  while ((++argv)[0]) {
    if (argv[0][0] == '-') {
      char option = argv[0][1];
      string argument = argv[1] != NULL ? argv[1] : "";

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
          runCpuIndividualTests
        (host, romData);
          runCpuIndividualTests
        (host, romData);
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
