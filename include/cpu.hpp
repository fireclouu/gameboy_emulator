/*
   │* cpu.hpp
   │* Copyright (C) 2022 fireclouu
   │*
   │* This program is free software: you can redistribute it and/or modify
   │* it under the terms of the GNU General Public License as published by
   │* the Free Software Foundation, either version 3 of the License, or
   │* (at your option) any later version.
   │*
   │* This program is distributed in the hope that it will be useful,
   │* but WITHOUT ANY WARRANTY; without even the implied warranty of
   │* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   │* GNU General Public License for more details.
   │*
   │* You should have received a copy of the GNU General Public License
   │* along with this program. If not, see <http://www.gnu.org/licenses/>.
   │*/

#ifndef SRC_INCLUDE_CPU_HPP_
#define SRC_INCLUDE_CPU_HPP_

#include <stdint.h>
#include "opcode.hpp"
#include "mmu.hpp"

enum opcodeInstruction {
    op_nop,
    op_ld_bc_d16,
    op_ld_mbc_a,
    op_inc_bc,
    op_inc_b,
    op_dec_b,
    op_ld_b_d8,
    op_rlca,
    op_ld_a16_sp,
    op_add_hl_bc,
    op_ld_a_mbc,
    op_dec_bc,
    op_inc_c,
    op_dec_c,
    op_ld_c_d8,
    op_rrca,
    op_stop_0,
    op_ld_de_d16,
    op_ld_mde_a,
    op_inc_de,
    op_inc_d,
    op_dec_d,
    op_ld_d_d8,
    op_rla,
    op_jr_r8,
    op_add_hl_de,
    op_ld_a_mde,
    op_dec_de,
    op_inc_e,
    op_dec_e,
    op_ld_e_d8,
    op_rra,
    op_jr_nz_r8,
    op_ld_hl_d16,
    op_ld_mhli_a,
    op_inc_hl,
    op_inc_h,
    op_dec_h,
    op_ld_h_d8,
    op_daa,
    op_jr_z_r8,
    op_add_hl_hl,
    op_ld_a_mhli,
    op_dec_hl,
    op_inc_l,
    op_dec_l,
    op_ld_l_d8,
    op_cpl,
    op_jr_nc_r8,
    op_ld_sp_d16,
    op_ld_mhld_a,
    op_inc_sp,
    op_inc_mhl,
    op_dec_mhl,
    op_ld_mhl_d8,
    op_scf,
    op_jr_c_r8,
    op_add_hl_sp,
    op_ld_a_mhld,
    op_dec_sp,
    op_inc_a,
    op_dec_a,
    op_ld_a_d8,
    op_ccf,
    op_ld_b_b,
    op_ld_b_c,
    op_ld_b_d,
    op_ld_b_e,
    op_ld_b_h,
    op_ld_b_l,
    op_ld_b_mhl,
    op_ld_b_a,
    op_ld_c_b,
    op_ld_c_c,
    op_ld_c_d,
    op_ld_c_e,
    op_ld_c_h,
    op_ld_c_l,
    op_ld_c_mhl,
    op_ld_c_a,
    op_ld_d_b,
    op_ld_d_c,
    op_ld_d_d,
    op_ld_d_e,
    op_ld_d_h,
    op_ld_d_l,
    op_ld_d_mhl,
    op_ld_d_a,
    op_ld_e_b,
    op_ld_e_c,
    op_ld_e_d,
    op_ld_e_e,
    op_ld_e_h,
    op_ld_e_l,
    op_ld_e_mhl,
    op_ld_e_a,
    op_ld_h_b,
    op_ld_h_c,
    op_ld_h_d,
    op_ld_h_e,
    op_ld_h_h,
    op_ld_h_l,
    op_ld_h_mhl,
    op_ld_h_a,
    op_ld_l_b,
    op_ld_l_c,
    op_ld_l_d,
    op_ld_l_e,
    op_ld_l_h,
    op_ld_l_l,
    op_ld_l_mhl,
    op_ld_l_a,
    op_ld_mhl_b,
    op_ld_mhl_c,
    op_ld_mhl_d,
    op_ld_mhl_e,
    op_ld_mhl_h,
    op_ld_mhl_l,
    op_halt,
    op_ld_mhl_a,
    op_ld_a_b,
    op_ld_a_c,
    op_ld_a_d,
    op_ld_a_e,
    op_ld_a_h,
    op_ld_a_l,
    op_ld_a_mhl,
    op_ld_a_a,
    op_add_a_b,
    op_add_a_c,
    op_add_a_d,
    op_add_a_e,
    op_add_a_h,
    op_add_a_l,
    op_add_a_mhl,
    op_add_a_a,
    op_adc_a_b,
    op_adc_a_c,
    op_adc_a_d,
    op_adc_a_e,
    op_adc_a_h,
    op_adc_a_l,
    op_adc_a_mhl,
    op_adc_a_a,
    op_sub_b,
    op_sub_c,
    op_sub_d,
    op_sub_e,
    op_sub_h,
    op_sub_l,
    op_sub_mhl,
    op_sub_a,
    op_sbc_a_b,
    op_sbc_a_c,
    op_sbc_a_d,
    op_sbc_a_e,
    op_sbc_a_h,
    op_sbc_a_l,
    op_sbc_a_mhl,
    op_sbc_a_a,
    op_and_b,
    op_and_c,
    op_and_d,
    op_and_e,
    op_and_h,
    op_and_l,
    op_and_mhl,
    op_and_a,
    op_xor_b,
    op_xor_c,
    op_xor_d,
    op_xor_e,
    op_xor_h,
    op_xor_l,
    op_xor_mhl,
    op_xor_a,
    op_or_b,
    op_or_c,
    op_or_d,
    op_or_e,
    op_or_h,
    op_or_l,
    op_or_mhl,
    op_or_a,
    op_cp_b,
    op_cp_c,
    op_cp_d,
    op_cp_e,
    op_cp_h,
    op_cp_l,
    op_cp_mhl,
    op_cp_a,
    op_ret_nz,
    op_pop_bc,
    op_jp_nz_a16,
    op_jp_a16,
    op_call_nz_a16,
    op_push_bc,
    op_add_a_d8,
    op_rst_00h,
    op_ret_z,
    op_ret,
    op_jp_z_a16,
    op_prefix_cb,
    op_call_z_a16,
    op_call_a16,
    op_adc_a_d8,
    op_rst_08h,
    op_ret_nc,
    op_pop_de,
    op_jp_nc_a16,
    op_disabled_01,
    op_call_nc_a16,
    op_push_de,
    op_sub_d8,
    op_rst_10h,
    op_ret_c,
    op_reti,
    op_jp_c_a16,
    op_disabled_02,
    op_call_c_a16,
    op_disabled_03,
    op_sbc_a_d8,
    op_rst_18h,
    op_ldh_a8_a,
    op_pop_hl,
    op_ld_ff00c_a,
    op_disabled_04,
    op_disabled_05,
    op_push_hl,
    op_and_d8,
    op_rst_20h,
    op_add_sp_r8,
    op_jp_mhl,
    op_ld_ma16_a,
    op_disabled_06,
    op_disabled_07,
    op_disabled_08,
    op_xor_d8,
    op_rst_28h,
    op_ldh_a_a8,
    op_pop_af,
    op_ldh_a_ff00_c,
    op_di,
    op_disabled_09,
    op_push_af,
    op_or_d8,
    op_rst_30h,
    op_ld_hl_sp_add_r8,
    op_ld_sp_hl,
    op_ld_a_ma16,
    op_ei,
    op_disabled_10,
    op_disabled_11,
    op_cp_d8,
    op_rst_38h,
};

