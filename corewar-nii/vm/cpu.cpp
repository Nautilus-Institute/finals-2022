#include <string.h>
#include <iostream>
#include <unistd.h>
#include "cpu.hpp"

#define EFLAGS_CF	is_carry()
#define EFLAGS_PF	is_parity()
#define EFLAGS_ZF	is_zero()
#define EFLAGS_SF	is_sign()
#define EFLAGS_OF	is_overflow()
#define EFLAGS_DF	is_direction()

CPU::CPU(VM* vm)
    : m_vm(vm)
{

}


//
// Memory accessing
//

uint32_t CPU::get_data32(uint32_t addr)
{
    uint8_t data[4] = {0};
    data[0] = *(uint32_t*)&m_vm->arena[addr % CORE_SIZE];
    data[1] = *(uint32_t*)&m_vm->arena[(addr + 1) % CORE_SIZE];
    data[2] = *(uint32_t*)&m_vm->arena[(addr + 2) % CORE_SIZE];
    data[3] = *(uint32_t*)&m_vm->arena[(addr + 3) % CORE_SIZE];
    return *(uint32_t*)data;
}

uint16_t CPU::get_data16(uint32_t addr)
{
    uint8_t data[2] = {0};
    data[0] = *(uint32_t*)&m_vm->arena[addr % CORE_SIZE];
    data[1] = *(uint32_t*)&m_vm->arena[(addr + 1) % CORE_SIZE];
    return *(uint16_t*)data;
}

uint8_t CPU::get_data8(uint32_t addr)
{
    return m_vm->arena[addr % CORE_SIZE];
}

void CPU::put_data32(uint32_t addr, uint32_t v)
{
    uint8_t data[4];
    *(uint32_t*)data = v;
    m_vm->arena[addr % CORE_SIZE] = data[0];
    m_vm->arena[(addr + 1) % CORE_SIZE] = data[1];
    m_vm->arena[(addr + 2) % CORE_SIZE] = data[2];
    m_vm->arena[(addr + 3) % CORE_SIZE] = data[3];
}

void CPU::put_data16(uint32_t addr, uint16_t v)
{
    uint8_t data[2];
    *(uint32_t*)data = v;
    m_vm->arena[addr % CORE_SIZE] = data[0];
    m_vm->arena[(addr + 1) % CORE_SIZE] = data[1];
}

void CPU::put_data8(uint32_t addr, uint8_t v)
{
    m_vm->arena[addr % CORE_SIZE] = v;
}

//
// Execution
//

