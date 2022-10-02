/*
 * cpu.cpp
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

#include "include/cpu.hpp"

#include <stdint.h>

#include <cstdint>

#include "include/opcode.hpp"

Cpu::Cpu() {
  currentTCycle = 0;
}
void Cpu::setMmu(Mmu *mmu) {
    this->mmu = mmu;
}
void Cpu::setHalt(bool *halt) {
    this->halt = halt;
}
void Cpu::checkFlagH(uint8_t left, uint8_t right, bool isSubtraction) {
  if (isSubtraction) {
    cpuRegister.flag_h = ((left & 0x0F) < (right & 0x0F)) ? 1 : 0;
  } else {
    cpuRegister.flag_h = (((left & 0x0F) + (right & 0x0F)) > 0x0F) ? 1 : 0;
  }
}
void Cpu::instructionStackPush(uint16_t addrValue) {
  uint8_t lsb = (addrValue & 0x00FF);
  uint8_t msb = (addrValue & 0xFF00) >> 8;
  mmu->writeByte(cpuRegister.sp--, msb);
  mmu->writeByte(cpuRegister.sp--, lsb);
  currentTCycle += 4;
}
uint16_t Cpu::instructionStackPop() {
  uint8_t lsb = mmu->readByte(++cpuRegister.sp);
  uint8_t msb = mmu->readByte(++cpuRegister.sp);
  return ((msb) << 8) + (lsb);
}
void Cpu::instructionRet() {
  cpuRegister.pc = instructionStackPop();
  currentTCycle += 4;
}
void Cpu::conditionalJpAdd(uint8_t flag, uint8_t expected, int8_t value) {
  if (flag == expected) {
    cpuRegister.pc += (value);
    currentTCycle += 4;
  }
}
void Cpu::conditionalJpA16(uint8_t flag, uint8_t expected, uint16_t value) {
  if (flag == expected) {
    cpuRegister.pc = value;
    currentTCycle += 4;
  }
}
void Cpu::conditionalRet(uint8_t flag, uint8_t expected) {
  if (flag == expected) {
    instructionRet();
  }
  currentTCycle += 4;
}
void Cpu::conditionalCall(uint16_t pc, uint8_t flag, uint8_t expected) {
  uint16_t nextWord = mmu->readShort(pc + 1);
  if (flag == expected) {
    instructionStackPush(pc + 3);
    cpuRegister.pc = nextWord;
  }
}
void Cpu::instructionAnd(uint8_t value) {
  cpuRegister.reg_a &= value;
  cpuRegister.flag_z = (cpuRegister.reg_a == 0);
  cpuRegister.flag_n = 0;
  cpuRegister.flag_h = 1;
  cpuRegister.flag_c = 0;
}
void Cpu::instructionXor(uint8_t value) {
  cpuRegister.reg_a ^= value;
  cpuRegister.flag_z = (cpuRegister.reg_a == 0);
  cpuRegister.flag_n = 0;
  cpuRegister.flag_h = 0;
  cpuRegister.flag_c = 0;
}
void Cpu::instructionOr(uint8_t value) {
  cpuRegister.reg_a |= value;
  cpuRegister.flag_z = (cpuRegister.reg_a == 0);
  cpuRegister.flag_n = 0;
  cpuRegister.flag_h = 0;
  cpuRegister.flag_c = 0;
}
void Cpu::instructionCp(uint8_t value) {
  cpuRegister.flag_z = (cpuRegister.reg_a == value);
  cpuRegister.flag_n = 1;
  checkFlagH(cpuRegister.reg_a, value, true);
  cpuRegister.flag_c = (cpuRegister.reg_a < value) ? 1 : 0;
}
void Cpu::instructionAdd(uint8_t value) {
  uint8_t accumulator = cpuRegister.reg_a;
  cpuRegister.reg_a += value;

  cpuRegister.flag_z = (cpuRegister.reg_a == 0);
  cpuRegister.flag_n = 0;
  checkFlagH(accumulator, value, false);
  cpuRegister.flag_c = ((accumulator + value) > 0xFF) ? 1 : 0;
}
void Cpu::instructionSub(uint8_t value) {
  uint8_t accumulator = cpuRegister.reg_a;
  cpuRegister.reg_a -= value;

  cpuRegister.flag_z = (cpuRegister.reg_a == 0);
  checkFlagH(accumulator, value, true);
  cpuRegister.flag_n = 1;
  cpuRegister.flag_c = (accumulator < value) ? 0 : 1;
}
uint8_t Cpu::instructionInc(uint8_t regAddrValue) {
  uint8_t valueBytePre = regAddrValue;
  uint8_t valueBytePost = regAddrValue + 1;
  cpuRegister.flag_z = (valueBytePost == 0);
  checkFlagH(valueBytePre, 1, false);
  cpuRegister.flag_n = 0;

  return valueBytePost;
}
uint8_t Cpu::instructionDec(uint8_t regAddrValue) {
  uint8_t valueBytePre = regAddrValue;
  uint8_t valueBytePost = regAddrValue - 1;
  cpuRegister.flag_z = (valueBytePost == 0);
  checkFlagH(valueBytePre, 1, true);
  cpuRegister.flag_n = 1;
  return valueBytePost;
}
int Cpu::decode(uint16_t opcodeAddr, uint8_t opcode) {
  currentTCycle = 0;
  uint16_t parseAddr;
  uint16_t parseAddrL, parseAddrR;
  uint16_t *holdAddrPtr;

  uint16_t currentPc = opcodeAddr;
  uint8_t currentOpcode = opcode;
  currentTCycle += 4;
  switch (currentOpcode) {
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
      cpuRegister.pc++;
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
    case 0x07: {
      uint8_t bit = ((cpuRegister.reg_a & 0x80) >> 7);
      cpuRegister.reg_a = (cpuRegister.reg_a << 1) + bit;
      cpuRegister.flag_z = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = bit;
    } break;
    // RLA
    case 0x17: {
      uint8_t bit = ((cpuRegister.reg_a & 0x80) >> 7);
      cpuRegister.reg_a = (cpuRegister.reg_a << 1) + cpuRegister.flag_c;
      cpuRegister.flag_z = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = bit;
    } break;
    // RRCA
    case 0x0F: {
      uint8_t bit = (cpuRegister.reg_a & 1);
      cpuRegister.reg_a = (bit << 7) + (cpuRegister.reg_a >> 1);
      cpuRegister.flag_z = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = bit;
    } break;
    // RRA
    case 0x1F: {
      uint8_t bit = (cpuRegister.reg_a & 1);
      cpuRegister.reg_a = (cpuRegister.flag_c << 7) + (cpuRegister.reg_a >> 1);
      cpuRegister.flag_z = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = bit;
    } break;

    // LOADS 8-bit
    // LD (reg, no HL), d8
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
    case 0x3E:
      parseAddrL = (currentOpcode & 0x38) >> 3;
      (*cpuRegister.reg[parseAddrL]) = mmu->readByte(currentPc + 1);
      break;
    // LD A,rr
    case 0x0A:
    case 0x1A:
      parseAddr = (currentOpcode & 0x30) >> 4;
      holdAddrPtr = (cpuRegister.reg_pair[parseAddr]);
      cpuRegister.reg_a = mmu->readByte(*holdAddrPtr);
      break;
    // LD A, HL+
    case 0x2A:
      cpuRegister.reg_a = mmu->readByte(cpuRegister.reg_pair_hl++);
      break;
    // LD A, HL-
    case 0x3A:
      cpuRegister.reg_a = mmu->readByte(cpuRegister.reg_pair_hl--);
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
      parseAddrR = (currentOpcode & 0x07);
      parseAddrL = (currentOpcode & 0x38) >> 3;
      (*cpuRegister.reg[parseAddrL]) = (*cpuRegister.reg[parseAddrR]);
      break;
    // LD reg, (HL)
    case 0x46:
    case 0x4E:
    case 0x56:
    case 0x5E:
    case 0x66:
    case 0x6E:
    case 0x7E: {
      uint8_t parseRegister = (currentOpcode & 0x38) >> 3;
      uint8_t value = mmu->readByte(cpuRegister.reg_pair_hl);
      (*cpuRegister.reg[parseRegister]) = value;
    } break;
    // LDH (a8), A
    case 0xE0:
      parseAddr = 0xFF00 + mmu->readByte(currentPc + 1);
      mmu->writeByte(parseAddr, cpuRegister.reg_a);
      break;
      // LDH A, (a8)
    case 0xF0:
      parseAddr = 0xFF00 + mmu->readByte(currentPc + 1);
      cpuRegister.reg_a = mmu->readByte(parseAddr);
      break;
      // LD (C), A
    case 0xE2:
      parseAddr = 0xFF00 + cpuRegister.reg_c;
      mmu->writeByte(parseAddr, cpuRegister.reg_a);
      break;
      // LDH A, (C)
    case 0xF2:
      parseAddr = 0xFF00 + cpuRegister.reg_c;
      cpuRegister.reg_a = mmu->readByte(parseAddr);
      break;

    // LOADS 16-bit
    // LD rr, d16
    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
      parseAddr = (0x30 & currentOpcode) >> 4;
      (*cpuRegister.reg_pair[parseAddr]) = mmu->readShort(currentPc + 1);
      break;
    // LD (rr), A
    case 0x02:
    case 0x12:
      parseAddr = (currentOpcode & 0x30) >> 4;
      holdAddrPtr = (cpuRegister.reg_pair[parseAddr]);
      mmu->writeByte(*holdAddrPtr, cpuRegister.reg_a);
      break;
    // LD (nn), SP
    case 0x08:
      parseAddr = mmu->readShort(currentPc + 1);
      mmu->writeByte(parseAddr, (cpuRegister.sp & 0xFF00) >> 8);
      mmu->writeByte(parseAddr + 1, (cpuRegister.sp & 0x00FF));
      break;
    // LD (HL+), A
    case 0x22:
      mmu->writeByte(cpuRegister.reg_pair_hl++, cpuRegister.reg_a);
      break;
    // LD (HL-), A
    case 0x32:
      mmu->writeByte(cpuRegister.reg_pair_hl--, cpuRegister.reg_a);
      break;
    // LD (HL), d8
    case 0x36: {
      uint8_t nextByte = mmu->readByte(currentPc + 1);
      mmu->writeByte(cpuRegister.reg_pair_hl, nextByte);
    } break;
    // LD (HL), reg
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x77:
      parseAddr = (currentOpcode & 0x07);
      mmu->writeByte(cpuRegister.reg_pair_hl, (*cpuRegister.reg[parseAddr]));
      break;
    // LD HL, SP+r8
    case 0xF8: {
      uint16_t sp = cpuRegister.sp;
      int8_t nextByte = mmu->readByte(currentPc + 1);
      uint16_t result = sp + nextByte;
      cpuRegister.reg_pair_hl = result;
      currentTCycle += 4;

      cpuRegister.flag_z = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = ((sp & 0x0F) + (nextByte & 0x0F) > 0x0F) ? 1 : 0;
      cpuRegister.flag_c = ((result & 0xFF) < (sp & 0xFF)) ? 1 : 0;
    } break;
    // LD SP, HL
    case 0xF9:
      cpuRegister.sp = cpuRegister.reg_pair_hl;
      currentTCycle += 4;
      break;
    // LD (a16), A
    case 0xEA:
      parseAddr = mmu->readShort(currentPc + 1);
      mmu->writeByte(parseAddr, cpuRegister.reg_a);
      break;
    // LD A, (a16)
    case 0xFA:
      parseAddr = mmu->readShort(currentPc + 1);
      cpuRegister.reg_a = mmu->readByte(parseAddr);
      break;

    // JUMPS AND STACKS
    // JR r8
    case 0x18: {
      int8_t nextByte = mmu->readByte(currentPc + 1);
      conditionalJpAdd(1, 1, nextByte);
    } break;
      // JP NZ, r8
    case 0x20:
      // JP Z, r8
    case 0x28: {
      uint8_t bit = ((currentOpcode & 0x08) >> 3);
      int8_t nextByte = mmu->readByte(currentPc + 1);
      conditionalJpAdd(cpuRegister.flag_z, bit, nextByte);
    } break;
      // JP NC, signed d8 + pc
    case 0x30:
      // JP C, signed d8 + pc
    case 0x38: {
      uint8_t bit = ((currentOpcode & 0x08) >> 3);
      int8_t nextByte = mmu->readByte(currentPc + 1);
      conditionalJpAdd(cpuRegister.flag_c, bit, nextByte);
    } break;
    // RET Z FLAG
    case 0xC0:
    case 0xC8: {
      uint8_t bit = ((currentOpcode & 0x08) >> 3);
      conditionalRet(cpuRegister.flag_z, bit);
    } break;
    // RET C FLAG
    case 0xD0:
    case 0xD8: {
      uint8_t bit = ((currentOpcode & 0x08) >> 3);
      conditionalRet(cpuRegister.flag_c, bit);
    } break;
    // RETI
    case 0xD9:
      instructionRet();
      break;
    // POP rr
    case 0xC1:
    case 0xD1:
    case 0xE1:
      parseAddr = (currentOpcode & 0x30) >> 4;
      (*cpuRegister.reg_pair[parseAddr]) = instructionStackPop();
      break;
    // POP AF
    case 0xF1:
      cpuRegister.reg_pair_af = (instructionStackPop() & 0xFFF0);
      break;
    // JP Z FLAG, a16
    case 0xC2:
    case 0xCA: {
      uint8_t bit = (currentOpcode & 0x08) >> 3;
      uint16_t nextWord = mmu->readShort(currentPc + 1);
      conditionalJpA16(cpuRegister.flag_z, bit, nextWord);
    } break;
    // JP a16
    case 0xC3: {
      uint16_t nextWord = mmu->readShort(currentPc + 1);
      conditionalJpA16(1, 1, nextWord);
    } break;
    // CALL Z FLAG
    case 0xC4:
    case 0xCC: {
      uint8_t bit = (currentOpcode & 0x08) >> 3;
      conditionalCall(currentPc, cpuRegister.flag_z, bit);
    } break;
    // CALL C FLAG
    case 0xD4:
    case 0xDC: {
      uint8_t bit = (currentOpcode & 0x08) >> 3;
      conditionalCall(currentPc, cpuRegister.flag_c, bit);
    } break;
    // JP C FLAG, a16
    case 0xD2:
    case 0xDA: {
      uint8_t bit = (currentOpcode & 0x08) >> 3;
      uint16_t nextWord = mmu->readShort(currentPc + 1);
      conditionalJpA16(cpuRegister.flag_c, bit, nextWord);
    } break;
    // PUSH rr
    case 0xC5:
    case 0xD5:
    case 0xE5:
      parseAddr = (0x30 & currentOpcode) >> 4;
      instructionStackPush((*cpuRegister.reg_pair[parseAddr]));
      break;
    // PUSH AF
    case 0xF5:
      instructionStackPush(cpuRegister.reg_pair_af);
      break;
    // RET
    case 0xC9:
      instructionRet();
      break;
    // CALL
    case 0xCD:
      conditionalCall(currentPc, 1, 1);
      break;
    // JP HL
    case 0xE9:
      cpuRegister.pc = cpuRegister.reg_pair_hl;
      break;
    // RST nn
    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF: {
      uint8_t rstAddr = (currentOpcode & 0x38);
      instructionStackPush(currentPc + 1);
      cpuRegister.pc = rstAddr;
    } break;
    // ALU 8-bit
    // INC reg
    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x3C:
      parseAddr = (currentOpcode & 0x38) >> 3;
      (*cpuRegister.reg[parseAddr]) =
          instructionInc((*cpuRegister.reg[parseAddr]));
      break;
    // INC (HL)
    case 0x34: {
      uint16_t hlAddress = cpuRegister.reg_pair_hl;
      uint8_t value = instructionInc(mmu->readByte(hlAddress));
      mmu->writeByte(hlAddress, value);
      currentTCycle += 4;
    } break;
    // DEC reg
    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x3D:
      parseAddr = (currentOpcode & 0x38) >> 3;
      (*cpuRegister.reg[parseAddr]) =
          instructionDec((*cpuRegister.reg[parseAddr]));
      break;
    // DEC (HL)
    case 0x35: {
      uint16_t hlAddress = cpuRegister.reg_pair_hl;
      uint8_t value = instructionDec(mmu->readByte(hlAddress));
      mmu->writeByte(hlAddress, value);
      currentTCycle += 4;
    } break;

    // ALU 16-bit
    // INC rr
    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
      parseAddr = (currentOpcode & 0x30) >> 4;
      (*cpuRegister.reg_pair[parseAddr])++;
      currentTCycle += 4;
      break;
    // ADD HL, rr
    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39: {
      uint8_t parsePairAddr = (currentOpcode & 0x30) >> 4;
      uint16_t *addrPtr = cpuRegister.reg_pair[parsePairAddr];
      uint16_t addrVal = (*addrPtr);
      uint16_t hl = cpuRegister.reg_pair_hl;
      cpuRegister.reg_pair_hl += (addrVal);

      cpuRegister.flag_n = 0;
      cpuRegister.flag_h =
          ((hl & 0x0FFF) + ((addrVal)&0x0FFF)) > 0x0FFF ? 1 : 0;
      cpuRegister.flag_c = (hl + (addrVal)) > 0xFFFF ? 1 : 0;
      currentTCycle += 4;
    } break;
    // DEC rr
    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
      parseAddr = (0x30 & currentOpcode) >> 4;
      (*cpuRegister.reg_pair[parseAddr])--;
      currentTCycle += 4;
      break;
    // ADD SP, r8
    case 0xE8: {
      uint16_t sp = cpuRegister.sp;
      int8_t nextByte = mmu->readByte(currentPc + 1);
      uint16_t result = sp + nextByte;
      cpuRegister.sp = result;
      currentTCycle += 8;

      cpuRegister.flag_z = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = (((sp & 0x0F) + (nextByte & 0x0F)) > 0x0F) ? 1 : 0;
      cpuRegister.flag_c = ((result & 0xFF) < (sp & 0xFF)) ? 1 : 0;
    } break;

    // LOGIC
    // ADD A, r
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    // ADD A, (HL)
    case 0x86:
    case 0x87:
    // ADC A, r
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    // ADC A, (HL)
    case 0x8E:
    case 0x8F:
    // ADD A, d8
    case 0xC6:
    // ADC A, d8
    case 0xCE: {
      uint8_t value;
      uint8_t bit = ((currentOpcode & 0x08) >> 3) ? cpuRegister.flag_c : 0;
      switch (currentOpcode) {
        case 0x86:
        case 0x8E:
          value = mmu->readByte(cpuRegister.reg_pair_hl);
          break;
        case 0xC6:
        case 0xCE:
          value = mmu->readByte(currentPc + 1);
          break;
        default:
          parseAddr = (currentOpcode & 0x07);
          value = (*cpuRegister.reg[parseAddr]);
      }
      value += bit;
      instructionAdd(value);
    } break;
    // SUB r
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    // SUB (HL)
    case 0x96:
    case 0x97:
    // SBC A, r
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    // SBC A, (HL)
    case 0x9E:
    case 0x9F:
    // SUB d8
    case 0xD6:
    // SBC A, d8
    case 0xDE: {
      uint8_t value;
      uint8_t bit = ((currentOpcode & 0x08) >> 3) ? cpuRegister.flag_c : 0;
      switch (currentOpcode) {
        case 0x96:
        case 0x9E:
          value = mmu->readByte(cpuRegister.reg_pair_hl);
          break;
        case 0xD6:
        case 0xDE:
          value = mmu->readByte(currentPc + 1);
          break;
        default:
          parseAddr = (currentOpcode & 0x07);
          value = (*cpuRegister.reg[parseAddr]);
      }
      value += bit;
      instructionSub(value);
    } break;
    // AND r
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA7:
    // XOR r
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
    case 0xAF:
    // OR r
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB7:
    // CP r
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBF: {
      parseAddr = (currentOpcode & 0x07);
      uint8_t parseInst = (currentOpcode & 0x18) >> 3;
      uint8_t value = (*cpuRegister.reg[parseAddr]);
      switch (parseInst) {
        case 0:
          instructionAnd(value);
          break;
        case 1:
          instructionXor(value);
          break;
        case 2:
          instructionOr(value);
          break;
        case 3:
          instructionCp(value);
          break;
      }
    } break;
    // AND (HL)
    case 0xA6:
    // XOR (HL)
    case 0xAE:
    // OR (HL)
    case 0xB6:
    // CP (HL)
    case 0xBE: {
      uint8_t parseInst = (currentOpcode & 0x18) >> 3;
      uint8_t nextByte = mmu->readByte(cpuRegister.reg_pair_hl);
      switch (parseInst) {
        case 0:
          instructionAnd(nextByte);
          break;
        case 1:
          instructionXor(nextByte);
          break;
        case 2:
          instructionOr(nextByte);
          break;
        case 3:
          instructionCp(nextByte);
          break;
      }
    } break;
    // AND d8
    case 0xE6:
    // XOR d8
    case 0xEE:
    // OR d8
    case 0xF6:
    // CP d8
    case 0xFE: {
      uint8_t parseInst = (currentOpcode & 0x18) >> 3;
      uint8_t value = mmu->readByte(currentPc + 1);
      switch (parseInst) {
        case 0:
          instructionAnd(value);
          break;
        case 1:
          instructionXor(value);
          break;
        case 2:
          instructionOr(value);
          break;
        case 3:
          instructionCp(value);
          break;
      }
    } break;
    // SCF
    case 0x37:
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = 1;
      break;
    // DAA
    // source - ehaskins.com
    case 0x27: {
      uint8_t accumulator = cpuRegister.reg_a;
      uint8_t adjust = 0;
      if (((!cpuRegister.flag_n) && (accumulator & 0x0F) > 0x09) ||
          cpuRegister.flag_h) {
        adjust |= 0x06;
      }
      if ((!cpuRegister.flag_n && (accumulator > 0x99)) || cpuRegister.flag_c) {
        adjust |= 0x60;
        cpuRegister.flag_c = 1;
      }
      accumulator += cpuRegister.flag_n ? -adjust : adjust;

      cpuRegister.reg_a = accumulator;
      cpuRegister.flag_z = !(cpuRegister.reg_a) ? 1 : 0;
      cpuRegister.flag_h = 0;
    } break;
    // CPL
    case 0x2F:
      cpuRegister.reg_a = ~cpuRegister.reg_a;
      cpuRegister.flag_n = 1;
      cpuRegister.flag_h = 1;
      break;
    // CCF
    case 0x3F:
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = (cpuRegister.flag_c) ? 0 : 1;
      break;
    // Prefix CB
    case 0xCB:
      cpuRegister.pc++;
      currentOpcode = mmu->readByte(currentPc + 1);
      decodeCb(currentPc + 1, currentOpcode);
      break;
    default:
      printf("\n0x%02X: Unknown opcode!\n", currentOpcode);
      *halt = true;
  }
  return currentTCycle;
}

int Cpu::decodeCb(uint16_t opcodeAddr, uint8_t opcode) {
  int tick = 0;
  uint8_t parseOpcodeAddr;
  uint8_t holdBit, holdByte;
  uint8_t *holdRegPtr;
  uint8_t holdMask, holdMemoryValueHl;
  parseOpcodeAddr = (opcode & 0x07);
  holdRegPtr = (cpuRegister.reg[parseOpcodeAddr]);

  currentTCycle += 4;
  switch (opcode) {
    // RLC r
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x07:
      holdBit = (((*holdRegPtr) & 0x80) >> 7);
      (*holdRegPtr) = ((*holdRegPtr) << 1) + holdBit;
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      break;
    // RRC r
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0F:
      holdBit = ((*holdRegPtr) & 0x01);
      (*holdRegPtr) = (holdBit << 7) + ((*holdRegPtr) >> 1);
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      break;
    // RL n
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x17:
      holdBit = ((*holdRegPtr) & 0x80) >> 7;
      (*holdRegPtr) = ((*holdRegPtr) << 1) + cpuRegister.flag_c;
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = holdBit;
      break;
    // RR r
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1F:
      holdBit = ((*holdRegPtr) & 1);
      (*holdRegPtr) = (cpuRegister.flag_c << 7) + ((*holdRegPtr) >> 1);
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      break;
    // SLA n
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x27:
      holdBit = (((*holdRegPtr) & 0x80) >> 7);
      (*holdRegPtr) = ((*holdRegPtr) << 1) & (0xFE);
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      break;
    // SRA n
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2F:
      holdByte = ((*holdRegPtr) & 0x80);
      cpuRegister.flag_c = ((*holdRegPtr) & 0x01);
      (*holdRegPtr) = holdByte + ((*holdRegPtr) >> 1);
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      break;
    // SWAP n
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x37: {
      uint8_t msb = ((*holdRegPtr) & 0xF0) >> 4;
      uint8_t lsb = (*holdRegPtr) & 0x0F;
      (*holdRegPtr) = (lsb << 4) + msb;
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = 0;
    } break;
    // SRL n
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3F:
      holdBit = ((*holdRegPtr) & 0x01);
      (*holdRegPtr) = ((*holdRegPtr) >> 1) & (0x7F);
      cpuRegister.flag_z = !(*holdRegPtr) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
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
      holdMask = 1 << ((opcode & 0x38) >> 3);
      cpuRegister.flag_z = (((*holdRegPtr) & holdMask) == 0) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 1;
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
      holdBit = ((opcode & 0x40) >> 6);
      holdByte = (opcode & 0x38) >> 3;
      holdMask = (holdBit) ? (1 << holdByte) : (0xFF ^ (1 << holdByte));
      (*holdRegPtr) =
          (holdBit) ? (*holdRegPtr | holdMask) : (*holdRegPtr & holdMask);
      break;

    // HL
    // RLC
    case 0x06:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = ((holdMemoryValueHl & 0x80) >> 7);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((holdMemoryValueHl << 1) + holdBit));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = (holdMemoryValueHl == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
      break;
    // RRC
    case 0x0E:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = (holdMemoryValueHl & 0x01);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((holdBit << 7) | ((holdMemoryValueHl) >> 1)));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = (holdMemoryValueHl == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
      break;
    // RL
    case 0x16:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = (holdMemoryValueHl & 0x80) >> 7;
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((holdMemoryValueHl << 1) + cpuRegister.flag_c));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = (holdMemoryValueHl == 0) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
      break;
    // RR
    case 0x1E:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = (holdMemoryValueHl & 1);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((cpuRegister.flag_c << 7) + ((holdMemoryValueHl) >> 1)));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = (holdMemoryValueHl == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
      break;
    // SLA
    case 0x26:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = (holdMemoryValueHl & 0x80) >> 7;
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((holdMemoryValueHl << 1) & (0xFE)));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = (holdMemoryValueHl == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
      break;
    // SRA
    case 0x2E:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdByte = ((holdMemoryValueHl)&0x80);
      cpuRegister.flag_c = ((holdMemoryValueHl)&0x01);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     (holdByte | ((holdMemoryValueHl) >> 1)));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = ((holdMemoryValueHl) == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      currentTCycle += 4;
      break;
    // SWAP
    case 0x36:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     (((holdMemoryValueHl)&0x0F) << 4) +
                         (((holdMemoryValueHl)&0xF0) >> 4));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = ((holdMemoryValueHl) == 0) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_c = 0;
      currentTCycle += 4;
      break;
    // SRL n
    case 0x3E:
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = ((holdMemoryValueHl)&0x01);
      mmu->writeByte(cpuRegister.reg_pair_hl,
                     ((holdMemoryValueHl) >> 1) & (0x7F));
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      cpuRegister.flag_z = ((holdMemoryValueHl) == 0) ? 1 : 0;
      cpuRegister.flag_h = 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_c = holdBit;
      currentTCycle += 4;
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
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdMask = 1 << ((opcode & 0x38) >> 3);
      cpuRegister.flag_z = ((holdMemoryValueHl & holdMask) == 0) ? 1 : 0;
      cpuRegister.flag_n = 0;
      cpuRegister.flag_h = 1;
      currentTCycle += 4;
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
      holdMemoryValueHl = mmu->readByte(cpuRegister.reg_pair_hl);
      holdBit = ((opcode & 0x40) >> 6);
      holdByte = (opcode & 0x38) >> 3;
      holdMask = (holdBit) ? (1 << holdByte) : (0xFF ^ (1 << holdByte));
      if (holdBit) {
        mmu->writeByte(cpuRegister.reg_pair_hl, holdMemoryValueHl | holdMask);
      } else {
        mmu->writeByte(cpuRegister.reg_pair_hl, holdMemoryValueHl & holdMask);
        currentTCycle += 4;
      }
      break;
    default:
      printf("0x%02X: Unknown CB opcode!\n", opcode);
      *halt = true;
  }
  return tick;
}
