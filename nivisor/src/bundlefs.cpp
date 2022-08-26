#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bundlefs.hpp"
#include "stdnivisor.h"

BundleFs::BundleFs(char *root)
{
  char resolved_path[PATH_MAX];

  if (realpath(root, resolved_path) == NULL)
  {
    DPRINTF("failed to resolve path in BundleFs constructor\n");
    abort();
  }

  m_root = strdup(resolved_path);
}

int BundleFs::Open(char *path, int flags)
{
  int fd = -1;

  if (strlen(path) > PATH_MAX)
  {
    DPRINTF("path longer than PATH_MAX\n");
    return -1;
  }

  char *path_copy = strdup(path);
  if (!path_copy)
  {
    DPRINTF("strdup failed\n");
    return -1;
  }

  char complete_path[PATH_MAX + PATH_MAX];
  if (snprintf(complete_path, sizeof(complete_path), "%s/%s", m_root, path) < 0)
  {
    DPRINTF("failed to print into path\n");
    goto out;
  }      

  DPRINTF("Appended path %s\n", complete_path);

  char resolved_path[PATH_MAX];
  if (realpath(complete_path, resolved_path) == NULL)
  {
    DPRINTF("failed to resolve path\n");
    goto out;
  }      

  if (strncmp(m_root, resolved_path, strlen(m_root)) != 0)
  {
    DPRINTF("Detected attempt to read outside rootfs, denying\n");
    goto out;
  }

  DPRINTF("Opening path %s\n", resolved_path);
  fd = open(resolved_path, flags);

 out:
  free(path_copy);
  return fd;
}
