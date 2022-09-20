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
#include "include/opcode.hpp"

bool program_stop;
uint8_t hold_u8_pre, hold_u8_post;
uint8_t *gb_memory = new uint8_t[0xFFFF];
uint8_t *ptr_op_reg_u8[8];
uint16_t *ptr_op_reg_u16[4];
char* FILE_PATH;
int rom_size;
GB_Register gb_register;
GB_Register *ptr_gb_reg;
Debugger *debugger;
bool debug_mode = 0;
// mmu
uint8_t read_byte(const uint8_t *memory, uint16_t addr) { return memory[addr]; }
uint16_t read_short(const uint8_t *memory, uint16_t addr) {
  return (memory[addr + 1] << 8) + memory[addr];
}
void write_byte(uint8_t *memory, uint16_t addr, uint8_t value) {
  if (addr < rom_size) {
    printf("ERROR (W): 0x%04X: overwriting ROM file!\n", addr);
    debugger->dbgEnd(1);
    program_stop = true;
  } else {
    memory[addr] = value;
  }
}
// mmu end
// flags start
void flag_do_h(GB_Register *param_reg, int value_left, int value_right,
               bool subtraction) {
  if (subtraction) {
    param_reg->flag_h =
        uint8_t((value_left - value_right) & 0xF0) < (value_left & 0xF0) ? 1
                                                                         : 0;
  } else {
    param_reg->flag_h =
        (((value_left & 0xF) + (value_right & 0xF)) > 0xF) ? 1 : 0;
  }
}

void build_ptr_op_reg_u8(GB_Register *param_reg,
                         uint8_t *param_ptr_op_reg_u8[8]) {
  param_ptr_op_reg_u8[0] = &param_reg->reg_b;  // 0
  param_ptr_op_reg_u8[1] = &param_reg->reg_c;  // 1
  param_ptr_op_reg_u8[2] = &param_reg->reg_d;  // 2
  param_ptr_op_reg_u8[3] = &param_reg->reg_e;  // 3
  param_ptr_op_reg_u8[4] = &param_reg->reg_h;  // 4
  param_ptr_op_reg_u8[5] = &param_reg->reg_l;  // 5
  param_ptr_op_reg_u8[6] = gb_memory;          // 6
  param_ptr_op_reg_u8[7] = &param_reg->reg_a;  // 7
}
void build_ptr_op_reg_u16(GB_Register *param_reg,
                          uint16_t *param_ptr_op_reg_u16[4]) {
  param_ptr_op_reg_u16[0] = &param_reg->reg_pair_bc;  // 0
  param_ptr_op_reg_u16[1] = &param_reg->reg_pair_de;  // 1
  param_ptr_op_reg_u16[2] = &param_reg->reg_pair_hl;  // 2
  param_ptr_op_reg_u16[3] = &param_reg->sp;           // 3
}

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
      case 'd':
        debug_mode = 1;
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
void init() {
  ptr_gb_reg = &gb_register;
  build_ptr_op_reg_u8(ptr_gb_reg, ptr_op_reg_u8);
  build_ptr_op_reg_u16(ptr_gb_reg, ptr_op_reg_u16);
  load_binary(gb_memory, rom_size, FILE_PATH);
  printf("%s: File loaded up to $%04X memory!\n", FILE_PATH, rom_size);
}

// cpu specific functions
void cpu_stack_push(GB_Register *param_reg, uint8_t *memory,
                    uint16_t addr_value) {
  write_byte(memory, param_reg->sp--, (addr_value & 0xFF00) >> 8);
  write_byte(memory, param_reg->sp--, (addr_value & 0x00FF));
}
uint16_t cpu_stack_pop(GB_Register *param_reg, uint8_t *memory) {
  hold_u8_pre = read_byte(memory, ++param_reg->sp);
  hold_u8_post = read_byte(memory, ++param_reg->sp);
  return ((hold_u8_post) << 8) + (hold_u8_pre);
}
void cpu_cc_jp_signed(GB_Register *param_reg, uint8_t param_flag,
                      uint8_t expected_bit, int8_t signed_byte_value) {
  if (param_flag == expected_bit) {
    param_reg->pc += int8_t(signed_byte_value);
  }
}
void cpu_cc_jp_unsigned(GB_Register *param_reg, uint8_t param_flag,
                        uint8_t expected_bit, uint8_t unsigned_byte_value) {
  if (param_flag == expected_bit) {
    param_reg->pc += uint8_t(unsigned_byte_value);
  }
}
void cpu_cc_ret(GB_Register *param_reg, uint8_t param_flag,
                uint8_t expected_bit) {
  if (param_flag == expected_bit) {
    param_reg->pc = cpu_stack_pop(ptr_gb_reg, gb_memory);
  }
}
void cpu_cc_call(GB_Register *param_reg, uint8_t *param_mem, uint8_t param_flag,
                 uint8_t expected_bit, uint16_t param_pc) {
  if (param_flag == expected_bit) {
    cpu_stack_push(param_reg, param_mem, param_pc + 3);
    param_reg->pc = read_short(param_mem, param_pc + 1);
  }
}
void cpu_instruction_xor(GB_Register *param_reg, uint8_t param_value) {
  param_reg->reg_a ^= param_value;
  param_reg->flag_z = (param_reg->reg_a == 0) ? 1 : 0;
  param_reg->flag_n = 0;
  param_reg->flag_h = 0;
  param_reg->flag_c = 0;
}
void cpu_instruction_cp(GB_Register *param_reg, uint8_t param_value) {
  param_reg->flag_z = (param_reg->reg_a == param_value) ? 1 : 0;
  param_reg->flag_n = 1;
  flag_do_h(param_reg, param_reg->reg_a, param_value, true);
  param_reg->flag_c = (param_reg->reg_a < param_value);
}
void cpu_instruction_or(GB_Register *param_reg, uint8_t param_value) {
  param_reg->reg_a |= param_value;
  param_reg->flag_z = (param_reg->reg_a == 0) ? 1 : 0;
  param_reg->flag_n = 0;
  param_reg->flag_h = 0;
  param_reg->flag_c = 0;
}
void cpu_instruction_and(GB_Register *param_reg, uint8_t param_value) {
  param_reg->reg_a &= param_value;
  param_reg->flag_z = (param_reg->reg_a == 0) ? 1 : 0;
  param_reg->flag_n = 0;
  param_reg->flag_h = 1;
  param_reg->flag_c = 0;
}

