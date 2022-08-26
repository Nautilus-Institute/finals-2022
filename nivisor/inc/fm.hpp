#pragma once

#include <map>

#include "stdnivisor.h"

class FileManager {
public:
  FileManager()
    : m_last_sfd(0)
  {}

  ~FileManager()
  {
    DPRINTF("In FileManager destructor\n");
  }
  
  int PutFile(int fd);
  int GetFile(int sfd);
  NIVISOR_STATUS RemoveFile(int sfd);

private:
  int m_last_sfd;
  std::map<int, int> m_file_map;
};
