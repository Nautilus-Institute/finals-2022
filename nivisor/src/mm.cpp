#include <memory>
#include <climits>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "stdnivisor.h"
#include "mm.hpp"
#include "ctx.hpp"

MemoryManager::MemoryManager(void) {
  m_lower_bound = 0x100000;
  m_upper_bound = ((uint64_t)1 << 47) - NIVISOR_PAGE_SIZE;

  m_last_allocated = m_lower_bound;

  // TODO use this
  m_usage = 0;
  m_max_usage = UINT_MAX;

  int memfd = memfd_create("nivisor-memory", 0);
  if (memfd < 0)
  {
    DPRINTF("Failed to create memfd\n");
    // throw exception
  }

  m_phys_mm = std::make_shared<PhysMemoryManager>(
                                          memfd,
                                          NIVISOR_MAX_SYSTEM_MEM);
}

MemoryManager::~MemoryManager() {
  DPRINTF("In MemoryManager destructor\n");
}

// VMA lazy paging functionality
Vma *Vma::CloneCOW()
{
  Vma *vma = new Vma(m_file_descriptor,
                     m_vaddr_start,
                     m_region_len,
                     m_flags,
                     m_prot);

  // copy over the parent's PMA
  vma->m_pma = m_pma;

  // copy mapped in state
  // uncomment this to disable POC but enable COW
  vma->m_faulted_in = m_faulted_in;

  // finally indicate if a write occurs at this address we need
  // to invoke our COW logic
  vma->SetCOW(true);

  return vma;
}

void MemoryManager::CopyPmaForWrite(Vma *vma, Pma **pma)
{
  if (!vma->GetCOW())
  {
    DPRINTF("Cannot split page which isn't COW'd page\n");
    return;
  }

  *pma = new Pma(vma->GetLength());

  void *original_h_addr = vma->GetHostAddr();
  if (original_h_addr == NULL)
  {
    DPRINTF("Mapping in original h_addr for COW copy\n");
    original_h_addr = MapVmaIntoHost(vma);
    if (original_h_addr == NULL)
    {
      DPRINTF("Failed to map in original PMA\n");
      return;
    }
  }

  long new_off = FindBackingMemSpace(vma->GetLength());
  if (new_off < 0)
  {
    DPRINTF("Failed to allocate underlying space for new PMA\n");
    return;
  }
  (*pma)->SetBackingOffset(new_off);

  void *new_h_addr = MapPmaIntoHost(*pma, vma->GetProt());
  if (new_h_addr == NULL)
  {
    DPRINTF("Failed to map in new VMA\n");
    return;
  }

  memcpy(new_h_addr, original_h_addr, vma->GetLength());
  DPRINTF("COW: New PMA region copied new: %p, old: %p (pma %lx)!\n",
          new_h_addr,
          original_h_addr,
          new_off);

  return;
}

uint64_t MemoryManager::NextFreeAddr(size_t length)
{
  Vma *vma = NULL;

  uint64_t guess = m_last_allocated;
  do
  {
    vma = VmaForAddr((void *)guess);
    if (vma)
    {
      // jump past the whole vma
      guess += vma->GetLength();
    }
    if (vma == NULL)
    {
      // this is really ugly logic to check the whole range...
      size_t i = 1;
      size_t chunks = length / NIVISOR_PAGE_SIZE;
      DPRINTF("Inspecting %lx chunks for %lx request\n", chunks, length);
      for(i = 1; i < chunks; i++)
      {
        vma = VmaForAddr((void *)(guess + i * NIVISOR_PAGE_SIZE));
        if (vma)
        {
          guess += vma->GetLength();
          break;
        }
      }
      // vma should remain NULL if we didn't find a conflict
    }
  }
  while (vma && guess < m_upper_bound);

  return guess;
}

