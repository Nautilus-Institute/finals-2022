#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include <exception>
#include <vector>

#include "vm.hpp"
#include "instr.hpp"

enum reg32_t { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, EIP, REGS_COUNT };
enum reg16_t { AX, CX, DX, BX, SP, BP, SI, DI };
enum reg8_t { AL, CL, DL, BL, AH, CH, DH, BH };

union Register {
	uint32_t reg32;
	uint16_t reg16;
	struct {
		uint8_t reg8_l;
		uint8_t reg8_h;
	};
};

union EFLAGS {
	uint32_t reg32;
	uint16_t reg16;

	struct {
		uint32_t CF : 1;
		uint32_t : 1;           // 1
		uint32_t PF : 1;
		uint32_t : 1;           // 0
		uint32_t AF : 1;
		uint32_t : 1;           // 0
		uint32_t ZF : 1;
		uint32_t SF : 1;
		uint32_t TF : 1;
		uint32_t IF : 1;
		uint32_t DF : 1;
		uint32_t OF : 1;
		uint32_t IOPL : 2;
		uint32_t NT : 1;
		uint32_t : 1;           // 0
		uint32_t RF : 1;
		uint32_t VM : 1;
		uint32_t AC : 1;
		uint32_t VIF : 1;
		uint32_t VIP : 1;
		uint32_t ID : 1;
	};
};

class CPU {
private:
    // Vulnerability: Overwrite the last register (which is EIP) to achieve arbitrary jumps
    Register registers[REGS_COUNT];
    EFLAGS eflags;
    bool b_halted;
    InstrData *instr;
    VM* m_vm;
    std::vector<uint32_t> stack;
public:
    CPU(VM* vm);

    uint32_t get_eip(void) const { return registers[EIP].reg32; };
    uint32_t get_reg(enum reg32_t n) const { if (n < REGS_COUNT) return registers[n].reg32; else throw std::exception(); };
    uint16_t get_reg(enum reg16_t n) const { if ((reg32_t)n < REGS_COUNT) return registers[n].reg16; else throw std::exception(); };
    uint8_t get_reg(enum reg8_t n) const { if ((reg32_t)n < REGS_COUNT) return n<AH ? registers[n].reg8_l : registers[n-AH].reg8_h; else throw std::exception(); };
    void set_eip(uint32_t v) { registers[EIP].reg32 = v; };
    void set_reg(enum reg32_t n, uint32_t v) { if (n < REGS_COUNT) registers[n].reg32 = v; else throw std::exception(); };
    void set_reg(enum reg16_t n, uint16_t v) { if ((reg32_t)n < REGS_COUNT) registers[n].reg16 = v; else throw std::exception(); };
    void set_reg(enum reg8_t n, uint8_t v) { if ((reg32_t)n < REGS_COUNT) (n<AH ? registers[n].reg8_l : registers[n-AH].reg8_h) = v; else throw std::exception(); };
    uint32_t update_eip(int32_t v) { return registers[EIP].reg32 += v; };
    uint32_t update_reg(enum reg32_t n, int32_t v) { if (n < REGS_COUNT) return registers[n].reg32 += v; else throw std::exception(); };
    uint16_t update_reg(enum reg16_t n, int16_t v) { if ((reg32_t)n < REGS_COUNT) return registers[n].reg16 += v; else throw std::exception(); };
    uint8_t update_reg(enum reg8_t n, int8_t v) { if ((reg32_t)n < REGS_COUNT) return (n<AH ? registers[n].reg8_l : registers[n-AH].reg8_h) += v; else throw std::exception(); };
    bool halted(void) const { return b_halted; };
    void halt(bool h) { b_halted = h; };;

    uint8_t get_code8() { return get_data8(get_eip()); };
    uint16_t get_code16() { return get_data16(get_eip()); };
    uint32_t get_code32() { return get_data32(get_eip()); };

    bool exec(InstrData* instr);

    // EFlags
    template <class T> uint32_t update_eflags_add(T v1, uint32_t v2);
    template <class T> uint32_t update_eflags_or(T v1, uint32_t v2);
    template <class T> uint32_t update_eflags_and(T v1, uint32_t v2);
    template <class T> uint32_t update_eflags_sub(T v1, uint32_t v2);
    template <class T> uint32_t update_eflags_mul(T v1, uint32_t v2);
    template <class T> uint32_t update_eflags_imul(T v1, int32_t v2);
    template <class T> uint32_t update_eflags_shl(T v, uint8_t c);
    template <class T> uint32_t update_eflags_shr(T v, uint8_t c);

