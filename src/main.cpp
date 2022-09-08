#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "main.hpp"

std::string file_path;
uint8_t reg_value, reg_variable, value_pre, value_post;
int rom_size;
uint8_t gb_memory[0xFFFF];
bool running;
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
uint8_t read_byte(uint8_t *memory, uint16_t addr) {
	return memory[addr];
};

uint16_t read_short(uint8_t *memory, uint16_t addr) {
	return (memory[addr + 1] << 8) | memory[addr];
};

void write_byte(uint8_t *memory, uint16_t addr, uint8_t value) {
	if (addr < 0x00ff) {
		printf("\n\nWRITE_ADDR: 0x%04X: invalid write! overwriting ROM FILE!\n", addr);
		running = false;
	}
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

bool is_file_exist(const std::string name) {
	bool value = false;
	std::ifstream stream(name, std::ios::binary | std::ios::in);
	value = stream.is_open();
	stream.close();
	return value;
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

void load_binary(uint8_t *memory, int memory_size, const std::string param_file_path) {
	std::ifstream stream;
	stream.open(param_file_path, std::ios::binary | std::ios::in);
	if(stream.is_open()) {
		while(stream.good()) {
			stream.read((char *)memory, memory_size);
		}
	}
	stream.close();
}

void print_memory(int memory_size) {
	for (int x = 0; x < memory_size; x++) {
		printf("%04X: 0x%02X   ", x, gb_memory[x]);
		if (((x +1) % 4) == 0) printf("\n");
	}
}

void print_step(Z80_Register reg_param, uint8_t *memory, std::string prefix) {
	printf("PC: %04X (%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  %s\n",
			reg_param.pc, memory[reg_param.pc],
			reg_param.af, reg_param.bc,
			reg_param.de, reg_param.hl,
			reg_param.sp, prefix.c_str());
	printf("MEMORY:\n");
	for (int x = 0; x < 4; x++) {
		printf("[ 0x%04X: 0x%02X ]\n", reg_param.pc + x, memory[reg_param.pc + x]);
	}
	printf("\n");
}

void dbg_stop_inject() {
	gb_memory[0] = 0;
	reg_gb_ptr->pc = 0;
}

void prefix_cb(int opcode) {
	switch(opcode) {
		// RLC b, r
	/*	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07:
			reg_variable = (opcode & 0x07);
			reg_gb_ptr->pc = 0xfffff;
			dbg_stop_inject();
			break;*/
		// BIT b, r ()
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
			reg_value = 1 << ((opcode & 0x38) >> 3); // MASK
			reg_variable = (opcode & 0x07);
			reg_variable = ( (reg_variable % 2) == 0) ? reg_variable + 1 : reg_variable - 1;
			reg_gb_ptr->flag.z = ( (reg_gb_ptr->register_general[reg_variable] & reg_value) == 0 );
			reg_gb_ptr->flag.n = 0;
			reg_gb_ptr->flag.h = 1;
			break;
		default:
			printf("0x%02X: Unknown CB opcode!\n", opcode);
	}
}

// cpu specific functions
void cpu_stack_push(Z80_Register *reg_param, uint8_t *memory, uint16_t addr_value) {
	write_byte(memory, reg_param->sp--, (addr_value & 0xFF00) >> 8);
	write_byte(memory, reg_param->sp--, (addr_value & 0x00FF));
}
uint16_t cpu_stack_pop(Z80_Register *reg_param, uint8_t *memory) {
	value_pre = read_byte(memory, ++reg_param->sp);
	value_post = read_byte(memory, ++reg_param->sp);
	return ( (value_post) << 8 ) | value_pre;
}

int main(int argc, char **argv) {
	printf("%s\n", (char*)TITLE);

	while ((++argv)[0]) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
				case 'i':
					file_path = argv[1];
					break;
				default:
					printf("-%c: Unknown option\n", argv[0][1]);
					return 1;
			}
		}
	}

	if (!is_file_exist(file_path)) {
		printf("%s: No file found in directory.\n", file_path.c_str());
		return 1;
	}

	reg_gb_ptr = &reg_gb;
	init_registers(reg_gb_ptr, 0);
	rom_size = read_file_size(file_path);

	if (rom_size == 0) {
		printf("%s: File loaded invalid. Size is 0 bytes!\n", file_path.c_str());
		return 1;
	}

	load_binary(gb_memory, rom_size, file_path);

	printf("%s: File loaded with %d bytes!\n",file_path.c_str(), rom_size);

	printf("PROGRAM START\n");

	uint8_t opcode;
	uint16_t pc, addr_hold;
	uint16_t *ptr_hold;
	running = true;
	while (true) {
		print_step(*reg_gb_ptr, gb_memory, "");
		pc = reg_gb_ptr->pc;
		opcode = gb_memory[pc];
		reg_gb_ptr->pc += OP_BYTES[opcode];
		switch (opcode) {
			case 0x00:
				printf("NOP TRIGGERED!\n");
				return 1;
			case 0xCD:
				cpu_stack_push(reg_gb_ptr, gb_memory, pc + 3);
				reg_gb_ptr->pc = read_short(gb_memory, pc + 1);
				break;
			// 2 byte addrpairs
			// LD A, 
			case 0x0A: // BC
				reg_gb_ptr->a = read_byte(gb_memory, reg_gb_ptr->bc);
				break;
			case 0x1A: // DE
				reg_gb_ptr->a = read_byte(gb_memory, reg_gb_ptr->de);
				break;
			case 0x7E: // HL
				reg_gb_ptr->a = read_byte(gb_memory, reg_gb_ptr->hl);
			// 2byte addresspair loads
			case 0x01: case 0x11: case 0x21: case 0x31:
				reg_variable = (0x30 & opcode) >> 4;
				if (opcode == 0x31) reg_variable++;
				ptr_hold = &reg_gb_ptr->bc + reg_variable;
				*ptr_hold = read_short(gb_memory, pc + 1);
				break;
			// increment jumps z0h-
			case 0x04: case 0x0C: case 0x14: case 0x1C:
			case 0x24: case 0x2C: case 0x3C:
				reg_variable = opcode & 0x38;
				value_pre = reg_gb_ptr->register_general[ reg_variable ];
				reg_gb_ptr->register_general[ reg_variable ]++;
				value_post = reg_gb_ptr->register_general[ reg_variable ];
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
				write_byte(gb_memory, reg_gb_ptr->hl, reg_gb_ptr->a);
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
				addr_hold = 0xFF00 | read_byte(gb_memory, pc + 1);
				write_byte(gb_memory, addr_hold, reg_gb_ptr->a);
				break;
			case 0xE2: // LD (C), A
				addr_hold = 0xFF00 | reg_gb_ptr->c;
				write_byte(gb_memory, addr_hold, reg_gb_ptr->a);
				break;
			case 0xF0: // LDH A, (a8)
				addr_hold = 0xFF00 | read_byte(gb_memory, pc + 1);
				reg_gb_ptr->a = read_byte(gb_memory, addr_hold);
				break;
			default:
				printf("0x%02X: Unknown opcode!\n", opcode);
				return 1;
		}
	}
	printf("PROGRAM ENDED\n");

	return 0;
}
