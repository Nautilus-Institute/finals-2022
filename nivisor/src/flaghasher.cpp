#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sha1.h"
#include "stdnivisor.h"
#include "flaghasher.hpp"

FlagHasher::FlagHasher()
{
  m_flag_fd = open(NIVISOR_FLAG_PATH, O_RDONLY);
  if (m_flag_fd < 0)
  {
    DPRINTF("Failed to open flag path: %s\n", NIVISOR_FLAG_PATH);
    abort();
  }
}

NIVISOR_STATUS FlagHasher::GenerateHash(unsigned char *out_hash)
{
  SHA1_CTX sha;
  char flag_data[128] = {0};
  size_t amt_read = read(m_flag_fd, flag_data, sizeof(flag_data));
  if (amt_read < 0)
  {
    DPRINTF("Failed to read flag data for hash!\n");
    return NIVISOR_IO_ERROR;
  }

  size_t i = 0;
  unsigned char *data_to_hash = (unsigned char *)flag_data;
  size_t data_to_hash_len = amt_read;

  unsigned char out_data[20];
  for (i = 0; i < 500; i++)
  {
    SHA1Init(&sha);

    SHA1Update(&sha,
               (const unsigned char *)data_to_hash,
               data_to_hash_len);

    SHA1Final(out_data, &sha);

    data_to_hash = out_data;
    data_to_hash_len = sizeof(out_data);
  }

  memset(flag_data, 0, sizeof(flag_data));
  memset(&sha, 0, sizeof(sha));

  memcpy(out_hash, out_data, sizeof(out_data));

  return NIVISOR_SUCCESS;
}