void prefix_cb(int opcode) {
  uint8_t parse_opcode_addr;
  uint8_t hold_bit, hold_byte;
  uint8_t *hold_ptr_u8;
  uint8_t hold_mask, hold_memval_hl;

  parse_opcode_addr = (opcode & 0x07);
  hold_ptr_u8 = ptr_op_reg_u8[parse_opcode_addr];

  switch (opcode) {
  // RLC r
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x07:
    hold_bit = (((*hold_ptr_u8) & 0x80) >> 7);
    (*hold_ptr_u8) = ((*hold_ptr_u8) << 1) + hold_bit;
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // RRC r
  case 0x08:
  case 0x09:
  case 0x0A:
  case 0x0B:
  case 0x0C:
  case 0x0D:
  case 0x0F:
    hold_bit = ((*hold_ptr_u8) & 0x01);
    (*hold_ptr_u8) = (hold_bit << 7) | ((*hold_ptr_u8) >> 1);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // RL n
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x17:
    hold_bit = ((*hold_ptr_u8) & 0x80) >> 7;
    (*hold_ptr_u8) = ((*hold_ptr_u8) << 1) + ptr_gb_reg->flag_c;
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // RR r
  case 0x18:
  case 0x19:
  case 0x1A:
  case 0x1B:
  case 0x1C:
  case 0x1D:
  case 0x1F:
    hold_bit = ((*hold_ptr_u8) & 1);
    (*hold_ptr_u8) = (ptr_gb_reg->flag_c << 7) + ((*hold_ptr_u8) >> 1);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // SLA n
  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x25:
  case 0x27:
    hold_bit = (((*hold_ptr_u8) & 0x80) >> 7);
    (*hold_ptr_u8) = ((*hold_ptr_u8) << 1) & (0xFE);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // SRA n
  case 0x28:
  case 0x29:
  case 0x2A:
  case 0x2B:
  case 0x2C:
  case 0x2D:
  case 0x2F:
    hold_byte = ((*hold_ptr_u8) & 0x80);
    ptr_gb_reg->flag_c = ((*hold_ptr_u8) & 0x01);
    (*hold_ptr_u8) = hold_byte | ((*hold_ptr_u8) >> 1);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    break;
  // SWAP n
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
  case 0x37:
    (*hold_ptr_u8) =
        (((*hold_ptr_u8) & 0x0F) << 4) + (((*hold_ptr_u8) & 0xF0) >> 4);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_c = 0;
    break;
  // SRL n
  case 0x38:
  case 0x39:
  case 0x3A:
  case 0x3B:
  case 0x3C:
  case 0x3D:
  case 0x3F:
    hold_bit = ((*hold_ptr_u8) & 0x01);
    (*hold_ptr_u8) = ((*hold_ptr_u8) >> 1) & (0x7F);
    ptr_gb_reg->flag_z = ((*hold_ptr_u8) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // BIT b, r
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x47:
  case 0x48:
  case 0x49:
  case 0x4A:
  case 0x4B:
  case 0x4C:
  case 0x4D:
  case 0x4F:
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x57:
  case 0x58:
  case 0x59:
  case 0x5A:
  case 0x5B:
  case 0x5C:
  case 0x5D:
  case 0x5F:
  case 0x60:
  case 0x61:
  case 0x62:
  case 0x63:
  case 0x64:
  case 0x65:
  case 0x67:
  case 0x68:
  case 0x69:
  case 0x6A:
  case 0x6B:
  case 0x6C:
  case 0x6D:
  case 0x6F:
  case 0x70:
  case 0x71:
  case 0x72:
  case 0x73:
  case 0x74:
  case 0x75:
  case 0x77:
  case 0x78:
  case 0x79:
  case 0x7A:
  case 0x7B:
  case 0x7C:
  case 0x7D:
  case 0x7F:
    hold_mask = 1 << ((opcode & 0x38) >> 3);
    ptr_gb_reg->flag_z = (((*hold_ptr_u8) & hold_mask) == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 1;
    break;
  // RES v, r
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
  case 0x84:
  case 0x85:
  case 0x87:
  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B:
  case 0x8C:
  case 0x8D:
  case 0x8F:
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x97:
  case 0x98:
  case 0x99:
  case 0x9A:
  case 0x9B:
  case 0x9C:
  case 0x9D:
  case 0x9F:
  case 0xA0:
  case 0xA1:
  case 0xA2:
  case 0xA3:
  case 0xA4:
  case 0xA5:
  case 0xA7:
  case 0xA8:
  case 0xA9:
  case 0xAA:
  case 0xAB:
  case 0xAC:
  case 0xAD:
  case 0xAF:
  case 0xB0:
  case 0xB1:
  case 0xB2:
  case 0xB3:
  case 0xB4:
  case 0xB5:
  case 0xB7:
  case 0xB8:
  case 0xB9:
  case 0xBA:
  case 0xBB:
  case 0xBC:
  case 0xBD:
  case 0xBF:
  // SET v, r
  case 0xC0:
  case 0xC1:
  case 0xC2:
  case 0xC3:
  case 0xC4:
  case 0xC5:
  case 0xC7:
  case 0xC8:
  case 0xC9:
  case 0xCA:
  case 0xCB:
  case 0xCC:
  case 0xCD:
  case 0xCF:
  case 0xD0:
  case 0xD1:
  case 0xD2:
  case 0xD3:
  case 0xD4:
  case 0xD5:
  case 0xD7:
  case 0xD8:
  case 0xD9:
  case 0xDA:
  case 0xDB:
  case 0xDC:
  case 0xDD:
  case 0xDF:
  case 0xE0:
  case 0xE1:
  case 0xE2:
  case 0xE3:
  case 0xE4:
  case 0xE5:
  case 0xE7:
  case 0xE8:
  case 0xE9:
  case 0xEA:
  case 0xEB:
  case 0xEC:
  case 0xED:
  case 0xEF:
  case 0xF0:
  case 0xF1:
  case 0xF2:
  case 0xF3:
  case 0xF4:
  case 0xF5:
  case 0xF7:
  case 0xF8:
  case 0xF9:
  case 0xFA:
  case 0xFB:
  case 0xFC:
  case 0xFD:
  case 0xFF:
    hold_bit = ((opcode & 0x40) >> 6);
    hold_byte = (opcode & 0x38) >> 3;
    hold_mask = (!hold_bit) ? (0xFF ^ (1 << hold_byte)) : (1 << hold_byte);
    (*hold_ptr_u8) &= hold_mask;
    break;

  // HL
  // RLC
  case 0x06:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = ((hold_memval_hl & 0x80) >> 7);
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
               ((hold_memval_hl << 1) + hold_bit));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = (hold_memval_hl == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // RRC
  case 0x0E:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = (hold_memval_hl & 0x01);
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
               ((hold_bit << 7) | ((hold_memval_hl) >> 1)));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = (hold_memval_hl == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
  // RL
  case 0x16:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = (hold_memval_hl & 0x80) >> 7;
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
               ((hold_memval_hl << 1) + ptr_gb_reg->flag_c));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = (hold_memval_hl == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // RR
  case 0x1E:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = (hold_memval_hl & 1);
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
               ((ptr_gb_reg->flag_c << 7) + ((*hold_ptr_u8) >> 1)));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = (hold_memval_hl == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // SLA
  case 0x26:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = (hold_memval_hl & 0x80) >> 7;
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
               ((hold_memval_hl << 1) & (0xFE)));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = (hold_memval_hl == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // SRA
  case 0x2E:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_byte = ((hold_memval_hl) & 0x80);
    ptr_gb_reg->flag_c = ((hold_memval_hl) & 0x01);
  write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
      (hold_byte | ((hold_memval_hl) >> 1)));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = ((hold_memval_hl) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    break;
  // SWAP
  case 0x36:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
        (((hold_memval_hl) & 0x0F) << 4) + (((hold_memval_hl) & 0xF0) >> 4));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = ((hold_memval_hl) == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_c = 0;
    break;
  // SRL n
  case 0x3E:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = ((hold_memval_hl) & 0x01);
  write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
             ((hold_memval_hl) >> 1) & (0x7F));
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    ptr_gb_reg->flag_z = ((hold_memval_hl) == 0) ? 1 : 0;
    ptr_gb_reg->flag_h = 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_c = hold_bit;
    break;
  // BIT v, HL
  case 0x46:
  case 0x4E:
  case 0x56:
  case 0x5E:
  case 0x66:
  case 0x6E:
  case 0x76:
  case 0x7E:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_mask = 1 << ((opcode & 0x38) >> 3);
    ptr_gb_reg->flag_z = ((hold_memval_hl & hold_mask) == 0) ? 1 : 0;
    ptr_gb_reg->flag_n = 0;
    ptr_gb_reg->flag_h = 1;
    break;
  // RES v, HL
  case 0x86:
  case 0x8E:
  case 0x96:
  case 0x9E:
  case 0xA6:
  case 0xAE:
  case 0xB6:
  case 0xBE:
  // SET v, HL
  case 0xC6:
  case 0xCE:
  case 0xD6:
  case 0xDE:
  case 0xE6:
  case 0xEE:
  case 0xF6:
  case 0xFE:
    hold_memval_hl = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
    hold_bit = ((opcode & 0x40) >> 6);
    hold_byte = (opcode & 0x38) >> 3;
    hold_mask = (!hold_bit) ? (0xFF ^ (1 << hold_byte)) : (1 << hold_byte);
  write_byte(gb_memory, ptr_gb_reg->reg_pair_hl, hold_memval_hl & hold_mask);
    break;
  default:
    printf("0x%02X: Unknown CB opcode!\n", opcode);
    program_stop = true;
  }
}

int main(int argc, char **argv) {
  uint8_t opcode, hold_byte, hold_bit;
  uint8_t parse_opcode_addr, parse_opcode_val;
  uint16_t hold_addr;
  uint16_t *hold_ptr_u16;

  printf("%s\n", TITLE);

  handle_user_argument(argc, argv);
  file_load(gb_memory, FILE_PATH);
  init();

  printf("PROGRAM START\n-----\n");
  program_stop = false;

  ptr_gb_reg->pc = 0x0100;
  ptr_gb_reg->sp = 0xFFFE;
  ptr_gb_reg->reg_a = 0x11;
  ptr_gb_reg->reg_f = 0x80;
  ptr_gb_reg->reg_b = 0x00;
  ptr_gb_reg->reg_c = 0x00;
  ptr_gb_reg->reg_pair_de = 0xFF56;
  ptr_gb_reg->reg_pair_hl = 0x000D;

  // debugs
  debugger = new Debugger(ptr_gb_reg, gb_memory);
  if (!debug_mode)
    debugger->disable();
  // signal(SIGINT, debugger->dbgEnd(1));

  while (!program_stop) {
    uint16_t pc;
    debugger->debugStepInteractive();
    // blarggs test - serial output
    if (gb_memory[0xff02] == 0x81) {
      char c = gb_memory[0xff01];
      printf("%c", c);
      gb_memory[0xff02] = 0x00;
    }

    pc = ptr_gb_reg->pc;
    opcode = gb_memory[pc];
    ptr_gb_reg->pc += OP_BYTES[opcode];

    switch (opcode) {
    // ILLEGAL
    case 0xD3:
    case 0xDB:
    case 0xDD:
    case 0xE3:
    case 0xE4:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xF4:
    case 0xFC:
    case 0xFD:
      printf("ACCESSED ILLEGAL OPCODE!\n");
      ptr_gb_reg->pc++;
      break;
    // SPECIAL
    // NOP
    case 0x00:
    case 0x10:
    case 0x76:
    case 0xF3:
    case 0xFB:
      break;
    // ROTATES AND SHIFTS
    // RLCA
    case 0x07:
      hold_bit = ((ptr_gb_reg->reg_a & 0x80) >> 7);
      ptr_gb_reg->reg_a = (ptr_gb_reg->reg_a << 1) + hold_bit;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_h = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_c = hold_bit;
      break;
    // RLA
    case 0x17:
      hold_bit = (ptr_gb_reg->reg_a & 0x80) >> 7;
      ptr_gb_reg->reg_a = (ptr_gb_reg->reg_a << 1) + ptr_gb_reg->flag_c;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_h = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_c = hold_bit;
      break;
    // RRCA
    case 0x0F:
      hold_bit = (ptr_gb_reg->reg_a & 1);
      ptr_gb_reg->reg_a = (hold_bit << 7) + (ptr_gb_reg->reg_a >> 1);
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_h = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_c = hold_bit;
      break;
    // RRA
    case 0x1F:
      hold_bit = (ptr_gb_reg->reg_a & 1);
      ptr_gb_reg->reg_a = (ptr_gb_reg->flag_c << 7) + (ptr_gb_reg->reg_a >> 1);
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_h = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_c = hold_bit;
      break;

    // LOADS 8-bit
    // LD (reg, no HL), d8
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
    case 0x3E:
      parse_opcode_addr = (opcode & 0x38) >> 3;
      (*ptr_op_reg_u8[parse_opcode_addr]) = read_byte(gb_memory, pc + 1);
      break;
    // LD A,rr
    case 0x0A:
    case 0x1A:
      parse_opcode_addr = (opcode & 0x30) >> 4;
      hold_addr = (*ptr_op_reg_u16[parse_opcode_addr]);
      ptr_gb_reg->reg_a = read_byte(gb_memory, hold_addr);
      break;
    // LD A, HL+
    case 0x2A:
      ptr_gb_reg->reg_a = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl++);
      break;
    // LD A, HL-
    case 0x3A:
      ptr_gb_reg->reg_a = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl--);
      break;
    // LD (reg, no HL), (reg, no HL)
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4F:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x57:
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6F:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7F:
      parse_opcode_val = (opcode & 7);
      parse_opcode_addr = (opcode & 0x38) >> 3;
      (*ptr_op_reg_u8[parse_opcode_addr]) = (*ptr_op_reg_u8[parse_opcode_val]);
      break;
    // LD reg, (HL)
    case 0x46:
    case 0x4E:
    case 0x56:
    case 0x5E:
    case 0x66:
    case 0x6E:
    case 0x7E:
      parse_opcode_addr = (opcode & 0x38) >> 3;
      hold_addr = (*ptr_op_reg_u8[parse_opcode_addr]);
      hold_byte = read_byte(gb_memory, hold_addr);
      (*ptr_op_reg_u8[parse_opcode_addr]) = hold_byte;
      break;
    // LDH (a8), A
    case 0xE0:
      hold_addr = 0xFF00 + read_byte(gb_memory, pc + 1);
      write_byte(gb_memory, hold_addr, ptr_gb_reg->reg_a);
      break;
  // LDH A, (a8)
    case 0xF0:
      hold_addr = 0xFF00 + read_byte(gb_memory, pc + 1);
      ptr_gb_reg->reg_a = read_byte(gb_memory, hold_addr);
      break;
  // LD (C), A
    case 0xE2:
      hold_addr = 0xFF00 + ptr_gb_reg->reg_c;
      write_byte(gb_memory, hold_addr, ptr_gb_reg->reg_a);
      break;
  // LDH A, (C)
    case 0xF2:
      hold_addr = 0xFF00 + ptr_gb_reg->reg_c;
      ptr_gb_reg->reg_a = read_byte(gb_memory, hold_addr);
      break;

    // LOADS 16-bit
    // LD rr, d8
    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
      parse_opcode_addr = (0x30 & opcode) >> 4;
      (*ptr_op_reg_u16[parse_opcode_addr]) = read_short(gb_memory, pc + 1);
      break;
    // LD (rr), A
    case 0x02:
    case 0x12:
      parse_opcode_addr = (opcode & 0x30) >> 4;
      hold_addr = (*ptr_op_reg_u16[parse_opcode_addr]);
      write_byte(gb_memory, hold_addr, ptr_gb_reg->reg_a);
      break;
    // LD (nn), SP
    case 0x08:
      hold_addr = read_short(gb_memory, pc + 1);
      write_byte(gb_memory, hold_addr, ptr_gb_reg->sp);
      break;
    // LD (HL+), A
    case 0x22:
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl++, ptr_gb_reg->reg_a);
      break;
    // LD (HL-), A
    case 0x32:
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl--, ptr_gb_reg->reg_a);
      break;
    // LD (HL), d8
    case 0x36:
      hold_byte = read_byte(gb_memory, pc + 1);
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl, hold_byte);
      break;
    // LD (HL), reg
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x77:
      parse_opcode_addr = (opcode & 0x07);
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl,
                 (*ptr_op_reg_u8[parse_opcode_addr]));
      break;
    // LD HL, SP+r8
    case 0xF8:
      hold_byte = read_byte(gb_memory, pc + 1);
      ptr_gb_reg->flag_h =
          uint32_t((ptr_gb_reg->sp & 0x0FFF) + ((hold_byte)&0x0FFF)) > 0x0FFF
              ? 1
              : 0;
      ptr_gb_reg->flag_c =
          uint32_t(ptr_gb_reg->sp + (int8_t)(hold_byte)) > 0xFFFF ? 1 : 0;
      ptr_gb_reg->flag_z = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->reg_pair_hl = (ptr_gb_reg->sp + (int8_t)(hold_byte));
      break;
    // LD SP, HL
    case 0xF9:
      ptr_gb_reg->sp = ptr_gb_reg->reg_pair_hl;
      break;
    // LD (nn), A
    case 0xEA:
      hold_addr = read_short(gb_memory, pc + 1);
      write_byte(gb_memory, hold_addr, ptr_gb_reg->reg_a);
      break;
    // LD A, d16
    case 0xFA:
      hold_addr = read_short(gb_memory, pc + 1);
      ptr_gb_reg->reg_a = read_byte(gb_memory, hold_addr);
      break;

    // JUMPS AND STACKS
    // JR r8
    case 0x18:
      ptr_gb_reg->pc += int8_t(read_byte(gb_memory, pc + 1));
      break;
  // JP NZ, signed d8 + pc
    case 0x20:
  // JP Z, signed d8 + pc
    case 0x28:
      parse_opcode_val = ((opcode & 0x08) >> 3);
      cpu_cc_jp_signed(ptr_gb_reg, ptr_gb_reg->flag_z, parse_opcode_val,
                       int8_t(read_byte(gb_memory, pc + 1)));
      break;
  // JP NC, signed d8 + pc
    case 0x30:
  // JP C, signed d8 + pc
    case 0x38:
      parse_opcode_val = ((opcode & 0x08) >> 3);
      cpu_cc_jp_signed(ptr_gb_reg, ptr_gb_reg->flag_c, parse_opcode_val,
                       int8_t(read_byte(gb_memory, pc + 1)));
      break;
    // RET Z FLAG
    case 0xC0:
    case 0xC8:
      parse_opcode_val = ((opcode & 0x08) >> 3);
      cpu_cc_ret(ptr_gb_reg, ptr_gb_reg->flag_z, parse_opcode_val);
      break;
    // RET C FLAG
    case 0xD0:
    case 0xD8:
      parse_opcode_val = ((opcode & 0x08) >> 3);
      cpu_cc_ret(ptr_gb_reg, ptr_gb_reg->flag_c, parse_opcode_val);
      break;
    // POP rr
    case 0xC1:
    case 0xD1:
    case 0xE1:
      parse_opcode_addr = (0x30 & opcode) >> 4;
      hold_ptr_u16 = (&ptr_gb_reg->reg_pair_bc) + parse_opcode_addr;
      (*hold_ptr_u16) = cpu_stack_pop(ptr_gb_reg, gb_memory);
      break;
    // POP AF
    case 0xF1:
      ptr_gb_reg->reg_pair_af = cpu_stack_pop(ptr_gb_reg, gb_memory) & 0xFFF0;
      break;
    // JP Z FLAG, a16
    case 0xC2:
    case 0xCA:
      parse_opcode_val = (0x08 & opcode) >> 3;
      cpu_cc_jp_unsigned(ptr_gb_reg, ptr_gb_reg->flag_z, parse_opcode_val,
                         read_short(gb_memory, pc + 1));
      break;
    // JP a16
    case 0xC3:
      ptr_gb_reg->pc = read_short(gb_memory, pc + 1);
      break;
    // CALL Z FLAG
    case 0xC4:
    case 0xCC:
      parse_opcode_val = (0x08 & opcode) >> 3;
      cpu_cc_call(ptr_gb_reg, gb_memory, ptr_gb_reg->flag_z, parse_opcode_val,
                  pc);
      break;
    // CALL C FLAG
    case 0xD4:
    case 0xDC:
      parse_opcode_val = (0x08 & opcode) >> 3;
      cpu_cc_call(ptr_gb_reg, gb_memory, ptr_gb_reg->flag_c, parse_opcode_val,
                  pc);
      break;
    // JP C FLAG, a16
    case 0xD2:
    case 0xDA:
      parse_opcode_val = (0x08 & opcode) >> 3;
      cpu_cc_jp_unsigned(ptr_gb_reg, ptr_gb_reg->flag_c, parse_opcode_val,
                         read_short(gb_memory, pc + 1));
      break;
    // PUSH rr
    case 0xC5:
    case 0xD5:
    case 0xE5:
      parse_opcode_addr = (0x30 & opcode) >> 4;
      hold_ptr_u16 = (&ptr_gb_reg->reg_pair_bc) + parse_opcode_addr;
      cpu_stack_push(ptr_gb_reg, gb_memory, (*hold_ptr_u16));
      break;
    // PUSH AF
    case 0xF5:
      cpu_stack_push(ptr_gb_reg, gb_memory, ptr_gb_reg->reg_pair_af & 0xFFF0);
      break;
    // RET
    case 0xC9:
      ptr_gb_reg->pc = cpu_stack_pop(ptr_gb_reg, gb_memory);
      break;
    // CALL
    case 0xCD:
      cpu_stack_push(ptr_gb_reg, gb_memory, pc + 3);
      ptr_gb_reg->pc = read_short(gb_memory, pc + 1);
      break;
    // JP HL
    case 0xE9:
      ptr_gb_reg->pc = ptr_gb_reg->reg_pair_hl;
      break;
    // RST nn
    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
      hold_byte = (opcode & 0x38);
      cpu_stack_push(ptr_gb_reg, gb_memory, pc + 1);
      ptr_gb_reg->pc = hold_byte;
      break;
    // ALU 8-bit
    // INC reg
    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x3C:
      parse_opcode_addr = (opcode & 0x38) >> 3;
      hold_u8_pre = (*ptr_op_reg_u8[parse_opcode_addr])++;
      hold_u8_post = (*ptr_op_reg_u8[parse_opcode_addr]);
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      flag_do_h(ptr_gb_reg, hold_u8_pre, 1, false);
      ptr_gb_reg->flag_n = 0;
      break;
    // INC (HL)
    case 0x34:
      hold_u8_pre = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      hold_u8_post = hold_u8_pre + 1;
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl, hold_u8_post);
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      flag_do_h(ptr_gb_reg, hold_u8_pre, 1, false);
      ptr_gb_reg->flag_n = 0;
      break;
    // DEC reg
    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x3D:
      parse_opcode_addr = (0x38 & opcode) >> 3;
      hold_u8_pre = (*ptr_op_reg_u8[parse_opcode_addr])--;
      hold_u8_post = (*ptr_op_reg_u8[parse_opcode_addr]);
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      flag_do_h(ptr_gb_reg, hold_u8_pre, 1, true);
      ptr_gb_reg->flag_n = 1;
      break;
    // DEC (HL)
    case 0x35:
      hold_u8_pre = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      hold_u8_post = hold_u8_pre - 1;
      write_byte(gb_memory, ptr_gb_reg->reg_pair_hl, hold_u8_post);
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      flag_do_h(ptr_gb_reg, hold_u8_pre, 1, true);
      ptr_gb_reg->flag_n = 1;
      break;

    // ALU 16-bit
    // INC rr
    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
      parse_opcode_addr = (0x30 & opcode) >> 4;
      (*ptr_op_reg_u16[parse_opcode_addr])++;
      break;
    // ADD HL, rr
    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
      parse_opcode_addr = (opcode & 0x30) >> 4;
      hold_ptr_u16 = ptr_op_reg_u16[parse_opcode_addr];
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_h = uint32_t((ptr_gb_reg->reg_pair_hl & 0x0FFF) +
                                    ((*hold_ptr_u16) & 0x0FFF)) > 0x0FFF
                               ? 1
                               : 0;
      ptr_gb_reg->flag_c =
          uint32_t(ptr_gb_reg->reg_pair_hl + (*hold_ptr_u16)) > 0xFFFF ? 1 : 0;
      ptr_gb_reg->reg_pair_hl += (*hold_ptr_u16);
      break;
    // DEC rr
    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
      parse_opcode_addr = (0x30 & opcode) >> 4;
      (*ptr_op_reg_u16[parse_opcode_addr])--;
      break;
    // ADD SP, r8
    case 0xE8:
      hold_byte = read_byte(gb_memory, pc + 1);
      ptr_gb_reg->flag_h =
          uint32_t((ptr_gb_reg->sp & 0x0FFF) + ((hold_byte)&0x0FFF)) > 0x0FFF
              ? 1
              : 0;
      ptr_gb_reg->flag_c =
          uint32_t(ptr_gb_reg->sp + (int8_t)(hold_byte)) > 0xFFFF ? 1 : 0;
      ptr_gb_reg->flag_z = 0;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->sp += ((int8_t)hold_byte);
      break;

    // LOGIC
    // ADD A, r
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x87:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]);
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // ADD A, (HL)
    case 0x86:
      hold_byte = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // ADD A, d8
    case 0xC6:
      hold_byte = read_byte(gb_memory, pc + 1);
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // ADC A, r
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x8F:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]) + ptr_gb_reg->flag_c;
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // ADC A, (HL)
    case 0x8E:
      hold_byte = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // ADC A, d8
    case 0xCE:
      hold_byte = read_byte(gb_memory, pc + 1);
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, false);
      ptr_gb_reg->flag_c =
          (uint16_t(ptr_gb_reg->reg_a + hold_byte) > 0xFF) ? 1 : 0;
      ptr_gb_reg->reg_a += hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 0;
      break;
    // SUB r
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x97:
      parse_opcode_addr = (opcode & 0x7);
      hold_u8_pre = (*ptr_op_reg_u8[parse_opcode_addr]);
      hold_u8_post = (ptr_gb_reg->reg_a - hold_u8_pre);
      ptr_gb_reg->reg_a = hold_u8_post;
      flag_do_h(ptr_gb_reg, hold_u8_pre, hold_u8_post, true);
      ptr_gb_reg->flag_c = (hold_u8_pre < hold_u8_post) ? 0 : 1;
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // SUB (HL)
    case 0x96:
      hold_u8_pre = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      hold_u8_post = (ptr_gb_reg->reg_a - hold_u8_pre);
      ptr_gb_reg->reg_a = hold_u8_post;
      flag_do_h(ptr_gb_reg, hold_u8_pre, hold_u8_post, true);
      ptr_gb_reg->flag_c = (hold_u8_pre < hold_u8_post) ? 0 : 1;
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // SUB d8
    case 0xD6:
      hold_u8_pre = read_byte(gb_memory, pc + 1);
      hold_u8_post = (ptr_gb_reg->reg_a - hold_u8_pre);
      ptr_gb_reg->reg_a = hold_u8_post;
      flag_do_h(ptr_gb_reg, hold_u8_pre, hold_u8_post, true);
      ptr_gb_reg->flag_c = (hold_u8_pre < hold_u8_post) ? 0 : 1;
      ptr_gb_reg->flag_z = (hold_u8_post == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // SBC A, r
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9F:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]) + ptr_gb_reg->flag_c;
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, true);
      ptr_gb_reg->flag_c = (ptr_gb_reg->reg_a < hold_byte) ? 0 : 1;
      ptr_gb_reg->reg_a -= hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // SBC A, (HL)
    case 0x9E:
      hold_byte =
          read_byte(gb_memory, ptr_gb_reg->reg_pair_hl) + ptr_gb_reg->flag_c;
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, true);
      ptr_gb_reg->flag_c = (ptr_gb_reg->reg_a < hold_byte) ? 0 : 1;
      ptr_gb_reg->reg_a -= hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // SBC A, d8
    case 0xDE:
      hold_byte = read_byte(gb_memory, pc + 1) + ptr_gb_reg->flag_c;
      flag_do_h(ptr_gb_reg, ptr_gb_reg->reg_a, hold_byte, true);
      ptr_gb_reg->flag_c = (ptr_gb_reg->reg_a < hold_byte) ? 0 : 1;
      ptr_gb_reg->reg_a -= hold_byte;
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_n = 1;
      break;
    // XOR r
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
    case 0xAF:
      parse_opcode_addr = (opcode & 0x07);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]);
      cpu_instruction_xor(ptr_gb_reg, hold_byte);
      break;
    // XOR (HL)
    case 0xAE:
      hold_byte = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      cpu_instruction_xor(ptr_gb_reg, hold_byte);
      break;
    // XOR d8
    case 0xEE:
      hold_byte = read_byte(gb_memory, pc + 1);
      cpu_instruction_xor(ptr_gb_reg, hold_byte);
      break;
    // AND r
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA7:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]);
      cpu_instruction_and(ptr_gb_reg, hold_byte);
      break;
    // AND (HL)
    case 0xA6:
      hold_byte = (read_byte(gb_memory, ptr_gb_reg->reg_pair_hl));
      cpu_instruction_and(ptr_gb_reg, hold_byte);
      break;
    // AND d8
    case 0xE6:
      hold_byte = (read_byte(gb_memory, pc + 1));
      cpu_instruction_and(ptr_gb_reg, hold_byte);
      break;
    // OR r
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB7:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]);
      cpu_instruction_or(ptr_gb_reg, hold_byte);
      break;
    // OR (HL)
    case 0xB6:
      hold_byte = (read_byte(gb_memory, ptr_gb_reg->reg_pair_hl));
      cpu_instruction_or(ptr_gb_reg, hold_byte);
      break;
    // OR d8
    case 0xF6:
      hold_byte = (read_byte(gb_memory, pc + 1));
      cpu_instruction_or(ptr_gb_reg, hold_byte);
      break;
    // CP r
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBF:
      parse_opcode_addr = (opcode & 0x7);
      hold_byte = (*ptr_op_reg_u8[parse_opcode_addr]);
      cpu_instruction_cp(ptr_gb_reg, hold_byte);
      break;
    // CP (HL)
    case 0xBE:
      hold_byte = read_byte(gb_memory, ptr_gb_reg->reg_pair_hl);
      cpu_instruction_cp(ptr_gb_reg, hold_byte);
      break;
    // CP d8
    case 0xFE:
      hold_byte = read_byte(gb_memory, pc + 1);
      cpu_instruction_cp(ptr_gb_reg, hold_byte);
      break;

    // SCF
    case 0x37:
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_h = 0;
      ptr_gb_reg->flag_c = 1;
      break;
    // DAA
    case 0x27:
      if ((ptr_gb_reg->reg_a & 0x0F) > 0x09 || ptr_gb_reg->flag_h) {
        ptr_gb_reg->reg_a += 0x06;
      }
      if (((ptr_gb_reg->reg_a & 0xF0) >> 4) > 0x09 || ptr_gb_reg->flag_c) {
        ptr_gb_reg->reg_a += 0x60;
        ptr_gb_reg->flag_c = 1;
      } else {
        ptr_gb_reg->flag_c = 0;
      }
      ptr_gb_reg->flag_z = (ptr_gb_reg->reg_a == 0) ? 1 : 0;
      ptr_gb_reg->flag_h = 0;
      break;
    // CPL
    case 0x2F:
      ptr_gb_reg->reg_a = ~ptr_gb_reg->reg_a;
      ptr_gb_reg->flag_n = 1;
      ptr_gb_reg->flag_h = 1;
      break;
    // CCF
    case 0x3F:
      ptr_gb_reg->flag_c = (ptr_gb_reg->flag_c) ? 0 : 1;
      ptr_gb_reg->flag_n = 0;
      ptr_gb_reg->flag_h = 0;
      break;
  // Prefix CB
    case 0xCB:
      debugger->debugStepInteractive();
      pc = ptr_gb_reg->pc;
      opcode = gb_memory[pc];
      ptr_gb_reg->pc++;
      prefix_cb(opcode);
      break;

    default:
      printf("0x%02X: Unknown opcode!\n", opcode);
      program_stop = 1;
    }
    read_byte(gb_memory, 1);
  }

  debugger->dbgEnd(0);
  printf("\n-----\nPROGRAM END\n");

  return 0;
}
