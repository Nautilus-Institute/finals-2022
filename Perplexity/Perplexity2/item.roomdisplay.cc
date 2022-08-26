#include "perplexity.h"

//////////
///Room Display
//////////

ItemRoomDisplay::ItemRoomDisplay() : Item("display", "steam punk looking display")
{
	_TurnedOn = 0;
}

void ItemRoomDisplay::ShowRooms()
{
	Room *CurRoom;
	size_t i, x;
	unsigned long RoomID;
	char RoomCode[32];
	int RoomChar;
	char OrigRoomChars[17] = {'G','Q','0','W','B','L','J','2','X','R','T','5','4','H','8','N','3'};
	char RoomChars[17];
	char TempRoomChars[17];

	//cycle through all discovered rooms, we muck with the pointer value so it can be used as a unique id of the area
	for(i = 0; i < DiscoveredRooms.Count(); i++)
	{
		CurRoom = (Room *)DiscoveredRooms[i];

		//generate an ascii version of the room id
		RoomID = (unsigned long)CurRoom;

		//adjust the room chars
		RoomChar = (i+1) % sizeof(RoomChars);
		RoomID ^= rotl(0xb236fdfa7ea1e492, RoomChar);

		memcpy(RoomChars, &OrigRoomChars[RoomChar], sizeof(RoomChars) - RoomChar);
		memcpy(&RoomChars[sizeof(RoomChars) - RoomChar], OrigRoomChars, RoomChar);
		RoomCode[0] = RoomChars[0];

		for(x = 1; RoomID; x++)
		{
			//get the right character
			RoomChar = RoomID % sizeof(RoomChars);
			RoomID = (RoomID - RoomChar) / sizeof(RoomChars);

			RoomCode[x] = RoomChars[RoomChar];

			char Buffer[18]; memcpy(Buffer, RoomChars, 17); Buffer[17] = 0;

			//rotate chars
			memcpy(TempRoomChars, RoomChars, sizeof(RoomChars));
			RoomChar = (RoomChar + 1) % sizeof(RoomChars);
			memcpy(RoomChars, &TempRoomChars[RoomChar], sizeof(RoomChars) - RoomChar);
			memcpy(&RoomChars[sizeof(RoomChars) - RoomChar], TempRoomChars, RoomChar);
		}

		RoomCode[x] = 0;
		Write_PrintF("Room %s - %s\n", RoomCode, CurRoom->GetShortDescription()->Get());
	}

	return;
}

int ItemRoomDisplay::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	if(strcmp(Commands[0], "activate") == 0)
	{
		//turn our display on and tell them what there is
		if(_TurnedOn)
			Write_PrintF("The screen is already on\n");
		else
		{
			_TurnedOn = 1;
			Write_PrintF("You press the power button and watch the screen light up\n");
		}
		return 1;
	}
	else if(strcmp(Commands[0], "view") == 0)
	{
		//tell them what is around if the display is on
		if(!_TurnedOn)
			Write_PrintF("You stare at a black screen\n");
		else
			ShowRooms();
		return 1;
	}
	else if(strcmp(Commands[0], "deactivate") == 0)
	{
		//turn the display off
		if(!_TurnedOn)
			Write_PrintF("The screen is already off\n");
		else
		{
			_TurnedOn = 0;
			Write_PrintF("You press the power button, watching the screen dim to black\n");
		}
		return 1;
	}

	//can't handle it
	return 0;
}

CString *ItemRoomDisplay::Describe()
{
	const char *Positions[2] = {"dark", "brightly lit"};
	CString *NewString = new CString(_Description);
	*NewString += " with a ";
	*NewString += Positions[_TurnedOn];
	*NewString += " screen";
	return NewString;
}