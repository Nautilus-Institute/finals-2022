#include <iostream>

#include "parser.hpp"

InstrFlags chk[0x200] = {
/* 0x0 */  {CHK_MODRM},
/* 0x1 */  {CHK_MODRM},
/* 0x2 */  {CHK_MODRM},
/* 0x3 */  {CHK_MODRM},
/* 0x4 */  {CHK_IMM8},
/* 0x5 */  {CHK_IMM32},
/* 0x6 */  {0},
/* 0x7 */  {0},
/* 0x8 */  {CHK_MODRM},
/* 0x9 */  {CHK_MODRM},
/* 0xa */  {CHK_MODRM},
/* 0xb */  {CHK_MODRM},
/* 0xc */  {CHK_IMM8},
/* 0xd */  {CHK_IMM32},
/* 0xe */  {0},
/* 0xf */  {0},
/* 0x10 */  {0},
/* 0x11 */  {0},
/* 0x12 */  {0},
/* 0x13 */  {0},
/* 0x14 */  {0},
/* 0x15 */  {0},
/* 0x16 */  {0},
/* 0x17 */  {0},
/* 0x18 */  {0},
/* 0x19 */  {0},
/* 0x1a */  {0},
/* 0x1b */  {0},
/* 0x1c */  {0},
/* 0x1d */  {0},
/* 0x1e */  {0},
/* 0x1f */  {0},
/* 0x20 */  {CHK_MODRM},
/* 0x21 */  {CHK_MODRM},
/* 0x22 */  {CHK_MODRM},
/* 0x23 */  {CHK_MODRM},
/* 0x24 */  {CHK_IMM8},
/* 0x25 */  {CHK_IMM32},
/* 0x26 */  {0},
/* 0x27 */  {0},
/* 0x28 */  {CHK_MODRM},
/* 0x29 */  {CHK_MODRM},
/* 0x2a */  {CHK_MODRM},
/* 0x2b */  {CHK_MODRM},
/* 0x2c */  {CHK_IMM8},
/* 0x2d */  {CHK_IMM32},
/* 0x2e */  {0},
/* 0x2f */  {0},
/* 0x30 */  {CHK_MODRM},
/* 0x31 */  {CHK_MODRM},
/* 0x32 */  {CHK_MODRM},
/* 0x33 */  {CHK_MODRM},
/* 0x34 */  {CHK_IMM8},
/* 0x35 */  {CHK_IMM32},
/* 0x36 */  {0},
/* 0x37 */  {0},
/* 0x38 */  {CHK_MODRM},
/* 0x39 */  {CHK_MODRM},
/* 0x3a */  {CHK_MODRM},
/* 0x3b */  {CHK_MODRM},
/* 0x3c */  {CHK_IMM8},
/* 0x3d */  {CHK_IMM32},
/* 0x3e */  {0},
/* 0x3f */  {0},
/* 0x40 */  {0},
/* 0x41 */  {0},
/* 0x42 */  {0},
/* 0x43 */  {0},
/* 0x44 */  {0},
/* 0x45 */  {0},
/* 0x46 */  {0},
/* 0x47 */  {0},
/* 0x48 */  {0},
/* 0x49 */  {0},
/* 0x4a */  {0},
/* 0x4b */  {0},
/* 0x4c */  {0},
/* 0x4d */  {0},
/* 0x4e */  {0},
/* 0x4f */  {0},
/* 0x50 */  {0},
/* 0x51 */  {0},
/* 0x52 */  {0},
/* 0x53 */  {0},
/* 0x54 */  {0},
/* 0x55 */  {0},
/* 0x56 */  {0},
/* 0x57 */  {0},
/* 0x58 */  {0},
/* 0x59 */  {0},
/* 0x5a */  {0},
/* 0x5b */  {0},
/* 0x5c */  {0},
/* 0x5d */  {0},
/* 0x5e */  {0},
/* 0x5f */  {0},
/* 0x60 */  {0},
/* 0x61 */  {0},
/* 0x62 */  {0},
/* 0x63 */  {0},
/* 0x64 */  {0},
/* 0x65 */  {0},
/* 0x66 */  {0},
/* 0x67 */  {0},
/* 0x68 */  {CHK_IMM32},
/* 0x69 */  {CHK_MODRM|CHK_IMM32},
/* 0x6a */  {CHK_IMM8},
/* 0x6b */  {CHK_MODRM|CHK_IMM8},
/* 0x6c */  {0},
/* 0x6d */  {0},
/* 0x6e */  {0},
/* 0x6f */  {0},
/* 0x70 */  {CHK_IMM8},
/* 0x71 */  {CHK_IMM8},
/* 0x72 */  {CHK_IMM8},
/* 0x73 */  {CHK_IMM8},
/* 0x74 */  {CHK_IMM8},
/* 0x75 */  {CHK_IMM8},
/* 0x76 */  {CHK_IMM8},
/* 0x77 */  {CHK_IMM8},
/* 0x78 */  {CHK_IMM8},
/* 0x79 */  {CHK_IMM8},
/* 0x7a */  {CHK_IMM8},
/* 0x7b */  {CHK_IMM8},
/* 0x7c */  {CHK_IMM8},
/* 0x7d */  {CHK_IMM8},
/* 0x7e */  {CHK_IMM8},
/* 0x7f */  {CHK_IMM8},
/* 0x80 */  {CHK_MODRM | CHK_IMM8},
/* 0x81 */  {CHK_MODRM | CHK_IMM32},
/* 0x82 */  {CHK_MODRM | CHK_IMM8},
/* 0x83 */  {CHK_MODRM | CHK_IMM8},
/* 0x84 */  {CHK_MODRM},
/* 0x85 */  {CHK_MODRM},
/* 0x86 */  {CHK_MODRM},
/* 0x87 */  {CHK_MODRM},
/* 0x88 */  {CHK_MODRM},
/* 0x89 */  {CHK_MODRM},
/* 0x8a */  {CHK_MODRM},
/* 0x8b */  {CHK_MODRM},
/* 0x8c */  {CHK_MODRM},
/* 0x8d */  {CHK_MODRM},
/* 0x8e */  {CHK_MODRM},
/* 0x8f */  {0},
/* 0x90 */  {CHK_IMM32},
/* 0x91 */  {CHK_IMM32},
/* 0x92 */  {CHK_IMM32},
/* 0x93 */  {CHK_IMM32},
/* 0x94 */  {CHK_IMM32},
/* 0x95 */  {CHK_IMM32},
/* 0x96 */  {CHK_IMM32},
/* 0x97 */  {CHK_IMM32},
/* 0x98 */  {0},
/* 0x99 */  {0},
/* 0x9a */  {CHK_PTR16 | CHK_IMM32},
/* 0x9b */  {0},
/* 0x9c */  {0},
/* 0x9d */  {0},
/* 0x9e */  {0},
/* 0x9f */  {0},
/* 0xa0 */  {CHK_MOFFS},
/* 0xa1 */  {CHK_MOFFS},
/* 0xa2 */  {CHK_MOFFS},
/* 0xa3 */  {CHK_MOFFS},
/* 0xa4 */  {0},
/* 0xa5 */  {0},
/* 0xa6 */  {0},
/* 0xa7 */  {0},
/* 0xa8 */  {CHK_IMM8},
/* 0xa9 */  {CHK_IMM32},
/* 0xaa */  {0},
/* 0xab */  {0},
/* 0xac */  {0},
/* 0xad */  {0},
/* 0xae */  {0},
/* 0xaf */  {0},
/* 0xb0 */  {CHK_IMM8},
/* 0xb1 */  {CHK_IMM8},
/* 0xb2 */  {CHK_IMM8},
/* 0xb3 */  {CHK_IMM8},
/* 0xb4 */  {CHK_IMM8},
/* 0xb5 */  {CHK_IMM8},
/* 0xb6 */  {CHK_IMM8},
/* 0xb7 */  {CHK_IMM8},
/* 0xb8 */  {CHK_IMM32},
/* 0xb9 */  {CHK_IMM32},
/* 0xba */  {CHK_IMM32},
/* 0xbb */  {CHK_IMM32},
/* 0xbc */  {CHK_IMM32},
/* 0xbd */  {CHK_IMM32},
/* 0xbe */  {CHK_IMM32},
/* 0xbf */  {CHK_IMM32},
/* 0xc0 */  {CHK_MODRM | CHK_IMM8},
/* 0xc1 */  {CHK_MODRM | CHK_IMM8},
/* 0xc2 */  {0},
/* 0xc3 */  {0},
/* 0xc4 */  {0},
/* 0xc5 */  {0},
/* 0xc6 */  {CHK_MODRM | CHK_IMM8},
/* 0xc7 */  {CHK_MODRM | CHK_IMM32},
/* 0xc8 */  {0},
/* 0xc9 */  {0},
/* 0xca */  {0},
/* 0xcb */  {0},
/* 0xcc */  {0},
/* 0xcd */  {CHK_IMM8},
/* 0xce */  {0},
/* 0xcf */  {0},
/* 0xd0 */  {0},
/* 0xd1 */  {0},
/* 0xd2 */  {0},
/* 0xd3 */  {CHK_MODRM},
/* 0xd4 */  {0},
/* 0xd5 */  {0},
/* 0xd6 */  {0},
/* 0xd7 */  {0},
/* 0xd8 */  {0},
/* 0xd9 */  {0},
/* 0xda */  {0},
/* 0xdb */  {0},
/* 0xdc */  {0},
/* 0xdd */  {0},
/* 0xde */  {0},
/* 0xdf */  {0},
/* 0xe0 */  {0},
/* 0xe1 */  {0},
/* 0xe2 */  {0},
/* 0xe3 */  {0},
/* 0xe4 */  {CHK_IMM8},
/* 0xe5 */  {CHK_IMM8},
/* 0xe6 */  {CHK_IMM8},
/* 0xe7 */  {CHK_IMM8},
/* 0xe8 */  {CHK_IMM32},
/* 0xe9 */  {CHK_IMM32},
/* 0xea */  {CHK_PTR16 | CHK_IMM32},
/* 0xeb */  {CHK_IMM8},
/* 0xec */  {0},
/* 0xed */  {0},
/* 0xee */  {0},
/* 0xef */  {0},
/* 0xf0 */  {0},
/* 0xf1 */  {0},
/* 0xf2 */  {0},
/* 0xf3 */  {0},
/* 0xf4 */  {0},
/* 0xf5 */  {0},
/* 0xf6 */  {CHK_MODRM},
/* 0xf7 */  {CHK_MODRM},
/* 0xf8 */  {0},
/* 0xf9 */  {0},
/* 0xfa */  {0},
/* 0xfb */  {0},
/* 0xfc */  {0},
/* 0xfd */  {0},
/* 0xfe */  {0},
/* 0xff */  {CHK_MODRM},
/* 0xf00 */  {CHK_MODRM},
/* 0xf01 */  {CHK_MODRM},
/* 0xf02 */  {0},
/* 0xf03 */  {0},
/* 0xf04 */  {0},
/* 0xf05 */  {0},
/* 0xf06 */  {0},
/* 0xf07 */  {0},
/* 0xf08 */  {0},
/* 0xf09 */  {0},
/* 0xf0a */  {0},
/* 0xf0b */  {0},
/* 0xf0c */  {0},
/* 0xf0d */  {0},
/* 0xf0e */  {0},
/* 0xf0f */  {0},
/* 0xf10 */  {0},
/* 0xf11 */  {0},
/* 0xf12 */  {0},
/* 0xf13 */  {0},
/* 0xf14 */  {0},
/* 0xf15 */  {0},
/* 0xf16 */  {0},
/* 0xf17 */  {0},
/* 0xf18 */  {0},
/* 0xf19 */  {0},
/* 0xf1a */  {0},
/* 0xf1b */  {0},
/* 0xf1c */  {0},
/* 0xf1d */  {0},
/* 0xf1e */  {0},
/* 0xf1f */  {0},
/* 0xf20 */  {CHK_MODRM},
/* 0xf21 */  {0},
/* 0xf22 */  {CHK_MODRM},
/* 0xf23 */  {0},
/* 0xf24 */  {0},
/* 0xf25 */  {0},
/* 0xf26 */  {0},
/* 0xf27 */  {0},
/* 0xf28 */  {0},
/* 0xf29 */  {0},
/* 0xf2a */  {0},
/* 0xf2b */  {0},
/* 0xf2c */  {0},
/* 0xf2d */  {0},
/* 0xf2e */  {0},
/* 0xf2f */  {0},
/* 0xf30 */  {0},
/* 0xf31 */  {0},
/* 0xf32 */  {0},
/* 0xf33 */  {0},
/* 0xf34 */  {0},
/* 0xf35 */  {0},
/* 0xf36 */  {0},
/* 0xf37 */  {0},
/* 0xf38 */  {0},
/* 0xf39 */  {0},
/* 0xf3a */  {0},
/* 0xf3b */  {0},
/* 0xf3c */  {0},
/* 0xf3d */  {0},
/* 0xf3e */  {0},
/* 0xf3f */  {0},
/* 0xf40 */  {0},
/* 0xf41 */  {0},
/* 0xf42 */  {0},
/* 0xf43 */  {0},
/* 0xf44 */  {0},
/* 0xf45 */  {0},
/* 0xf46 */  {0},
/* 0xf47 */  {0},
/* 0xf48 */  {0},
/* 0xf49 */  {0},
/* 0xf4a */  {0},
/* 0xf4b */  {0},
/* 0xf4c */  {0},
/* 0xf4d */  {0},
/* 0xf4e */  {0},
/* 0xf4f */  {0},
/* 0xf50 */  {0},
/* 0xf51 */  {0},
/* 0xf52 */  {0},
/* 0xf53 */  {0},
/* 0xf54 */  {0},
/* 0xf55 */  {0},
/* 0xf56 */  {0},
/* 0xf57 */  {0},
/* 0xf58 */  {0},
/* 0xf59 */  {0},
/* 0xf5a */  {0},
/* 0xf5b */  {0},
/* 0xf5c */  {0},
/* 0xf5d */  {0},
/* 0xf5e */  {0},
/* 0xf5f */  {0},
/* 0xf60 */  {0},
/* 0xf61 */  {0},
/* 0xf62 */  {0},
/* 0xf63 */  {0},
/* 0xf64 */  {0},
/* 0xf65 */  {0},
/* 0xf66 */  {0},
/* 0xf67 */  {0},
/* 0xf68 */  {0},
/* 0xf69 */  {0},
/* 0xf6a */  {0},
/* 0xf6b */  {0},
/* 0xf6c */  {0},
/* 0xf6d */  {0},
/* 0xf6e */  {0},
/* 0xf6f */  {0},
/* 0xf70 */  {0},
/* 0xf71 */  {0},
/* 0xf72 */  {0},
/* 0xf73 */  {0},
/* 0xf74 */  {0},
/* 0xf75 */  {0},
/* 0xf76 */  {0},
/* 0xf77 */  {0},
/* 0xf78 */  {0},
/* 0xf79 */  {0},
/* 0xf7a */  {0},
/* 0xf7b */  {0},
/* 0xf7c */  {0},
/* 0xf7d */  {0},
/* 0xf7e */  {0},
/* 0xf7f */  {0},
/* 0xf80 */  {CHK_IMM32},
/* 0xf81 */  {CHK_IMM32},
/* 0xf82 */  {CHK_IMM32},
/* 0xf83 */  {CHK_IMM32},
/* 0xf84 */  {CHK_IMM32},
/* 0xf85 */  {CHK_IMM32},
/* 0xf86 */  {CHK_IMM32},
/* 0xf87 */  {CHK_IMM32},
/* 0xf88 */  {CHK_IMM32},
/* 0xf89 */  {CHK_IMM32},
/* 0xf8a */  {CHK_IMM32},
/* 0xf8b */  {CHK_IMM32},
/* 0xf8c */  {CHK_IMM32},
/* 0xf8d */  {CHK_IMM32},
/* 0xf8e */  {CHK_IMM32},
/* 0xf8f */  {CHK_IMM32},
/* 0xf90 */  {CHK_MODRM},
/* 0xf91 */  {CHK_MODRM},
/* 0xf92 */  {CHK_MODRM},
/* 0xf93 */  {CHK_MODRM},
/* 0xf94 */  {CHK_MODRM},
/* 0xf95 */  {CHK_MODRM},
/* 0xf96 */  {CHK_MODRM},
/* 0xf97 */  {CHK_MODRM},
/* 0xf98 */  {CHK_MODRM},
/* 0xf99 */  {CHK_MODRM},
/* 0xf9a */  {CHK_MODRM},
/* 0xf9b */  {CHK_MODRM},
/* 0xf9c */  {CHK_MODRM},
/* 0xf9d */  {CHK_MODRM},
/* 0xf9e */  {CHK_MODRM},
/* 0xf9f */  {CHK_MODRM},
/* 0xfa0 */  {0},
/* 0xfa1 */  {0},
/* 0xfa2 */  {0},
/* 0xfa3 */  {0},
/* 0xfa4 */  {0},
/* 0xfa5 */  {0},
/* 0xfa6 */  {0},
/* 0xfa7 */  {0},
/* 0xfa8 */  {0},
/* 0xfa9 */  {0},
/* 0xfaa */  {0},
/* 0xfab */  {0},
/* 0xfac */  {0},
/* 0xfad */  {0},
/* 0xfae */  {0},
/* 0xfaf */  {CHK_MODRM},
/* 0xfb0 */  {0},
/* 0xfb1 */  {0},
/* 0xfb2 */  {0},
/* 0xfb3 */  {0},
/* 0xfb4 */  {0},
/* 0xfb5 */  {0},
/* 0xfb6 */  {CHK_MODRM},
/* 0xfb7 */  {CHK_MODRM},
/* 0xfb8 */  {0},
/* 0xfb9 */  {0},
/* 0xfba */  {0},
/* 0xfbb */  {0},
/* 0xfbc */  {0},
/* 0xfbd */  {0},
/* 0xfbe */  {CHK_MODRM},
/* 0xfbf */  {CHK_MODRM},
/* 0xfc0 */  {0},
/* 0xfc1 */  {0},
/* 0xfc2 */  {0},
/* 0xfc3 */  {0},
/* 0xfc4 */  {0},
/* 0xfc5 */  {0},
/* 0xfc6 */  {0},
/* 0xfc7 */  {0},
/* 0xfc8 */  {0},
/* 0xfc9 */  {0},
/* 0xfca */  {0},
/* 0xfcb */  {0},
/* 0xfcc */  {0},
/* 0xfcd */  {0},
/* 0xfce */  {0},
/* 0xfcf */  {0},
/* 0xfd0 */  {0},
/* 0xfd1 */  {0},
/* 0xfd2 */  {0},
/* 0xfd3 */  {0},
/* 0xfd4 */  {0},
/* 0xfd5 */  {0},
/* 0xfd6 */  {0},
/* 0xfd7 */  {0},
/* 0xfd8 */  {0},
/* 0xfd9 */  {0},
/* 0xfda */  {0},
/* 0xfdb */  {0},
/* 0xfdc */  {0},
/* 0xfdd */  {0},
/* 0xfde */  {0},
/* 0xfdf */  {0},
/* 0xfe0 */  {0},
/* 0xfe1 */  {0},
/* 0xfe2 */  {0},
/* 0xfe3 */  {0},
/* 0xfe4 */  {0},
/* 0xfe5 */  {0},
/* 0xfe6 */  {0},
/* 0xfe7 */  {0},
/* 0xfe8 */  {0},
/* 0xfe9 */  {0},
/* 0xfea */  {0},
/* 0xfeb */  {0},
/* 0xfec */  {0},
/* 0xfed */  {0},
/* 0xfee */  {0},
/* 0xfef */  {0},
/* 0xff0 */  {0},
/* 0xff1 */  {0},
/* 0xff2 */  {0},
/* 0xff3 */  {0},
/* 0xff4 */  {0},
/* 0xff5 */  {0},
/* 0xff6 */  {0},
/* 0xff7 */  {0},
/* 0xff8 */  {0},
/* 0xff9 */  {0},
/* 0xffa */  {0},
/* 0xffb */  {0},
/* 0xffc */  {0},
/* 0xffd */  {0},
/* 0xffe */  {0},
/* 0xfff */  {0},
};

