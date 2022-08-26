#ifndef PERPLEXITY_DIRECTION_KEYDOOR
#define PERPLEXITY_DIRECTION_KEYDOOR
#include "direction.h"
#include <stddef.h>

//////////
//KeyDoor
/////////

typedef class DirectionKeyDoor : public Direction
{
	public:
		DirectionKeyDoor(Room *Rooms, List *Directions, uint16_t RequiredKey);
		Room *MoveDirection();
		CString *Description();
		int Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		DoorType IsDoor();
		int LockState(int State);
		uint16_t RequiredKey();

	private:
		int _State;			//bit 0 - unlocked, bit 1 - opened
		uint16_t _RequiredKey;
} DirectionKeyDoor;

#endif