    uint32_t get_eflags() { return eflags.reg32; };
    void set_eflags(uint32_t v) { eflags.reg32 = v; };
    uint16_t get_flags() { return eflags.reg16; };
    void set_flags(uint16_t v) { eflags.reg16 = v; };

    bool is_carry() { return eflags.CF; };
    bool is_parity() { return eflags.PF; };
    bool is_zero() { return eflags.ZF; };
    bool is_sign() { return eflags.SF; };
    bool is_overflow() { return eflags.OF; };
    bool is_interrupt() { return eflags.IF; };
    bool is_direction() { return eflags.DF; };
    void set_carry(bool carry) { eflags.CF = carry; };
    void set_parity(bool parity) { eflags.PF = parity; };
    void set_zero(bool zero) { eflags.ZF = zero; };
    void set_sign(bool sign) { eflags.SF = sign; };
    void set_overflow(bool over) { eflags.OF = over; };
    void set_interrupt(bool interrupt) { eflags.IF = interrupt; };
    void set_direction(bool dir) { eflags.DF = dir; };
    bool chk_parity(uint8_t v);

    // Utility functions
    void set_rm32(uint32_t value);
    uint32_t get_rm32();
    void set_r32(uint32_t value);
    uint32_t get_r32();
    void set_moffs32(uint32_t value);
    uint32_t get_moffs32();
    void set_rm16(uint16_t value);
    uint16_t get_rm16();
    void set_r16(uint16_t value);
    uint16_t get_r16();
    void set_moffs16(uint16_t value);
    uint16_t get_moffs16();
    void set_rm8(uint8_t value);
    uint8_t get_rm8();
    void set_r8(uint8_t value);
    void set_moffs8(uint8_t value);
    uint8_t get_moffs8();
    uint8_t get_r8();
    uint32_t get_m();
    uint32_t calc_modrm();
    uint32_t calc_modrm16();
    uint32_t calc_modrm32();
    uint32_t calc_sib();

    uint32_t pop32() { if (stack.size() == 0) { return 0; } uint32_t v = stack.back(); stack.pop_back(); return v;}
    void push32(uint32_t v) { stack.push_back(v); }
    uint32_t get_data32(uint32_t addr);
    void put_data32(uint32_t addr, uint32_t v);
    uint16_t get_data16(uint32_t addr);
    void put_data16(uint32_t addr, uint16_t v);
    uint8_t get_data8(uint32_t addr);
    void put_data8(uint32_t addr, uint8_t v);

