#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "main.hpp"

std::string FILE_PATH;
int rom_size;
uint8_t hold_u8_pre, hold_u8_post;
uint8_t gb_memory[0xFFFF];
uint8_t *ptr_op_reg_u8[8];
uint16_t *ptr_op_reg_u16[4];
bool program_stop;
Z80_Register gb_register;
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
	return (memory[addr + 1] << 8) + memory[addr];
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
void flag_do_z(Z80_Register *param_reg, int value) {
	param_reg->flag.z = (!value);
}

void flag_do_n(Z80_Register *param_reg, int value) {
	param_reg->flag.n = value;
}

void flag_do_h(Z80_Register *param_reg, int value_left, int value_right, bool subtraction) {
	if (subtraction) {
		param_reg->flag.h = uint8_t((value_left & 0xF) - (value_right & 0xF)) > value_left ? 1 : 0;
	} else {
		param_reg->flag.h = ( ((value_left & 0xF) + (value_right & 0xF)) & 0x10) ? 1 : 0;
	}
}

void build_ptr_op_reg_u8(Z80_Register *param_reg, uint8_t *param_ptr_op_reg_u8[8]) {
	param_ptr_op_reg_u8[0] = &param_reg->b; // 0
	param_ptr_op_reg_u8[1] = &param_reg->c; // 1
	param_ptr_op_reg_u8[2] = &param_reg->d; // 2
	param_ptr_op_reg_u8[3] = &param_reg->e; // 3
	param_ptr_op_reg_u8[4] = &param_reg->h; // 4
	param_ptr_op_reg_u8[5] = &param_reg->l; // 5
	param_ptr_op_reg_u8[6] = gb_memory; // 6
	param_ptr_op_reg_u8[7] = &param_reg->a; // 7
}

void build_ptr_op_reg_u16(Z80_Register *param_reg, uint16_t *param_ptr_op_reg_u16[4]) {
	param_ptr_op_reg_u16[0] = &param_reg->bc; // 0
	param_ptr_op_reg_u16[1] = &param_reg->de; // 1
	param_ptr_op_reg_u16[2] = &param_reg->hl; // 2
	param_ptr_op_reg_u16[3] = &param_reg->sp; // 3
}

void print_memory(uint8_t *param_memory, int param_mem_size) {
	for (int x = 0; x < param_mem_size; x++) {
		printf("%04X: 0x%02X   ", x, param_memory[x]);
		if (((x +1) % 4) == 0) printf("\n");
	}
}

void print_step(Z80_Register param_reg, uint8_t *memory, std::string prefix) {
	printf("PC: %04X (0x%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  %s\n",
			param_reg.pc, memory[param_reg.pc],
			param_reg.af, param_reg.bc,
			param_reg.de, param_reg.hl,
			param_reg.sp, prefix.c_str());
	printf("MEMORY and STACK:\n");
	for (int x = 0; x < 4; x++) {
		int hold_pc = param_reg.pc + x;
		int hold_sp = param_reg.sp + x;
		if (hold_pc <= 0xFFFF) {
			printf("[ 0x%04X: 0x%02X ]   ", hold_pc, memory[hold_pc]);
		}
		if (hold_sp <= 0xFFFF) {
			printf("[ 0x%04X: 0x%02X ]", hold_sp, memory[hold_sp]);
		}
		printf("\n");
	}
	printf("\n");
}

// cpu specific functions
void cpu_stack_push(Z80_Register *param_reg, uint8_t *memory, uint16_t addr_value) {
	write_byte(memory, param_reg->sp--, (addr_value & 0x00FF));
	write_byte(memory, param_reg->sp--, (addr_value & 0xFF00) >> 8);
}
uint16_t cpu_stack_pop(Z80_Register *param_reg, uint8_t *memory) {
	hold_u8_pre = read_byte(memory, ++param_reg->sp);
	hold_u8_post = read_byte(memory, ++param_reg->sp);
	return ( (hold_u8_pre) << 8 ) | (hold_u8_post);
}

void cpu_conditon_jump_signed(Z80_Register *param_reg, uint8_t param_flag, uint8_t expected_bit, int8_t signed_byte_value) {
	if (param_flag == expected_bit) {
		param_reg->pc += signed_byte_value;
	}
}

