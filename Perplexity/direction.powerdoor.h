#ifndef PERPLEXITY_DIRECTION_POWERDOOR
#define PERPLEXITY_DIRECTION_POWERDOOR
#include "direction.h"

//////////
//PowerDoor
/////////

typedef class DirectionPowerDoor : Direction
{
	public:
		DirectionPowerDoor(Room *Rooms, List *Directions);
		Room *MoveDirection();
		CString *Description();
		DoorType IsDoor();
		int PowerOn(int State);

	private:
		int _IsPowered;
} DirectionPowerDoor;

#endif