Parser::Parser()
    : m_vm(NULL), m_cpu(NULL), m_instr(NULL)
{

}

uint8_t Parser::parse_prefix()
{
	uint8_t code, chsz = 0;

	while (true) {
		code = m_cpu->get_code8();
		switch (code) {
			case 0x26:
                // es
			case 0x2e:
                // cs
			case 0x36:
                // ss
			case 0x3e:
                // ds
			case 0x64:
                // fs
			case 0x65:
                // gs
                goto set_pre;
			case 0x66:
				chsz |= CHSZ_OP;
				goto next;
			case 0x67:
				chsz |= CHSZ_AD;
				goto next;
			case 0xf2:
				m_instr->pre_repeat = REPNZ;
				goto next;
			case 0xf3:
				m_instr->pre_repeat = REPZ;
				goto next;
			default:
				return chsz;
				
set_pre:			m_instr->prefix = code;
next:				m_cpu->update_eip(1);
				break;
		}
	}
}


void Parser::parse(VM* vm, CPU* cpu, InstrData* instr)
{
    m_vm = vm;
    m_cpu = cpu;
    m_instr = instr;

	uint16_t opcode;

	parse_opcode();

	opcode = m_instr->opcode;
	if(opcode >> 8 == 0x0f) {
		opcode = (opcode & 0xff) | 0x0100;
    }

    if (chk[opcode].modrm) {
		parse_modrm_sib_disp();
    }

	if (chk[opcode].imm32) {
		m_instr->imm32 = m_cpu->get_code32();
#ifdef DEBUG
        std::cerr << "imm32: " << m_instr->imm32 << std::endl;
#endif
		UPDATE_EIP(4);
	}
	else if(chk[opcode].imm16) {
		m_instr->imm16 = m_cpu->get_code16();
#ifdef DEBUG
        std::cerr << "imm16: " << m_instr->imm16 << std::endl;
#endif
		UPDATE_EIP(2);
	}
	else if(chk[opcode].imm8) {
		m_instr->imm8 = (int8_t)m_cpu->get_code8();
#ifdef DEBUG
        std::cerr << "imm8: " << m_instr->imm8 << std::endl;
#endif
		UPDATE_EIP(1);
	}
	// if (chk[opcode].ptr16) {
	// 	m_instr->ptr16 = m_cpu->get_code16();
    //     std::cerr << "ptr16: 0x" << m_instr->ptr16 << std::endl;
	// 	UPDATE_EIP(2);
	// }

    if (chk[opcode].moffs) {
		parse_moffs();
    }
}