bool CPU::exec(InstrData* instr_)
{
    instr = instr_;
    switch (instr->opcode) {
        case 0x00:
            add_rm8_r8(); break;
        case 0x01:
            add_rm32_r32(); break;
        case 0x02:
            add_r8_rm8(); break;
        case 0x03:
            add_r32_rm32(); break;
        case 0x04:
            add_al_imm8(); break;
        case 0x05:
            add_eax_imm32(); break;
        case 0x06:
            push_es(); break;
        case 0x07:
            pop_es(); break;
        case 0x08:
            or_rm8_r8(); break;
        case 0x09:
            or_rm32_r32(); break;
        case 0x0a:
            or_r8_rm8(); break;
        case 0x0b:
            or_r32_rm32(); break;
        case 0x0c:
            or_al_imm8(); break;
        case 0x0d:
            or_eax_imm32(); break;
        case 0x16:
            push_ss(); break;
        case 0x17:
            pop_ss(); break;
        case 0x1e:
            push_ds(); break;
        case 0x1f:
            pop_ds(); break;
        case 0x20:
            and_rm8_r8(); break;
        case 0x21:
            and_rm32_r32(); break;
        case 0x22:
            and_r8_rm8(); break;
        case 0x23:
            and_r32_rm32(); break;
        case 0x24:
            and_al_imm8(); break;
        case 0x25:
            and_eax_imm32(); break;
        case 0x28:
            sub_rm8_r8(); break;
        case 0x29:
            sub_rm32_r32(); break;
        case 0x2a:
            sub_r8_rm8(); break;
        case 0x2b:
            sub_r32_rm32(); break;
        case 0x2c:
            sub_al_imm8(); break;
        case 0x2d:
            sub_eax_imm32(); break;
        case 0x30:
            xor_rm8_r8(); break;
        case 0x31:
            xor_rm32_r32(); break;
        case 0x32:
            xor_r8_rm8(); break;
        case 0x33:
            xor_r32_rm32(); break;
        case 0x34:
            xor_al_imm8(); break;
        case 0x35:
            xor_eax_imm32(); break;
        case 0x38:
            cmp_rm8_r8(); break;
        case 0x39:
            cmp_rm32_r32(); break;
        case 0x3a:
            cmp_r8_rm8(); break;
        case 0x3b:
            cmp_r32_rm32(); break;
        case 0x3c:
            cmp_al_imm8(); break;
        case 0x3d:
            cmp_eax_imm32(); break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            inc_r32(); break;
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4e:
        case 0x4f:
            dec_r32(); break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            push_r32(); break;
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5e:
        case 0x5f:
            pop_r32(); break;
        case 0x60:
            pushad(); break;
        case 0x61:
            popad(); break;
        case 0x68:
            push_imm32(); break;
        case 0x69:
            imul_r32_rm32_imm32(); break;
        case 0x6a:
            push_imm8(); break;
        case 0x6b:
            imul_r32_rm32_imm8(); break;
        case 0x70:
            jo_rel8(); break;
        case 0x71:
            jno_rel8(); break;
        case 0x72:
            jb_rel8(); break;
        case 0x73:
            jnb_rel8(); break;
        case 0x74:
            jz_rel8(); break;
        case 0x75:
            jnz_rel8(); break;
        case 0x76:
            jbe_rel8(); break;
        case 0x77:
            ja_rel8(); break;
        case 0x78:
            js_rel8(); break;
        case 0x79:
            jns_rel8(); break;
        case 0x7a:
            jp_rel8(); break;
        case 0x7b:
            jnp_rel8(); break;
        case 0x7c:
            jl_rel8(); break;
        case 0x7d:
            jnl_rel8(); break;
        case 0x7e:
            jle_rel8(); break;
        case 0x7f:
            jnle_rel8(); break;
        case 0x80:
            code_80(); break;
        case 0x81:
            code_81(); break;
        case 0x82:
            code_82(); break;
        case 0x83:
            code_83(); break;
        case 0x84:
            test_rm8_r8(); break;
        case 0x85:
            test_rm32_r32(); break;
        case 0x86:
            xchg_r8_rm8(); break;
        case 0x87:
            xchg_r32_rm32(); break;
        case 0x88:
            mov_rm8_r8(); break;
        case 0x89:
            mov_rm32_r32(); break;
        case 0x8a:
            mov_r8_rm8(); break;
        case 0x8b:
            mov_r32_rm32(); break;
        case 0x8c:
            mov_rm32_sreg(); break;
        case 0x8d:
            lea_r32_m32(); break;
        case 0x8e:
            mov_sreg_rm16(); break;
        case 0x90:
            nop(); break;
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
            xchg_r32_eax(); break;
        case 0x98:
            cwde(); break;
        case 0x99:
            cdq(); break;
        case 0x9a:
            callf_ptr16_32(); break;
        case 0x9c:
            pushf(); break;
        case 0x9d:
            popf(); break;
        case 0xa0:
            mov_al_moffs8(); break;
        case 0xa1:
            mov_eax_moffs32(); break;
        case 0xa2:
            mov_moffs8_al(); break;
        case 0xa3:
            mov_moffs32_eax(); break;
        case 0xa6:
            cmps_m8_m8(); break;
        case 0xa7:
            cmps_m32_m32(); break;
        case 0xa8:
            test_al_imm8(); break;
        case 0xa9:
            test_eax_imm32(); break;
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb6:
        case 0xb7:
            mov_r8_imm8(); break;
        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf:
            mov_r32_imm32(); break;
        case 0xc0:
            code_c0(); break;
        case 0xc1:
            code_c1(); break;
        case 0xc3:
            ret(); break;
        case 0xc6:
            mov_rm8_imm8(); break;
        case 0xc7:
            mov_rm32_imm32(); break;
        case 0xc9:
            leave(); break;
        case 0xcb:
            retf(); break;
        case 0xcc:
            int3(); break;
        case 0xcd:
            int_imm8(); break;
        case 0xcf:
            iret(); break;
        case 0xd3:
            code_d3(); break;
        case 0xe4:
            in_al_imm8(); break;
        case 0xe5:
            in_eax_imm8(); break;
        case 0xe6:
            out_imm8_al(); break;
        case 0xe7:
            out_imm8_eax(); break;
        case 0xe8:
            call_rel32(); break;
        case 0xea:
            jmpf_ptr16_32(); break;
        case 0xeb:
            jmp(); break;
        case 0xec:
            in_al_dx(); break;
        case 0xed:
            in_eax_dx(); break;
        case 0xee:
            out_dx_al(); break;
        case 0xef:
            out_dx_eax(); break;
        case 0xf4:
            hlt(); break;
        case 0xf6:
            code_f6(); break;
        case 0xf7:
            code_f7(); break;
        case 0xfa:
            cli(); break;
        case 0xfb:
            sti(); break;
        case 0xfc:
            cld(); break;
        case 0xfd:
            std(); break;
        case 0xff:
            code_ff(); break;
        case 0xf20:
            mov_r32_crn(); break;
        case 0xf22:
            mov_crn_r32(); break;
        case 0xf80:
            jo_rel32(); break;
        case 0xf81:
            jno_rel32(); break;
        case 0xf82:
            jb_rel32(); break;
        case 0xf83:
            jnb_rel32(); break;
        case 0xf84:
            jz_rel32(); break;
        case 0xf85:
            jnz_rel32(); break;
        case 0xf86:
            jbe_rel32(); break;
        case 0xf87:
            ja_rel32(); break;
        case 0xf88:
            js_rel32(); break;
        case 0xf89:
            jns_rel32(); break;
        case 0xf8a:
            jp_rel32(); break;
        case 0xf8b:
            jnp_rel32(); break;
        case 0xf8c:
            jl_rel32(); break;
        case 0xf8d:
            jnl_rel32(); break;
        case 0xf8e:
            jle_rel32(); break;
        case 0xf8f:
            jnle_rel32(); break;
        case 0xf90:
            seto_rm8(); break;
        case 0xf91:
            setno_rm8(); break;
        case 0xf92:
            setb_rm8(); break;
        case 0xf93:
            setnb_rm8(); break;
        case 0xf94:
            setz_rm8(); break;
        case 0xf95:
            setnz_rm8(); break;
        case 0xf96:
            setbe_rm8(); break;
        case 0xf97:
            seta_rm8(); break;
        case 0xf98:
            sets_rm8(); break;
        case 0xf99:
            setns_rm8(); break;
        case 0xf9a:
            setp_rm8(); break;
        case 0xf9b:
            setnp_rm8(); break;
        case 0xf9c:
            setl_rm8(); break;
        case 0xf9d:
            setnl_rm8(); break;
        case 0xf9e:
            setle_rm8(); break;
        case 0xf9f:
            setnle_rm8(); break;
        case 0xfaf:
            imul_r32_rm32(); break;
        case 0xfb6:
            movzx_r32_rm8(); break;
        case 0xfb7:
            movzx_r32_rm16(); break;
        case 0xfbe:
            movsx_r32_rm8(); break;
        case 0xfbf:
            movsx_r32_rm16(); break;
    }
    return true;
}

void CPU::add_rm8_r8()
{
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	set_rm8(rm8+r8);
	update_eflags_add(rm8, r8);
}

void CPU::add_r8_rm8()
{
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(r8+rm8);
	update_eflags_add(r8, rm8);
}

void CPU::add_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	set_reg(AL, al + instr->imm8);
	update_eflags_add(al, instr->imm8);
}

void CPU::or_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	set_rm8(rm8|r8);
	update_eflags_or(rm8, r8);
}

void CPU::or_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	set_reg(AL, al | instr->imm8);
	update_eflags_or(al, instr->imm8);
}

void CPU::or_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(r8 | rm8);
	update_eflags_or(r8, rm8);
}

void CPU::and_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	set_rm8(rm8 & r8);
	update_eflags_and(rm8, r8);
}

void CPU::and_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(r8&rm8);
	update_eflags_and(r8, rm8);
}

void CPU::and_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	set_reg(AL, al & instr->imm8);
	update_eflags_and(al, instr->imm8);
}

void CPU::sub_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	set_rm8(rm8 - r8);
	update_eflags_sub(rm8, r8);
}

void CPU::sub_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(r8-rm8);
	update_eflags_sub(r8, rm8);
}

void CPU::sub_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	set_reg(AL, al - instr->imm8);
	update_eflags_sub(al, instr->imm8);
}

void CPU::xor_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	set_rm8(rm8 ^ r8);
}

void CPU::xor_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(r8 ^ rm8);
}

void CPU::xor_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	set_reg(AL, al ^ instr->imm8);
}

void CPU::cmp_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	update_eflags_sub(rm8, r8);
}

void CPU::cmp_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	update_eflags_sub(r8, rm8);
}

void CPU::cmp_al_imm8(){
	uint8_t al;

	al = get_reg(AL);
	update_eflags_sub(al, instr->imm8);
}

