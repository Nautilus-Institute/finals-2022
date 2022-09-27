#ifndef PERPLEXITY_ITEM_FIREPLACE
#define PERPLEXITY_ITEM_FIREPLACE

#include "item.h"

typedef class ItemFireplace : public Item
{
	public:
		ItemFireplace();
		int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		int CanAccept(Item *NewItem);
		int AddItem(Item *NewItem);
		int RemoveItem(Person *CurPerson, const char *ItemName);
		CString *Describe();
	
	private:
		Item *_Item;
		int _IsLit;
		Item *_HasWood;
} ItemFireplace;

#endif