// dangerous as it returns a raw pointer, should be private
Vma *MemoryManager::VmaForAddr(void *addr)
{
  uint64_t a_addr = ((uint64_t)addr) & ~(NIVISOR_PAGE_SIZE-1);
  DPRINTF("Looking for VMA at %lx\n", a_addr);

  // fast path
  Vma *vma = NULL;

  auto e = m_vmas.find(a_addr);
  if (e == m_vmas.end())
  {
    // slow path
    auto it = m_vmas.begin();
    for (it; it != m_vmas.end(); it++)
    {
      std::unique_ptr<Vma> & v = it->second;
      uint64_t start = v->GetAddr();
      uint64_t end = start + v->GetLength();
      if (a_addr > start && a_addr < end)
      {
        vma = v.get();
        break;
      }
    }
  }
  else
  {
    vma = e->second.get();
  }

  return vma;
}

void *MemoryManager::MapVmaIntoHost(Vma *vma)
{
  DPRINTF("Creating host-side map for %lx %lx\n",
          vma->GetAddr(), vma->GetBackingOffset());

  return MapPmaIntoHost(vma->GetPma().get(), vma->GetProt());
}

void *MemoryManager::MapPmaIntoHost(Pma *pma, int prot)
{
  void *h_addr = NULL;

  if (!pma->MappedInBacking())
  {
    long new_off = FindBackingMemSpace(pma->GetLength());
    if (new_off < 0)
    {
      DPRINTF("Failed to allocate underlying PMA memory for host mapping\n");
      return NULL;
    }
    pma->SetBackingOffset(new_off);
  }

  h_addr = mmap(NULL,
                pma->GetLength(),
                PROT_READ|PROT_WRITE|PROT_EXEC, // TODO, are we allowing guests to insert EXEC mem?
                MAP_SHARED, // TODO, always ignore vma->flags? probably
                GetMemFd(),
                pma->GetBackingOffset());

  if (h_addr == MAP_FAILED)
  {
    DPRINTF("Failed to map backing store region into host\n");
    return NULL;
  }

  pma->SetHostAddr(h_addr);

  return h_addr;
}

// Translate an address from user process address space into an
// address in the host which represents the underlying page
// addiitonally checks permissions of the requests VMA translation
// Note: page will be mapped into host address space if it's not already
void *MemoryManager::TranslateAddr(void *addr, int prot)
{
  Vma *vma = VmaForAddr(addr);
  if (vma == NULL)
  {
    DPRINTF("Failed to find Vma corresponding to %p\n", addr);
    return NULL;
  }

  if (!(vma->GetProt() & prot))
  {
    DPRINTF("Failed permission check translating addr %lx wanted %d had %d", vma->GetAddr(), prot, vma->GetProt());
    return NULL;
  }

  // if this is a COW mapping we need to update the mapping's
  // PMA
  if ((prot & PROT_WRITE) && vma->GetCOW())
  {
    DPRINTF("Write to COW page detected in syscall, copying..\n");
    Pma *pma = NULL;
    CopyPmaForWrite(vma, &pma);
    if (pma == NULL)
    {
      DPRINTF("Couldn't split PMA due to memory error!\n");
      return NULL;
    }
    vma->SetPma(pma);

    // turn off COW now that we're done
    vma->SetCOW(false);
  }

  void *h_addr = vma->GetHostAddr();
  if (h_addr == NULL)
  {
    h_addr = MapVmaIntoHost(vma);

    if (h_addr == NULL)
    {
      return NULL;
    }
  }

  size_t delta = (uint64_t)addr - vma->GetAddr();
  return (uint8_t *)h_addr + delta;
}

// returns the user address of the allocated VMA
// returns a negative number on failure
int64_t MemoryManager::NewVma(uint64_t addr, size_t length, int prot, int flags, int fd)
{
  uint64_t dest_addr = (uint64_t) addr;
  if (addr == 0)
  {
    dest_addr = NextFreeAddr(length);
    if (dest_addr == m_upper_bound)
    {
      return -NIVISOR_NO_SPACE;
    }
  }

  if (VmaForAddr((void *)dest_addr) != NULL)
  {
    return -NIVISOR_INVALID_ARG;
  }

  // if no addr was specified update the last allocated trait
  if (addr == 0)
  {
    m_last_allocated = dest_addr + length;
  }

  DPRINTF("Creating new VMA\n");
  std::unique_ptr<Vma> v = std::make_unique<Vma>(fd, (int64_t)dest_addr, length, flags, prot);

  m_vmas[(uint64_t)dest_addr] = std::move(v);

  return dest_addr;
}