#define JCC_REL8(cc, is_flag) \
void CPU::j ## cc ## _rel8(){ \
	if(is_flag) \
		update_eip(instr->imm8); \
}

JCC_REL8(o, EFLAGS_OF)
JCC_REL8(no, !EFLAGS_OF)
JCC_REL8(b, EFLAGS_CF)
JCC_REL8(nb, !EFLAGS_CF)
JCC_REL8(z, EFLAGS_ZF)
JCC_REL8(nz, !EFLAGS_ZF)
JCC_REL8(be, EFLAGS_CF || EFLAGS_ZF)
JCC_REL8(a, !(EFLAGS_CF || EFLAGS_ZF))
JCC_REL8(s, EFLAGS_SF)
JCC_REL8(ns, !EFLAGS_SF)
JCC_REL8(p, EFLAGS_PF)
JCC_REL8(np, !EFLAGS_PF)
JCC_REL8(l, EFLAGS_SF != EFLAGS_OF)
JCC_REL8(nl, EFLAGS_SF == EFLAGS_OF)
JCC_REL8(le, EFLAGS_ZF || (EFLAGS_SF != EFLAGS_OF))
JCC_REL8(nle, !EFLAGS_ZF && (EFLAGS_SF == EFLAGS_OF))

void CPU::test_rm8_r8(){
	uint8_t rm8, r8;

	rm8 = get_rm8();
	r8 = get_r8();
	update_eflags_and(rm8, r8);
}

void CPU::xchg_r8_rm8(){
	uint8_t r8, rm8;

	r8 = get_r8();
	rm8 = get_rm8();
	set_r8(rm8);
	set_rm8(r8);
}

void CPU::mov_rm8_r8(){
	uint8_t r8;

	r8 = get_r8();
	set_rm8(r8);
}

void CPU::mov_r8_rm8(){
	uint8_t rm8;

	rm8 = get_rm8();
	set_r8(rm8);
}

void CPU::mov_sreg_rm16()
{
    // Do not support sections
}

void CPU::nop()
{

}

void CPU::mov_al_moffs8()
{
	set_reg(AL, get_moffs8());
}

void CPU::mov_moffs8_al()
{
	set_moffs8(get_reg(AL));
}

void CPU::test_al_imm8()
{
	uint8_t al;

	al = get_reg(AL);
	update_eflags_and(al, instr->imm8);
}

void CPU::mov_r8_imm8()
{
	uint8_t reg;

	reg = OPCODE & ((1<<3)-1);
	set_reg(static_cast<reg8_t>(reg), instr->imm8);
}

void CPU::mov_rm8_imm8()
{
	set_rm8(instr->imm8);
}

void CPU::retf()
{
    // Do not support far-rets
}

void CPU::int3()
{
    // Dead dead dead
    halt(true);
}

void CPU::int_imm8()
{
    if (instr->imm8 == 0x80) {
        // syscall
#ifdef DEBUG
        std::cerr << "syscall" << std::endl;
#endif
        uint32_t eax = get_reg(EAX);
        if (eax == 0) {
            // exit
            halt(true);
        } else if (eax == 1) {
            // read
            uint64_t ecx = get_reg(ECX);
            uint64_t edx = get_reg(EDX);
            uint64_t ebx = get_reg(EBX);
            uint64_t target = (ecx << 32) | edx;
            ::read(0, (void*)target, ebx);
        } else if (eax == 2) {
            // write
            uint64_t ecx = get_reg(ECX);
            uint64_t edx = get_reg(EDX);
            uint64_t ebx = get_reg(EBX);
            uint64_t target = (ecx << 32) | edx;
            ::write(0, (void*)target, ebx);
        }
    }
}

void CPU::iret(){
    // Do not support irets
}

void CPU::in_al_imm8()
{
    // TODO: Do we want to support in?
	// set_reg(AL, EMU->in_io8((uint8_t)instr->imm8));
}

void CPU::out_imm8_al()
{
    // TODO: Do we want to support out?
	//uint8_t al;

	// al = get_reg(AL);
	// EMU->out_io8((uint8_t)instr->imm8, al);
}

void CPU::jmp()
{
	registers[EIP].reg32 += (int32_t)((int8_t)instr->imm8);
}

void CPU::in_al_dx()
{
    // TODO: Do we want to support in?
	//uint16_t dx;

	// dx = get_reg(DX);
	// set_reg(AL, EMU->in_io8(dx));
}

void CPU::out_dx_al()
{
    // TODO: Do we want to support out?
	//uint16_t dx;
	//uint8_t al;

	// dx = get_reg(DX);
	// al = get_reg(AL);
	// EMU->out_io8(dx, al);
}

void CPU::cli()
{
    // Do not support cli
}

void CPU::sti()
{
    // Do not support sti
}

void CPU::cld()
{
    // Do not support cld
}

void CPU::std()
{
    // Do not support std
}

void CPU::hlt()
{
	halt(true);
}

void CPU::mov_r32_crn()
{
    // Do not support crN registers
}

void CPU::mov_crn_r32()
{
    // Do not support crN registers
}

#define SETCC_RM8(cc, is_flag) \
void CPU::set ## cc ## _rm8(){ \
	set_reg(static_cast<reg32_t>(instr->modrm.rm), is_flag); \
}

SETCC_RM8(o, EFLAGS_OF)
SETCC_RM8(no, !EFLAGS_OF)
SETCC_RM8(b, EFLAGS_CF)
SETCC_RM8(nb, !EFLAGS_CF)
SETCC_RM8(z, EFLAGS_ZF)
SETCC_RM8(nz, !EFLAGS_ZF)
SETCC_RM8(be, EFLAGS_CF || EFLAGS_ZF)
SETCC_RM8(a, !(EFLAGS_CF || EFLAGS_ZF))
SETCC_RM8(s, EFLAGS_SF)
SETCC_RM8(ns, !EFLAGS_SF)
SETCC_RM8(p, EFLAGS_PF)
SETCC_RM8(np, !EFLAGS_PF)
SETCC_RM8(l, EFLAGS_SF != EFLAGS_OF)
SETCC_RM8(nl, EFLAGS_SF == EFLAGS_OF)
SETCC_RM8(le, EFLAGS_ZF || (EFLAGS_SF != EFLAGS_OF))
SETCC_RM8(nle, !EFLAGS_ZF && (EFLAGS_SF == EFLAGS_OF))

void CPU::code_80()
{
	switch(REG) {
		case 0:
            add_rm8_imm8(); break;
		case 1:
            or_rm8_imm8(); break;
		case 2:
            adc_rm8_imm8(); break;
		case 3:
            sbb_rm8_imm8(); break;
		case 4:
            and_rm8_imm8(); break;
		case 5:
            sub_rm8_imm8(); break;
		case 6:
            xor_rm8_imm8(); break;
		case 7:
            cmp_rm8_imm8(); break;
		default:
            std::cerr << "unsupported 0x80 /" << REG << std::endl;
            break;
	}
}

