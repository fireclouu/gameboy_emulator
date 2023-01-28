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

#include "../include/cpu.hpp"

Cpu::Cpu() {
    this->initializeRegisters();
}
void Cpu::setMmu(Mmu *mmu) { this->mmu = mmu; }
void Cpu::setHalt(bool *halt) { this->halt = halt; }
void Cpu::checkFlagH(uint8_t left, uint8_t right, bool isSubtraction) {
    if (isSubtraction) {
        uint8_t result = (left & 0x0F) - (right & 0x0F);
        cpuRegister.flag_h = ((result & 0x0F) > (left & 0x0F));
    } else {
        uint8_t result = (left & 0x0F) + (right & 0x0F);
        cpuRegister.flag_h = (result > 0x0F);
    }
}
void Cpu::instructionStackPush(uint16_t addrValue) {
    uint8_t lsb = (addrValue & 0x00FF);
    uint8_t msb = (addrValue & 0xFF00) >> 8;
    mmu->writeByte(--cpuRegister.sp, msb);
    mmu->writeByte(--cpuRegister.sp, lsb);
}
uint16_t Cpu::instructionStackPop() {
    uint8_t lsb = mmu->readByte(cpuRegister.sp++);
    uint8_t msb = mmu->readByte(cpuRegister.sp++);
    return ((msb) << 8) + (lsb);
}
void Cpu::instructionRet() {
    cpuRegister.pc = instructionStackPop();
}
bool Cpu::conditionalJpAdd(uint8_t flag, uint8_t expected, int8_t value) {
    if (flag == expected) {
        cpuRegister.pc += (value);
        return true;
    }
        return false;
}
bool Cpu::conditionalJpA16(uint8_t flag, uint8_t expected, uint16_t value) {
    if (flag == expected) {
        cpuRegister.pc = value;
        return true;
    }
    return false;
}
bool Cpu::conditionalRet(uint8_t flag, uint8_t expected) {
    if (flag == expected) {
        instructionRet();
        return true;
    }
    return false;
}
bool Cpu::conditionalCall(uint16_t pc, uint8_t flag, uint8_t expected) {
    uint16_t nextWord = mmu->readShort(pc + 1);
    if (flag == expected) {
        instructionStackPush(pc + 3);
        cpuRegister.pc = nextWord;
        return true;
    }
    return false;
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
    uint8_t accumulator = cpuRegister.reg_a;
    uint8_t result = accumulator - value;
    cpuRegister.flag_z = (result == 0);
    cpuRegister.flag_n = 1;
    checkFlagH(accumulator, value, true);
    cpuRegister.flag_c = (result > accumulator);
}
void Cpu::instructionAdd(uint8_t value) {
    uint8_t accumulator = cpuRegister.reg_a;
    uint8_t result = accumulator + value;
    cpuRegister.reg_a = result;
    cpuRegister.flag_z = (cpuRegister.reg_a == 0);
    cpuRegister.flag_n = 0;
    checkFlagH(accumulator, value, false);
    cpuRegister.flag_c = (result < accumulator);
}
void Cpu::instructionAdc(uint8_t value) {
    uint8_t accumulator = cpuRegister.reg_a;
    uint8_t carry = cpuRegister.flag_c;
    uint16_t result = accumulator + value + carry;
    cpuRegister.reg_a = result;
    cpuRegister.flag_z = (!(result & 0xFF));
    cpuRegister.flag_n = 0;
    cpuRegister.flag_h = ((accumulator & 0x0F) + (value & 0x0F) + carry) > 0x0F;
    cpuRegister.flag_c = (result > 0xFF);
}
void Cpu::instructionSbc(uint8_t value) {
    uint8_t accumulator = cpuRegister.reg_a;
    uint8_t carry = cpuRegister.flag_c;
    uint16_t result = (accumulator - (value + carry));
    cpuRegister.reg_a = result;
    cpuRegister.flag_z = (!(result & 0xFF));
    cpuRegister.flag_n = 1;
    cpuRegister.flag_h = ((accumulator & 0x0F) < ((value & 0x0F) + carry));
    cpuRegister.flag_c = (result > 0xFF);
}
void Cpu::instructionSub(uint8_t value) {
    uint8_t accumulator = cpuRegister.reg_a;
    uint8_t result = accumulator - value;
    cpuRegister.reg_a = result;
    cpuRegister.flag_z = (result == 0);
    cpuRegister.flag_n = 1;
    checkFlagH(accumulator, value, true);
    cpuRegister.flag_c = (result > accumulator);
}
uint8_t Cpu::instructionInc(uint8_t regAddrValue) {
    uint8_t valueBytePre = regAddrValue;
    uint8_t valueBytePost = regAddrValue + 1;
    cpuRegister.flag_z = (valueBytePost == 0);
    cpuRegister.flag_n = 0;
    checkFlagH(valueBytePre, 1, false);
    return valueBytePost;
}
uint8_t Cpu::instructionDec(uint8_t regAddrValue) {
    uint8_t valueBytePre = regAddrValue;
    uint8_t valueBytePost = regAddrValue - 1;
    cpuRegister.flag_z = (valueBytePost == 0);
    cpuRegister.flag_n = 1;
    checkFlagH(valueBytePre, 1, true);
    return valueBytePost;
}
uint8_t Cpu::decode(uint8_t opcode) {
    // tick
    uint8_t tick = OP_CYCLE[opcode];
    // pc
    uint16_t currentPc = cpuRegister.pc;
    cpuRegister.pc += OP_BYTES[opcode];
    uint16_t parseAddr;
    uint16_t parseAddrL, parseAddrR;
    uint16_t *holdAddrPtr;
    switch (opcode) {
        // ILLEGAL
        case op_disabled_01:
        case op_disabled_02:
        case op_disabled_03:
        case op_disabled_04:
        case op_disabled_05:
        case op_disabled_06:
        case op_disabled_07:
        case op_disabled_08:
        case op_disabled_09:
        case op_disabled_10:
        case op_disabled_11:
            // ACCESSED ILLEGAL OPCODE!\n
            cpuRegister.pc++;
            break;
            // SPECIAL
        case op_nop:
        case op_stop_0:
        case op_halt:
        case op_di:
        case op_ei:
            break;
            // ROTATES AND SHIFTS
        case op_rlca: {
                          uint8_t bit = ((cpuRegister.reg_a & 0x80) >> 7);
                          cpuRegister.reg_a = (cpuRegister.reg_a << 1) + bit;
                          cpuRegister.flag_z = 0;
                          cpuRegister.flag_h = 0;
                          cpuRegister.flag_n = 0;
                          cpuRegister.flag_c = bit;
                      }
                      break;
        case op_rla: {
                         uint8_t bit = ((cpuRegister.reg_a & 0x80) >> 7);
                         cpuRegister.reg_a = (cpuRegister.reg_a << 1) + cpuRegister.flag_c;
                         cpuRegister.flag_z = 0;
                         cpuRegister.flag_h = 0;
                         cpuRegister.flag_n = 0;
                         cpuRegister.flag_c = bit;
                     }
                     break;
        case op_rrca: {
                          uint8_t bit = (cpuRegister.reg_a & 1);
                          cpuRegister.reg_a = (bit << 7) + (cpuRegister.reg_a >> 1);
                          cpuRegister.flag_z = 0;
                          cpuRegister.flag_h = 0;
                          cpuRegister.flag_n = 0;
                          cpuRegister.flag_c = bit;
                      }
                      break;
        case op_rra: {
                         uint8_t bit = (cpuRegister.reg_a & 1);
                         cpuRegister.reg_a = (cpuRegister.flag_c << 7) + (cpuRegister.reg_a >> 1);
                         cpuRegister.flag_z = 0;
                         cpuRegister.flag_h = 0;
                         cpuRegister.flag_n = 0;
                         cpuRegister.flag_c = bit;
                     }
                     break;
                     // LOADS 8-bit
        case op_ld_b_d8:
        case op_ld_c_d8:
        case op_ld_d_d8:
        case op_ld_e_d8:
        case op_ld_h_d8:
        case op_ld_l_d8:
        case op_ld_a_d8: {
                             parseAddrL = (opcode & 0x38) >> 3;
                             (*cpuRegister.reg[parseAddrL]) = mmu->readByte(currentPc + 1);
                         }
                         break;
                         // LD A,rr
        case op_ld_a_mbc:
        case op_ld_a_mde:
                         parseAddr = (opcode & 0x30) >> 4;
                         holdAddrPtr = (cpuRegister.reg_pair[parseAddr]);
                         cpuRegister.reg_a = mmu->readByte(*holdAddrPtr);
                         break;
                         // LD A, HL+/-
        case op_ld_a_mhli:
                         cpuRegister.reg_a = mmu->readByte(cpuRegister.reg_pair_hl++);
                         break;
        case op_ld_a_mhld:
                         cpuRegister.reg_a = mmu->readByte(cpuRegister.reg_pair_hl--);
                         break;
                         // LD (reg, no HL), (reg, no HL)
        case op_ld_b_b:
        case op_ld_b_c:
        case op_ld_b_d:
        case op_ld_b_e:
        case op_ld_b_h:
        case op_ld_b_l:
        case op_ld_b_a:
        case op_ld_c_b:
        case op_ld_c_c:
        case op_ld_c_d:
        case op_ld_c_e:
        case op_ld_c_h:
        case op_ld_c_l:
        case op_ld_c_a:
        case op_ld_d_b:
        case op_ld_d_c:
        case op_ld_d_d:
        case op_ld_d_e:
        case op_ld_d_h:
        case op_ld_d_l:
        case op_ld_d_a:
        case op_ld_e_b:
        case op_ld_e_c:
        case op_ld_e_d:
        case op_ld_e_e:
        case op_ld_e_h:
        case op_ld_e_l:
        case op_ld_e_a:
        case op_ld_h_b:
        case op_ld_h_c:
        case op_ld_h_d:
        case op_ld_h_e:
        case op_ld_h_h:
        case op_ld_h_l:
        case op_ld_h_a:
        case op_ld_l_b:
        case op_ld_l_c:
        case op_ld_l_d:
        case op_ld_l_e:
        case op_ld_l_h:
        case op_ld_l_l:
        case op_ld_l_a:
        case op_ld_a_b:
        case op_ld_a_c:
        case op_ld_a_d:
        case op_ld_a_e:
        case op_ld_a_h:
        case op_ld_a_l:
        case op_ld_a_a:
                         parseAddrR = (opcode & 0x07);
                         parseAddrL = (opcode & 0x38) >> 3;
                         (*cpuRegister.reg[parseAddrL]) = (*cpuRegister.reg[parseAddrR]);
                         break;
                         // LD reg, (HL)
        case op_ld_b_mhl:
        case op_ld_c_mhl:
        case op_ld_d_mhl:
        case op_ld_e_mhl:
        case op_ld_h_mhl:
        case op_ld_l_mhl:
        case op_ld_a_mhl: {
                              uint8_t parseRegister = (opcode & 0x38) >> 3;
                              uint8_t value = mmu->readByte(cpuRegister.reg_pair_hl);
                              (*cpuRegister.reg[parseRegister]) = value;
                          }
                          break;
        case op_ldh_a8_a:
                          parseAddr = 0xFF00 + mmu->readByte(currentPc + 1);
                          mmu->writeByte(parseAddr, cpuRegister.reg_a);
                          break;
        case op_ldh_a_a8:
                          parseAddr = 0xFF00 + mmu->readByte(currentPc + 1);
                          cpuRegister.reg_a = mmu->readByte(parseAddr);
                          break;
        case op_ld_ff00c_a:
                          parseAddr = 0xFF00 + cpuRegister.reg_c;
                          mmu->writeByte(parseAddr, cpuRegister.reg_a);
                          break;
        case op_ldh_a_ff00_c:
                          parseAddr = 0xFF00 + cpuRegister.reg_c;
                          cpuRegister.reg_a = mmu->readByte(parseAddr);
                          break;
                          // LOADS 16-bit
        case op_ld_bc_d16:
        case op_ld_de_d16:
        case op_ld_hl_d16:
        case op_ld_sp_d16:
                          parseAddr = (0x30 & opcode) >> 4;
                          (*cpuRegister.reg_pair[parseAddr]) = mmu->readShort(currentPc + 1);
                          break;
        case op_ld_mbc_a:
        case op_ld_mde_a:
                          parseAddr = (opcode & 0x30) >> 4;
                          holdAddrPtr = (cpuRegister.reg_pair[parseAddr]);
                          mmu->writeByte(*holdAddrPtr, cpuRegister.reg_a);
                          break;
        case op_ld_a16_sp:
                          parseAddr = mmu->readShort(currentPc + 1);
                          mmu->writeByte(parseAddr, (cpuRegister.sp & 0x00FF));
                          mmu->writeByte(parseAddr + 1, (cpuRegister.sp & 0xFF00) >> 8);
                          break;
        case op_ld_mhli_a:
                          mmu->writeByte(cpuRegister.reg_pair_hl++, cpuRegister.reg_a);
                          break;
        case op_ld_mhld_a:
                          mmu->writeByte(cpuRegister.reg_pair_hl--, cpuRegister.reg_a);
                          break;
        case op_ld_mhl_d8: {
                               uint8_t nextByte = mmu->readByte(currentPc + 1);
                               mmu->writeByte(cpuRegister.reg_pair_hl, nextByte);
                           }
                           break;
                           // LD (HL), reg
        case op_ld_mhl_b:
        case op_ld_mhl_c:
        case op_ld_mhl_d:
        case op_ld_mhl_e:
        case op_ld_mhl_h:
        case op_ld_mhl_l:
        case op_ld_mhl_a:
                           parseAddr = (opcode & 0x07);
                           mmu->writeByte(cpuRegister.reg_pair_hl, (*cpuRegister.reg[parseAddr]));
                           break;
        case op_ld_hl_sp_add_r8: {
                                     int8_t nextByte = mmu->readByte(currentPc + 1);
                                     uint16_t sp = cpuRegister.sp;
                                     uint16_t result = sp + nextByte;
                                     cpuRegister.reg_pair_hl = result;
                                     cpuRegister.flag_z = 0;
                                     cpuRegister.flag_n = 0;
                                     cpuRegister.flag_h = ((result & 0x0F) < (sp & 0x0F));
                                     cpuRegister.flag_c = ((result & 0xFF) < (sp & 0xFF));
                                 }
                                 break;
        case op_ld_sp_hl:
                                 cpuRegister.sp = cpuRegister.reg_pair_hl;
                                 break;
        case op_ld_ma16_a:
                                 parseAddr = mmu->readShort(currentPc + 1);
                                 mmu->writeByte(parseAddr, cpuRegister.reg_a);
                                 break;
        case op_ld_a_ma16:
                                 parseAddr = mmu->readShort(currentPc + 1);
                                 cpuRegister.reg_a = mmu->readByte(parseAddr);
                                 break;
                                 // JUMPS AND STACKS
        case op_jr_r8: {
                           int8_t nextByte = mmu->readByte(currentPc + 1);
                           conditionalJpAdd(1, 1, nextByte);
                       }
                       break;
        case op_jr_nz_r8:
        case op_jr_z_r8: {
                             uint8_t bit = ((opcode & 0x08) >> 3);
                             int8_t nextByte = mmu->readByte(currentPc + 1);
                             bool result = conditionalJpAdd(cpuRegister.flag_z, bit, nextByte);
                             tick = result ? 12 : 8;
                         }
                         break;
        case op_jr_nc_r8:
        case op_jr_c_r8: {
                             uint8_t bit = ((opcode & 0x08) >> 3);
                             int8_t nextByte = mmu->readByte(currentPc + 1);
                             bool result = conditionalJpAdd(cpuRegister.flag_c, bit, nextByte);
                             tick = result ? 12 : 8;
                         }
                         break;
        case op_ret_nz:
        case op_ret_z: {
                           uint8_t bit = ((opcode & 0x08) >> 3);
                           bool result = conditionalRet(cpuRegister.flag_z, bit);
                           tick = result ? 20 : 8;
                       }
                       break;
        case op_ret_nc:
        case op_ret_c: {
                           uint8_t bit = ((opcode & 0x08) >> 3);
                           bool result = conditionalRet(cpuRegister.flag_c, bit);
                           tick = result ? 20 : 8;
                       }
                       break;
        case op_reti:
                       instructionRet();
                       break;
        case op_pop_bc:
        case op_pop_de:
        case op_pop_hl:
                       parseAddr = (opcode & 0x30) >> 4;
                       (*cpuRegister.reg_pair[parseAddr]) = instructionStackPop();
                       break;
        case op_pop_af:
                       cpuRegister.reg_pair_af = (instructionStackPop() & 0xFFF0);
                       break;
        case op_jp_nz_a16:
        case op_jp_z_a16: {
                              uint8_t bit = (opcode & 0x08) >> 3;
                              uint16_t nextWord = mmu->readShort(currentPc + 1);
                              bool result = conditionalJpA16(cpuRegister.flag_z, bit, nextWord);
                              tick = result ? 16 : 12;
                          }
                          break;
        case op_jp_nc_a16:
        case op_jp_c_a16: {
                              uint8_t bit = (opcode & 0x08) >> 3;
                              uint16_t nextWord = mmu->readShort(currentPc + 1);
                              bool result = conditionalJpA16(cpuRegister.flag_c, bit, nextWord);
                              tick = result ? 16 : 12;
                          }
                          break;
        case op_jp_a16: {
                            uint16_t nextWord = mmu->readShort(currentPc + 1);
                            conditionalJpA16(1, 1, nextWord);
                        }
                        break;
        case op_call_nz_a16:
        case op_call_z_a16: {
                                uint8_t bit = (opcode & 0x08) >> 3;
                                bool result = conditionalCall(currentPc, cpuRegister.flag_z, bit);
                                tick = result ? 24 : 12;
                            }
                            break;

        case op_call_nc_a16:
        case op_call_c_a16: {
                                uint8_t bit = (opcode & 0x08) >> 3;
                                bool result = conditionalCall(currentPc, cpuRegister.flag_c, bit);
                                tick = result ? 24 : 12;
                            }
                            break;
                            // PUSH rr
        case op_push_bc:
        case op_push_de:
        case op_push_hl:
                            parseAddr = (0x30 & opcode) >> 4;
                            instructionStackPush((*cpuRegister.reg_pair[parseAddr]));
                            break;
        case op_push_af:
                            instructionStackPush(cpuRegister.reg_pair_af);
                            break;
        case op_ret:
                            instructionRet();
                            break;
        case op_call_a16:
                            conditionalCall(currentPc, 1, 1);
                            break;
        case op_jp_mhl:
                            cpuRegister.pc = cpuRegister.reg_pair_hl;
                            break;
        case op_rst_00h:
        case op_rst_08h:
        case op_rst_10h:
        case op_rst_18h:
        case op_rst_20h:
        case op_rst_28h:
        case op_rst_30h:
        case op_rst_38h: {
                             uint8_t rstAddr = (opcode & 0x38);
                             instructionStackPush(currentPc + 1);
                             cpuRegister.pc = rstAddr;
                         }
                         break;
                         // ALU 8-bit
                         // INC reg
        case op_inc_b:
        case op_inc_c:
        case op_inc_d:
        case op_inc_e:
        case op_inc_h:
        case op_inc_l:
        case op_inc_a:
                         parseAddr = (opcode & 0x38) >> 3;
                         (*cpuRegister.reg[parseAddr]) =
                             instructionInc((*cpuRegister.reg[parseAddr]));
                         break;
        case op_inc_mhl: {
                             uint16_t hlAddress = cpuRegister.reg_pair_hl;
                             uint8_t value = instructionInc(mmu->readByte(hlAddress));
                             mmu->writeByte(hlAddress, value);
                         }
                         break;
                         // DEC reg
        case op_dec_b:
        case op_dec_c:
        case op_dec_d:
        case op_dec_e:
        case op_dec_h:
        case op_dec_l:
        case op_dec_a:
                         parseAddr = (opcode & 0x38) >> 3;
                         (*cpuRegister.reg[parseAddr]) =
                             instructionDec((*cpuRegister.reg[parseAddr]));
                         break;
        case op_dec_mhl: {
                             uint16_t hlAddress = cpuRegister.reg_pair_hl;
                             uint8_t value = instructionDec(mmu->readByte(hlAddress));
                             mmu->writeByte(hlAddress, value);
                         }
                         break;
                         // ALU 16-bit
                         // INC rr
        case op_inc_bc:
        case op_inc_de:
        case op_inc_hl:
        case op_inc_sp:
                         parseAddr = (opcode & 0x30) >> 4;
                         (*cpuRegister.reg_pair[parseAddr])++;
                         break;
                         // ADD HL, rr
        case op_add_hl_bc:
        case op_add_hl_de:
        case op_add_hl_hl:
        case op_add_hl_sp: {
                               uint8_t parsePairAddr = (opcode & 0x30) >> 4;
                               uint16_t *addrPtr = cpuRegister.reg_pair[parsePairAddr];
                               uint16_t addrVal = (*addrPtr);
                               uint16_t hl = cpuRegister.reg_pair_hl;
                               cpuRegister.reg_pair_hl += (addrVal);
                               cpuRegister.flag_n = 0;
                               cpuRegister.flag_h = ((hl & 0x0FFF) + ((addrVal)&0x0FFF)) > 0x0FFF;
                               cpuRegister.flag_c = uint16_t(hl + (addrVal)) < hl;
                           }
                           break;
        case op_dec_bc:
        case op_dec_de:
        case op_dec_hl:
        case op_dec_sp:
                           parseAddr = (0x30 & opcode) >> 4;
                           (*cpuRegister.reg_pair[parseAddr])--;
                           break;
        case op_add_sp_r8: {
                               int8_t nextByte = mmu->readByte(currentPc + 1);
                               uint16_t sp = cpuRegister.sp;
                               uint16_t result = sp + nextByte;
                               cpuRegister.sp = result;
                               cpuRegister.flag_z = 0;
                               cpuRegister.flag_n = 0;
                               cpuRegister.flag_h = (result & 0x0F) < (sp & 0x0F);
                               cpuRegister.flag_c = (result & 0xFF) < (sp & 0xFF);
                           }
                           break;
                           // LOGIC
                           // ADD A, r
        case op_add_a_b:
        case op_add_a_c:
        case op_add_a_d:
        case op_add_a_e:
        case op_add_a_h:
        case op_add_a_l:
        case op_add_a_mhl:
        case op_add_a_a:
        case op_add_a_d8: {
                              uint8_t value;
                              switch (opcode) {
                                  case op_add_a_mhl:
                                      value = mmu->readByte(cpuRegister.reg_pair_hl);
                                      break;
                                  case op_add_a_d8:
                                      value = mmu->readByte(currentPc + 1);
                                      break;
                                  default:
                                      parseAddr = (opcode & 0x07);
                                      value = (*cpuRegister.reg[parseAddr]);
                              }
                              instructionAdd(value);
                          }
                          break;
                          // ADC A, r
        case op_adc_a_b:
        case op_adc_a_c:
        case op_adc_a_d:
        case op_adc_a_e:
        case op_adc_a_h:
        case op_adc_a_l:
        case op_adc_a_mhl:
        case op_adc_a_a:
        case op_adc_a_d8: {
                              uint8_t value;
                              switch (opcode) {
                                  case 0x8E:
                                      value = mmu->readByte(cpuRegister.reg_pair_hl);
                                      break;
                                  case 0xCE:
                                      value = mmu->readByte(currentPc + 1);
                                      break;
                                  default:
                                      parseAddr = (opcode & 0x07);
                                      value = (*cpuRegister.reg[parseAddr]);
                              }
                              instructionAdc(value);
                          }
                          break;
                          // SUB r
        case op_sub_b:
        case op_sub_c:
        case op_sub_d:
        case op_sub_e:
        case op_sub_h:
        case op_sub_l:
        case op_sub_mhl:
        case op_sub_a:
        case op_sub_d8: {
                            uint8_t value;
                            switch (opcode) {
                                case 0x96:
                                    value = mmu->readByte(cpuRegister.reg_pair_hl);
                                    break;
                                case 0xD6:
                                    value = mmu->readByte(currentPc + 1);
                                    break;
                                default:
                                    parseAddr = (opcode & 0x07);
                                    value = (*cpuRegister.reg[parseAddr]);
                            }
                            instructionSub(value);
                        }
                        break;
                        // SBC A, r
        case op_sbc_a_b:
        case op_sbc_a_c:
        case op_sbc_a_d:
        case op_sbc_a_e:
        case op_sbc_a_h:
        case op_sbc_a_l:
        case op_sbc_a_mhl:
        case op_sbc_a_a:
        case op_sbc_a_d8: {
                              uint8_t value;
                              switch (opcode) {
                                  case 0x9E:
                                      value = mmu->readByte(cpuRegister.reg_pair_hl);
                                      break;
                                  case 0xDE:
                                      value = mmu->readByte(currentPc + 1);
                                      break;
                                  default:
                                      parseAddr = (opcode & 0x07);
                                      value = (*cpuRegister.reg[parseAddr]);
                              }
                              instructionSbc(value);
                          }
                          break;
        case op_and_b:
        case op_and_c:
        case op_and_d:
        case op_and_e:
        case op_and_h:
        case op_and_l:
        case op_and_a:
        case op_xor_b:
        case op_xor_c:
        case op_xor_d:
        case op_xor_e:
        case op_xor_h:
        case op_xor_l:
        case op_xor_a:
        case op_or_b:
        case op_or_c:
        case op_or_d:
        case op_or_e:
        case op_or_h:
        case op_or_l:
        case op_or_a:
        case op_cp_b:
        case op_cp_c:
        case op_cp_d:
        case op_cp_e:
        case op_cp_h:
        case op_cp_l:
        case op_cp_a: {
                          parseAddr = (opcode & 0x07);
                          uint8_t parseInst = (opcode & 0x18) >> 3;
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
                      }
                      break;

        case op_and_mhl:
        case op_xor_mhl:
        case op_or_mhl:
        case op_cp_mhl: {
                            uint8_t parseInst = (opcode & 0x18) >> 3;
                            uint8_t hlValue = mmu->readByte(cpuRegister.reg_pair_hl);
                            switch (parseInst) {
                                case 0:
                                    instructionAnd(hlValue);
                                    break;
                                case 1:
                                    instructionXor(hlValue);
                                    break;
                                case 2:
                                    instructionOr(hlValue);
                                    break;
                                case 3:
                                    instructionCp(hlValue);
                                    break;
                            }
                        }
                        break;

        case op_and_d8:
        case op_xor_d8:
        case op_or_d8:
        case op_cp_d8: {
                           uint8_t parseInst = (opcode & 0x18) >> 3;
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
                       }
                       break;
        case op_scf:
                       cpuRegister.flag_n = 0;
                       cpuRegister.flag_h = 0;
                       cpuRegister.flag_c = 1;
                       break;
                       // source - ehaskins.com
        case op_daa: {
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
                     }
                     break;
        case op_cpl:
                     cpuRegister.reg_a = ~cpuRegister.reg_a;
                     cpuRegister.flag_n = 1;
                     cpuRegister.flag_h = 1;
                     break;
        case op_ccf:
                     cpuRegister.flag_n = 0;
                     cpuRegister.flag_h = 0;
                     cpuRegister.flag_c = (cpuRegister.flag_c) ? 0 : 1;
                     break;
        case op_prefix_cb:
                     cpuRegister.pc++;
                     currentPc++;
                     opcode = mmu->readByte(currentPc);
                     tick += decodeCb(opcode);
                     break;
        default:
                     // unknown opcode
                     *halt = true;
    }
    return tick;
}
uint8_t Cpu::decodeCb(uint8_t opcode) {
    // tick
    uint8_t tick = OP_CYCLE[opcode];
    // pc
    uint8_t parseOpcodeAddr;
    uint8_t holdBit, holdByte;
    uint8_t *holdRegPtr;
    uint8_t holdMask, holdMemoryValueHl;
    parseOpcodeAddr = (opcode & 0x07);
    holdRegPtr = (cpuRegister.reg[parseOpcodeAddr]);
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
                   }
                   break;
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
                   }
                   break;
        default:
                   // unknown cb opcode
                   *halt = true;
    }
    return tick;
}
void Cpu::initializeRegisters() {
    this->cpuRegister.reg_a = 0;
    this->cpuRegister.reg_b = 0;
    this->cpuRegister.reg_c = 0;
    this->cpuRegister.reg_d = 0;
    this->cpuRegister.reg_e = 0;
    this->cpuRegister.reg_h = 0;
    this->cpuRegister.reg_l = 0;
    this->cpuRegister.flag_z = 0;
    this->cpuRegister.flag_h = 0;
    this->cpuRegister.flag_n = 0;
    this->cpuRegister.flag_c = 0;
    this->cpuRegister.ZEROFILL = 0;
    this->cpuRegister.pc = 0;
    this->cpuRegister.sp = 0;
}
