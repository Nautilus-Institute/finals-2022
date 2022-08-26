#ifndef PERPLEXITY_CSTRING
#define PERPLEXITY_CSTRING

#include "list.h"
#include "stddef.h"

typedef class CString
{
	public:
		CString();
		CString(const char *str);
		CString(char *str);
		~CString();
		void operator+=(const char *str);
		void operator+=(const CString *str);
		void operator+=(size_t Value);
		const char *Get();
		unsigned int Len();
		void Clear();
	
	private:
		char *_FinalString;
		int _Len;
		List _Strings;
} CString;

#endif