void Parser::parse_opcode()
{
	m_instr->opcode = m_cpu->get_code8();
	UPDATE_EIP(1);
	
	// two-byte opcode
	if (m_instr->opcode == 0x0f) {
		m_instr->opcode = (m_instr->opcode << 8) + m_cpu->get_code8();
		UPDATE_EIP(1);
	}
}

void Parser::parse_modrm_sib_disp()
{
	m_instr->_modrm = m_cpu->get_code8();
	UPDATE_EIP(1);

    parse_modrm32();
}

void Parser::parse_modrm32()
{
	if (m_instr->modrm.mod != 3 && m_instr->modrm.rm == 4) {
		m_instr->_sib = m_cpu->get_code8();
		UPDATE_EIP(1);
        std::cerr << "[scale:" << m_instr->sib.scale
            << " index:" << m_instr->sib.index
            << " base:" << m_instr->sib.base
            << "] " << std::endl;
	}

	if (m_instr->modrm.mod == 2
            || (m_instr->modrm.mod == 0 && m_instr->modrm.rm == 5)
            || (m_instr->modrm.mod == 0 && m_instr->sib.base == 5)) {
		m_instr->disp32 = m_cpu->get_code32();
		UPDATE_EIP(4);
	}
	else if (m_instr->modrm.mod == 1) {
		m_instr->disp8 = (int8_t)m_cpu->get_code8();
		UPDATE_EIP(1);
	}
}

void Parser::parse_modrm16()
{
	if ((m_instr->modrm.mod == 0 && m_instr->modrm.rm == 6)
            || m_instr->modrm.mod == 2) {
		m_instr->disp16 = m_cpu->get_code32();
		UPDATE_EIP(2);
	}
	else if (m_instr->modrm.mod == 1) {
		m_instr->disp8 = (int8_t)m_cpu->get_code8();
		UPDATE_EIP(1);
	}
}

void Parser::parse_moffs(void)
{
    m_instr->moffs = m_cpu->get_code32();
    UPDATE_EIP(4);
}

