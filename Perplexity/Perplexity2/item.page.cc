#include "perplexity.h"
#include <unistd.h>

//////////
//Page
//////////

size_t Paper_PageCount = 0;

ItemPage::ItemPage() : Item("", "")
{
	_Data = 0;
	_DataLen = 0;
	_AllocSize = 0;
	_Crumpled = 0;

	//update our name with a page count
	Paper_PageCount++;
	_PageName += "page ";
	_PageName += Paper_PageCount;
	_Name = _PageName.Get();
	_PageNumber = Paper_PageCount;
}

ItemPage::~ItemPage()
{
	if(_Data)
		free(_Data);
}

void ItemPage::WriteOnPage()
{
	char Buffer[256];
	ssize_t ReadLen;
	char LastChar = 0;
	bool FoundEnd = false;

	Write_PrintF("Please provide what to add to the end of the page. End with two newlines.\n");

	//run in a loop, we avoid GetUserInput as it breaks up on spaces which doesn't help us
	//BUG: we assume we are reading normal text and don't validate for any special or odd characters
	while(!FoundEnd) {
		//BUG: we fail to null terminate after the read
		memset(Buffer, 0, sizeof(Buffer));
		ReadLen = read(0, Buffer, sizeof(Buffer));
		if(ReadLen <= 0)
			_exit(0);

		//v2, check for null bytes, force people to rewrite the buffer as a result
		//BUG: this lets you go past the end of the buffer
		ReadLen = strlen(Buffer);
		if((!LastChar && (ReadLen < 2)) || (ReadLen < 1))
			break;

		//check the last two chars for two newlines
		if((LastChar == '\n') && (Buffer[0] == '\n')) {
			//last char was a newline and we got another newline at the beginning
			//call it done after setting the newline to 0 in the buffer
			//BUG: combine this with failure to provide newline at the end of the buffer
			//to not have the data null terminated
			_DataLen--;
			break;
		}
		else if(Buffer[ReadLen-1] == '\n' && Buffer[ReadLen-2] == '\n') {
			ReadLen -= 2;
			FoundEnd = true;
		}

		//make sure there is room
		if((_DataLen + ReadLen + 2) > _AllocSize) {
			_Data = (char *)realloc(_Data, _DataLen + ReadLen + 2);
			_AllocSize = _DataLen + ReadLen + 2;
		}

		//if we have data already then add a newline
		if(_DataLen && _Data[_DataLen - 1] != '\n') {
			_Data[_DataLen] = '\n';
			_DataLen++;
		}

		//copy the data in
		memcpy(&_Data[_DataLen], Buffer, ReadLen);
		_DataLen += ReadLen;

		//null terminate when we get 2 newlines
		//BUG: we can have data without null termination
		if(_DataLen >= 2 && ((_Data[_DataLen-1] == '\n') && (_Data[_DataLen-2] == '\n'))) {
			_Data[_DataLen] = 0;
		}

		LastChar = _Data[_DataLen - 1];
	};
}

int ItemPage::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//go through possible commands and handle them

	//on paper x write
	if((strcmp(Commands[0], "on") == 0) && Commands[3])
	{
		if(_Crumpled) {
			Write_PrintF("The paper is crumpled\n");
			return 1;
		}

		//we know Commands[1] and Commands[2] matched us, check Commands[3]
		strtolower(Commands[3]);
		if(strcmp(Commands[3], "write") != 0) {
			Write_PrintF("Unknown command for %s\n", _Name);
			return 0;
		}

		WriteOnPage();
		return 1;
	}
	else if(strcmp(Commands[0], "read") == 0) {
		//print out the data we currently have written
		if(!_DataLen) {
			WriteString("The page is empty\n");
			return 1;
		}

		if(_Crumpled) {
			WriteString("The paper is crumpled and unreadable\n");
			return 1;
		}

		Write_PrintF("The following is written on the page:\n%s\n", _Data);
		return 1;
	}
	else if(strcmp(Commands[0], "erase") == 0) {
		//erase the page
		_DataLen = 0;
		WriteString("You drag your hand down the page watching the writing fade\n");
		return 1;
	}
	else if(strcmp(Commands[0], "crumple") == 0) {
		//crumple the paper
		if(_Crumpled) {
			WriteString("The paper is already crumpled\n");
		}
		else {
			_Crumpled = true;
			WriteString("You crumple the piece of paper\n");
		}
		return 1;
	}
	else if(strcmp(Commands[0], "uncrumple") == 0) {
		uint16_t LFSR, LFSRBit;
		size_t i;

		if(!_Crumpled) {
			WriteString("The paper is not crumpled\n");
			return 1;
		}

		//uncrumple the paper
		_Crumpled = false;
		Write_PrintF("You do your best to remove the wrinkles in the paper but the original writing is lost\n");

		//give them a new blob of random information without giving back the original info
		//BUG we don't actually reset the data allowing a memory leak
		if(_Data) {
			LFSR = *(uint16_t *)_Data;
			char *Temp = (char *)malloc(_DataLen);
			free(_Data);
			_Data = Temp;

			//xor the data to look like garbage
			for(i = 0; i < _DataLen; i++) {
				LFSRBit = ((LFSR >> 0) ^ (LFSR >> 11) ^ (LFSR >> 13) ^ (LFSR >> 3) ^ (LFSR >> 2) ^ (LFSR >> 7)) & 1;
				LFSR = ((LFSR >> 1) | LFSRBit << 15);
				_Data[i] = _Data[i] ^ (LFSR & 0xff);
			}
		}
		return 1;
	}

	//handle putting the paper into a journal
	return Item::HandleAction(Commands, CurRoom, CurPerson, OnPerson);
}

CString *ItemPage::Describe()
{
	CString *Ret;

	if(_Crumpled)
		return new CString("a crumpled piece of paper");

	if(_DataLen == 0) {
		Ret = new CString("an empty");
	}
	else {
		Ret = new CString("a");
	}

	*Ret += " piece of paper with the page number ";
	*Ret += _PageNumber;
	*Ret += " in the corner";
	return Ret;
}

bool ItemPage::IsCrumpled()
{
	return _Crumpled;
}

size_t ItemPage::GetPageNumber()
{
	return _PageNumber;
}