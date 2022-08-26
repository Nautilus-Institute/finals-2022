#ifndef __NAUTILUS_RANDOM
#define __NAUTILUS_RANDOM

#include <stdint.h>

extern void NautilusInitRandFile(char *Filename);

// WARNING! If using InitRandData it will NOT securely erase the data buffer which may be a memory leak
// of flag data, use explicit_bzero to properly erase memory and avoid flag data leaks
extern void NautilusInitRandData(void *Data, uint64_t Len);

// functions to get random data
extern void NautilusGetRandData(void *Data, uint64_t Len);
extern uint64_t NautilusGetRandVal();

#endif