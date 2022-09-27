#include "perplexity.h"

///////////////////
//Direction
///////////////////
Direction::Direction(Room *Room, List *Directions)
{
	//clear fields
	_directions = Directions;
	_Room = Room;
}

Direction::~Direction()
{
	if(_directions)
		delete(_directions);
}

Room *Direction::MoveDirection()
{
	return _Room;
}

List *Direction::GetDirections()
{
	return _directions;
}

CString *Direction::Description()
{
	return new CString(_Room->GetShortDescription()->Get());
}

int Direction::Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//no actions by default
	Write_PrintF("Unknown command %s for direction\n", Commands[0]);
	return 0;
}

Direction::DoorType Direction::IsDoor()
{
	//not a door by default
	return DoorType_None;
}
