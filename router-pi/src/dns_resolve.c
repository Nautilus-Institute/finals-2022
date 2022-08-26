// Stolen from https://github.com/jart/cosmopolitan

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "dns_resolve.h"

#define DNS_TYPE_A     1
#define DNS_TYPE_NS    2
#define DNS_TYPE_CNAME 5
#define DNS_TYPE_SOA   6
#define DNS_TYPE_PTR   12
#define DNS_TYPE_MX    15
#define DNS_TYPE_TXT   16

#define DNS_CLASS_IN 1

#define DNS_NAME_MAX  256
#define DNS_LABEL_MAX  256
#define READ16BE(S) ((255 & (S)[0]) << 8 | (255 & (S)[1]))

typedef struct dnsheader_t {
  uint16_t id;      /* transaction id */
  uint8_t bf1;      /* bit field 1 */
  uint8_t bf2;      /* bit field 2 */
  uint16_t qdcount; /* question count */
  uint16_t ancount; /* answer count */
  uint16_t nscount; /* nameserver count */
  uint16_t arcount; /* additional record count */
} dnsheader;

typedef struct dnsquestion_t {
  const char *qname;
  uint16_t qtype;
  uint16_t qclass;
} dnsquestion;


static void serialize_dns_header(uint8_t p[restrict 12], const struct dnsheader_t *h)
{
  p[0x0] = h->id >> 8;
  p[0x1] = h->id;
  p[0x2] = h->bf1;
  p[0x3] = h->bf2;
  p[0x4] = h->qdcount >> 8;
  p[0x5] = h->qdcount;
  p[0x6] = h->ancount >> 8;
  p[0x7] = h->ancount;
  p[0x8] = h->nscount >> 8;
  p[0x9] = h->nscount;
  p[0xa] = h->arcount >> 8;
  p[0xb] = h->arcount;
}

static void deserialize_dns_header(struct dnsheader_t *h, const uint8_t p[restrict 12]) {
  h->id = READ16BE(p);
  h->bf1 = p[2];
  h->bf2 = p[3];
  h->qdcount = READ16BE(p + 4);
  h->ancount = READ16BE(p + 6);
  h->nscount = READ16BE(p + 8);
  h->arcount = READ16BE(p + 10);
}

static int pascalify_dns_name(uint8_t *buf, size_t size, const char *name)
{
  size_t i, j, k, namelen;
  if ((namelen = strlen(name)) > DNS_NAME_MAX) return -1;
  i = 0;
  if (size || namelen) {
    if (namelen + 1 > size) return -2;
    buf[0] = '\0';
    j = 0;
    for (;;) {
      for (k = 0; name[j + k] && name[j + k] != '.'; ++k) {
        buf[i + k + 1] = name[j + k];
      }
      if (k) {
        if (k > DNS_LABEL_MAX) return -1;
        buf[i] = k;
        i += k + 1;
      }
      j += k + 1;
      if (!name[j - 1]) {
        break;
      }
    }
    buf[i] = '\0';
  }
  return i;
}

int serialize_dns_question(uint8_t *buf, size_t size, const struct dnsquestion_t *dq)
{
  int wrote;
  if ((wrote = pascalify_dns_name(buf, size, dq->qname)) == -1) return -1;
  if (wrote + 1 + 4 > size) return -2;
  buf[wrote + 1] = dq->qtype >> 8;
  buf[wrote + 2] = dq->qtype;
  buf[wrote + 3] = dq->qclass >> 8;
  buf[wrote + 4] = dq->qclass;
  return wrote + 5;
}

int resolve_dns_reverse(const ResolvConf *resolvconf, int af, const char *name, char *buf, size_t bufsize)
{
  int rc, fd, n;
  struct dnsquestion_t q;
  struct dnsheader_t h, h2;
  uint8_t *p, *pe, msg[512];
  uint16_t rtype, rclass, rdlength;
  if (af != AF_INET && af != AF_UNSPEC) return -1;
  if (!resolvconf->nameservers.n) return 0;
  bzero(&h, sizeof(h));
  rc = -2;
  h.id = ((uint64_t)rand() << 32) | (rand());
  h.bf1 = 1; /* recursion desired */
  h.qdcount = 1;
  q.qname = name;
  q.qtype = DNS_TYPE_PTR;
  q.qclass = DNS_CLASS_IN;
  bzero(msg, sizeof(msg));
  serialize_dns_header(msg, &h);
  if ((n = serialize_dns_question(msg + 12, 500, &q)) == -1) return -1;
  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) return -1;
  if (sendto(fd, msg, 12 + n, 0, resolvconf->nameservers.p,
             sizeof(*resolvconf->nameservers.p)) == 12 + n &&
      (n = read(fd, msg, 512)) >= 12) {
    deserialize_dns_header(&h2, msg);
    if (h2.id == h.id) {
      rc = 0;
      if (h2.ancount) {
        p = msg + 12;
        pe = msg + n;
        while (p < pe && h2.qdcount) {
          p += strnlen((char *)p, pe - p) + 1 + 4;
          h2.qdcount--;
        }
        if (p + 1 < pe) {
          if ((p[0] & 0b11000000) == 0b11000000) { /* name pointer */
            p += 2;
          } else {
            p += strnlen((char *)p, pe - p) + 1;
          }
          if (p + 2 + 2 + 4 + 2 < pe) {
            rtype = READ16BE(p), p += 2;
            rclass = READ16BE(p), p += 2;
            /* ttl */ p += 4;
            rdlength = READ16BE(p), p += 2;
            if (p + rdlength <= pe && rtype == DNS_TYPE_PTR &&
                rclass == DNS_CLASS_IN) {
              if (strnlen((char *)p, pe - p) + 1 > bufsize) {
#ifdef VULN
                  // We do the real evil thing of only storing the first `bufsize` characters later
                  for (; !isalnum((char)(*p)) && p < pe; p++) rdlength--;
                  for (char *tmp = (char *)p; rdlength > 0 && *tmp != '\0';
                       tmp++) {
                    /* each label is alphanumeric or hyphen
                     * any other character is assumed separator */
                    if (!isalnum(*tmp) && *tmp != '-') *tmp = '.';
                    rdlength--;
                  }
                  strncpy(buf, (char *)p, bufsize);
#ifdef DEBUG
                  fprintf(stderr, "resolved: %s\n", buf);
#endif
#else
                rc = -1;
#endif
              }
              else {
                /* domain name starts with a letter */
                for (; !isalnum((char)(*p)) && p < pe; p++) rdlength--;
                for (char *tmp = (char *)p; rdlength > 0 && *tmp != '\0';
                     tmp++) {
                  /* each label is alphanumeric or hyphen
                   * any other character is assumed separator */
                  if (!isalnum(*tmp) && *tmp != '-') *tmp = '.';
                  rdlength--;
                }
                strcpy(buf, (char *)p);
              }
            } else
              rc = -1;
          }
        }
      }
    }
  }
  close(fd);
  return rc;
}
