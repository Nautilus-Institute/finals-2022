#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "cookies.h"

// TODO: Support multi-user instances

#define MAXCOOKIES 64

cookie_t cookies[MAXCOOKIES] = { {0, 0} };
int cookie_index = 0;

void add_cookie(const char* name, const char* value)
{
    cookies[cookie_index].name = strdup(name);
    cookies[cookie_index].value = strdup(value);
    cookie_index++;
}

char* get_setcookie()
{
    char* buf = malloc(65536);
    memset(buf, 0, 65536);
    for (int i = 0; i < cookie_index; ++i) {
        strcat(buf, "Set-Cookie: ");
        strcat(buf, cookies[i].name);
        strcat(buf, "=");
        strcat(buf, cookies[i].value);
        strcat(buf, "\r\n");
    }
    // Free all cookies
    for (int i = 0; i < cookie_index; ++i) {
        free(cookies[i].name);
        free(cookies[i].value);
        cookies[i].name = NULL;
        cookies[i].value = NULL;
    }
    cookie_index = 0;
    return buf;
}

char* get_cookie(header_t* headers, int header_count, const char* name)
{
    // go through headers and find "Cookies"
    for (int i = 0; i < header_count; ++i) {
        header_t *h = &headers[i];
        if (!strcasecmp(h->name, "cookie")) {
            // found it!
            char* value = strdup(h->value);
            char* a = strtok(value, ";");
            while (a != NULL) {
                // remove prefixing spaces
                char *p = a;
                while (*p == ' ') {
                    ++p;
                }
                if (!strncmp(p, name, strlen(name))
                        && p[strlen(name)] == '=') {
                    char* b = strdup(p + strlen(name) + 1);
                    free(value);
                    return b;
                }
                a = strtok(NULL, ";");
            }
            free(value);
        }
    }
    return NULL;
}
