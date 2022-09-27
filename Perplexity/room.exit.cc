#include "perplexity.h"
#include <time.h>

RoomExit::RoomExit()
{
	_ShortDescription += "a green pasture";
	_Description += "You have found ";
	_Description += &_ShortDescription;
}

RoomExit::~RoomExit()
{
}

void RoomExit::Describe()
{
	time_t EndTime;
	char MsgBuffer[200];

	EndTime = time(0);

	EndTime -= StartTime;

	//BUG: this is all bad
	if(EndTime < 60) {
		sprintf(MsgBuffer, "Are you a bot %s? That was a very fast solve.\n", UserData->GetName()->Get());
	}
	else if(EndTime < (60*3)) {
		sprintf(MsgBuffer, "You have excellent problem solving skills, congrats on solving this challenge %s!\n", UserData->GetName()->Get());
	}
	else {
		sprintf(MsgBuffer, "Congrats on solving this challenge %s\n", UserData->GetName()->Get());
	}

	//BUG: this is bad
	Write_PrintF(MsgBuffer);

	FoundEnd = true;
}

void RoomExit::SetShortDescription(const char *desc)
{
}

void RoomExit::SetShortDescription(CString *desc)
{
}
