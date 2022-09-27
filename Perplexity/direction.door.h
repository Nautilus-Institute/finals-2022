#ifndef PERPLEXITY_DIRECTION_DOOR
#define PERPLEXITY_DIRECTION_DOOR
#include "direction.h"

/////////////
//Normal door
/////////////
typedef class DirectionDoor : public Direction
{
	public:
		DirectionDoor(Room *Rooms, List *Directions);
		Room *MoveDirection();
		CString *Description();
		int Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		DoorType IsDoor();

	private:
		int _IsOpened;
} DirectionDoor;

#endif
