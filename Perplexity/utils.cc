#include "perplexity.h"
#include <unistd.h>
#include <stdarg.h>

void WriteString(const char *Msg)
{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-result"
	write(1, Msg, strlen(Msg));
	fflush(stdout);
	#pragma GCC diagnostic pop
}

void strtolower(char *Str)
{
	//lowercase a string
	while(*Str)
	{
		if((*Str >= 'A') && (*Str <= 'Z'))
			*Str = *Str + 0x20;
		Str++;
	};
}

int IsNumber(char *Str)
{
	//make sure this is a numeric value
	while(*Str)
	{
		if((*Str < '0') || (*Str > '9'))
			return 0;

		Str++;
	}

	return 1;
}

void Write_PrintF(const char *Msg, ...)
{
	va_list args;
	va_start(args, Msg);
	vprintf(Msg, args);
	va_end(args);
	fflush(stdout);
}