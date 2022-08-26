#pragma once

#include "err.h"
#include <stdio.h>

#define NIVISOR_MAX_ARGS 128
#define NIVISOR_MAX_ARG_LEN 256

#define NIVISOR_MAX_PROCESSES 30

#define NIVISOR_CLONE_UNSHARE_VM 1
#define NIVISOR_CLONE_UNSHARE_FS 2

#define NIVISOR_READ   1
#define NIVISOR_WRITE  2
#define NIVISOR_CREATE 4

#ifdef DEBUG
#define DPRINTF(format, ...) do { \
  fprintf (stderr, "[%s] ", __func__); \
  fprintf (stderr, format __VA_OPT__(,) __VA_ARGS__); \
} while (0)
#else
#define DPRINTF(s, ...)
#endif

void fatal(const char *);
