#include <stdint.h>
#include "const.h"


typedef struct PLAYER_t {
    uint16_t player_id;
    char player_name[32];
    uint16_t proc_ids[MAX_PROCESSES]; // TODO: Use a linked list
    uint16_t next_proc_offset;
    int8_t current_procs; // VULN: Having more than 127 processes at the same time kills a player
    uint32_t arena_start;
    uint32_t arena_end;
    uint32_t arena_size;
} PLAYER;


typedef struct PROCESS_t {
    PLAYER* player;
    uint16_t proc_id;
    uint8_t state;
    uint32_t registers[MAX_REGS];
} PROCESS;


#pragma pack(1)
typedef struct INSTRUCTION_t{
    uint8_t opcode;
    int16_t a;
    uint8_t a_mode;
    int16_t b;
    uint8_t b_mode;
} INSTR;
#pragma pack()

typedef struct VM_t {
    uint16_t curr_proc_id;
    uint16_t players;
    INSTR arena[CORE_SIZE + 10];
} VM;