void CPU::code_82()
{
	code_80();
}

void CPU::code_c0()
{
	switch (REG) {
		case 4:
            shl_rm8_imm8(); break;
		case 5:
            shr_rm8_imm8(); break;
		case 6:
            sal_rm8_imm8(); break;
		case 7:
            sar_rm8_imm8(); break;
		default:
            std::cerr << "unsupported 0xc0 /" << REG << std::endl;
	}
}

void CPU::code_f6()
{
	switch (REG) {
		case 0:
            test_rm8_imm8();	break;
		case 2:
            not_rm8();		break;
		case 3:
            neg_rm8();		break;
		case 4:
            mul_ax_al_rm8();	break;
		case 5:
            imul_ax_al_rm8();	break;
		case 6:
            div_al_ah_rm8();	break;
		case 7:
            idiv_al_ah_rm8();	break;
		default:
                std::cerr << "not implemented: 0xf6 /" << REG << std::endl;
	}
}

void CPU::add_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 + instr->imm8);
	update_eflags_add(rm8, instr->imm8);
}

void CPU::or_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8|instr->imm8);
	update_eflags_or(rm8, instr->imm8);
}

void CPU::adc_rm8_imm8()
{
	uint8_t rm8, cf;

	rm8 = get_rm8();
	cf = EFLAGS_CF;
	set_rm8(rm8 + instr->imm8 + cf);
	update_eflags_add(rm8, instr->imm8 + cf);
}

void CPU::sbb_rm8_imm8()
{
	uint8_t rm8, cf;
    uint8_t result;

	rm8 = get_rm8();
	cf = EFLAGS_CF;
    result = rm8 - instr->imm8 - cf;
	set_rm8(result);
	update_eflags_sub(rm8, instr->imm8 + cf);
}

void CPU::and_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 & instr->imm8);
	update_eflags_and(rm8, instr->imm8);
}

void CPU::sub_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 - instr->imm8);
	update_eflags_sub(rm8, instr->imm8);
}

void CPU::xor_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 ^ instr->imm8);
}

void CPU::cmp_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	update_eflags_sub(rm8, instr->imm8);
}

void CPU::shl_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 << instr->imm8);
	update_eflags_shl(rm8, instr->imm8);
}

void CPU::shr_rm8_imm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(rm8 >> instr->imm8);
	update_eflags_shr(rm8, instr->imm8);
}

void CPU::sal_rm8_imm8()
{
	int8_t rm8_s;

	rm8_s = get_rm8();
	set_rm8(rm8_s << instr->imm8);
}

void CPU::sar_rm8_imm8()
{
	int8_t rm8_s;

	rm8_s = get_rm8();
	set_rm8(rm8_s >> instr->imm8);
}

void CPU::test_rm8_imm8()
{
	uint8_t rm8, imm8;

	rm8 = get_rm8();
	imm8 = get_code8();
	update_eip(1);
	update_eflags_and(rm8, imm8);
}

void CPU::not_rm8()
{
	uint8_t rm8;

	rm8 = get_rm8();
	set_rm8(~rm8);
}

void CPU::neg_rm8()
{
	int8_t rm8_s;

	rm8_s = get_rm8();
	set_rm8(-rm8_s);
	update_eflags_sub((uint8_t)0, rm8_s);
}

void CPU::mul_ax_al_rm8()
{
	uint8_t rm8, al;
	uint16_t val;

	rm8 = get_rm8();
	al = get_reg(AL);
	val = al*rm8;

	set_reg(AX, val);

	update_eflags_mul(al, rm8);
}

void CPU::imul_ax_al_rm8()
{
	int8_t rm8_s, al_s;
	int16_t val_s;

	rm8_s = get_rm8();
	al_s = get_reg(AL);
	val_s = al_s*rm8_s;

	set_reg(AX, val_s);

	update_eflags_imul(al_s, rm8_s);
}

void CPU::div_al_ah_rm8(){
	uint8_t rm8;
	uint16_t ax;

	rm8 = get_rm8();
	ax = get_reg(AX);

	set_reg(AL, ax/rm8);
	set_reg(AH, ax%rm8);
}

void CPU::idiv_al_ah_rm8(){
	int8_t rm8_s;
	int16_t ax_s;

	rm8_s = get_rm8();
	ax_s = get_reg(AX);

	set_reg(AL, ax_s/rm8_s);
	set_reg(AH, ax_s%rm8_s);
}

void CPU::add_rm32_r32(){
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	set_rm32(rm32+r32);
	update_eflags_add(rm32, r32);
}

void CPU::add_r32_rm32(){
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(r32+rm32);
	update_eflags_add(r32, rm32);
}

void CPU::add_eax_imm32(){
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EAX, eax+instr->imm32);
	update_eflags_add(eax, instr->imm32);
}

void CPU::push_es(){
    // We don't support segments
}

void CPU::pop_es(){
    // We don't support segments
}

void CPU::or_rm32_r32(){
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	set_rm32(rm32|r32);
	update_eflags_or(rm32, r32);
}

void CPU::or_r32_rm32(){
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(r32|rm32);
	update_eflags_or(r32, rm32);
}

void CPU::or_eax_imm32(){
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EAX, eax|instr->imm32);
	update_eflags_or(eax, instr->imm32);
}

void CPU::push_ss(){
    // We don't support segments
}

void CPU::pop_ss(){
    // We don't support segments
}

void CPU::push_ds(){
    // We don't support segments
}

void CPU::pop_ds(){
    // We don't support segments
}

void CPU::and_rm32_r32()
{
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	set_rm32(rm32&r32);
	update_eflags_and(rm32, r32);
}

void CPU::and_r32_rm32()
{
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(r32&rm32);
	update_eflags_and(r32, rm32);
}

void CPU::and_eax_imm32()
{
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EAX, eax&instr->imm32);
	update_eflags_and(eax, instr->imm32);
}

void CPU::sub_rm32_r32()
{
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	set_rm32(rm32-r32);
	update_eflags_sub(rm32, r32);
}

void CPU::sub_r32_rm32()
{
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(r32-rm32);
	update_eflags_sub(r32, rm32);
}

void CPU::sub_eax_imm32()
{
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EAX, eax-instr->imm32);
	update_eflags_sub(eax, instr->imm32);
}

void CPU::xor_rm32_r32()
{
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	set_rm32(rm32^r32);
}

void CPU::xor_r32_rm32(){
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(r32^rm32);
}

void CPU::xor_eax_imm32(){
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EAX, eax^instr->imm32);
}

void CPU::cmp_rm32_r32(){
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	update_eflags_sub(rm32, r32);
}

