#include "perplexity.h"

/////////////////////////////
//DirectionKeyDoor
/////////////////////////////
DirectionKeyDoor::DirectionKeyDoor(Room *Room, List *Directions, uint16_t KeyType) : Direction(Room, Directions)
{
	_State = 0;
	_RequiredKey = KeyType;
}

int DirectionKeyDoor::Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//interact with the door as needed
	CString *Description;

	//if open requested then see if it is already open
	if(strcmp(Commands[0], "open") == 0)
	{
		if(_State & 0x02)
		{
			WriteString("The door is already open\n");
			return 0;
		}
		else if(!(_State & 1))
		{
			WriteString("The door is currently locked\n");
			return 0;
		}

		//door not open, open it
		_State |= 0x02;
		Description = _Room->GetShortDescription();
		Write_PrintF("You see %s beyond the door\n", Description->Get());
		return 1;
	}

	if(strcmp(Commands[0], "close") == 0)
	{
		if(!(_State & 0x02))
		{
			WriteString("The door is already closed\n");
			return 0;
		}

		//door not closed, close it
		_State &= ~0x02;
		WriteString("You close the door\n");
		return 1;
	}

	Write_PrintF("Unknown command %s for door\n", Commands[0]);
	return 0;
}

CString *DirectionKeyDoor::Description()
{
	CString *Ret;

	//if the door is locked then report as such
	if(!(_State & 0x01))
	{
		//get the value to indicate on the door
		Ret = new CString("a locked closed door with the number '");
		*Ret += _RequiredKey;
		*Ret += "' engraved above the lock";
		return Ret;
	}
	//if the door is closed then report as such
	if(!(_State & 0x02))
		return new CString("an unlocked closed door");

	//generate the string to return
	Ret = new CString("an open door leading to ");
	*Ret += _Room->GetShortDescription();

	return Ret;
}

Direction::DoorType DirectionKeyDoor::IsDoor()
{
	return DoorType_KeyLock;
}

Room *DirectionKeyDoor::MoveDirection()
{
	//if the door is opened then move
	if(_State == 0x3)
		return _Room;

	//report if the door is locked or just closed
	if(!(_State & 0x01))
		WriteString("Door is currently locked\n");
	else
		WriteString("Door is currently closed\n");
	return 0;
}

int DirectionKeyDoor::LockState(int NewState)
{
	//if the door is opened then fail
	if(_State & 2)
		return -1;

	//flip the lock status and report back as the item use will report accordingly
	_State ^= 0x01;
	return _State & 0x01;
}

uint16_t DirectionKeyDoor::RequiredKey()
{
	return _RequiredKey;
}