// TODO UNTESTED
Vma *MemoryManager::VmaForPageIn(void *addr)
{
  uint64_t a_addr = ((uint64_t)addr) & ~(NIVISOR_PAGE_SIZE-1);
  DPRINTF("Looking for VMA at %lx\n", a_addr);

  Vma *vma = VmaForAddr(addr);
  // reflect that this is a real segfault, no page exists
  if (vma == NULL)
  {
    return NULL;
  }

  // TODO allow this to recycle old regions
  if (!vma->MappedInBacking())
  {
    long new_off = FindBackingMemSpace(vma->GetLength());
    if (new_off < 0)
    {
      DPRINTF("Failed to allocate underlying backing for VMA\n");
      return NULL;
    }
    vma->SetBackingOffset(new_off);
  }

  return vma;
}

long PhysMemoryManager::FindBackingMemSpace(size_t length)
{
  // TODO find holes in existing backing space
  long new_off = m_memfd_length;
  long new_end = m_memfd_length + length;

  if (new_end > m_max_usage)
  {
    DPRINTF("Request for memory exceeded max usage!\n");
    return -1;
  }

  DPRINTF("Expanding backing store %ld -> %ld\n", new_off, new_end);
  if (ftruncate(m_memfd, new_end) < 0)
  {
    DPRINTF("Failed to expand memory space for child: %s\n", strerror(errno));
    return -1;
  }

  m_memfd_length = new_end;

  return new_off;
}

void *MemoryManager::DoHostMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{

  // attempt to mmap the region
  void *out_addr = mmap(addr, length, prot, flags, fd, offset);
  if (out_addr == MAP_FAILED)
  {
    DPRINTF("out_addr: %p\n", MAP_FAILED);
    DPRINTF("Failed to mmap: %s\n", strerror(errno));
    return MAP_FAILED;
  }

  std::unique_ptr<Vma> v = std::make_unique<Vma>(fd, (uint64_t)out_addr, length, flags, prot);

  // Replace this with a call to the constructor
  v->SetHostAddr(out_addr);

  m_vmas[(uint64_t)out_addr] = std::move(v);

  return out_addr;
}

void *MemoryManager::DoHostAnonMmap(void *addr, size_t length, int prot, int flags)
{
  DPRINTF("DoHostAnonMmap\n");

  long offset = FindBackingMemSpace(length);
  if (offset < 0)
  {
    DPRINTF("Failed to allocate/find free room in the backing memory store\n");
    return MAP_FAILED;
  }

  void *out_addr = DoHostMmap(addr, length, prot, flags, GetMemFd(), offset);
  if (out_addr == MAP_FAILED)
  {
    DPRINTF("Failed to allocate space in the backing store\n");
    return MAP_FAILED;
  }

  // we know this must exist if DoMmap succeeded
  auto e = m_vmas.find((uint64_t)out_addr);

  // we just need to fix up the backing store
  e->second->SetBackingOffset(offset);

  return out_addr;
}

