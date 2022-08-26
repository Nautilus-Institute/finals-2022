// Reference: x86emu/include/instruction/instruction.hpp

#ifndef _INSTR_H
#define _INSTR_H

#include <map>

#define EMU				get_emu()
#define GET_EIP()			m_cpu->get_eip()
#define GET_IP()			EMU->get_ip()
#define SET_EIP(v)			EMU->set_eip(v)
#define SET_IP(v)			EMU->set_ip(v)
#define UPDATE_EIP(v)			m_cpu->update_eip(v)
#define GET_GPREG(reg)			EMU->get_gpreg(reg)
#define SET_GPREG(reg, v)		EMU->set_gpreg(reg, v)
#define UPDATE_GPREG(reg, v)		EMU->update_gpreg(reg, v)

#define READ_MEM32(addr)		EMU->get_data32(select_segment(), addr)
#define READ_MEM16(addr)		EMU->get_data16(select_segment(), addr)
#define READ_MEM8(addr)			EMU->get_data8(select_segment(), addr)
#define WRITE_MEM32(addr, v)		EMU->put_data32(select_segment(), addr, v)
#define WRITE_MEM16(addr, v)		EMU->put_data16(select_segment(), addr, v)
#define WRITE_MEM8(addr, v)		EMU->put_data8(select_segment(), addr, v)
#define PUSH32(v)			EMU->push32(v)
#define PUSH16(v)			EMU->push16(v)
#define POP32()				EMU->pop32()
#define POP16()				EMU->pop16()

#define PREFIX		(instr->prefix)
#define OPCODE		(instr->opcode)
#define _MODRM		(instr->_modrm)
#define MOD		(instr->modrm.mod)
#define REG		(instr->modrm.reg)
#define RM		(instr->modrm.rm)
#define _SIB		(instr->_sib)
#define SCALE		(instr->sib.scale)
#define INDEX		(instr->sib.index)
#define BASE		(instr->sib.base)
#define DISP32		(instr->disp32)
#define DISP16		(instr->disp16)
#define DISP8		(instr->disp8)
#define IMM32		(instr->imm32)
#define IMM16		(instr->imm16)
#define IMM8		(instr->imm8)
#define PTR16		(instr->ptr16)
#define MOFFS		(instr->moffs)
#define PRE_SEGMENT	(instr->pre_segment)
#define PRE_REPEAT	(instr->pre_repeat)
#define SEGMENT		(instr->segment)

#define MAX_OPCODE	0x200

struct ModRM {  
        uint8_t rm : 3; 
        uint8_t reg : 3; 
        uint8_t mod : 2; 
};

struct SIB {  
        uint8_t base : 3; 
        uint8_t index : 3; 
        uint8_t scale : 2; 
};

enum rep_t { NONE, REPZ, REPNZ };

struct InstrData {
	uint16_t prefix;
	uint16_t pre_segment;
	rep_t pre_repeat;

	uint16_t segment;
	uint16_t opcode;
	union {
		uint8_t _modrm;
		struct ModRM modrm;
	};
	union {
		uint8_t _sib;
		struct SIB sib;
	};
	union {
		int8_t disp8;
		int16_t disp16;
		int32_t disp32;
	};
	union {
		int8_t imm8;
		int16_t imm16;
		int32_t imm32;
	};
	int16_t ptr16;
	uint32_t moffs;
};

#define CHK_MODRM 	(1<<0)
#define CHK_IMM32 	(1<<1)
#define CHK_IMM16 	(1<<2)
#define CHK_IMM8 	(1<<3)
#define CHK_PTR16 	(1<<4)
#define CHK_MOFFS 	(1<<5)

#define CHSZ_NONE	0
#define CHSZ_OP		1
#define CHSZ_AD		2

union InstrFlags {
	uint8_t flags;
	struct {
		uint8_t modrm : 1;
		uint8_t imm32 : 1;
		uint8_t imm16 : 1;
		uint8_t imm8 : 1;
		uint8_t ptr16 : 1;
		uint8_t moffs : 1;
		uint8_t moffs8 : 1;
	};
};



#endif
