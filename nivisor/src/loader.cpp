#include <memory>

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "ctx.hpp"
#include "loader.hpp"
#include "stdnivisor.h"

// I think loader can entirely be done in just C?
// it's only goal is to take a binary on disk and
// populate a ThreadContext and by extention a MemoryManager

void *MapAndLoadSegment(ThreadContext & ctx,
                        void *vaddr,
                        int exe_fd,
                        size_t length,
                        size_t aligned_length,
                        int flags,
                        int seg_prot = PROT_READ|PROT_WRITE)
{

  void *addr;

  addr = ctx.GetMm()->DoHostAnonMmap(vaddr, aligned_length, PROT_READ|PROT_WRITE, flags);
  if (addr == MAP_FAILED)
  {
    return MAP_FAILED;
  }

  // we assume these MapAndLoad calls are called in order
  if (read(exe_fd, addr, length) < length)
  {
    // Unmap on failure
    ctx.GetMm()->DoHostMunmap(addr, aligned_length);
    return MAP_FAILED;
  }

  if (seg_prot != (PROT_READ|PROT_WRITE)) {
    if (ctx.GetMm()->DoHostMprotect((uint64_t)addr, aligned_length, seg_prot) != NIVISOR_SUCCESS)
    {
    ctx.GetMm()->DoHostMunmap(addr, aligned_length);
    return MAP_FAILED;
    }
  }

  return addr;
}

NIVISOR_STATUS LoaderAout::Load(ThreadContext & ctx, int exe_fd)
{
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  aouthdr_t exec = {};
  void *text;
  uint32_t text_size;
  void *data;
  uint32_t data_size;
  void *syms;
  uint32_t syms_size;
  
  if (lseek(exe_fd, 0, SEEK_SET) == -1)
  {
    DPRINTF("Failed to reset executable's internal offset\n");
    rc = NIVISOR_IO_ERROR;
    goto out;
  }

  if (read(exe_fd, &exec, sizeof(aouthdr_t)) != sizeof(aouthdr_t))
  {
    DPRINTF("Failed to read in AOUT header\n");
    rc = NIVISOR_IO_ERROR;
    goto out;
  }

  if ((exec.a_midmag >> 16 & 0xff) != AOUT_M_386)
  {
    DPRINTF("Wrong machine time found in AOUT\n");
    rc = NIVISOR_INVALID_ARG;
    goto out;
  }

  DPRINTF("aout.a_midmag: %x\n", exec.a_midmag);
  DPRINTF("aout.a_text: %x\n", exec.a_text);  
  DPRINTF("aout.a_data: %x\n", exec.a_data);
  DPRINTF("aout.a_bss: %x\n", exec.a_bss);
  DPRINTF("aout.a_syms: %x\n", exec.a_syms);
  DPRINTF("aout.a_entry: %x\n", exec.a_entry);
  DPRINTF("aout.a_trsize: %x\n", exec.a_trsize);
  DPRINTF("aout.a_drsize: %x\n", exec.a_drsize);

  text_size = NIVISOR_PAGE_ALIGN(exec.a_text);
  data_size = NIVISOR_PAGE_ALIGN(exec.a_data);
  syms_size = NIVISOR_PAGE_ALIGN(exec.a_syms);

  // TODO, respect the magic of a_midmag and allocate text and data accordingly
  // TODO, these are anonymous mappings with the context of the file copied in
  // that means that these mappings are also precommitted. probably not a bit deal but not realistic
  rc = NIVISOR_FAILED_LOAD;
  if (exec.a_text > 0)
  {
    // TODO map a random address when AOUT loading is better
    text = MapAndLoadSegment(ctx, (void *)0x40000, exe_fd, exec.a_text, text_size, MAP_FIXED|MAP_SHARED, PROT_READ|PROT_EXEC);
    if (text == MAP_FAILED)
    {
      DPRINTF("Failed to map and load text segment\n");
      goto out;
    }
    m_text = (uint64_t)text;
  }

  if (exec.a_data > 0)
  {
    data = MapAndLoadSegment(ctx, NULL, exe_fd, exec.a_data, data_size, MAP_SHARED, PROT_READ|PROT_WRITE);
    if (data == MAP_FAILED)
    {
      DPRINTF("Failed to map and load data segment\n");
      goto out;
    }
    m_data = (uint64_t)data;
  }

  if (exec.a_syms > 0)
  {
    syms = MapAndLoadSegment(ctx, NULL, exe_fd, exec.a_syms, syms_size, MAP_SHARED, PROT_READ);
    if (syms == MAP_FAILED)
    {
      DPRINTF("Failed to map and load syms segment\n");
      goto out;
    }
    m_syms = (uint64_t)syms;
  }
  rc = NIVISOR_SUCCESS;

  DPRINTF("Loaded text at %p\n", text);
  DPRINTF("Loaded data at %p\n", data);
  DPRINTF("Loaded syms at %p\n", syms);

  m_entrypoint = (uint64_t)m_text;

 out:
  close(exe_fd);
  m_loaderror = rc;
  return rc;
}

