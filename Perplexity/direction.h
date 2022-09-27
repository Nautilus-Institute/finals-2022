#ifndef PERPLEXITY_DIRECTION
#define PERPLEXITY_DIRECTION

#include "list.h"
#include "cstring.h"

//create the base class for directions
typedef class Room Room;
typedef class Person Person;

/////////////
//Normal open direction
/////////////
typedef class Direction
{
	public:
		typedef enum DoorType
		{
			DoorType_None,
			DoorType_Powered,
			DoorType_NoLock,
			DoorType_Teleporter,
			DoorType_KeyLock,
			DoorType_WheelLock,
			DoorType_COUNT
		} DoorType;

		Direction(Room *Rooms, List *Directions);
		~Direction();
		List *GetDirections();
		virtual Room *MoveDirection();
		virtual int Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		virtual CString *Description();
		virtual DoorType IsDoor();

	protected:
		List *_directions;
		Room *_Room;
} Direction;

#endif
