#ifdef POOL
#include "pool.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
MemPool *create_mempool(void *mem, uint32_t size) {
  int i;
  MemPool *mpool = reinterpret_cast<MemPool *>(mem);
  // this will be our memory block START
  void *start = (char *)mem + HEADER_SIZE;
  size -= HEADER_SIZE;
  // this will be our memory block END
  void *end = (char *)start + size;
#ifdef GROWUP
  mpool->mem_end = (char *)end;
#else
  mpool->mem_start = (char *)start;
#endif

  uint32_t num_blocks = size/BLOCK_SIZE;
  for (i=0; i < INIT_STEP; i++) {
#ifdef GROWUP
    *((uint32_t *)((char *)end-(i*BLOCK_SIZE))) = i+1;
#else
    *((uint32_t *)((char *)start+(i*BLOCK_SIZE))) = i+1;
#endif
  }

  mpool->m_head = 0;
  mpool->m_num_blocks_allocd = 0;
  mpool->m_num_blocks_total = num_blocks;
  mpool->m_blocksize = BLOCK_SIZE;
  mpool->m_num_blocks_initialized = INIT_STEP;
  mpool->m_num_blocks_free = INIT_STEP;
  return mpool;
}

uint32_t MemPool::GetIdxFromPtr(void *ptr) {
#ifdef GROWUP
  return (((uint64_t)mem_end - (uint64_t)ptr) / BLOCK_SIZE);
#else
  return (( (uint64_t)ptr - (uint64_t)mem_start) / BLOCK_SIZE);
#endif
}

void *MemPool::GetBlockN(uint32_t n) {
#ifdef GROWUP
  return mem_end-(n*BLOCK_SIZE);
#else
  return mem_start+(n*BLOCK_SIZE);
#endif
}

#ifdef DEBUG
void MemPool::Print(bool header_only) {
  //printf("[+] mpool->num_allocs1: %ld\n", num_allocs1);
  //printf("[+] mpool->num_allocs2: %ld\n", num_allocs2);
  //printf("[+] mpool->m_high_mark: %ld\n", m_high_mark);
  //printf("[+] mpool: %p\n", this);
  //printf("[+] mpool->m_head: %d\n", m_head);
  //printf("[+] mpool->m_num_blocks_allocd: %d\n", m_num_blocks_allocd);
  //printf("[+] mpool->m_num_blocks_free: %d\n", m_num_blocks_free);
  //printf("[+] mpool->m_num_blocks_initialized: %d\n", m_num_blocks_initialized);
  //printf("[+] mpool->m_num_blocks_total: %d\n", m_num_blocks_total);
#ifdef GROWUP
  //printf("[+] mpool->m_end: %p\n", mem_end);
#else
  //printf("[+] mpool->mem_start: %p\n", mem_start);
#endif
  //printf("[+] mpool->m_mem: %p\n", m_mem);
  if (header_only) {
    //printf("\n\n");
    return;
  }

 // printf("[+] mpool: %p\n", this);
 // printf("[+] &mpool->m_head: %p\n", &m_head);
 // printf("[+] &mpool->m_num_blocks_allocd: %p\n", &m_num_blocks_allocd);
 // printf("[+] &mpool->m_num_blocks_free: %p\n", &m_num_blocks_free);
 // printf("[+] &mpool->m_num_blocks_initialized: %p\n", &m_num_blocks_initialized);
 // printf("[+] &mpool->m_num_blocks_total: %p\n", &m_num_blocks_total);
 // printf("[+] &mpool->m_mem: %p\n", m_mem);
#ifdef GROWUP
  //printf("[+] mpool->mem_end: %p\n", &mem_end);
#else
  //printf("[+] mpool->mem_start: %p\n", &mem_start);
#endif
  //printf("[+] mpool->m_high_mark: %p\n", &m_high_mark);
  uint32_t to_print = m_num_blocks_initialized > m_num_blocks_total ? m_num_blocks_initialized : m_num_blocks_total;
  for (int i=0; i < to_print; i++) {
    void *tmp = GetBlockN(i);
    //printf("tmp: [%d]%p  --  %d\n",i, tmp, *(uint32_t *)tmp);
  }
}
#endif

/*
int MemPool::MmapFallback(void) {
  void *new_mem = mmap(NULL, 1000*BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  if (new_mem == MAP_FAILED)
    abort();
  memcpy(new_mem, mem_start, m_num_blocks_total*BLOCK_SIZE);
  m_num_blocks_total = 1000;
  mem_start = (char *)new_mem;
  is_mmap = true;
  printf("Got some sick new mem yo\n");
  return 0;
}
*/
int MemPool::InitMoreBlocks(void) {
#ifndef BUG1
  uint32_t num_blocks_to_init;
  if ( (m_num_blocks_total - m_num_blocks_initialized) < INIT_STEP)
    num_blocks_to_init = (m_num_blocks_total - m_num_blocks_initialized);
  else
    num_blocks_to_init = INIT_STEP;
#else
  if (m_num_blocks_initialized >= m_num_blocks_total) {
    //MmapFallback();
    return -1;
  }
  uint32_t num_blocks_to_init = INIT_STEP;
#endif

  int i;
  for (i = 0; i < num_blocks_to_init; i++) {
    int idx = i+m_num_blocks_initialized;
#ifdef GROWUP
    //printf("idx: %d\n", idx);
    //printf("initializing: %p to %d\n", ((uint32_t *)((char *)mem_end-(idx*BLOCK_SIZE))), idx+1);
    //Print(true);
    *((uint32_t *)((char *)mem_end-(idx*BLOCK_SIZE))) = idx+1;
#else
    //printf("initializing: %p to %d\n", ((uint32_t *)((char *)mem_start+(idx*BLOCK_SIZE))), idx+1);
    *((uint32_t *)((char *)mem_start+(idx*BLOCK_SIZE))) = idx+1;
#endif
  }
  m_num_blocks_initialized += num_blocks_to_init;
  m_num_blocks_free += num_blocks_to_init;
  return 0;
}

