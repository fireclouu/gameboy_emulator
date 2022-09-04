#pragma once // not caring with mutliple clangcomp version
#include <stdint.h>

#define TITLE "Z80BOY"

const int FLAG_POS_ZERO        = 7;
const int FLAG_POS_SUBTRACT    = 6;
const int FLAG_POS_HALF_CARRY  = 5;
const int FLAG_POS_CARRY       = 4;

const int FLAG_MASK_ZERO       = 0b10000000;
const int FLAG_MASK_SUBTRACT   = 0b01000000;
const int FLAG_MASK_HALF_CARRY = 0b00100000;
const int FLAG_MASK_CARRY      = 0b00010000;

struct Z80_Register {
	union {
		uint8_t register_general[8];
		struct {
			union {
				uint16_t bc;
				struct {
					uint8_t c;
					uint8_t b;
				};
			};
			union {
				uint16_t de;
				struct {
					uint8_t e;
					uint8_t d;
				};
			};
			union {
				uint16_t hl;
				struct {
					uint8_t l;
					uint8_t h;
				};
			};
			union {
				uint16_t af;
				struct {
					union {
						uint8_t f;
						struct {
							uint8_t ZEROFILL:4;
							uint8_t c:1, h:1, n:1, z:1;
						} flag;
					};
					uint8_t a;
				};
			};
		};
	};
	uint16_t sp;
	uint16_t pc;
};

