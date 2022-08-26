#pragma once

#include <unistd.h>

#define NIVISOR_STR_VALUE(arg) #arg
#define NIVISOR_NAME(name) NIVISOR_STR_VALUE(name)

#ifndef FLAG_PATH
#define FLAG_PATH ./flag
#endif

#define NIVISOR_FLAG_PATH NIVISOR_NAME(FLAG_PATH)

// Maybe at /home/service/naersk/flag

class FlagHasher {
public:
  FlagHasher();

  ~FlagHasher()
  {
    DPRINTF("In FlagHasher destructor\n");
    close(m_flag_fd);
  }

  NIVISOR_STATUS GenerateHash(unsigned char *out_hash);

private:
  int m_flag_fd;
};
