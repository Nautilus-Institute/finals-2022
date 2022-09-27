#ifndef PERPLEXITY_ITEM_POWERSWITCH
#define PERPLEXITY_ITEM_POWERSWITCH

#include "item.h"

typedef class DirectionPowerDoor DirectionPowerDoor;

/////////////////////////
//Power Switch
/////////////////////////

typedef class ItemPowerSwitch : public Item
{
	public:
		ItemPowerSwitch();
		int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		CString *Describe();
		void AddDoor(DirectionPowerDoor *Door);

	private:
		Room *_CurRoom;
		List *_Doors;
		int _State;
} ItemPowerSwitch;

#endif