void CPU::cmp_r32_rm32(){
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	update_eflags_sub(r32, rm32);
}

void CPU::cmp_eax_imm32(){
	uint32_t eax;

	eax = get_reg(EAX);
	update_eflags_sub(eax, instr->imm32);
}

void CPU::inc_r32(){
	uint8_t reg;
	uint32_t r32;

	reg = OPCODE & ((1<<3)-1);
	r32 = get_reg(static_cast<reg32_t>(reg));
	set_reg(static_cast<reg32_t>(reg), r32+1);
	update_eflags_add(r32, 1);
}

void CPU::dec_r32(){
	uint8_t reg;
	uint32_t r32;

	reg = OPCODE & ((1<<3)-1);
	r32 = get_reg(static_cast<reg32_t>(reg));
	set_reg(static_cast<reg32_t>(reg), r32-1);
	update_eflags_sub(r32, 1);
#ifdef DEBUG
    fprintf(stderr, "ZFLAG %d\n", eflags.ZF);
    fprintf(stderr, "eax %d\n", get_reg(EAX));
#endif
}

void CPU::push_r32(){
	uint8_t reg;

	reg = OPCODE & ((1<<3)-1);
	push32(get_reg(static_cast<reg32_t>(reg)));
}

void CPU::pop_r32(){
	uint8_t reg;

	reg = OPCODE & ((1<<3)-1);
	set_reg(static_cast<reg32_t>(reg), pop32());
}

void CPU::pushad(){
	uint32_t esp;
	
	esp = get_reg(ESP);

	push32(get_reg(EAX));
	push32(get_reg(ECX));
	push32(get_reg(EDX));
	push32(get_reg(EBX));
	push32(esp);
	push32(get_reg(EBP));
	push32(get_reg(ESI));
	push32(get_reg(EDI));
}

void CPU::popad(){
	uint32_t esp;

	set_reg(EDI, pop32());
	set_reg(ESI, pop32());
	set_reg(EBP, pop32());
	esp = pop32();
	set_reg(EBX, pop32());
	set_reg(EDX, pop32());
	set_reg(ECX, pop32());
	set_reg(EAX, pop32());

	set_reg(ESP, esp);
}

void CPU::push_imm32(){
	push32(instr->imm32);
}

void CPU::imul_r32_rm32_imm32(){
	int32_t rm32_s;

	rm32_s = get_rm32();
	set_r32(rm32_s*instr->imm32);
	update_eflags_imul(rm32_s, instr->imm32);
}

void CPU::push_imm8(){
	push32(instr->imm8);
}

void CPU::imul_r32_rm32_imm8(){
	int32_t rm32_s;

	rm32_s = get_rm32();
	set_r32(rm32_s*instr->imm8);
	update_eflags_imul(rm32_s, instr->imm8);
}

void CPU::test_rm32_r32(){
	uint32_t rm32, r32;

	rm32 = get_rm32();
	r32 = get_r32();
	update_eflags_and(rm32, r32);
}

void CPU::xchg_r32_rm32(){
	uint32_t r32, rm32;

	r32 = get_r32();
	rm32 = get_rm32();
	set_r32(rm32);
	set_rm32(r32);
}

void CPU::mov_rm32_r32(){
	uint32_t r32;

	r32 = get_r32();
	set_rm32(r32);
}

void CPU::mov_r32_rm32(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_r32(rm32);
}

void CPU::mov_rm32_sreg()
{
    // We do not support sections
}

void CPU::lea_r32_m32(){
	uint32_t m32;

	m32 = get_m();
	set_r32(m32);
}

void CPU::xchg_r32_eax(){
	uint32_t r32, eax;

	r32 = get_r32();
	eax = get_reg(EAX);
	set_r32(eax);
	set_reg(EAX, r32);
}

void CPU::cwde(){
	int16_t ax_s;

	ax_s = get_reg(AX);
	set_reg(EAX, ax_s);
}

void CPU::cdq(){
	uint32_t eax;

	eax = get_reg(EAX);
	set_reg(EDX, eax&(1<<31) ? -1 : 0);
}

void CPU::callf_ptr16_32()
{
    // We do not support far calls
}

void CPU::pushf()
{
	push32(get_eflags());
}

void CPU::popf()
{
	set_eflags(pop32());
}

void CPU::mov_eax_moffs32()
{
	set_reg(EAX, get_moffs32());
}

void CPU::mov_moffs32_eax(){
	set_moffs32(get_reg(EAX));
}

void CPU::cmps_m8_m8()
{
	uint8_t m8_s, m8_d;

repeat:
	m8_s = get_data8(get_reg(ESI));
	m8_d = get_data8(get_reg(EDI));
	update_eflags_sub(m8_s, m8_d);

	set_reg(ESI, EFLAGS_DF ? -1 : 1);
	set_reg(EDI, EFLAGS_DF ? -1 : 1);
	
	if(PRE_REPEAT){
		set_reg(ECX, -1);
		switch(PRE_REPEAT){
			case REPZ:
				if(!get_reg(ECX) || !EFLAGS_ZF)
					break;
				goto repeat;
			case REPNZ:
				if(!get_reg(ECX) || EFLAGS_ZF)
					break;
				goto repeat;
			default:
				break;
		}
	}
}

void CPU::cmps_m32_m32()
{
	uint32_t m32_s, m32_d;

repeat:
	m32_s = get_data32(get_reg(ESI));
	m32_d = get_data32(get_reg(EDI));
	update_eflags_sub(m32_s, m32_d);

	set_reg(ESI, EFLAGS_DF ? -1 : 1);
	set_reg(EDI, EFLAGS_DF ? -1 : 1);

	if(PRE_REPEAT){
		set_reg(ECX, -1);
		switch(PRE_REPEAT){
			case REPZ:
				if(!get_reg(ECX) || !EFLAGS_ZF)
					break;
				goto repeat;
			case REPNZ:
				if(!get_reg(ECX) || EFLAGS_ZF)
					break;
				goto repeat;
			default:
				break;
		}
	}
}

void CPU::test_eax_imm32()
{
	uint32_t eax;

	eax = get_reg(EAX);
	update_eflags_and(eax, instr->imm32);
}

void CPU::mov_r32_imm32()
{
	uint8_t reg;

	reg = OPCODE & ((1<<3)-1);
	set_reg(static_cast<reg32_t>(reg), instr->imm32);
}

void CPU::ret()
{
    set_eip(pop32());
}

void CPU::mov_rm32_imm32(){
	set_rm32(instr->imm32);
}

void CPU::leave(){
	uint32_t ebp;

	ebp = get_reg(EBP);
	set_reg(ESP, ebp);
	set_reg(EBP, pop32());
}

void CPU::in_eax_imm8()
{
    // TODO: Do we want to support in/out?
}

