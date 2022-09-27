#ifndef PERPLEXITY_ITEM_JOURNAL
#define PERPLEXITY_ITEM_JOURNAL

#include "item.h"
#include "list.h"

typedef class ItemJournal : public Item
{
	public:
		ItemJournal();
		~ItemJournal();
		virtual int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		virtual int CanAccept(Item *NewItem);
		virtual int AddItem(Item *NewItem);
		virtual int RemoveItem(Person *CurPerson, const char *Name);
		virtual CString *Describe();
		virtual CString *Inspect();
	
	private:
		List _Pages;
} ItemJournal;

#endif