NIVISOR_STATUS MemoryManager::DoHostMprotect(uint64_t addr, size_t length, int new_prot)
{
  // find Vma from addr
  auto e = m_vmas.find(addr);
  if (e == m_vmas.end())
  {
    return NIVISOR_NO_ELEM;
  }

  // enforce that vma is the proper length
  std::unique_ptr<Vma> & v = e->second;
  if (v->GetLength() != length)
  {
    return NIVISOR_INVALID_ARG;
  }

  // call mprotect
  if (v->MprotectSelf(new_prot) != 0)
  {
    return NIVISOR_IO_ERROR;
  }

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS MemoryManager::DoHostMunmap(void *addr, size_t length)
{
  // find VMA from addr
  uint64_t addr_key = (uint64_t)addr;

  auto e = m_vmas.find(addr_key);
  if (e == m_vmas.end())
  {
    return NIVISOR_NO_ELEM;
  }

  std::unique_ptr<Vma> & v = e->second;
  if (v->GetLength() != length)
  {
    return NIVISOR_INVALID_ARG;
  }

  m_vmas.erase(addr_key);

  return NIVISOR_SUCCESS;
}

// Clear out all VMAs
NIVISOR_STATUS MemoryManager::Clear(ThreadContext *ctx)
{
  for (auto &[addr, vma]: m_vmas)
  {
    DPRINTF("Removing VMA %lx %lx %lx\n", addr, vma->GetLength(), vma->GetBackingOffset());

    int64_t _rc = 0;
    DPRINTF("Munmap %lx %lx\n", vma->GetAddr(), vma->GetLength());
    if (ctx->DoSyscall(SYS_munmap,
                       &_rc,
                       2,
                       vma->GetAddr(),
                       vma->GetLength()) != NIVISOR_SUCCESS)
    {
      DPRINTF("Failed to inject munmap syscall into child\n");
      return NIVISOR_IO_ERROR;
    }

    if (_rc != 0)
    {
      DPRINTF("Munmap failed %ld\n", _rc);
      return NIVISOR_IO_ERROR;
    }
  }

  m_vmas.clear();

  return NIVISOR_SUCCESS;
}

bool MemoryManager::COWifyVma(ThreadContext *ctx, Vma *vma)
{
  int64_t _rc = 0;

  // we only bother with this if the vma has been paged in
  if (vma->IsGuestMapped())
  {
    size_t i = 0;
    // realistically this would probably be a bitmap
    // that tracked mmap'd pages
    size_t region_pages = vma->GetLength() / NIVISOR_PAGE_SIZE;
    for (i = 0; i < region_pages; i++)
    {
      if (ctx->DoSyscall(SYS_mprotect,
                         &_rc,
                         3,
                         vma->GetAddr() + (i * NIVISOR_PAGE_SIZE),
                         NIVISOR_PAGE_SIZE,
                         PROT_READ) != NIVISOR_SUCCESS)
      {
        DPRINTF("Failed to inject mprotect syscall to set pages read-only\n");
        return false;
      }

      if (_rc < 0 && _rc != -ENOMEM)
      {
        DPRINTF("Mprotect (%lx) failed setting COW pages read-only: %s\n",
                vma->GetAddr(), strerror(abs(_rc)));
        return false;
      }

    }
    DPRINTF("Mprotected %lx %lx\n", vma->GetAddr(), vma->GetLength());
  }

  vma->SetCOW(true);

  return true;
}

void Vma::SetPma(Pma *pma)
{
  // set the new PMA pointer
  m_pma = std::shared_ptr<Pma>(pma);
}

// returns raw pointer, returned pointer must be immediately
// wrapped in smart pointer
MemoryManager *MemoryManager::CloneCOW(ThreadContext *parent, ThreadContext *child)
{
  MemoryManager *m = new MemoryManager();

  for (auto &[addr, vma]: m_vmas)
  {
    DPRINTF("Cloning VMA %lx %lx\n", addr, vma->GetLength());

    Vma *cloned_vma = vma->CloneCOW();

    // TODO set all pages to be PROT_READ so we can COW them
    if (!COWifyVma(parent, vma.get()))
    {
      DPRINTF("Failed to COWify parent's fork of page\n");
      return NULL;
    }

    if (!COWifyVma(child, cloned_vma))
    {
      DPRINTF("Failed to COWify child's fork of page\n");
      return NULL;
    }

    m->InsertVma(cloned_vma);
  }

  m->SetPhysMm(m_phys_mm);

  return m;
}

void MemoryManager::InsertVma(Vma *vma)
{
  m_vmas[vma->GetAddr()] = std::unique_ptr<Vma>(vma);
}
