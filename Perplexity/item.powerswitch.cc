#include "perplexity.h"

//////////
//Power switch
//////////

ItemPowerSwitch::ItemPowerSwitch() : Item("switch", "an old style large switch on the wall")
{
	_Doors = new List();
}

void ItemPowerSwitch::AddDoor(DirectionPowerDoor *NewDoor)
{
	//add the door to the list of doors we power
	*_Doors += NewDoor;
}

int ItemPowerSwitch::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	unsigned int ID;
	unsigned int Count;
	DirectionPowerDoor *CurDoor;

	//go through possible commands and handle them

	//flip the switch on or off
	if(strcmp(Commands[0], "flip") == 0)
	{
		//flip our state
		_State = ~_State;

		//set all the doors
		Count = _Doors->Count();
		for(ID = 0; ID < Count; ID++)
		{
			CurDoor = (DirectionPowerDoor *)(*_Doors)[ID];
			CurDoor->PowerOn(_State);
		}

		//report the position of the switch
		if(_State)
			Write_PrintF("You turn the %s on\n", _Description);
		else
			Write_PrintF("You turn the %s off\n", _Description);
		return 1;
	}

	Write_PrintF("Unknown command %s for item %s\n", Commands[0], _Name);
	return 0;
}

CString *ItemPowerSwitch::Describe()
{
	const char *Positions[2] = {"on", "off"};
	CString *NewString = new CString(_Description);
	*NewString += " in the ";
	*NewString += Positions[_State + 1];
	*NewString += " position";
	return NewString;
}