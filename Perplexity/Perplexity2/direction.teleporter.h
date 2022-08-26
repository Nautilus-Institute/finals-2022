#ifndef PERPLEXITY_DIRECTION_TELEPORTER
#define PERPLEXITY_DIRECTION_TELEPORTER
#include "direction.h"
#include <stdint.h>

typedef class DirectionTeleporter : public Direction
{
	public:
		DirectionTeleporter(List *Directions);
		Room *MoveDirection();
		CString *Description();
		int Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		DoorType IsDoor();
		int PowerOn(int State);

	private:
		int _IsPowered;
		int _IsOn;
		Room *DummyRoom;
} DirectionTeleporter;

void Teleporter_GetMemoryMap();

#endif
