#include "perplexity.h"

/////////////////////////////
//DirectionDoor
/////////////////////////////
DirectionDoor::DirectionDoor(Room *Room, List *Directions) : Direction(Room, Directions)
{
	_IsOpened = 0;
}

int DirectionDoor::Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//interact with the door as needed
	CString *Description;

	//if open requested then see if it is already open
	if(strcmp(Commands[0], "open") == 0)
	{
		if(_IsOpened)
		{
			WriteString("The door is already open\n");
			return 0;
		}

		//door not open, open it
		_IsOpened = 1;
		Description = _Room->GetShortDescription();
		Write_PrintF("You see %s beyond the door\n", Description->Get());
		return 1;
	}

	if(strcmp(Commands[0], "close") == 0)
	{
		if(!_IsOpened)
		{
			WriteString("The door is already closed\n");
			return 0;
		}

		//door not closed, close it
		_IsOpened = 0;
		WriteString("You close the door\n");
		return 1;
	}

	Write_PrintF("Unknown command %s for door\n", Commands[0]);
	return 0;
}

CString *DirectionDoor::Description()
{
	CString *Ret;

	//if the door is closed then report as such
	if(!_IsOpened)
		return new CString("a closed door");

	//generate the string to return
	Ret = new CString("an open door leading to ");
	*Ret += _Room->GetShortDescription();

	return Ret;
}

Direction::DoorType DirectionDoor::IsDoor()
{
	return DoorType_NoLock;
}

Room *DirectionDoor::MoveDirection()
{
	//if the door is opened then move
	if(_IsOpened)
		return _Room;

	WriteString("Door is currently closed\n");
	return 0;
}

char UserInputBuffer[200] = "a";	//force location of the user data so we can abuse type confusion and use the user data assuming we find a valid item
				//plan is to setup via the sharpie, write on a non-item
