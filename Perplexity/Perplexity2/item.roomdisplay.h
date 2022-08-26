#ifndef PERPLEXITY_ITEM_ROOMDISPLAY
#define PERPLEXITY_ITEM_ROOMDISPLAY

#include "item.h"

typedef class ItemRoomDisplay : public Item
{
	public:
		ItemRoomDisplay();
		virtual int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		virtual CString *Describe();

	private:
		void ShowRooms();
		int _TurnedOn;
} ItemRoomDisplay;

#endif
