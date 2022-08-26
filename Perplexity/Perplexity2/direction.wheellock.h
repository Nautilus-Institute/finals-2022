#ifndef PERPLEXITY_DIRECTION_WHEELLOCKDOOR
#define PERPLEXITY_DIRECTION_WHEELLOCKDOOR
#include "direction.h"
#include <stdint.h>
#include "item.shapewheel.h"

typedef class DirectionWheelLockDoor : public Direction
{
	public:
		DirectionWheelLockDoor(Room *Room, List *Directions, List *Wheels);
		Room *MoveDirection();
		CString *Description();
		int Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		DoorType IsDoor();
		int AddWheel(ItemShapeWheel *Wheel);
		bool ValidateCombo(size_t *ExactMatchCount, size_t *MatchOutOfPlace);
		List *GetWheels();

	private:
		List _Wheels;
		List _Symbols;
		bool _IsOpen;
} DirectionWheelLockDoor;

#endif