void *MemPool::AllocBlock(void) {
  if (m_num_blocks_allocd >= m_num_blocks_initialized) {
#ifndef BUG1
    if (m_num_blocks_initialized >= m_num_blocks_total) {
      // out of memory
      return NULL;
    }
    else {
      if (InitMoreBlocks()) {
        return NULL;
      }
    }
#else
    //printf("Initializing more blocks\n");
    if (InitMoreBlocks()) {
      //printf("not very amore\n");
      return NULL;
    }
    //else {
    //  Print();
    //}
#endif
    //Print();
    //printf("\n-------------\n");
  }
  void *free_block = GetBlockN(m_head);
  // update our head
  // this free block stores the index of the next free block
  m_head = *((uint32_t *)free_block);
  // account for the block we just allocd
  uint32_t our_idx = GetIdxFromPtr(free_block);
  if (m_high_mark < our_idx)
    m_high_mark = our_idx;
  m_num_blocks_allocd++;
  m_num_blocks_free--;
  return free_block;
}

int MemPool::FreeBlock(void *to_free) {
  // traverse the list for no more than m_num_blocks_initialized
  int i=0;
  void *cur = NULL;
  uint32_t to_free_idx = GetIdxFromPtr(to_free);
  // set this blocks next idx to the cur head
  //printf("Free'ing: %p  idx: %d\n", to_free, to_free_idx);
  *((uint32_t *)to_free) = m_head;

  // now set the head to this block
  m_head = to_free_idx;
  m_num_blocks_allocd--;
  m_num_blocks_free++;
  return 0;
}
/*
void alloc_test(MemPool *mpool) {
  void *block0 = NULL;
  void *block1 = NULL;
  void *block2 = NULL;
  void *block3 = NULL;
  void *block4 = NULL;

  block0 = mpool->AllocBlock();
  printf("block0: %p   idx: %d\n", block0, mpool->GetIdxFromPtr(block0));
  memset(block0, 0xff, BLOCK_SIZE);

  block1= mpool->AllocBlock();
  printf("block1: %p   idx: %d\n", block1, mpool->GetIdxFromPtr(block1));
  memset(block1, 0xff, BLOCK_SIZE);

  block2 = mpool->AllocBlock();
  printf("block2: %p   idx: %d\n", block2, mpool->GetIdxFromPtr(block2));
  memset(block2, 0xff, BLOCK_SIZE);

  block3 = mpool->AllocBlock();
  printf("block3: %p   idx: %d\n", block3, mpool->GetIdxFromPtr(block3));
  memset(block3, 0xff, BLOCK_SIZE);

  block4 = mpool->AllocBlock();
  printf("block4: %p   idx: %d\n", block4, mpool->GetIdxFromPtr(block4));
  memset(block4, 0xff, BLOCK_SIZE);

  mpool->Print();
  mpool->FreeBlock(block2);
  printf("Just free'd block2\n");
  mpool->Print();

  mpool->FreeBlock(block0);
  mpool->FreeBlock(block1);
  mpool->FreeBlock(block3);
  printf("Free'd All but one \n");
  mpool->Print();
  printf("Free'd the rest\n");
  mpool->FreeBlock(block4);
  mpool->Print();
}

void alloc_all_test(MemPool *mpool) {
  uint32_t free_blocks = mpool->m_num_blocks_free;
  void **ptr_arr = (void **)calloc(sizeof(void *), free_blocks);
  int i;

  for (i=0; i < free_blocks; i++) {
    ptr_arr[i] = mpool->AllocBlock();
    memset(ptr_arr[i], 0xff, BLOCK_SIZE);

  }
  mpool->Print();
  printf("[+] Alloc'ing another. Should init more\n");
  void **ptr_arr2 = (void **)calloc(sizeof(void *), free_blocks);
  for (i=0; i < 20; i++) {
    ptr_arr2[i] = mpool->AllocBlock();
    memset(ptr_arr2[i], 0xff, BLOCK_SIZE);
  }
  mpool->Print();
  void *new_block = mpool->AllocBlock();
  printf("new_block: %p\n", new_block);
}

int main(void) {
  MemPool *mpool = create_mempool(heap_mem, HEAP_SIZE);
  //mpool->Print();
  //alloc_test(mpool);
  alloc_all_test(mpool);
  return 0;
}
*/
#endif