void CPU::out_imm8_eax()
{
    // TODO: Do we want to support in/out?
}

void CPU::call_rel32()
{
	push32(get_eip());
	update_eip(instr->imm32);
}

void CPU::jmp_rel32()
{
	update_eip(instr->imm32);
}

void CPU::jmpf_ptr16_32()
{
    // We do not support far jumps
}

void CPU::in_eax_dx()
{
    // TODO: Do we want to support in/out?
}

void CPU::out_dx_eax()
{
    // TODO: Do we want to support in/out?
}

#define JCC_REL32(cc, is_flag) \
void CPU::j ## cc ## _rel32(){ \
	if(is_flag) \
		update_eip(instr->imm32); \
}

JCC_REL32(o, EFLAGS_OF)
JCC_REL32(no, !EFLAGS_OF)
JCC_REL32(b, EFLAGS_CF)
JCC_REL32(nb, !EFLAGS_CF)
JCC_REL32(z, EFLAGS_ZF)
JCC_REL32(nz, !EFLAGS_ZF)
JCC_REL32(be, EFLAGS_CF || EFLAGS_ZF)
JCC_REL32(a, !(EFLAGS_CF || EFLAGS_ZF))
JCC_REL32(s, EFLAGS_SF)
JCC_REL32(ns, !EFLAGS_SF)
JCC_REL32(p, EFLAGS_PF)
JCC_REL32(np, !EFLAGS_PF)
JCC_REL32(l, EFLAGS_SF != EFLAGS_OF)
JCC_REL32(nl, EFLAGS_SF == EFLAGS_OF)
JCC_REL32(le, EFLAGS_ZF || (EFLAGS_SF != EFLAGS_OF))
JCC_REL32(nle, !EFLAGS_ZF && (EFLAGS_SF == EFLAGS_OF))

void CPU::imul_r32_rm32(){
	int16_t r32_s, rm32_s;

	r32_s = get_r32();
	rm32_s = get_rm32();
	set_r32(r32_s*rm32_s);
	update_eflags_imul(r32_s, rm32_s);
}

void CPU::movzx_r32_rm8(){
	uint8_t rm8;

	rm8 = get_rm8();
	set_r32(rm8);
}

void CPU::movzx_r32_rm16(){
	uint16_t rm16;

	rm16 = get_rm16();
	set_r32(rm16);
}

void CPU::movsx_r32_rm8(){
	int8_t rm8_s;

	rm8_s = get_rm8();
	set_r32(rm8_s);
}

void CPU::movsx_r32_rm16(){
	int16_t rm16_s;

	rm16_s = get_rm16();
	set_r32(rm16_s);
}

/******************************************************************/

void CPU::code_81()
{
	switch(REG){
		case 0:	add_rm32_imm32();	break;
		case 1:	or_rm32_imm32();	break;
		case 2:	adc_rm32_imm32();	break;
		case 3:	sbb_rm32_imm32();	break;
		case 4:	and_rm32_imm32();	break;
		case 5:	sub_rm32_imm32();	break;
		case 6:	xor_rm32_imm32();	break;
		case 7:	cmp_rm32_imm32();	break;
		default:
            std::cerr << "not implemented: 0x81 /" << REG << std::endl;
	}
}

void CPU::code_83()
{
	switch(REG){
		case 0:	add_rm32_imm8();	break;
		case 1:	or_rm32_imm8();		break;
		case 2:	adc_rm32_imm8();	break;
		case 3:	sbb_rm32_imm8();	break;
		case 4:	and_rm32_imm8();	break;
		case 5:	sub_rm32_imm8();	break;
		case 6:	xor_rm32_imm8();	break;
		case 7:	cmp_rm32_imm8();	break;
		default:
            std::cerr << "not implemented: 0x83 /" << REG << std::endl;
	}
}

void CPU::code_c1()
{
	switch(REG){
		case 4: shl_rm32_imm8();        break;
		case 5: shr_rm32_imm8();        break;
		case 6: sal_rm32_imm8();        break;	// ?
		case 7: sar_rm32_imm8();        break;
		default:
            std::cerr << "not implemented: 0xc1 /" << REG << std::endl;
	}
}

void CPU::code_d3()
{
	switch(REG) {
		case 4: shl_rm32_cl();        break;
		case 5: shr_rm32_cl();        break;
		case 6: sal_rm32_cl();        break;	// ?
		case 7: sar_rm32_cl();        break;
		default:
            std::cerr << "not implemented: 0xd3 /" << REG << std::endl;
	}
}

void CPU::code_f7()
{
	switch(REG){
		case 0:	test_rm32_imm32();	break;
		case 2:	not_rm32();		break;
		case 3:	neg_rm32();		break;
		case 4:	mul_edx_eax_rm32();	break;
		case 5:	imul_edx_eax_rm32();	break;
		case 6:	div_edx_eax_rm32();	break;
		case 7:	idiv_edx_eax_rm32();	break;
		default:
            std::cerr << "not implemented: 0xf7 /" << REG << std::endl;
	}
}

void CPU::code_ff()
{
	switch(REG) {
		case 0:	inc_rm32();		break;
		case 1:	dec_rm32();		break;
		case 2:	call_rm32();		break;
		case 3:	callf_m16_32();		break;
		case 4:	jmp_rm32();		break;
		case 5:	jmpf_m16_32();		break;
		case 6:	push_rm32();		break;
		default:
            std::cerr << "not implemented: 0xff /" << REG << std::endl;
	}
}


void CPU::add_rm32_imm32(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32+instr->imm32);
	update_eflags_add(rm32, instr->imm32);
}

void CPU::or_rm32_imm32(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32|instr->imm32);
	update_eflags_or(rm32, instr->imm32);
}

void CPU::adc_rm32_imm32(){
	uint32_t rm32;
	uint8_t cf;

	rm32 = get_rm32();
	cf = EFLAGS_CF;
	set_rm32(rm32+instr->imm32+cf);
	update_eflags_add(rm32, instr->imm32+cf);
}

void CPU::sbb_rm32_imm32(){
	uint32_t rm32;
	uint8_t cf;

	rm32 = get_rm32();
	cf = EFLAGS_CF;
	set_rm32(rm32-instr->imm32-cf);
	update_eflags_sub(rm32, instr->imm32+cf);
}

void CPU::and_rm32_imm32(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32&instr->imm32);
	update_eflags_and(rm32, instr->imm32);
}

void CPU::sub_rm32_imm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32-instr->imm32);
	update_eflags_sub(rm32, instr->imm32);
}

void CPU::xor_rm32_imm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32^instr->imm32);
}

void CPU::cmp_rm32_imm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	update_eflags_sub(rm32, instr->imm32);
}

