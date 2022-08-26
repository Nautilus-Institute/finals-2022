#include <stdint.h>
#include <stdio.h>


#define ET_READ 1
#define ET_WRITE 2
#define ET_INIT 3
#define ET_FORK 4
#define ET_DEAD 5


#pragma pack(1)
typedef struct event_t {
    int type;
    union {
        struct {
            uint16_t proc_id;
            uint32_t offset;
        } read;
        struct {
            uint16_t proc_id;
            uint32_t offset;
        } write;
        struct {
            uint16_t player_id;
            uint16_t proc_id;
            uint32_t offset;
            uint32_t program_size;
        } init;
        struct {
            uint16_t parent_proc_id;
            uint16_t child_proc_id;
        } proc_fork;
        struct {
            uint16_t proc_id;
        } proc_dead;
    };
} Event;
#pragma pack()


#ifdef VIS
FILE *f = NULL;
#endif

void vis_init()
{
#ifdef VIS
    f = fopen("vis.trace", "wb");
#endif
}

void vis_write(uint16_t proc_id, uint32_t offset)
{
#ifdef VIS
    Event event;
    event.type = ET_WRITE;
    event.write.proc_id = proc_id;
    event.write.offset = offset;
    fwrite(&event, 1, sizeof(Event), f);
#endif
}

void vis_read(uint16_t proc_id, uint32_t offset)
{
#ifdef VIS
    Event event;
    event.type = ET_READ;
    event.read.proc_id = proc_id;
    event.read.offset = offset;
    fwrite(&event, 1, sizeof(Event), f);
#endif
}

void vis_proc_init(uint16_t player_id, uint16_t proc_id, uint32_t offset, uint32_t program_size)
{
#ifdef VIS
    Event event;
    event.type = ET_INIT;
    event.init.player_id = player_id;
    event.init.proc_id = proc_id;
    event.init.offset = offset;
    event.init.program_size = program_size;
    fwrite(&event, 1, sizeof(Event), f);
#endif
}

void vis_proc_dead(uint16_t proc_id)
{
#ifdef VIS
    Event event;
    event.type = ET_DEAD;
    event.proc_dead.proc_id = proc_id;
    fwrite(&event, 1, sizeof(Event), f);
#endif
}

void vis_proc_fork(uint16_t parent, uint16_t child)
{
#ifdef VIS
    Event event;
    event.type = ET_FORK;
    event.proc_fork.parent_proc_id = parent;
    event.proc_fork.child_proc_id = child;
    fwrite(&event, 1, sizeof(Event), f);
#endif
}

void vis_finalize()
{
#ifdef VIS
    if (f != NULL) {
        fclose(f);
        f = NULL;
    }
#endif
}
