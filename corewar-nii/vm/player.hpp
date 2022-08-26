#ifndef _PLAYER_H 
#define _PLAYER_H

#include <stdint.h>
#include "const.h"


class Player {
 public:
     uint16_t player_id;
     char player_name[32];
     uint16_t proc_ids[MAX_PROCESSES]; // TODO: Use a linked list
     uint16_t next_proc_offset;
     uint16_t current_procs;
};

#endif