    // Instruction handlers
    void add_rm8_r8();
    void add_rm32_r32();
    void add_r8_rm8();
    void add_r32_rm32();
    void add_al_imm8();
    void add_eax_imm32();
    void push_es();
    void pop_es();
    void or_rm8_r8();
    void or_rm32_r32();
    void or_r8_rm8();
    void or_r32_rm32();
    void or_al_imm8(); 
    void or_eax_imm32();
    void push_ss(); 
    void pop_ss();
    void push_ds();
    void pop_ds();
    void and_rm8_r8();
    void and_rm32_r32();
    void and_r8_rm8();
    void and_r32_rm32();
    void and_al_imm8();
    void and_eax_imm32();
    void sub_rm8_r8();
    void sub_rm32_r32();
    void sub_r8_rm8();
    void sub_r32_rm32();
    void sub_al_imm8();
    void sub_eax_imm32();
    void xor_rm8_r8();
    void xor_rm32_r32();
    void xor_r8_rm8();
    void xor_r32_rm32();
    void xor_al_imm8();
    void xor_eax_imm32();
    void cmp_rm8_r8();
    void cmp_rm32_r32();
    void cmp_r8_rm8();
    void cmp_r32_rm32();
    void cmp_al_imm8();
    void cmp_eax_imm32();
    void inc_r32();
    void dec_r32();
    void push_r32();
    void pop_r32();
    void pushad();
    void popad();
    void push_imm32();
    void imul_r32_rm32_imm32();
    void push_imm8();
    void imul_r32_rm32_imm8();
    void jo_rel8();
    void jno_rel8();
    void jb_rel8();
    void jnb_rel8();
    void jz_rel8();
    void jnz_rel8();
    void jbe_rel8();
    void ja_rel8();
    void js_rel8();
    void jns_rel8();
    void jp_rel8();
    void jnp_rel8();
    void jl_rel8();
    void jnl_rel8();
    void jle_rel8();
    void jnle_rel8();
    void code_80();
    void code_81();
    void code_82();
    void code_83();
    void test_rm8_r8();
    void test_rm32_r32();
    void xchg_r8_rm8();
    void xchg_r32_rm32();
    void mov_rm8_r8();
    void mov_rm32_r32();
    void mov_r8_rm8();
    void mov_r32_rm32();
    void mov_rm32_sreg();
    void lea_r32_m32();
    void mov_sreg_rm16();
    void nop();
    void xchg_r32_eax();
    void cwde();
    void cdq();
    void callf_ptr16_32();
    void pushf();
    void popf();
    void mov_al_moffs8();
    void mov_eax_moffs32();
    void mov_moffs8_al();
    void mov_moffs32_eax();
    void cmps_m8_m8();
    void cmps_m32_m32();
    void test_al_imm8();
    void test_eax_imm32();
    void mov_r8_imm8();
    void mov_r32_imm32();
    void code_c0();
    void code_c1();
    void ret();
    void mov_rm8_imm8();
    void mov_rm32_imm32();
    void leave();
    void retf();
    void int3();
    void int_imm8();
    void iret();
    void code_d3();
    void in_al_imm8();
    void in_eax_imm8();
    void out_imm8_al();
    void out_imm8_eax();
    void call_rel32();
    void jmpf_ptr16_32();
    void jmp();
    void jmp_rel32();
    void in_al_dx();
    void in_eax_dx();
    void out_dx_al();
    void out_dx_eax();
    void hlt();
    void code_f6();
    void code_f7();
    void cli();
    void sti();
    void cld();
    void std();
    void code_ff();
    void code_0f00();
    void code_0f01();
    void mov_r32_crn();
    void mov_crn_r32();
    void jo_rel32();
    void jno_rel32();
    void jb_rel32();
    void jnb_rel32();
    void jz_rel32();
    void jnz_rel32();
    void jbe_rel32();
    void ja_rel32();
    void js_rel32();
    void jns_rel32();
    void jp_rel32();
    void jnp_rel32();
    void jl_rel32();
    void jnl_rel32();
    void jle_rel32();
    void jnle_rel32();
    void seto_rm8();
    void setno_rm8();
    void setb_rm8();
    void setnb_rm8();
    void setz_rm8();
    void setnz_rm8();
    void setbe_rm8();
    void seta_rm8();
    void sets_rm8();
    void setns_rm8();
    void setp_rm8();
    void setnp_rm8();
    void setl_rm8();
    void setnl_rm8();
    void setle_rm8();
    void setnle_rm8();
    void imul_r32_rm32();
    void movzx_r32_rm8();
    void movzx_r32_rm16();
    void movsx_r32_rm8();
    void movsx_r32_rm16();

    void add_rm8_imm8();
    void or_rm8_imm8();
    void adc_rm8_imm8();
    void sbb_rm8_imm8();
    void and_rm8_imm8();
    void sub_rm8_imm8();
    void xor_rm8_imm8();
    void cmp_rm8_imm8();

    void shl_rm8_imm8();
    void shr_rm8_imm8();
    void sal_rm8_imm8();
    void sar_rm8_imm8();

    void test_rm8_imm8();
    void not_rm8();
    void neg_rm8();
    void mul_ax_al_rm8();
    void imul_ax_al_rm8();
    void div_al_ah_rm8();
    void idiv_al_ah_rm8();

    void add_rm32_imm32();
    void or_rm32_imm32();
    void adc_rm32_imm32();
    void sbb_rm32_imm32();
    void and_rm32_imm32();
    void sub_rm32_imm32();
    void xor_rm32_imm32();
    void cmp_rm32_imm32();

    void add_rm32_imm8();
    void or_rm32_imm8();
    void adc_rm32_imm8();
    void sbb_rm32_imm8();
    void and_rm32_imm8();
    void sub_rm32_imm8();
    void xor_rm32_imm8();
    void cmp_rm32_imm8();

    void shl_rm32_imm8();
    void shl_rm32_cl();
    void shr_rm32_imm8();
    void shr_rm32_cl();
    void sal_rm32_imm8();
    void sal_rm32_cl();
    void sar_rm32_imm8();
    void sar_rm32_cl();

    void test_rm32_imm32();
    void not_rm32();
    void neg_rm32();
    void mul_edx_eax_rm32();
    void imul_edx_eax_rm32();
    void div_edx_eax_rm32();
    void idiv_edx_eax_rm32();

    void inc_rm32();
    void dec_rm32();
    void call_rm32();
    void callf_m16_32();
    void jmp_rm32();
    void jmpf_m16_32();
    void push_rm32();

};

#endif
