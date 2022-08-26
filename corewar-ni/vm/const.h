#define CORE_SIZE 16384
#define MAX_REGS 8
#define MAX_PROGRAM_SIZE 100  // In terms of instructions
#define MAX_PROCESSES 65535
#define MAX_CYCLE 200000


// Instructions

#define DAT 0
#define MOV 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define JMP 7
#define JMZ 8
#define JMN 9
#define DJN 10
#define SPL 11
#define CMP 12
#define SEQ 13
#define SNE 14
#define SLT 15
#define LDP 16
#define STP 17
#define NOP 18

// Process states

#define NONEXIST 0
#define ACTIVE 1
#define DEAD 2

// Opcode mode

#define OPMODE_MEM 0
#define OPMODE_CONST 1  /* # */
#define OPMODE_RELATIVE 2 /* @ */

// Registers

#define REG_PC 0
