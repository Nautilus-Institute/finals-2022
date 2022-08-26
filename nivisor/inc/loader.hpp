#pragma once

#include <memory>

#include <stdint.h>
#include "stdnivisor.h"

class ThreadContext;

typedef struct stack_vector_t {
  uint32_t argc;
  char **argv;
  char *args[];
} stack_vector_t;

// give a clue to players by replicating the real ELF value
#define NIVISOR_AT_NULL   0
#define NIVISOR_AT_RANDOM 25

typedef struct auxv_t {
  uint32_t a_type;
  uint64_t a_value;
} auxv_t;

// aout defines
#define AOUT_OMAGIC 0407
#define AOUT_NMAGIC 0410
#define AOUT_ZMAGIC 0413
#define AOUT_QMAGIC 0314

#define AOUT_M_386   100

typedef struct aouthdr_t {
  uint32_t  a_midmag;
  uint32_t  a_text;
  uint32_t  a_data;
  uint32_t  a_bss;
  uint32_t  a_syms;
  uint32_t  a_entry;
  uint32_t  a_trsize;
  uint32_t  a_drsize;
} aouthdr_t;

class Loader {
public:
  Loader()
    : m_loaderror(NIVISOR_SUCCESS)
  {};
  virtual NIVISOR_STATUS Load(ThreadContext & ctx, int exe_fd) { return NIVISOR_NOT_IMPL; };
  virtual NIVISOR_STATUS GetEntrypointRegState(struct user_regs_struct *regs) { return NIVISOR_NOT_IMPL; };
  NIVISOR_STATUS SetEntrypointState(ThreadContext *ctx);
  NIVISOR_STATUS GetLoadError() { return m_loaderror; };

protected:
  NIVISOR_STATUS m_loaderror;
};

class LoaderAout : public Loader {
public:
  LoaderAout()
    : Loader(),
      m_entrypoint(0),
      m_text(0),
      m_data(0),
      m_syms(0)
  {};
  NIVISOR_STATUS Load(ThreadContext & ctx, int exe_fd) override;
  NIVISOR_STATUS GetEntrypointRegState(struct user_regs_struct *regs) override;

private:
  uint64_t m_entrypoint;
  uint64_t m_text;
  uint64_t m_data;
  uint64_t m_syms;
};

#define MAX_RELOCS 1024
#define BLUE_MAGIC 0x424c5545
#define NUM_SECTIONS 3
#define INTERP_MASK 0x200

#define PAGE_ROUND_UP(x) ( (((uint64_t)(x)) + PAGE_SIZE-1)  & (~(PAGE_SIZE-1)) ) 
#define BITS_TO_BYTES(x) ( x >> 3 )

static char const* SECTION_TYPE_TO_STRING[3] = {
  "RODATA",
  "DATA",
  "TEXT"
};

enum SECTION_TYPE {
  RODATA = 0,
  DATA = 1,
  TEXT = 2,
  MAX_SECTION
};

struct __attribute__((__packed__)) reloc {
  // TODO: Can this also be negative?
  int64_t patch_offset;
  enum SECTION_TYPE patch_section;
  int64_t value_offset;
  enum SECTION_TYPE value_section;
  uint8_t size;
};

struct __attribute__((__packed__)) additional_info {
  char linker_path[128];
  uint32_t version;
};

class LoaderBlue : public Loader {
public:
  NIVISOR_STATUS Load(ThreadContext & ctx, int exe_fd) override;
  NIVISOR_STATUS GetEntrypointRegState(struct user_regs_struct *regs) override;
  int read_header(ThreadContext &ctx, int fd);
  int size_for_section(enum SECTION_TYPE section, uint64_t *size);
  int alloc_section(ThreadContext &ctx, enum SECTION_TYPE section, uint64_t vaddr, int flags, int prot);
  int alloc_sections(ThreadContext &ctx);
  int read_section(int fd, enum SECTION_TYPE section);
  int read_sections(int fd);
  int change_section_perm(ThreadContext &ctx, enum SECTION_TYPE section, int prot);
  int fixup_sections(ThreadContext &ctx);

private:
  uint32_t m_magic;
  uint32_t m_flags;
  uint64_t m_text_size;
  uint64_t m_rodata_size;
  uint64_t m_data_size;
  uint64_t m_entrypoint;
  uint64_t m_num_relocs;
  struct additional_info m_info;
  struct reloc m_relocs[MAX_RELOCS];
  void *m_sections[NUM_SECTIONS];

  // non file members
  int m_exe_fd;
};

#define NIVISOR_PAGE_ALIGN(x) (((x + NIVISOR_PAGE_SIZE) >> 12) << 12)

std::unique_ptr<Loader> LoadBinary(ThreadContext &, char *);
