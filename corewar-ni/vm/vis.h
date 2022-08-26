#include <stdint.h>

void vis_init();
void vis_write(uint16_t proc_id, uint32_t offset);
void vis_read(uint16_t proc_id, uint32_t offset);
void vis_proc_init(uint16_t player_id, uint16_t proc_id, uint32_t offset, uint32_t program_size);
void vis_proc_fork(uint16_t parent, uint16_t child);
void vis_proc_dead(uint16_t proc_id);
void vis_finalize();