void CPU::add_rm32_imm8()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32+instr->imm8);
	update_eflags_add(rm32, instr->imm8);
}

void CPU::or_rm32_imm8()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32|instr->imm8);
	update_eflags_or(rm32, instr->imm8);
}

void CPU::adc_rm32_imm8(){
	uint32_t rm32;
	uint8_t cf;

	rm32 = get_rm32();
	cf = EFLAGS_CF;
	set_rm32(rm32+instr->imm8+cf);
	update_eflags_add(rm32, instr->imm8+cf);
}

void CPU::sbb_rm32_imm8(){
	uint32_t rm32;
	uint8_t cf;

	rm32 = get_rm32();
	cf = EFLAGS_CF;
	set_rm32(rm32-instr->imm8-cf);
	update_eflags_sub(rm32, instr->imm8+cf);
}

void CPU::and_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32&instr->imm8);
	update_eflags_and(rm32, instr->imm8);
}

void CPU::sub_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32-instr->imm8);
	update_eflags_sub(rm32, instr->imm8);
}

void CPU::xor_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32^instr->imm8);
}

void CPU::cmp_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	update_eflags_sub(rm32, instr->imm8);
}

void CPU::shl_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32<<instr->imm8);
	update_eflags_shl(rm32, instr->imm8);
}

void CPU::shr_rm32_imm8(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32>>instr->imm8);
	update_eflags_shr(rm32, instr->imm8);
}

void CPU::sal_rm32_imm8(){
	int32_t rm32_s;

	rm32_s = get_rm32();
	set_rm32(rm32_s<<instr->imm8);
}

void CPU::sar_rm32_imm8(){
	int32_t rm32_s;

	rm32_s = get_rm32();
	set_rm32(rm32_s>>instr->imm8);
}

void CPU::shl_rm32_cl()
{
	uint32_t rm32;
	uint8_t cl;

	rm32 = get_rm32();
	cl = get_reg(CL);
	set_rm32(rm32<<cl);
	update_eflags_shl(rm32, cl);
}

void CPU::shr_rm32_cl(){
	uint32_t rm32;
	uint8_t cl;

	rm32 = get_rm32();
	cl = get_reg(CL);
	set_rm32(rm32>>cl);
	update_eflags_shr(rm32, cl);
}

void CPU::sal_rm32_cl(){
	int32_t rm32_s;
	uint8_t cl;

	rm32_s = get_rm32();
	cl = get_reg(CL);
	set_rm32(rm32_s<<cl);
}

void CPU::sar_rm32_cl(){
	int32_t rm32_s;
	uint8_t cl;

	rm32_s = get_rm32();
	cl = get_reg(CL);
	set_rm32(rm32_s>>cl);
}

void CPU::test_rm32_imm32()
{
	uint32_t rm32, imm32;

	rm32 = get_rm32();
	imm32 = get_code32();
	update_eip(4);
	update_eflags_and(rm32, imm32);
}

void CPU::not_rm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(~rm32);
}

void CPU::neg_rm32()
{
	int32_t rm32_s;

	rm32_s = get_rm32();
	set_rm32(-rm32_s);
	update_eflags_sub((uint32_t)0, rm32_s);
}

void CPU::mul_edx_eax_rm32()
{
	uint32_t rm32, eax;
	uint64_t val;

	rm32 = get_rm32();
	eax = get_reg(EAX);
	val = eax*rm32;

	set_reg(EAX, val);
	set_reg(EDX, val>>32);

	update_eflags_mul(eax, rm32);
}

void CPU::imul_edx_eax_rm32()
{
	int32_t rm32_s, eax_s;
	int64_t val_s;

	rm32_s = get_rm32();
	eax_s = get_reg(EAX);
	val_s = eax_s*rm32_s;

	set_reg(EAX, val_s);
	set_reg(EDX, val_s>>32);

	update_eflags_imul(eax_s, rm32_s);
}

void CPU::div_edx_eax_rm32()
{
	uint32_t rm32;
	uint64_t val;

	rm32 = get_rm32();
    if (rm32 == 0) {
        return;
    }
	val = get_reg(EDX);
	val <<= 32;
	val |= get_reg(EAX);

	set_reg(EAX, val/rm32);
	set_reg(EDX, val%rm32);
}

void CPU::idiv_edx_eax_rm32()
{
	int32_t rm32_s;
	int64_t val_s;

	rm32_s = get_rm32();
    if (rm32_s == 0) {
        return;
    }
	val_s = get_reg(EDX);
	val_s <<= 32;
	val_s |= get_reg(EAX);

	set_reg(EAX, val_s/rm32_s);
	set_reg(EDX, val_s%rm32_s);
}

void CPU::inc_rm32(){
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32+1);
	update_eflags_add(rm32, 1);
}

void CPU::dec_rm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_rm32(rm32-1);
	update_eflags_sub(rm32, 1);
}

void CPU::call_rm32()
{
	uint32_t rm32;

	rm32 = get_rm32();

	push32(get_eip());
	set_eip(rm32);
}

void CPU::callf_m16_32()
{
    // Do not support far calls
}

void CPU::jmp_rm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	set_eip(rm32);
}

void CPU::jmpf_m16_32()
{
    // Do not support far jumps
}

void CPU::push_rm32()
{
	uint32_t rm32;

	rm32 = get_rm32();
	push32(rm32);
}

void CPU::set_rm32(uint32_t value)
{
    if (MOD == 3) {
		set_reg(static_cast<reg32_t>(RM), value);
    }
    else {
		put_data32(calc_modrm(), value);
    }
}

uint32_t CPU::get_rm32()
{
	if (MOD == 3)
		return get_reg(static_cast<reg32_t>(RM));
	else
		return get_data32(calc_modrm());
}

void CPU::set_r32(uint32_t value)
{
	set_reg(static_cast<reg32_t>(REG), value);
}

uint32_t CPU::get_r32()
{
	return get_reg(static_cast<reg32_t>(REG));
}

void CPU::set_moffs32(uint32_t value)
{
	return put_data32(MOFFS, value);
}

uint32_t CPU::get_moffs32()
{
	return get_data32(MOFFS);
}

void CPU::set_rm16(uint16_t value)
{
	if(MOD == 3)
		set_reg(static_cast<reg16_t>(RM), value);
	else
		put_data16(calc_modrm(), value);
}

uint16_t CPU::get_rm16()
{
	if(MOD == 3)
		return get_reg(static_cast<reg16_t>(RM));
	else
		return get_data16(calc_modrm());
}

void CPU::set_r16(uint16_t value)
{
	set_reg(static_cast<reg16_t>(REG), value);
}

uint16_t CPU::get_r16()
{
	return get_reg(static_cast<reg16_t>(REG));
}

void CPU::set_moffs16(uint16_t value)
{
	return put_data16(MOFFS, value);
}

