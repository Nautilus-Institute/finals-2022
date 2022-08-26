#ifndef _DNS_RESOLVE_H
#define _DNS_RESOLVE_H

#include <stdint.h>
#include <string.h>


typedef struct nameservers_t {
  size_t i, n;
  struct sockaddr_in *p;
} Nameservers;

typedef struct resolvconf_t {
  Nameservers nameservers;
} ResolvConf;

int resolve_dns_reverse(const ResolvConf *resolvconf, int af, const char *name, char *buf, size_t bufsize);

#endif
