// Reference: x86emu/include/instruction/instruction.hpp

#ifndef _PARSER_H
#define _PARSER_H

#include "vm.hpp"
#include "cpu.hpp"
#include "instr.hpp"

class Parser {
	public:
        Parser();
		void parse(VM* vm, CPU* cpu, InstrData* instr);
		uint8_t parse_prefix();
	private:
		void parse_opcode();
		void parse_modrm_sib_disp();
		void parse_modrm16();
		void parse_modrm32();
		void parse_moffs();

        VM* m_vm;
        CPU* m_cpu;
        InstrData* m_instr;
};


#endif
