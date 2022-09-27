#include "cstring.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>

typedef struct CStringStruct
{
	int Type;	//0 = static, String is a pointer. 1 = allocated, &String is the actual data
	int Len;	//length of string
	char *String;	//either a pointer or actual data
} CStringStruct;

CString::CString()
{
	//create a list for incoming strings
	_Len = 0;
	_FinalString = 0;
}

CString::CString(char *NewString)
{
	//init with a string
	_Len = 0;
	_FinalString = 0;

	*this += NewString;
}

CString::CString(const char *NewString)
{
	CStringStruct *NewEntry;

	//init with a static string
	_Len = strlen(NewString);
	_FinalString = 0;

	//allocate a special entry to show it is static and add it to the list
	NewEntry = (CStringStruct *)malloc(sizeof(CStringStruct));
	NewEntry->Type = 0;
	NewEntry->Len = _Len;
	NewEntry->String = (char *)NewString;
	_Strings += NewEntry;
}

CString::~CString()
{
	//destroy and free any memory from our strings
	char *CurEntry;

	//first, free the final string if it exists
	if(_FinalString)
		free(_FinalString);

	//each of the list entries are copies of the original incoming string so free each
	while(_Strings.Count())
	{
		CurEntry = (char *)_Strings.RemoveItemByID(0);
		free(CurEntry);
	}
}

void CString::operator+=(const char *Str)
{
	int StrLen;
	CStringStruct *NewEntry;

	//if we have a final string then get rid of it so get will regenerate
	if(_FinalString)
	{
		free(_FinalString);
		_FinalString = 0;
	}

	//add str to our list
	StrLen = strlen(Str);

	//update our length
	_Len += StrLen;

	//allocate an entry to show it is allocated and add it to the list
	NewEntry = (CStringStruct *)malloc(StrLen + 8);	//+4 for type flag at beginning, +4 for length
	NewEntry->Type = 1;
	NewEntry->Len = StrLen;
	memcpy(&NewEntry->String, Str, StrLen);

	//add to the list
	_Strings += NewEntry;
}

void CString::operator+=(const CString* NewString)
{
	size_t NewStrCount;
	int i;
	int CopySize;
	CStringStruct *CurEntry;
	CStringStruct *NewEntry;
	List *NewStringList;

	//if we have a final string then get rid of it so get will regenerate
	if(_FinalString)
	{
		free(_FinalString);
		_FinalString = 0;
	}

	//cycle through the list copying what we need to copy
	NewStringList = (List *)&(NewString->_Strings);
	NewStrCount = NewStringList->Count();
	for(i = 0; i < NewStrCount; i++)
	{
		//get an entry and check it for static or dynamic
		CurEntry = (CStringStruct *)(*NewStringList)[i];
		if(CurEntry->Type == 0)
		{
			//allocate a proper size block and copy the original
			NewEntry = (CStringStruct *)malloc(sizeof(CStringStruct));
			CopySize = sizeof(CStringStruct);
		}
		else
		{
			//dynamic, allocate a proper size to copy all data into
			CopySize = CurEntry->Len + 8;
			NewEntry = (CStringStruct *)malloc(CopySize);
		}

		//copy then add it
		memcpy(NewEntry, CurEntry, CopySize);
		_Strings += NewEntry;
	}

	//update our length
	_Len += NewString->_Len;
}

void CString::operator+=(size_t Value)
{
	int StrLen;
	char ValueBuffer[32];
	CStringStruct *NewEntry;

	//if we have a final string then get rid of it so get will regenerate
	if(_FinalString)
	{
		free(_FinalString);
		_FinalString = 0;
	}

	sprintf(ValueBuffer, "%ld", Value);

	//add str to our list
	StrLen = strlen(ValueBuffer);

	//update our length
	_Len += StrLen;

	//allocate an entry to show it is allocated and add it to the list
	NewEntry = (CStringStruct *)malloc(StrLen + 8);	//+4 for type flag at beginning, +4 for length
	NewEntry->Type = 1;
	NewEntry->Len = StrLen;
	memcpy(&NewEntry->String, ValueBuffer, StrLen);

	//add to the list
	_Strings += NewEntry;
}

const char *CString::Get()
{
	char *NewStr;
	int i, Count;
	char *NewStrPos;
	CStringStruct *CurEntry;

	//get a final string
	if(_FinalString)
		return _FinalString;

	//no final string, generate one
	NewStr = (char *)malloc(_Len + 1);
	NewStrPos = NewStr;

	//loop through finding all strings
	Count = _Strings.Count();
	for(i = 0; i < Count; i++)
	{
		//get the string
		CurEntry = (CStringStruct *)_Strings[i];

		//copy it in
		if(CurEntry->Type == 0)
			memcpy(NewStrPos, CurEntry->String, CurEntry->Len);
		else
			memcpy(NewStrPos, &CurEntry->String, CurEntry->Len);

		//advance our pointer
		NewStrPos += CurEntry->Len;
	}

	//make sure it is null terminated
	*NewStrPos = 0;

	//store off our allocation and return it
	_FinalString = NewStr;
	return NewStr;
}

unsigned int CString::Len()
{
	return _Len;
}

void CString::Clear()
{
	char *CurEntry;

	_Len = 0;
	if(_FinalString)
		free(_FinalString);
	_FinalString = 0;

	//erase all entries
	while(_Strings.Count())
	{
		CurEntry = (char *)_Strings.RemoveItemByID(0);
		free(CurEntry);
	}
}