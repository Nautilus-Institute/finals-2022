#pragma once

class BundleFs {
  public:
  BundleFs(char *root);

  int Open(char *path, int flags);

  private:
  char *m_root;
};
