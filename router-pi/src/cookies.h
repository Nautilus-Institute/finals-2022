#include "common.h"

typedef struct {
    char *name;
    char *value;
} cookie_t;

void add_cookie(const char* name, const char* value);
char* get_setcookie();
char* get_cookie(header_t* headers, int header_count, const char* name);
