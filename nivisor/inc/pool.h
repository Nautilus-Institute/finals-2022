#ifdef POOL
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "myrb.h"

template <typename T, typename U>
constexpr ssize_t offsetof_impl(T const* t, U T::* a) {
    return (char const*)t - (char const*)&(t->*a) >= 0 ?
           (char const*)t - (char const*)&(t->*a)      :
           (char const*)&(t->*a) - (char const*)t;
}

#define offsetof(Type_, Attr_)                          \
    offsetof_impl((Type_ const*)nullptr, &Type_::Attr_)

#define HEAP_SIZE HEADER_SIZE + BLOCK_SIZE*21
#define HEADER_SIZE sizeof(MemPool)
#define BLOCK_SIZE sizeof(class RBNode<uint64_t, class SchedEntity *>)
//#define BLOCK_SIZE 64
//#define BLOCK_SIZE 64
#define INIT_STEP 10

class MemPool {
  public:
    MemPool(void *mem, uint32_t size);
    uint32_t GetHead(void) {return m_head; }
    void *AllocBlock(void);
#ifdef DEBUG
    void Print(bool header_only=false);
#endif
    int FreeBlock(void *);
    void *GetBlockN(uint32_t n);
    uint32_t GetIdxFromPtr(void *ptr);
    int InitMoreBlocks(void);
    //int MmapFallback(void);


    uint32_t m_high_mark;
    uint32_t m_head;
    uint32_t m_blocksize;
    uint32_t m_num_blocks_allocd;
    uint32_t m_num_blocks_free;
    uint32_t m_num_blocks_initialized;
    uint32_t m_num_blocks_total;
#ifdef GROWUP
    char *mem_end;
#else
    char *mem_start;
#endif
    char m_mem[];
};

MemPool *create_mempool(void *mem, uint32_t size);

#endif
