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
#include "include/opcode.hpp"
#include "include/cpu.hpp"
#include "include/mmu.hpp"
#include "include/debug.hpp"

char *FILE_PATH;
int rom_size;

// binary specific function
void handle_user_argument(int param_argc, char **param_argv) {
  while ((++param_argv)[0]) {
    if (param_argv[0][0] == '-') {
      switch (param_argv[0][1]) {
      case 'i':
        if ((param_argv[1] == nullptr) || std::string(param_argv[1]).empty()) {
          printf("error: provide file path\n");
          exit(1);
        } else {
          FILE_PATH = param_argv[1];
        }
        break;
      default:
        printf("-%c: Unknown option\n", param_argv[0][1]);
        exit(1);
      }
    }
  }
}
void load_binary(uint8_t *memory, int memory_size,
                 std::string param_file_path) {
  std::ifstream stream;
  stream.open(param_file_path, std::ios::binary | std::ios::in);
  if (stream.is_open()) {
    while (stream.good()) {
      stream.read(reinterpret_cast<char *>(memory), memory_size);
    }
  }
  stream.close();
}
bool is_file_exist(std::string name) {
  bool value = false;
  std::ifstream stream(name, std::ios::binary | std::ios::in);
  value = stream.is_open();
  stream.close();
  return value;
}
int read_file_size(std::string name) {
  int x = 0;
  std::ifstream stream(name, std::ios::binary | std::ios::in);
  if (stream.is_open()) {
    stream.seekg(0, std::ios::end);
    x = stream.tellg();
  }

  stream.close();
  return x;
}
void file_load(const uint8_t *param_memory, char *param_file_path) {
  if (is_file_exist(param_file_path)) {
    rom_size = read_file_size(param_file_path);
    if (rom_size == 0) {
      printf("%s: File invalid! 0 bytes.\n", param_file_path);
      exit(1);
    }
  } else {
    printf("%s: No file found in directory.\n", param_file_path);
    exit(1);
  }
}

// init
void init(uint8_t *param_memory) {
  load_binary(param_memory, rom_size, FILE_PATH);
  printf("%s: File loaded up to $%04X memory!\n", FILE_PATH, rom_size);
}

int main(int argc, char **argv) {
  uint8_t *gb_memory = new uint8_t[0xFFFF];
  bool prghalt = false;
  printf("%s\n", TITLE);

  handle_user_argument(argc, argv);
  file_load(gb_memory, FILE_PATH);
  init(gb_memory);

  printf("PROGRAM START\n-----\n");
  Mmu *mmu = new Mmu(gb_memory, rom_size, &prghalt);
  Cpu *cpu = new Cpu(mmu);
  Debug *debug = new Debug(cpu, mmu);

  cpu->cpuRegister.pc = 0x0100;
  cpu->cpuRegister.sp = 0xFFFE;
  cpu->cpuRegister.reg_a = 0x11;
  cpu->cpuRegister.reg_f = 0x80;
  cpu->cpuRegister.reg_b = 0x00;
  cpu->cpuRegister.reg_c = 0x00;
  cpu->cpuRegister.reg_pair_de = 0xFF56;
  cpu->cpuRegister.reg_pair_hl = 0x000D;

  while (!prghalt) {
    uint16_t pc;
    uint8_t opcode;
    // blarggs test - serial output
    if (gb_memory[0xff02] == 0x81) {
      char c = gb_memory[0xff01];
      printf("%c", c);
      gb_memory[0xff02] = 0x00;
    }
    //debug->startDebug();
    pc = cpu->cpuRegister.pc;
    opcode = gb_memory[pc];
    cpu->cpuRegister.pc += OP_BYTES[opcode];
    prghalt =  cpu->decode(pc, opcode);
  }
    //debug->endDebug();
  printf("\n-----\nPROGRAM END\n");
  return 0;
}
