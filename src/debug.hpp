#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <math.h>
#include <string>
#include "main.hpp"
#include "opcode.hpp"

using namespace std;

class Debugger {
	private:
		bool isOpcode = 0, isIterator = 1, printQuiet = 0;
		uint8_t opcodeVal = 0;
		uint64_t iterator = 0, newValueIterator = 0;
		Z80_Register *addrRegister;
		uint8_t *addrMemory;
		uint64_t op_used[0xFF] = {};

		void debugInteract();
		void debugPrintStep(string msg);

	public:
		Debugger(Z80_Register *z80Register, uint8_t *memory);
		void print_memory(uint8_t *param_memory, int param_mem_size);
		void debugStepInteractive();
		void dbgEnd(int param);
};
#endif // DEBUG_H