NIVISOR_STATUS LoaderAout::GetEntrypointRegState(struct user_regs_struct *regs)
{

  regs->rip = m_entrypoint;

  // dumb convention for now
  // - rax is text pointer
  // - rbx is data
  // - rcx is syms
  // is this helpful to the application? no idea
  regs->rax = m_text;
  regs->rbx = m_data;
  regs->rcx = m_syms;

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS Loader::SetEntrypointState(ThreadContext *ctx)
{
  uint64_t cached_rsp = 0;
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;
  struct user_regs_struct regs = {0};

  rc = ctx->GetRegs(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to get regs\n");
    goto out;
  }

  regs.r15 = 0;
  regs.r14 = 0;
  regs.r13 = 0;
  regs.r12 = 0;
  regs.r11 = 0;
  regs.r10 = 0;
  regs.r9  = 0;
  regs.r8  = 0;
  regs.rax = 0;
  regs.rbx = 0;
  regs.rcx = 0;
  regs.rdx = 0;
  regs.rdi = 0;
  regs.rsi = 0;

  rc = GetEntrypointRegState(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to retrieve entrypoint register contents\n");
    goto out;
  }

  rc = ctx->SetRegs(&regs);
  if (rc != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed to set regs\n");
    goto out;
  }

 out:
  return rc;
}

std::unique_ptr<Loader> LoadBinary(ThreadContext & ctx, char *path)
{
  uint32_t magic = 0;
  uint16_t aout_magic = 0;
  std::unique_ptr<Loader> loader = NULL;
  
  // TODO: construct path from bundle OR assume path is already absolute in bundle
  // TODO: use bundle_open?
  int fd = ctx.GetFs()->Open(path, O_RDONLY);
  if (fd < 0)
  {
    DPRINTF("error opening path %s\n", path);
    return NULL;
  }

  if (read(fd, &magic, sizeof(magic)) < sizeof(magic))
  {
    DPRINTF("error reading magic from file %s\n", path);
    goto out;
  }

  DPRINTF("Magic %x\n", magic);
  if (magic == 0x464c457f)
  {
    DPRINTF("ELF not supported!\n");
    goto out;
  }

  // TODO: Do this in a reasonable way
  switch (magic) {
  case BLUE_MAGIC:
    loader = std::unique_ptr<Loader>(new LoaderBlue());
    loader->Load(ctx, fd);
    goto out;
  default:
    DPRINTF("No Blue\n");
    break;
  }

#ifdef AOUT_SUPPORT
  aout_magic = magic & 0xffff;
  switch (aout_magic)
  {
  case AOUT_OMAGIC:
  case AOUT_NMAGIC:
  case AOUT_ZMAGIC:
  case AOUT_QMAGIC:
    loader = std::unique_ptr<Loader>(new LoaderAout());
    loader->Load(ctx, fd);
    break;
  default:
    DPRINTF("Unrecognized executable file magic\n");
  }
#endif

 out:
  close(fd);
  return std::move(loader);
}


/* ######################### */
/* ### LOADER BLUE BEGIN ### */
/* ######################### */

NIVISOR_STATUS LoaderBlue::GetEntrypointRegState(struct user_regs_struct *regs) {
  NIVISOR_STATUS rc = NIVISOR_SUCCESS;

  regs->rip = m_entrypoint + (uint64_t)m_sections[TEXT];

  // dumb convention for now
  // - rax is text pointer
  // - rbx is data
  // - rcx is syms
  // is this helpful to the application? no idea
  regs->rax = (uint64_t)this->m_sections[TEXT];
  regs->rbx = (uint64_t)this->m_sections[DATA];

  // TODO set rsp and build an auxiliary vector

  return NIVISOR_SUCCESS;
}

NIVISOR_STATUS LoaderBlue::Load(ThreadContext & ctx, int fd) {
  // TODO: INIT MEMBER VARS
  //struct dong d;
  //memset(&d, 0, sizeof(struct dong));
  int err = 0;
  m_exe_fd = fd;

  if (this->read_header(ctx, fd)) {
    DPRINTF("read header failed\n");
    err = -1;
    goto exit;
  }
  DPRINTF("[+] Read header\n");

  if (this->alloc_sections(ctx)) {
    DPRINTF("alloc sections failed\n");
    err = -1;
    goto exit;
  }
  DPRINTF("[+] Allocated sections\n");

  if (this->fixup_sections(ctx)) {
    DPRINTF("fixup sections failed\n");
    err = -1;
    goto exit;
  }
  DPRINTF("[+] Applied relocations\n");

exit:
  if (err < 0)
  {
    m_loaderror = NIVISOR_FAILED_LOAD;
  }

  return err;
}

int LoaderBlue::read_header(ThreadContext &ctx, int fd) {
  int nread = 0;
  if ( (read(fd, &m_flags, sizeof(m_flags))) !=  sizeof(m_flags) ) return -1;
  if ( (read(fd, &m_text_size, sizeof(m_text_size))) !=  sizeof(m_text_size) ) return -1;
  if ( (read(fd, &m_rodata_size, sizeof(m_rodata_size))) !=  sizeof(m_rodata_size) ) return -1;
  if ( (read(fd, &m_data_size, sizeof(m_data_size))) !=  sizeof(m_data_size) ) return -1;
  if ( (read(fd, &m_entrypoint, sizeof(m_entrypoint))) !=  sizeof(m_entrypoint) ) return -1;
  if ( (read(fd, &m_num_relocs, sizeof(m_num_relocs))) !=  sizeof(m_num_relocs) ) return -1;

  if ( (read(fd, &m_relocs, sizeof(m_relocs))) != sizeof(m_relocs) ) return -1;

  if (this->m_num_relocs > MAX_RELOCS) {
    DPRINTF("Too many relocations specified\n");
    return -1;
  }

  if ( (read(fd, &m_info, sizeof(m_info))) !=  sizeof(m_info) ) return -1;

  return 0;
}

int LoaderBlue::size_for_section(enum SECTION_TYPE section, uint64_t *size) {
  switch (section) {
    case TEXT:
      *size = m_text_size;
      break;
    case RODATA:
      *size = m_rodata_size;
      break;
    case DATA:
      *size = m_data_size;
      break;
    default:
      DPRINTF("BAD SECTION in size_for_section\n");
      return -1;
  }
  return 0;
}

int LoaderBlue::alloc_section(ThreadContext &ctx, enum SECTION_TYPE section, uint64_t vaddr, int flags, int prot=PROT_READ|PROT_WRITE) {
  if (section >= MAX_SECTION) {
    DPRINTF("Bad section\n");
    return -1;
  }
  
  uint64_t size = 0;
  if (this->size_for_section(section, &size))    return -1;

  if (!size) {
    return 0;
  }

  uint64_t alligned_size = NIVISOR_PAGE_ALIGN(size);
  m_sections[section] = MapAndLoadSegment(ctx, (void *)vaddr, m_exe_fd, size, alligned_size, flags);
  
  if (m_sections[section] == MAP_FAILED) {
    DPRINTF("Failed to allocate %s\n", SECTION_TYPE_TO_STRING[section]);
    return -1;
  }

  return 0;
}

int LoaderBlue::alloc_sections(ThreadContext &ctx) {
  if (this->alloc_section(ctx, TEXT, 0, MAP_SHARED))
    return -1;

  if (this->alloc_section(ctx, RODATA, 0, MAP_SHARED))
    return -1;

  if (this->alloc_section(ctx, DATA, 0, MAP_SHARED))
    return -1;

  return 0;
}

int LoaderBlue::read_section(int fd, enum SECTION_TYPE section) {
  uint64_t size = 0;
  int nread = 0;

  if (this->size_for_section(section, &size)) return -1;

  DPRINTF("reading 0x%lx bytes into section %s\n", size, SECTION_TYPE_TO_STRING[section]);
  nread = read(fd, m_sections[section], size);
  if (nread != size) {
    DPRINTF("Failed to read all of %s\n", SECTION_TYPE_TO_STRING[section]);
    return -1;
  }

  return 0;
}

int LoaderBlue::change_section_perm(ThreadContext &ctx, enum SECTION_TYPE section, int prot) {
  uint64_t size = 0;
  if (this->size_for_section(section, &size))
  {
    DPRINTF("Failed to get section size\n");
    return -1;
  }

  if (size == 0)
  {
    DPRINTF("No such section %s, skipping...\n", SECTION_TYPE_TO_STRING[section]);
    return 0;
  }

  NIVISOR_STATUS status = \
    ctx.GetMm()->DoHostMprotect((uint64_t)m_sections[section],\
                                NIVISOR_PAGE_ALIGN(size),\
                                prot);

  if (status != NIVISOR_SUCCESS)
  {
    DPRINTF("Failed mprotect section %s: %d\n",
            SECTION_TYPE_TO_STRING[section],
            status);

    ctx.GetMm()->DoHostMunmap(m_sections[section], NIVISOR_PAGE_ALIGN(size));
    return -1;
  }

  return 0;
}

int LoaderBlue::fixup_sections(ThreadContext &ctx) {
/*
struct __attribute__((__packed__)) reloc {
  uint64_t patch_offset;
  enum SECTION_TYPE patch_section;
  uint64_t value_offset;
  enum SECTION_TYPE value_section;
  uint8_t size;
};
*/
  int i = 0;
  struct reloc *r;
  
  for (i=0; i < m_num_relocs; i++) {

    r = &m_relocs[i];
    // redundant 
    if (r->patch_section >= MAX_SECTION || r->value_section >= MAX_SECTION) {
      DPRINTF("Bad section in reloc\n");
      return -1;
    }

    if (r->size > 8) {
      DPRINTF("Unsupported relocation size: %d\n", r->size);
      return -1;
    }

    uint64_t patch_section_size = 0;
    uint64_t value_section_size = 0;
    if (this->size_for_section(r->patch_section, &patch_section_size))    return -1;
    if (this->size_for_section(r->value_section, &value_section_size))    return -1;


    DPRINTF("r->patch_section: %s\n", SECTION_TYPE_TO_STRING[r->patch_section]);
    DPRINTF("r->value_section: %s\n", SECTION_TYPE_TO_STRING[r->value_section]);


    uint64_t patch_addr = (uint64_t)m_sections[r->patch_section] + r->patch_offset;
    DPRINTF("r->patch_offset: 0x%08x\n", r->patch_offset);

    if (patch_addr + r->size > (uint64_t)m_sections[r->patch_section] + patch_section_size ||
        patch_addr < (uint64_t)m_sections[r->patch_section]) {
      DPRINTF("Bad patch addr: %p\n", patch_addr);
      return -1;
    }

    //uint64_t patch_value = patch_addr - (uint64_t)d->sections[r->value_section] + r->value_offset;
    uint64_t patch_value = (uint64_t)m_sections[r->value_section] + r->value_offset - patch_addr;

    DPRINTF("Patch value is: 0x%lx\n", patch_value);
    memcpy((void *)patch_addr, &patch_value, r->size);
  }

  if (this->change_section_perm(ctx, TEXT, PROT_NONE|PROT_EXEC|PROT_READ)) {
    DPRINTF("Failed to change permisions on TEXT\n");
    return -1;
  }

  if (this->change_section_perm(ctx, RODATA, PROT_NONE|PROT_READ)) {
    DPRINTF("Failed to change permisions on RODATA\n");
    return -1;
  }
  
  return 0;
}
