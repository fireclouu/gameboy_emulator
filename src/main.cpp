#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "main.hpp"

const std::string FILE_PATH = "assets/gb_bios.bin";
int bit_position, reg_position, value_pre, value_post;
int rom_size;
uint8_t gb_memory[0xFFFF] = {};

Z80_Register reg_gb;
Z80_Register *reg_gb_ptr;

const int OP_BYTES[] = {
	1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
	1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
	2, 1, 1, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
	2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1,
};

// mmu
uint8_t read_byte(uint8_t *memory, int addr) {
	return memory[addr];
};

uint16_t read_short(uint8_t *memory, int addr) {
	return read_byte(memory, addr + 1) << 8 | 
		read_byte(memory, addr); 
};

void write_byte(uint8_t *memory, uint16_t addr, uint8_t value) {
	memory[addr] = value;
};
// mmu end

// flags start
void flag_check_zero(Z80_Register *reg_param, int value) {
	reg_param->flag.z = (value == 0);
}

void flag_check_half(Z80_Register *reg_param, int value_before, int value_after) {
	reg_param->flag.h = ( (value_before & 0xF) + (value_after & 0xF) & 0x10);
}

void flag_check_subtract(Z80_Register *reg_param, int value) {
	reg_param->flag.n = value;
}

int read_file_size(const std::string name) {
	int x = 0;
	std::ifstream stream(name, std::ios::binary | std::ios::in);
	if (stream.is_open()) {
		stream.seekg(0, std::ios::end);
		x = stream.tellg();
	}
	stream.close();
	return x;
}

void init_registers(Z80_Register *reg_param, int value) {
	reg_param->pc = reg_param->sp =
		reg_param->af = reg_param->bc =
		reg_param->de = reg_param->hl = value;
}

void load_binary(uint8_t *memory, int memory_size, const std::string file_path) {
	std::ifstream stream;
	stream.open(file_path, std::ios::binary | std::ios::in);
	if(stream.is_open()) {
		while(stream.good()) {
			stream.read((char *)memory,
					sizeof(char) * memory_size);
		}
	}
	stream.close();
}

void print_memory(int memory_size) {
	for (int x = 0; x < memory_size; x++) {
		printf("%03d: 0x%02X   ", x, gb_memory[x]);
		if (((x +1) % 4) == 0) printf("\n");
	}
}

void print_step(Z80_Register reg_param, uint8_t *memory, std::string prefix) {
	printf("PC: %04d (%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  %s\n",
			reg_param.pc, memory[reg_param.pc],
			reg_param.af, reg_param.bc,
			reg_param.de, reg_param.hl,
			reg_param.sp, prefix.c_str());
}

void prefix_cb(int opcode) {
	switch(opcode) {
		// BIT b, r ()
		case 0x40 ... 0x7F:
			bit_position = (opcode & 0b00111000) >> 3;
			reg_position = (opcode & 0b00000111);
			reg_position = ( (reg_position % 2) == 0) ?
				reg_position + 1 : reg_position - 1;
			if (reg_position == 6) {
				printf("memory hl not implemented yet.\n");
				break;
			}
			reg_gb_ptr->flag.z = ( (reg_gb_ptr->register_general[reg_position] &
						bit_position) == 0);
			reg_gb_ptr->flag.n = 0;
			reg_gb_ptr->flag.h = 1;
			break;
		default:
			printf("0x%02X: Unknown CB opcode!\n", opcode);
	}
}

int main(int argc, char **argv) {
	reg_gb_ptr = &reg_gb;
	printf("%s\n", (char*)TITLE);

	init_registers(reg_gb_ptr, 0);
	rom_size = read_file_size(FILE_PATH);
	load_binary(gb_memory, rom_size, FILE_PATH);
	printf("%s loaded with %d bytes!\n",FILE_PATH.c_str(), rom_size);
	printf("PROGRAM START\n");
	uint8_t pc, opcode, byte_hold;
	uint16_t *ptr_hold;
	while (true) {
		print_step(*reg_gb_ptr, gb_memory, "");
		pc = reg_gb_ptr->pc;
		opcode = gb_memory[pc];
		reg_gb_ptr->pc += OP_BYTES[opcode];
		switch (opcode) {
			// 2byte addresspair loads
			case 0x01: case 0x11: case 0x21: case 0x31:
				reg_position = (0x30 & opcode) >> 4;
				if (opcode == 0x31) reg_position++;
				ptr_hold = &reg_gb_ptr->bc + reg_position;
				*ptr_hold = read_short(gb_memory, pc + 1);
				break;
			// increment jumps z0h-
			case 0x04: case 0x0C: case 0x14: case 0x1C:
			case 0x24: case 0x2C: case 0x34: case 0x3C:
				if (opcode == 0x34) {
					printf("0x%02X: unimplemented!!\n", opcode);
					break;
				}
				reg_position = opcode & 0b00111000;
				value_pre = reg_gb_ptr->register_general[ reg_position ];
				reg_gb_ptr->register_general[ reg_position ]++;
				value_post = reg_gb_ptr->register_general[ reg_position ];
				flag_check_zero(reg_gb_ptr, value_post);
				flag_check_half(reg_gb_ptr, value_pre, value_post);
				flag_check_subtract(reg_gb_ptr, 0);
				break;
			case 0x0E: // LD C, d8
				reg_gb_ptr->c = read_byte(gb_memory, pc + 1);
				break;
			case 0x20: // JMP NZ, signed d8 + pc
				if (reg_gb_ptr->flag.z == 0) {
					reg_gb_ptr->pc += int8_t(read_byte(gb_memory, pc + 1));
				}
				break;
			case 0x3E: // LD A, d8
				reg_gb_ptr->a = read_byte(gb_memory, pc + 1);
				break;
			case 0x32: // LD (HL-), A
				write_byte(gb_memory, reg_gb_ptr->hl--, reg_gb_ptr->a);
				break;
			case 0x77: // LD (HL), A
				reg_gb_ptr->a = read_byte(gb_memory, reg_gb_ptr->hl);
				break;
			case 0xAF: // XOR A
				reg_gb_ptr->a ^= reg_gb_ptr->a;
				reg_gb_ptr->f = (reg_gb_ptr->a == 0) << FLAG_POS_ZERO;
				reg_gb_ptr->f &= FLAG_MASK_ZERO;
				break;
			case 0xCB:
				print_step(*reg_gb_ptr, gb_memory, "(CB)");
				pc = reg_gb_ptr->pc;
				opcode = gb_memory[pc];
				reg_gb_ptr->pc++;
				prefix_cb(opcode);
				break;

			// LOADS ($ff(a8))
			case 0xE0: // LDH (a8), A
			case 0XF0: // LDH A, (a8)
				byte_hold = 0xFF00 | read_byte(gb_memory, pc + 1);
				if (opcode == 0xE0) {
					write_byte(gb_memory, byte_hold, reg_gb_ptr->a);
				} else if (opcode == 0xF0) {
					reg_gb_ptr->a = read_byte(gb_memory, byte_hold);
				}
				break;
			case 0xE2: // LD A, (C)
				reg_gb_ptr->a = read_byte(gb_memory, (0xFF00 | reg_gb_ptr->c) );
				break;
			default:
				printf("0x%02X: Unknown opcode!\n", opcode);
				printf("PROGRAM ENDED\n");
				return 1;
		}
	}

	return 0;
}