uint16_t CPU::get_moffs16()
{
	return get_data16(MOFFS);
}

void CPU::set_rm8(uint8_t value)
{
	if(MOD == 3)
		set_reg(static_cast<reg8_t>(RM), value);
	else 
		put_data8(calc_modrm(), value);
}

uint8_t CPU::get_rm8()
{
	if(MOD == 3)
		return get_reg(static_cast<reg8_t>(RM));
	else
		return get_data8(calc_modrm());
}

void CPU::set_r8(uint8_t value)
{
	set_reg(static_cast<reg8_t>(REG), value);
}

void CPU::set_moffs8(uint8_t value)
{
	return put_data8(MOFFS, value);
}

uint8_t CPU::get_moffs8()
{
	return get_data8(MOFFS);
}

uint8_t CPU::get_r8()
{
	return get_reg(static_cast<reg8_t>(REG));
}

uint32_t CPU::get_m()
{
	return calc_modrm();
}


uint32_t CPU::calc_modrm()
{
    return calc_modrm32();
}

uint32_t CPU::calc_modrm32()
{
	uint32_t addr = 0;

	switch(MOD){
		case 1:
			addr += DISP8;
			break;
		case 2:
			addr += DISP32;
			break;
	}

	switch(RM){
		case 4:
			addr += calc_sib();
			break;
		case 5:
			if(MOD == 0){
				addr += DISP32;
				break;
			}
		default:
			addr += get_reg(static_cast<reg32_t>(RM));
	}

	return addr;
}

uint32_t CPU::calc_sib()
{
	uint32_t base = -1;

    if(BASE == 5 && MOD == 0) {
		base = DISP32;
    }
	else if (BASE == 4) {
		if(SCALE == 0) {		// BASE == 4, INDEX ==4, SCALE == 0 : [esp]
			base = 0;
		}
        else {
			std::cerr << "not implemented SIB (base = " << BASE <<
                ", index = " << INDEX <<
                ", scale = " << SCALE << ")" << std::endl;
            return -1;
        }
	}
	else {
		base = get_reg(static_cast<reg32_t>(BASE));
	}

	return base + get_reg(static_cast<reg32_t>(INDEX)) * (1<<SCALE);
}

// eflags
template uint32_t CPU::update_eflags_add(uint32_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_add(uint16_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_add(uint8_t v1, uint32_t v2);
template <class T> uint32_t CPU::update_eflags_add(T v1, uint32_t v2){
	bool s1, s2, sr;
	uint64_t result;
	uint8_t size;

	v2 = (T)v2;
	result = (uint64_t)v1 + v2;
	size = sizeof(T)*8;

	s1 = v1 >> (size-1);
	s2 = v2 >> (size-1);
	sr = (result >> (size-1)) & 1;

	set_carry(result >> size);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign(sr);
	set_overflow(!(s1^s2) && s1^sr);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_or(uint32_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_or(uint16_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_or(uint8_t v1, uint32_t v2);
template <class T> uint32_t CPU::update_eflags_or(T v1, uint32_t v2){
	T result;
	uint8_t size;

	v2 = (T)v2;
	result = v1 | v2;
	size = sizeof(T)*8;

	set_carry(0);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign((result >> (size-1)) & 1);
	set_overflow(0);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_and(uint32_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_and(uint16_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_and(uint8_t v1, uint32_t v2);
template <class T> uint32_t CPU::update_eflags_and(T v1, uint32_t v2){
	T result;
	uint8_t size;

	v2 = (T)v2;
	result = v1 & v2;
	size = sizeof(T)*8;

	set_carry(0);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign((result >> (size-1)) & 1);
	set_overflow(0);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_sub(uint32_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_sub(uint16_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_sub(uint8_t v1, uint32_t v2);
template <class T> uint32_t CPU::update_eflags_sub(T v1, uint32_t v2){
	bool s1, s2, sr;
	uint64_t result;
	uint8_t size;

	v2 = (T)v2;
	result = (uint64_t)v1 - v2;
	size = sizeof(T)*8;

	s1 = v1 >> (size-1);
	s2 = v2 >> (size-1);
	sr = (result >> (size-1)) & 1;

	set_carry(result >> size);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign(sr);
	set_overflow(s1^s2 && s1^sr);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_mul(uint32_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_mul(uint16_t v1, uint32_t v2);
template uint32_t CPU::update_eflags_mul(uint8_t v1, uint32_t v2);
template <class T> uint32_t CPU::update_eflags_mul(T v1, uint32_t v2){
	uint64_t result;
	uint8_t size;

	v2 = (T)v2;
	result = (uint64_t)v1 * v2;
	size = sizeof(T)*8;

	set_carry(result >> size);
	set_overflow(result >> size);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_imul(int32_t v1, int32_t v2);
template uint32_t CPU::update_eflags_imul(int16_t v1, int32_t v2);
template uint32_t CPU::update_eflags_imul(int8_t v1, int32_t v2);
template <class T> uint32_t CPU::update_eflags_imul(T v1, int32_t v2){
	int64_t result;
	uint8_t size;

	v2 = (T)v2;
	result = (int64_t)v1 * v2;
	size = sizeof(T)*8;

	set_carry((result >> size) != -1);
	set_overflow((result >> size) != -1);

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_shl(uint32_t v, uint8_t c);
template uint32_t CPU::update_eflags_shl(uint16_t v, uint8_t c);
template uint32_t CPU::update_eflags_shl(uint8_t v, uint8_t c);
template <class T> uint32_t CPU::update_eflags_shl(T v, uint8_t c){
	T result;
	uint8_t size;

	result = v << c;
	size = sizeof(T)*8;

	set_carry((v >> (size-c)) & 1);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign((result >> (size-1)) & 1);
	if(c==1)
		set_overflow(((v >> (size-1)) & 1) ^ ((v >> (size-2)) & 1));

	return eflags.reg32;
}

template uint32_t CPU::update_eflags_shr(uint32_t v, uint8_t c);
template uint32_t CPU::update_eflags_shr(uint16_t v, uint8_t c);
template uint32_t CPU::update_eflags_shr(uint8_t v, uint8_t c);
template <class T> uint32_t CPU::update_eflags_shr(T v, uint8_t c){
	T result;
	uint8_t size;

	result = v >> c;
	size = sizeof(T)*8;

	set_carry((v >> (c-1)) & 1);
	set_parity(chk_parity(result & 0xff));
	set_zero(!result);
	set_sign((result >> (size-1)) & 1);
	if(c==1)
		set_overflow((v >> (size-1)) & 1);

	return eflags.reg32;
}

bool CPU::chk_parity(uint8_t v){
	bool p = true;

	for(int i=0; i<8; i++)
		p ^= (v>>i) & 1;

	return p;
}

