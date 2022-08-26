#ifndef _VM_H
#define _VM_H

#include <stdint.h>
#include "const.h"


class Process {
public:
    uint16_t proc_id;
    uint8_t state;
    uint32_t registers[MAX_REGS];
};


class VM {
public:
    VM();
public:
    uint16_t curr_proc_id;
    uint16_t players;
    uint8_t arena[CORE_SIZE];
};

#endif
