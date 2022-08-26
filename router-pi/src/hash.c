#include <stdint.h>
#include <string.h>

uint64_t stupid_hash(char* s)
{
    uint64_t r = 0;
    for (int i = 0; i < strlen(s); ++i) {
        r = r ^ 0x47b000000000f55c;
        r <<= 8;
        r = (r << 32) | (r >> 32);
        r |= s[i];
        r = ~r;
    }
    return r;
}

