#include <unistd.h>

#include "fm.hpp"
#include "stdnivisor.h"

int FileManager::PutFile(int fd)
{
  int next_sfd = m_last_sfd;

  m_file_map[next_sfd] = fd;

  m_last_sfd++;

  return next_sfd;
}

int FileManager::GetFile(int sfd)
{
  auto f = m_file_map.find(sfd);
  if (f == m_file_map.end())
  {
    return -NIVISOR_NO_ELEM;
  }

  return f->second;
}

NIVISOR_STATUS FileManager::RemoveFile(int sfd)
{
  auto f = m_file_map.find(sfd);
  if (f == m_file_map.end())
  {
    return -NIVISOR_NO_ELEM;
  }

  int fd = f->second;

  close(fd);

  m_file_map.erase(sfd);

  return NIVISOR_SUCCESS;
}
