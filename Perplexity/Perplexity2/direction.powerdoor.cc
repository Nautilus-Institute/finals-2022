#include "perplexity.h"

///////////////////
//DirectionPowerDoor
///////////////////
DirectionPowerDoor::DirectionPowerDoor(Room *Room, List *Directions) : Direction(Room, Directions)
{
	//clear fields
	_IsPowered = 0;
}

Room *DirectionPowerDoor::MoveDirection()
{
	if(_IsPowered)
		return _Room;

	WriteString("Door is not open due to no power\n");
	return 0;
}

int DirectionPowerDoor::PowerOn(int Powered)
{
	_IsPowered = Powered;
	return 0;
}

CString *DirectionPowerDoor::Description()
{
	//if no power then indicate that the door is closed
	if(!_IsPowered)
		return new CString("a powered door with no power");
	else
		return new CString(_Room->GetShortDescription()->Get());
}

Direction::DoorType DirectionPowerDoor::IsDoor()
{
	//we are a door, return the type of door
	return DoorType_Powered;
}