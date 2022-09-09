#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "main.hpp"

std::string file_path;
int rom_size;
uint8_t reg_value, reg_variable, value_pre, value_post;
uint8_t gb_memory[0xFFFF];
uint8_t *ptr_op_reg_u8[8];
uint16_t *ptr_op_reg_u16[4];
bool program_stop;
Z80_Register reg_gb;
Z80_Register *ptr_gb_reg;

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
		program_stop = true;
	}
	memory[addr] = value;
};
// mmu end

// debugger
void dbg_checks(Z80_Register *param_reg) {
	if ( (param_reg->f) & 0x0F) {
		printf("flag lsb illegally written!\n");
		program_stop = true;
	}
}
// debugger end

// flags start
void flag_check_zero(Z80_Register *param_reg, int value) {
	param_reg->flag.z = (value == 0);
}

void flag_check_half(Z80_Register *param_reg, int value_before, int value_after) {
	param_reg->flag.h = ( (value_before & 0xF) + (value_after & 0xF) & 0x10);
}

void flag_check_subtract(Z80_Register *param_reg, int value) {
	param_reg->flag.n = value;
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

void init_registers(Z80_Register *param_reg, int value) {
	param_reg->pc = param_reg->sp =
		param_reg->af = param_reg->bc =
		param_reg->de = param_reg->hl = value;
}

void build_ptr_op_reg_u8(Z80_Register *param_reg, uint8_t *param_ptr_op_reg_u8[8]) {
	param_ptr_op_reg_u8[0] = &param_reg->b; // 0
	param_ptr_op_reg_u8[1] = &param_reg->c; // 1
	param_ptr_op_reg_u8[2] = &param_reg->d; // 2
	param_ptr_op_reg_u8[3] = &param_reg->e; // 3
	param_ptr_op_reg_u8[4] = &param_reg->h; // 4
	param_ptr_op_reg_u8[5] = &param_reg->l; // 5
	param_ptr_op_reg_u8[6] = &gb_memory[param_reg->hl]; // 6
	param_ptr_op_reg_u8[7] = &param_reg->a; // 7
}

void build_ptr_op_reg_u16(Z80_Register *param_reg, uint16_t *param_ptr_op_reg_u16[4]) {
	param_ptr_op_reg_u16[0] = &param_reg->bc; // 0
	param_ptr_op_reg_u16[1] = &param_reg->de; // 1
	param_ptr_op_reg_u16[2] = &param_reg->hl; // 2
	param_ptr_op_reg_u16[3] = &param_reg->sp; // 3
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

void print_step(Z80_Register param_reg, uint8_t *memory, std::string prefix) {
	printf("PC: %04X (0x%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  %s\n",
			param_reg.pc, memory[param_reg.pc],
			param_reg.af, param_reg.bc,
			param_reg.de, param_reg.hl,
			param_reg.sp, prefix.c_str());
	printf("MEMORY:\n");
	for (int x = 0; x < 4; x++) {
		printf("[ 0x%04X: 0x%02X ]\n", param_reg.pc + x, memory[param_reg.pc + x]);
	}
	printf("\n");
}

void prefix_cb(int opcode) {
	reg_variable = (opcode & 0x07);
	uint8_t hold_bit;
	uint8_t *hold_ptr_u8 = ptr_op_reg_u8[reg_variable];
	switch(opcode) {
		// RLC b, r
	/*	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07:
			reg_variable = (opcode & 0x07);
			ptr_gb_reg->pc = 0xfffff;
			dbg_stop_inject();
			break;*/
		// RL n
		case 0x10 ... 0x17:
			hold_bit = (*hold_ptr_u8 & 0x80) >> 7;
			*hold_ptr_u8 = (*hold_ptr_u8 << 1) | ptr_gb_reg->flag.c;
			ptr_gb_reg->flag.z = ( (*hold_ptr_u8) == 0 );
			ptr_gb_reg->flag.n = 0;
			ptr_gb_reg->flag.h = 0;
			ptr_gb_reg->flag.c = hold_bit;
			break;
		// BIT b, r ()
		case 0x40 ... 0x7F:
			reg_value = 1 << ((opcode & 0x38) >> 3); // MASK
			ptr_gb_reg->flag.z = ( (*hold_ptr_u8 & reg_value) == 0 );
			ptr_gb_reg->flag.n = 0;
			ptr_gb_reg->flag.h = 1;
			break;
		default:
			printf("0x%02X: Unknown CB opcode!\n", opcode);
			program_stop = true;
	}
}

// cpu specific functions
void cpu_stack_push(Z80_Register *param_reg, uint8_t *memory, uint16_t addr_value) {
	write_byte(memory, param_reg->sp--, (addr_value & 0xFF00) >> 8);
	write_byte(memory, param_reg->sp--, (addr_value & 0x00FF));
}
uint16_t cpu_stack_pop(Z80_Register *param_reg, uint8_t *memory) {
	value_pre = read_byte(memory, ++param_reg->sp);
	value_post = read_byte(memory, ++param_reg->sp);
	return ( (value_post) << 8 ) | value_pre;
}

int main(int argc, char **argv) {
	printf("%s\n", (char*)TITLE);

	while ((++argv)[0]) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
				case 'i':
					if (!(argv[1] == nullptr)) file_path = argv[1];
					if (file_path.empty()) {
						printf("error: provide file path\n");
						return 1;
					}
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

	ptr_gb_reg = &reg_gb;
	init_registers(ptr_gb_reg, 0);
	build_ptr_op_reg_u8(ptr_gb_reg, ptr_op_reg_u8);
	build_ptr_op_reg_u16(ptr_gb_reg, ptr_op_reg_u16);
	rom_size = read_file_size(file_path);

	if (rom_size == 0) {
		printf("%s: File loaded invalid. Size is 0 bytes!\n", file_path.c_str());
		return 1;
	}

	load_binary(gb_memory, rom_size, file_path);

	printf("%s: File loaded with %d bytes!\n",file_path.c_str(), rom_size);

	printf("PROGRAM START\n");

	uint8_t opcode, hold_byte;
	uint16_t pc, hold_addr;
	uint16_t *hold_ptr_u16;
	program_stop = false;

	while (!program_stop) {
		print_step(*ptr_gb_reg, gb_memory, "");
		pc = ptr_gb_reg->pc;
		opcode = gb_memory[pc];
		ptr_gb_reg->pc += OP_BYTES[opcode];

		switch (opcode) {
			// LD (reg, no HL), (reg, no HL)
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
			case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
			case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
			case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
			case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
				reg_value = (opcode & 7);
				reg_variable = (opcode & 0x38) >> 3;
				*ptr_op_reg_u8[reg_variable] = *ptr_op_reg_u8[reg_value];
				break;
			// LD reg, (HL)
			case 0x46: case 0x4E:
			case 0x56: case 0x5E:
			case 0x66: case 0x6E:
			case 0x76: case 0x7E:
				reg_variable = (opcode & 0x38) >> 3;
				*ptr_op_reg_u8[reg_variable] = read_byte(gb_memory, ptr_gb_reg->hl);
				break;
			// LD (HL), reg
			case 0x70: case 0x75: case 0x77:
				reg_value = (opcode & 0x07);
				write_byte(gb_memory, ptr_gb_reg->hl, *ptr_op_reg_u8[reg_value]);
				break;
			// LD (reg, no HL), d8
			case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x3E:
				reg_variable = (opcode & 0x38) >> 3;
				*ptr_op_reg_u8[reg_variable] =  read_byte(gb_memory, pc + 1);
				break;
			// LD (HL), d8
			case 0x36:
				hold_byte = read_byte(gb_memory, pc + 1);
				write_byte(gb_memory, ptr_gb_reg->hl, hold_byte);
				break;



			// PUSH
			case 0xC5: case 0xD5: case 0xE5: case 0xF5: 
				reg_variable = (0x3 & opcode);
				hold_ptr_u16 = &ptr_gb_reg->bc + reg_variable;
				cpu_stack_push(ptr_gb_reg, gb_memory, *hold_ptr_u16);
				break;
			case 0x00: // NOP
				break;
			case 0xCD:
				cpu_stack_push(ptr_gb_reg, gb_memory, pc + 3);
				ptr_gb_reg->pc = read_short(gb_memory, pc + 1);
				break;
			// 2 byte addrpairs
			// LD A, 
			case 0x0A: // BC
				ptr_gb_reg->a = read_byte(gb_memory, ptr_gb_reg->bc);
				break;
			case 0x1A: // DE
				ptr_gb_reg->a = read_byte(gb_memory, ptr_gb_reg->de);
				break;
			// LD rr, d8
			case 0x01: case 0x11: case 0x21: case 0x31:
				reg_variable = (0x30 & opcode) >> 4;
				*ptr_op_reg_u16[reg_variable] = read_short(gb_memory, pc + 1);
				break;
			// INC reg
			case 0x04: case 0x0C: case 0x14: case 0x1C:
			case 0x24: case 0x2C: case 0x3C:
				reg_variable = opcode & 0x38 >> 3;
				value_pre = *ptr_op_reg_u8[reg_variable]++;
				value_post = *ptr_op_reg_u8[reg_variable];
				flag_check_zero(ptr_gb_reg, value_post);
				flag_check_half(ptr_gb_reg, value_pre, value_post);
				flag_check_subtract(ptr_gb_reg, 0);
				break;
			case 0x20: // JMP NZ, signed d8 + pc
				if (ptr_gb_reg->flag.z == 0) {
					ptr_gb_reg->pc += int8_t(read_byte(gb_memory, pc + 1));
				}
				break;
			case 0x32: // LD (HL-), A
				write_byte(gb_memory, ptr_gb_reg->hl--, ptr_gb_reg->a);
				break;
			case 0xAF: // XOR A
				ptr_gb_reg->a ^= ptr_gb_reg->a;
				ptr_gb_reg->f = (ptr_gb_reg->a == 0) << FLAG_POS_ZERO;
				ptr_gb_reg->f &= FLAG_MASK_ZERO;
				break;
			case 0xCB: // PREFIX CB
				print_step(*ptr_gb_reg, gb_memory, "(CB)");
				pc = ptr_gb_reg->pc;
				opcode = gb_memory[pc];
				ptr_gb_reg->pc++;
				prefix_cb(opcode);
				break;
			// LOADS ($ff(a8))
			case 0xE0: // LDH (a8), A
				hold_addr = 0xFF00 | read_byte(gb_memory, pc + 1);
				write_byte(gb_memory, hold_addr, ptr_gb_reg->a);
				break;
			case 0xE2: // LD (C), A
				hold_addr = 0xFF00 | ptr_gb_reg->c;
				write_byte(gb_memory, hold_addr, ptr_gb_reg->a);
				break;
			case 0xF0: // LDH A, (a8)
				hold_addr = 0xFF00 | read_byte(gb_memory, pc + 1);
				ptr_gb_reg->a = read_byte(gb_memory, hold_addr);
				break;
			default:
				printf("0x%02X: Unknown opcode!\n", opcode);
				return 1;
		}
	}
	printf("PROGRAM ENDED\n");

	return 0;
}
