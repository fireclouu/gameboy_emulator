#include "memorytest.hpp"
#include "main.hpp"

const char *COLOR_GREEN = "\033[32m";
const char *COLOR_RED = "\033[31m";
const char *COLOR_RESET = "\033[0m";

static inline int print_compare_fancy(char *var_name, int val1, int val2) {
	int result = 0;
	if (val1 != val2) {
		printf("%c >> %sVALUE: 0x%04X   EXPECTED: 0x%04X%s\n", var_name, COLOR_RED, val1, val2, COLOR_RESET);
		result = 1;
	} else {
		printf("%sVALUE: 0x%04X   EXPECTED: 0x%04X%s\n", COLOR_GREEN, val1, val2, COLOR_RESET);
	}
	return result;
}

static inline void debug_memory_test(Z80_Register *param_reg) {
	printf("%s%s: MEMALLOC AND TEST MODE!%s\n", COLOR_GREEN, TITLE, COLOR_RESET);
	printf("CPU Register struct size: %lu\n", sizeof(param_reg->register_general));
	// test registers
	printf("\nGeneral Register memddrs: \n");
	printf("B  %p\n", (&param_reg->b));
	printf("C  %p\n", (&param_reg->c));
	printf("D  %p\n", (&param_reg->d));
	printf("E  %p\n", (&param_reg->e));
	printf("H  %p\n", (&param_reg->h));
	printf("L  %p\n", (&param_reg->l));
	printf("A  %p\n", (&param_reg->a));
	printf("F  %p\n", (&param_reg->f));

	printf("\nPaired Register memddrs: \n");
	printf("BC %p\n", (&param_reg->bc));
	printf("DE %p\n", (&param_reg->de));
	printf("HL %p\n", (&param_reg->hl));
	printf("AF %p\n", (&param_reg->af));
	printf("SP %p\n", (&param_reg->sp));
	printf("PC %p\n", (&param_reg->pc));

	printf("\nTest direct memaddr set jumping:\n");
	printf("(writing \"0xF1F2\" to register-pair DE using BC pointer, effectively jumping to one-2 byte in size (expecting DE memaddr))\n");
	// 1 jump equivalent to 2 byte
	uint16_t *ptr = &param_reg->bc + (1 * sizeof(uint8_t));
	printf("ptr now points to  %p\n", (ptr));
	*ptr = 0xF1F2;
	printf("de should be 0xF1F2 (0x%02X)\n", (param_reg->de));
	// testvalues
	printf("\ntestreading union\n");
	param_reg->b = 0xBB;
	param_reg->c = 0xCC;
	param_reg->d = 0xDD;
	param_reg->e = 0xEE;
	param_reg->h = 0x22;
	param_reg->l = 0x11;
	param_reg->a = 0x55;
	param_reg->f = 0xFF;

	param_reg->pc = 1234;
	param_reg->sp = 0x5678;

	print_compare_fancy("B", 0xBB, param_reg->register_general[1]);
	print_compare_fancy("C", 0xCC, param_reg->register_general[0]);
	print_compare_fancy("D", 0xDD, param_reg->register_general[3]);
	print_compare_fancy("E", 0xEE, param_reg->register_general[2]);
	print_compare_fancy("H", 0x22, param_reg->register_general[5]);
	print_compare_fancy("L", 0x11, param_reg->register_general[4]);
	print_compare_fancy("A", 0x55, param_reg->register_general[7]);
	print_compare_fancy("F", 0xFF, param_reg->register_general[6]);

	printf("\ntestreading 16bit structs in union\n");
	param_reg->bc = 0x1122;
	param_reg->de = 0x3344;
	param_reg->hl = 0x5566;
	param_reg->af = 0x7788;
	printf("bc should be 0x1122 (0x%02X)\n", (param_reg->bc));
	printf("de should be 0x3344 (0x%02X)\n", (param_reg->de));
	printf("hl should be 0x5566 (0x%02X)\n", (param_reg->hl));
	printf("af should be 0x7788 (0x%02X)\n", (param_reg->af));
	// test flags znhc
	param_reg->f = 0; // reset
	param_reg->flag.z = 1;
	printf("\ntestreading flags\n");
	printf("flag z set. expect 0x80 (0x%02X)\n", (param_reg->f));
	param_reg->flag.h = 1;
	printf("flag h set. expect 0xA0 (0x%02X)\n", (param_reg->f));
	param_reg->flag.c = 1;
	printf("flag c set. expect 0xB0 (0x%02X)\n", (param_reg->f));
	param_reg->flag.n = 1;
	printf("flag n set. expect 0xF0 (0x%02X)\n", (param_reg->f));
}

int main(int argc, char **argv) {
	Z80_Register test_reg;
	debug_memory_test(&test_reg);
	return 0;
}