// binary specific function
int handle_user_argument(int param_argc, char **param_argv) {
	int exit_code = 0;
	while ((++param_argv)[0]) {
		if (param_argv[0][0] == '-') {
			switch (param_argv[0][1]) {
				case 'i':
					if ((param_argv[1] == nullptr) || std::string(param_argv[1]).empty()) {
						printf("error: provide file path\n");
						exit_code = 1;
					} else {
						FILE_PATH = param_argv[1];
					}
					break;
				default:
					printf("-%c: Unknown option\n", param_argv[0][1]);
					exit_code = 1;
			}
		}
	}
	return exit_code;
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

int file_load(uint8_t *param_memory, std::string param_file_path) {
	int exit_code = 0;

	if (is_file_exist(param_file_path)) {
		rom_size = read_file_size(param_file_path);
		if (rom_size == 0) {
			printf("%s: File invalid! 0 bytes.\n", param_file_path.c_str());
			exit_code = 1;
		}
	} else {
		printf("%s: No file found in directory.\n", param_file_path.c_str());
		exit_code = 1;
	}

	return exit_code;
}

// init
int init() {
	int exit_code = 0;

	ptr_gb_reg = &gb_register;
	build_ptr_op_reg_u8(ptr_gb_reg, ptr_op_reg_u8);
	build_ptr_op_reg_u16(ptr_gb_reg, ptr_op_reg_u16);
	load_binary(gb_memory, rom_size, FILE_PATH);
	printf("%s: File loaded with %d bytes!\n",FILE_PATH.c_str(), rom_size);
	return exit_code;
}

// PREFIX CB
void prefix_cb(int opcode) {
	uint8_t reg_variable_u8, reg_value_u8;
	uint8_t hold_bit;
	uint8_t *hold_ptr_u8;

	reg_variable_u8 = (opcode & 0x07);
	hold_ptr_u8 = ptr_op_reg_u8[reg_variable_u8];

	switch(opcode) {
		// RLC b, r
	/*	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07:
			reg_variable_u8 = (opcode & 0x07);
			ptr_gb_reg->pc = 0xfffff;
			dbg_stop_inject();
			break;*/
		// RL n
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17:
			hold_bit = ((*hold_ptr_u8) & 0x80) >> 7;
			*hold_ptr_u8 = ((*hold_ptr_u8) << 1) + ptr_gb_reg->flag.c;
			ptr_gb_reg->flag.z = ( (*hold_ptr_u8) == 0 ) ? 1 : 0;
			ptr_gb_reg->flag.n = 0;
			ptr_gb_reg->flag.h = 0;
			ptr_gb_reg->flag.c = hold_bit;
			break;
		// BIT b, r
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
			reg_value_u8 = 1 << ((opcode & 0x38) >> 3); // MASK
			ptr_gb_reg->flag.z = ( ((*hold_ptr_u8) & reg_value_u8) == 0 ) ? 1 : 0;
			ptr_gb_reg->flag.n = 0;
			ptr_gb_reg->flag.h = 1;
			break;
		default:
			printf("0x%02X: Unknown CB opcode!\n", opcode);
			program_stop = true;
	}
}

int main(int argc, char **argv) {
	uint8_t opcode, hold_byte, hold_bit;
	uint8_t reg_variable_u8, reg_value_u8;
	uint16_t pc, hold_addr;
	uint16_t *hold_ptr_u16;

	printf("%s\n", (char*)TITLE);

	if( handle_user_argument(argc, argv) ||
			file_load(gb_memory, FILE_PATH)  ||
			init()
			) return 1;

	printf("PROGRAM START\n");
	program_stop = false;

	while (!program_stop) {
		print_step(*ptr_gb_reg, gb_memory, "");
		pc = ptr_gb_reg->pc;
		opcode = gb_memory[pc];
		ptr_gb_reg->pc += OP_BYTES[opcode];

		switch (opcode) {
			// ROTATES AND SHIFTS
			// RLCA
			case 0x07:
				hold_bit = ((ptr_gb_reg->a & 0x80) >> 7);
				ptr_gb_reg->a = (ptr_gb_reg->a << 1) + hold_bit;
				ptr_gb_reg->flag.z = ( ptr_gb_reg->a == 0 ) ? 1 : 0;
				ptr_gb_reg->flag.h = 0;
				ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.c = hold_bit;
				break;
			// RLA
			case 0x17:
				hold_bit = (ptr_gb_reg->a & 0x80) >> 7;
				ptr_gb_reg->a = (ptr_gb_reg->a << 1) + ptr_gb_reg->flag.c;
				ptr_gb_reg->flag.z = (ptr_gb_reg->a == 0) ? 1 : 0;
				ptr_gb_reg->flag.h = ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.c = hold_bit;
				break;
			// RRCA
			case 0x0F:
				hold_bit = (ptr_gb_reg->a & 1);
				ptr_gb_reg->a = (hold_bit << 7) + (ptr_gb_reg->a >> 1);
				ptr_gb_reg->flag.z = (ptr_gb_reg->a == 0) ? 1 : 0;
				ptr_gb_reg->flag.h = 0;
				ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.c = hold_bit;
				break;
			// RRA
			case 0x1F:
				hold_bit = (ptr_gb_reg->a & 1);
				ptr_gb_reg->a = (ptr_gb_reg->c << 7) + (ptr_gb_reg->a >> 1);
				ptr_gb_reg->flag.z = (ptr_gb_reg->a == 0) ? 1 : 0;
				ptr_gb_reg->flag.h = ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.c = hold_bit;
				break;

			// LOADS 8-bit
			// LD (reg, no HL), d8
			case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x3E:
				reg_variable_u8 = (opcode & 0x38) >> 3;
				(*ptr_op_reg_u8[reg_variable_u8]) =  read_byte(gb_memory, pc + 1);
				break;
			// LD A,rr
			case 0x0A: case 0x1A:
				reg_variable_u8 = (opcode & 0x30) >> 4;
				hold_addr = (*ptr_op_reg_u16[reg_variable_u8]);
				ptr_gb_reg->a = read_byte(gb_memory, hold_addr);
				break;
			// LD A, HL+
			case 0x2A:
				ptr_gb_reg->a = read_byte(gb_memory, ptr_gb_reg->hl++);
				break;
			// LD A, HL-
			case 0x3A:
				ptr_gb_reg->a = read_byte(gb_memory, ptr_gb_reg->hl--);
				break;
			// LD (reg, no HL), (reg, no HL)
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
			case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
			case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
			case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
			case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
				reg_value_u8 = (opcode & 7);
				reg_variable_u8 = (opcode & 0x38) >> 3;
				(*ptr_op_reg_u8[reg_variable_u8]) = (*ptr_op_reg_u8[reg_value_u8]);
				break;
			// LD reg, (HL)
			case 0x46: case 0x4E: case 0x56: case 0x5E: case 0x66: case 0x6E: case 0x7E:
				reg_variable_u8 = (opcode & 0x38) >> 3;
				(*ptr_op_reg_u8[reg_variable_u8]) = read_byte(gb_memory, ptr_gb_reg->hl);
				break;
			case 0xE0: // LDH (a8), A
				hold_addr = 0xFF00 + read_byte(gb_memory, pc + 1);
				write_byte(gb_memory, hold_addr, ptr_gb_reg->a);
				break;
			case 0xF0: // LDH A, (a8)
				hold_addr = 0xFF00 + read_byte(gb_memory, pc + 1);
				ptr_gb_reg->a = read_byte(gb_memory, hold_addr);
				break;
			case 0xE2: // LD (C), A
				hold_addr = 0xFF00 + ptr_gb_reg->c;
				write_byte(gb_memory, hold_addr, ptr_gb_reg->a);
				break;
			case 0xF2: // LDH A, (C)
				hold_addr = 0xFF00 + ptr_gb_reg->c;
				ptr_gb_reg->a = read_byte(gb_memory, hold_addr);
				break;

			// LOADS 16-bit
			// LD rr, d8
			case 0x01: case 0x11: case 0x21: case 0x31:
				reg_variable_u8 = (0x30 & opcode) >> 4;
				(*ptr_op_reg_u16[reg_variable_u8]) = read_short(gb_memory, pc + 1);
				break;
			// LD (rr), A
			case 0x02: case 0x12:
				reg_variable_u8 = (opcode & 0x30) >> 4;
				write_byte(gb_memory, (*ptr_op_reg_u16[reg_variable_u8]), ptr_gb_reg->a);
				break;
			// LD (a16), SP
			case 0x08:
				hold_addr = read_short(gb_memory, ptr_gb_reg->pc + 1);
				write_byte(gb_memory, hold_addr, ptr_gb_reg->sp);
				break;
			// LD (HL+), A
			case 0x22:
				write_byte(gb_memory, ptr_gb_reg->hl++, ptr_gb_reg->a);
				break;
			// LD (HL-), A
			case 0x32:
				write_byte(gb_memory, ptr_gb_reg->hl--, ptr_gb_reg->a);
				break;
			// LD (HL), d8
			case 0x36:
				hold_byte = read_byte(gb_memory, pc + 1);
				write_byte(gb_memory, ptr_gb_reg->hl, hold_byte);
				break;
			// LD (HL), reg
			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
				reg_value_u8 = (opcode & 0x07);
				write_byte(gb_memory, ptr_gb_reg->hl, (*ptr_op_reg_u8[reg_value_u8]) );
				break;

			// JUMPS AND STACKS
			// JR r8
			case 0x18:
				ptr_gb_reg->pc += int8_t(read_byte(gb_memory, pc + 1));
				break;
			case 0x20: // JMP NZ, signed d8 + pc
			case 0x28: // JMP Z, signed d8 + pc
				reg_value_u8 = ((opcode & 0x08) >> 3);
				cpu_conditon_jump_signed(ptr_gb_reg, ptr_gb_reg->flag.z, reg_value_u8, int8_t(read_byte(gb_memory, pc + 1)));
				break;
			case 0x30: // JMP NC, signed d8 + pc
			case 0x38: // JMP C, signed d8 + pc
				reg_value_u8 = ((opcode & 0x08) >> 3);
				cpu_conditon_jump_signed(ptr_gb_reg, ptr_gb_reg->flag.c, reg_value_u8, int8_t(read_byte(gb_memory, pc + 1)));
				break;
			// POP rr
			case 0xC1: case 0xD1: case 0xE1:
				reg_variable_u8 = (0x30 & opcode) >> 4;
				hold_ptr_u16 = (&ptr_gb_reg->bc) + reg_variable_u8;
				(*hold_ptr_u16) = cpu_stack_pop(ptr_gb_reg, gb_memory);
				break;
			// PUSH rr
			case 0xC5: case 0xD5: case 0xE5:
				reg_variable_u8 = (0x30 & opcode) >> 4;
				hold_ptr_u16 = (&ptr_gb_reg->bc) + reg_variable_u8;
				cpu_stack_push(ptr_gb_reg, gb_memory, (*hold_ptr_u16));
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

			// ALU 8-bit
			// INC reg
			case 0x04: case 0x0C: case 0x14: case 0x1C: case 0x24: case 0x2C: case 0x3C:
				reg_variable_u8 = opcode & 0x38 >> 3;
				hold_u8_pre = (*ptr_op_reg_u8[reg_variable_u8])++;
				hold_u8_post = (*ptr_op_reg_u8[reg_variable_u8]);
				flag_do_z(ptr_gb_reg, hold_u8_post);
				flag_do_h(ptr_gb_reg, hold_u8_pre, 1, false);
				ptr_gb_reg->flag.n = 0;
				break;
			// INC (HL)
			case 0x34:
				hold_u8_pre = read_byte(gb_memory, ptr_gb_reg->hl);
				hold_u8_post = hold_u8_pre + 1;
				write_byte(gb_memory, ptr_gb_reg->hl, hold_u8_post);
				flag_do_z(ptr_gb_reg, hold_u8_post);
				flag_do_h(ptr_gb_reg, hold_u8_pre, 1, false);
				ptr_gb_reg->flag.n = 0;
				break;
			// DEC (HL)
			case 0x35:
				hold_u8_pre = read_byte(gb_memory, ptr_gb_reg->hl);
				hold_u8_post = hold_u8_pre - 1;
				write_byte(gb_memory, ptr_gb_reg->hl, hold_u8_post);
				flag_do_z(ptr_gb_reg, hold_u8_post);
				flag_do_h(ptr_gb_reg, hold_u8_pre, 1, true);
				ptr_gb_reg->flag.n = 1;
				break;
			// DEC reg
			case 0x05: case 0x0D: case 0x15: case 0x1D: case 0x25: case 0x2D: case 0x3D:
				reg_variable_u8 = (0x30 & opcode) >> 4;
				hold_u8_pre = (*ptr_op_reg_u8[reg_variable_u8])--;
				hold_u8_post = (*ptr_op_reg_u8[reg_variable_u8]);
				flag_do_z(ptr_gb_reg, hold_u8_post);
				flag_do_h(ptr_gb_reg, hold_u8_pre, 1, true);
				ptr_gb_reg->flag.n = 1;
				break;
				
			// ALU 16-bit
			// INC rr
			case 0x03: case 0x13: case 0x23: case 0x33: 
				reg_variable_u8 = (0x30 & opcode) >> 4;
				(*ptr_op_reg_u16[reg_variable_u8])++;
				break;
			// ADD HL, rr
			case 0x09: case 0x19: case 0x29: case 0x39:
				reg_variable_u8 = (opcode & 0x30) >> 4;
				hold_ptr_u16 = ptr_op_reg_u16[reg_variable_u8];
				ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.h = uint32_t((ptr_gb_reg->hl & 0x0FFF) + ((*hold_ptr_u16) & 0x0FFF)) > 0x0FFF ? 1 : 0;
				ptr_gb_reg->flag.c = uint32_t( ptr_gb_reg->hl + (*hold_ptr_u16) ) > 0xFFFF? 1 : 0;
				ptr_gb_reg->hl += (*hold_ptr_u16);
				break;
			// DEC rr
			case 0x0B: case 0x1B: case 0x2B: case 0x3B: 
				reg_variable_u8 = (0x30 & opcode) >> 4;
				(*ptr_op_reg_u16[reg_variable_u8])--;
				break;

			// SCF
			case 0x37:
				ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.h = 0;
				ptr_gb_reg->flag.c = 1;
				break;
			// DAA
			case 0x27:
				if ((ptr_gb_reg->a & 0x0F) > 0x09 || ptr_gb_reg->flag.h) {
					ptr_gb_reg->a += 0x06;
				}
				if ( ((ptr_gb_reg->a & 0xF0) >> 4) > 0x09 || ptr_gb_reg->flag.c) {
					ptr_gb_reg->a += 0x60;
					ptr_gb_reg->flag.c = 1;
				} else {
					ptr_gb_reg->flag.c = 1;
				}
				ptr_gb_reg->flag.z = ( ptr_gb_reg->a == 0 ) ? 1 : 0;
				ptr_gb_reg->flag.h = 0;
				break;
			// CPL
			case 0x2F:
				ptr_gb_reg->a = ~ptr_gb_reg->a;
				ptr_gb_reg->flag.n = 1;
				ptr_gb_reg->flag.h = 1;
				break;
			// CCF
			case 0x3F:
				ptr_gb_reg->flag.c = (ptr_gb_reg->flag.c) ? 0 : 1;
				ptr_gb_reg->flag.n = 0;
				ptr_gb_reg->flag.h = 0;
				break;
			// CP r
			case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
				reg_variable_u8 = (opcode & 0x7);
				hold_byte = (*ptr_op_reg_u8[reg_variable_u8]);
				ptr_gb_reg->flag.z = (ptr_gb_reg->a == hold_byte);
				ptr_gb_reg->flag.n = 1;
				flag_do_h(ptr_gb_reg, ptr_gb_reg->a, hold_byte, true);
				ptr_gb_reg->flag.c = (ptr_gb_reg->a < hold_byte);
				break;
			// CP d8
			case 0xFE:
				hold_byte = read_byte(gb_memory, pc + 1);
				ptr_gb_reg->flag.z = (ptr_gb_reg->a == hold_byte);
				ptr_gb_reg->flag.n = 1;
				flag_do_h(ptr_gb_reg, ptr_gb_reg->a, hold_byte, true);
				ptr_gb_reg->flag.c = (ptr_gb_reg->a < hold_byte);
				break;
			// NOP
			case 0x00:
				break;
			case 0xAF: // XOR A
				ptr_gb_reg->a ^= ptr_gb_reg->a;
				flag_do_z(ptr_gb_reg, ptr_gb_reg->a);
				ptr_gb_reg->f &= FLAG_MASK_ZERO;
				break;
			case 0xCB: // PREFIX CB
				print_step(*ptr_gb_reg, gb_memory, "(CB)");
				pc = ptr_gb_reg->pc;
				opcode = gb_memory[pc];
				ptr_gb_reg->pc++;
				prefix_cb(opcode);
				break;
			
			default:
				printf("0x%02X: Unknown opcode!\n", opcode);
				return 1;
		}
	}
	printf("PROGRAM ENDED\n");

	return 0;
}
