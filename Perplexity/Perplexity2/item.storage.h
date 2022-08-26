#ifndef PERPLEXITY_ITEM_STORAGE
#define PERPLEXITY_ITEM_STORAGE

#include "item.h"

typedef class ItemStorage : public Item
{
	public:
		ItemStorage(const char *Name, const char *Description);
		~ItemStorage();
		int CanAccept(Item *NewItem);
		CString *Inspect();
		int AddItem(Item *NewItem);
		int RemoveItem(Person *CurPerson, const char *Name);
		CString *Describe();

	private:
		List _Items;
} ItemStorage;

#endif
