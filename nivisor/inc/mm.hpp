#pragma once

#include <map>
#include <memory>

#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>

#include "stdnivisor.h"

#define NIVISOR_PAGE_SIZE 0x1000
#define NIVISOR_PAGE_SHIFT 12
#define NIVISOR_MAX_MAP_LENGTH 0x1000000

#define NIVISOR_STACK_SIZE 0x40000
#define NIVISOR_STACK_ADDR 0x7dfff0000 - NIVISOR_STACK_SIZE
#define NIVISOR_STACK_ALIGN(x) (((x + 16) >> 4) << 4)

#define NIVISOR_MAX_SYSTEM_MEM 0x6400000

class ThreadContext;

// Generic Memory Area abstract class
class Ma {
public:
  Ma() {};

  virtual ~Ma() {};
};

// physically mapped area
class Pma : public Ma {
 public:
  Pma(uint64_t length)
    : m_backing_off(-1),
      m_host_addr(NULL),
      m_length(length)
  {
    DPRINTF("Pma Allocated\n");
  }
  virtual ~Pma()
  {
    DPRINTF("In Pma destructor\n");
    munmap(m_host_addr, m_length);
  }

  long GetBackingOffset() { return m_backing_off; }
  void *GetHostAddr() { return m_host_addr; }
  int64_t GetLength() { return m_length; }

  void SetBackingOffset(long off) { m_backing_off = off; }
  void SetHostAddr(void *host_addr) { m_host_addr = host_addr; }

  bool MappedInBacking() { return m_backing_off >= 0; }

  void CloneCOW();

 private:
  long m_backing_off;
  void *m_host_addr;
  uint64_t m_length;
};

class PhysMemoryManager {
 public:
  PhysMemoryManager(int memfd, size_t max_usage)
    : m_memfd(memfd),
      m_memfd_length(0),
      m_max_usage(max_usage)
  {};
  ~PhysMemoryManager()
  {
    DPRINTF("In PhysMemoryManager destructor\n");
    close(m_memfd);
  }

  int GetMemFd() { return m_memfd; }
  long FindBackingMemSpace(size_t length);

 private:
  int m_memfd;
  size_t m_memfd_length;
  size_t m_max_usage;
};

// virtually mapped area
class Vma : public Ma {
 public:
  Vma(int fd, uint64_t start, uint64_t len, int flags, int prot)
    : m_file_descriptor(fd),
      m_vaddr_start(start),
      m_region_len(len),
      m_flags(flags),
      m_prot(prot),
      m_cow(false),
      m_pma(std::make_shared<Pma>(len)),
      m_faulted_in(false)
  {}
  virtual ~Vma()
  {
    DPRINTF("In Vma destructor\n");
    //munmap((void *)m_vaddr_start, m_region_len);
  }

  uint64_t GetAddr() { return m_vaddr_start; }
  uint64_t GetLength() { return m_region_len; }
  int GetFlags() { return m_flags; }
  int GetProt() { return m_prot; }
  int GetFd() { return m_file_descriptor; }
  long GetBackingOffset() { return m_pma->GetBackingOffset(); }
  void *GetHostAddr() { return m_pma->GetHostAddr(); }
  std::shared_ptr<Pma> GetPma() { return m_pma; }

  void SetPma(Pma *pma);
  void SetBackingOffset(long off) { m_pma->SetBackingOffset(off); }
  void SetHostAddr(void *host_addr) { m_pma->SetHostAddr(host_addr); }
  void SetProt(int prot) { m_prot = prot; }

  Vma *CloneCOW();
  void SetCOW(bool cow) { m_cow = cow; }
  bool GetCOW() { return m_cow; }

  int MprotectSelf(int new_prot) {
    m_prot = new_prot;
    return mprotect((void *)m_vaddr_start, m_region_len, new_prot);
  }

  bool MappedInBacking() { return m_pma->MappedInBacking(); }

  void SetGuestMapped(bool faulted_in) { m_faulted_in = faulted_in; }
  bool IsGuestMapped() { return m_faulted_in; }

 private:
  int m_file_descriptor;
  uint64_t m_vaddr_start;
  uint64_t m_region_len;
  int m_flags;
  int m_prot;
  bool m_faulted_in;

  std::shared_ptr<Pma> m_pma;

  bool m_cow;
};

class MemoryManager {
 public:
  MemoryManager();
  ~MemoryManager();

  int GetMemFd() { return m_phys_mm->GetMemFd(); }
  void SetPhysMm(std::shared_ptr<PhysMemoryManager> phys_mm)
  {
    m_phys_mm = phys_mm;
  }

  int64_t NewVma(uint64_t addr, size_t length, int prot, int flags, int fd);
  Vma *VmaForPageIn(void *addr);
  Vma *VmaForAddr(void *addr);

  long FindBackingMemSpace(size_t length)
  {
      return m_phys_mm->FindBackingMemSpace(length);
  }

  void *TranslateAddr(void *addr, int prot);
  
  void *DoHostMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
  void *DoHostAnonMmap(void *addr, size_t length, int prot, int flags);
  NIVISOR_STATUS DoHostMprotect(uint64_t addr, size_t length, int new_prot);
  NIVISOR_STATUS DoHostMunmap(void *addr, size_t length);
  NIVISOR_STATUS Clear(ThreadContext *ctx);

  void CopyPmaForWrite(Vma *vma, Pma **pma);
  bool COWifyVma(ThreadContext *ctx, Vma *vma);
  MemoryManager *CloneCOW(ThreadContext *parent, ThreadContext *child);

 protected:
  void InsertVma(Vma *vma);

 private:
  void *MapVmaIntoHost(Vma *vma);
  void *MapPmaIntoHost(Pma *pma, int prot);
  uint64_t NextFreeAddr(size_t length);
  
  // for memory file operations
  std::shared_ptr<PhysMemoryManager> m_phys_mm;

  uint64_t m_lower_bound;
  uint64_t m_upper_bound;
  uint64_t m_last_allocated;
  size_t m_usage;
  size_t m_max_usage;
  std::map<uint64_t, std::unique_ptr<Vma>> m_vmas;
};
