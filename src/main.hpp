#ifndef MAIN_HPP
#define MAIN_HPP
#define TITLE "GBEMU_V2"
#include <stdint.h>

const int FLAG_POS_ZERO        = 7;
const int FLAG_POS_SUBTRACT    = 6;
const int FLAG_POS_HALF_CARRY  = 5;
const int FLAG_POS_CARRY       = 4;

const int FLAG_MASK_ZERO       = 0x80;
const int FLAG_MASK_SUBTRACT   = 0x40;
const int FLAG_MASK_HALF_CARRY = 0x20;
const int FLAG_MASK_CARRY      = 0x10;

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
							uint8_t flag_c:1, flag_h:1, flag_n:1, flag_z:1;
						};
					};
					uint8_t a;
				};
			};
		};
	};
	uint16_t sp;
	uint16_t pc;
};

#endif // MAIN_HPP
