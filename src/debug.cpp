#include <iostream>
#include <math.h>
#include <string>
#include "main.hpp"
#include "opcode.hpp"
#include "debug.hpp"

using namespace std;


Debugger::Debugger(Z80_Register *z80Register, uint8_t *memory) {
	addrRegister = z80Register;
	addrMemory = memory;
	newValueIterator = 0;
	isIterator = 1;
}
void Debugger::debugStepInteractive() {
	op_used[addrMemory[addrRegister->pc]]++;
	if (!printQuiet) debugPrintStep("");
	if ((addrMemory[addrRegister->pc] == opcodeVal && isOpcode) ||
			(iterator >= newValueIterator && isIterator)) {
		debugInteract();
	}
	iterator++;
}
void Debugger::dbgEnd(int param) {
	printf("\nOPCODE TALLY: (%ld iterations)\n", iterator);
	for (int a = 0; a < 0xFF; a++) {
		if (op_used[a] == 0) continue;
		printf("0x%02X (%s):  %ld\n", a, OP_INSTRUCTION[a].c_str(), op_used[a]);
	}
	printf("PROGRAM ENDED\n");
	exit(param);
}
void Debugger::debugInteract() {
	int inputSize = 100;
	char input[inputSize];
	string inputToInt;

	for(int x = 0; x < inputSize; x++) {
		input[x] = '\0';
	}

	printf("Enter argument:\n");
	cin.getline(input, inputSize);
	printf("\n");

	// convert to int
	inputToInt = (input + 1);
	uint64_t convertedValue = input[1] != '\0' ? stoul(inputToInt) : 1; 
	if (input[0] == '\0') return;
	switch(input[0]) {
		case 'o':
			opcodeVal = convertedValue;
			isOpcode = true;
			isIterator = false;
			printQuiet = false;
			break;
		case 'i':
			newValueIterator = iterator + convertedValue;
			isIterator = true;
			isOpcode = false;
			printQuiet = false;
			break;
		case 'f': case '\0':
			newValueIterator = iterator + convertedValue;
			isIterator = true;
			isOpcode = false;
			printQuiet = true;
			break;
		case 'q':
			exit(0);
			break;
		default:
			debugInteract();
	}
}
void Debugger::debugPrintStep(string msg) {
	printf("PC: %04X (0x%02X)  AF: %04X  BC: %04X  DE: %04X  HL: %04X  SP: %04X  %s\n",
			addrRegister->pc, addrMemory[addrRegister->pc],
			addrRegister->af, addrRegister->bc,
			addrRegister->de, addrRegister->hl,
			addrRegister->sp, msg.c_str());
	printf("MEMORY and STACK:\n");
	for (int x = 0; x < 4; x++) {
		int hold_pc = addrRegister->pc + x;
		int hold_sp = addrRegister->sp + x;
		if (hold_pc <= 0xFFFF) {
			printf("[ 0x%04X: 0x%02X ]   ", hold_pc, addrMemory[hold_pc]);
		}
		if (hold_sp <= 0xFFFF) {
			printf("[ 0x%04X: 0x%02X ]", hold_sp, addrMemory[hold_sp]);
		}
		printf("\n");
	}
	printf("\n");
}
void Debugger::print_memory(uint8_t *param_memory, int param_mem_size) {
	for (int x = 0; x < param_mem_size; x++) {
		printf("%04X: 0x%02X   ", x, param_memory[x]);
		if (((x +1) % 4) == 0) printf("\n");
	}
}
