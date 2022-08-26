#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

static const char cb32[] =
	"abcdefghijklmnopqrstuvwxyz012345";
static const char cb32_ucase[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
static unsigned char rev32[256];
static int reverse_init = 0;

inline static void base32_reverse_init(void)
{
	int i;
	unsigned char c;

	if (!reverse_init) {
		memset(rev32, 0, 256);
		for (i = 0; i < 32; i++) {
			c = cb32[i];
			rev32[(int) c] = i;
			c = cb32_ucase[i];
			rev32[(int) c] = i;
		}
		reverse_init = 1;
	}
}

int b32_5to8(int in)
{
	return cb32[in & 31];
}

int b32_8to5(int in)
{
	base32_reverse_init();
	return rev32[in];
}

#define REV32(x) rev32[(int) (x)]

/*
 * Fills *buf with max. *buflen bytes, decoded from slen chars in *str.
 * Decoding stops early when *str contains \0.
 * Illegal encoded chars are assumed to decode to zero.
 *
 * NOTE: *buf space should be at least 1 byte _more_ than *buflen
 * to hold a trailing '\0' that is added (though *buf will usually
 * contain full-binary data).
 *
 * return value    : #bytes filled in buf   (excluding \0)
 */
static int base32_decode(void *buf, size_t *buflen, const char *str,
			 size_t slen)
{
	unsigned char *ubuf = (unsigned char *) buf;
	int iout = 0;	/* to-be-filled output byte */
	int iin = 0;	/* next input char to use in decoding */

	base32_reverse_init();

	/* Note: Don't bother to optimize manually. GCC optimizes
	   better(!) when using simplistic array indexing. */

	while (1) {
		if (iout >= *buflen || iin + 1 >= slen ||
		    str[iin] == '\0' || str[iin + 1] == '\0')
			break;
		ubuf[iout] = ((REV32(str[iin]) & 0x1f) << 3) |
			     ((REV32(str[iin + 1]) & 0x1c) >> 2);
		iin++;  		/* 0 used up, iin=1 */
		iout++;

		if (iout >= *buflen || iin + 2 >= slen ||
		    str[iin] == '\0' || str[iin + 1] == '\0' ||
		    str[iin + 2] == '\0')
			break;
		ubuf[iout] = ((REV32(str[iin]) & 0x03) << 6) |
			     ((REV32(str[iin + 1]) & 0x1f) << 1) |
			     ((REV32(str[iin + 2]) & 0x10) >> 4);
		iin += 2;  		/* 1,2 used up, iin=3 */
		iout++;

		if (iout >= *buflen || iin + 1 >= slen ||
		    str[iin] == '\0' || str[iin + 1] == '\0')
			break;
		ubuf[iout] = ((REV32(str[iin]) & 0x0f) << 4) |
			     ((REV32(str[iin + 1]) & 0x1e) >> 1);
		iin++;  		/* 3 used up, iin=4 */
		iout++;

		if (iout >= *buflen || iin + 2 >= slen ||
		    str[iin] == '\0' || str[iin + 1] == '\0' ||
		    str[iin + 2] == '\0')
			break;
		ubuf[iout] = ((REV32(str[iin]) & 0x01) << 7) |
			     ((REV32(str[iin + 1]) & 0x1f) << 2) |
			     ((REV32(str[iin + 2]) & 0x18) >> 3);
		iin += 2;  		/* 4,5 used up, iin=6 */
		iout++;

		if (iout >= *buflen || iin + 1 >= slen ||
		    str[iin] == '\0' || str[iin + 1] == '\0')
			break;
		ubuf[iout] = ((REV32(str[iin]) & 0x07) << 5) |
			     ((REV32(str[iin + 1]) & 0x1f));
		iin += 2;  		/* 6,7 used up, iin=8 */
		iout++;
	}

	ubuf[iout] = '\0';

	return iout;
}

void attempt_b32decode(char* buf)
{
    for (int i = 0; i < strlen(buf); ++i) {
        if (isalpha(buf[i]) || buf[i] == '=' || (buf[i] >= '2' && buf[i] <= '7')) {
            ;
        }
        else {
            // oops, not base32-encoded
            return;
        }
    }
    for (int i = 0; i < strlen(buf); ++i) {
        if (islower(buf[i])) {
            buf[i] = buf[i] - 32;
        }
    }
    char* out = malloc(strlen(buf));
    size_t out_len;
    base32_decode(out, &out_len, buf, strlen(buf));
#ifdef DEBUG
    fprintf(stderr, "Decoded: %s\n", out);
#endif
    strcpy(buf, out);
    free(out);
}