struct CpuRegister {
    union {
        uint8_t all_reg[8];
        struct {
            union {
                uint16_t reg_pair_bc;
                struct {
                    uint8_t reg_c, reg_b;
                };
            };
            union {
                uint16_t reg_pair_de;
                struct {
                    uint8_t reg_e, reg_d;
                };
            };
            union {
                uint16_t reg_pair_hl;
                struct {
                    uint8_t reg_l, reg_h;
                };
            };
            union {
                uint16_t reg_pair_af;
                struct {
                    union {
                        uint8_t reg_f;
                        struct {
                            uint8_t ZEROFILL : 4;
                            uint8_t flag_c : 1, flag_h : 1, flag_n : 1, flag_z : 1;
                        };
                    };
                    uint8_t reg_a;
                };
            };
        };
    };
    uint16_t sp;
    uint16_t pc;
    uint8_t* reg[8] = {&reg_b, &reg_c, &reg_d,  &reg_e,
        &reg_h, &reg_l, nullptr, &reg_a};
    uint16_t* reg_pair[4] = {&reg_pair_bc, &reg_pair_de, &reg_pair_hl, &sp};
};

class Cpu {
    private:
        // datatypes and struct
        // class declaration
        Mmu* mmu;
        // functions
        uint8_t decodeCb(uint8_t opcode);
        uint8_t instructionInc(uint8_t regAddrValue);
        uint8_t instructionDec(uint8_t regAddrValue);
        uint16_t instructionStackPop();
        void checkFlagH(uint8_t left, uint8_t right, bool isSubtraction);
        void conditionalJpAdd(uint8_t flag, uint8_t expected, int8_t value);
        void conditionalJpA16(uint8_t flag, uint8_t expected, uint16_t value);
        void conditionalRet(uint8_t flag, uint8_t expected);
        void conditionalCall(uint16_t pc, uint8_t flag, uint8_t expected);
        void initializeRegisters();
        void instructionRet();
        void instructionCall(uint16_t pc);
        void instructionAnd(uint8_t value);
        void instructionXor(uint8_t value);
        void instructionOr(uint8_t value);
        void instructionCp(uint8_t value);
        void instructionAdd(uint8_t value);
        void instructionAdc(uint8_t value);
        void instructionSbc(uint8_t value);
        void instructionSub(uint8_t value);

    public:
        Cpu();
        ~Cpu();
        bool* halt;
        struct CpuRegister cpuRegister = {};
        void setMmu(Mmu* mmu);
        void setHalt(bool* halt);
        void instructionStackPush(uint16_t addr_value);
        uint8_t decode(uint8_t opcode);
};
#endif  // SRC_INCLUDE_CPU_HPP_
