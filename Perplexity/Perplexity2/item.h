#ifndef PERPLEXITY_ITEM
#define PERPLEXITY_ITEM

#include "list.h"

typedef class Item Item;
typedef class Room Room;
typedef class Person Person;
typedef class CString CString;

typedef class Item
{
	public:
		typedef enum ItemUseType
		{
			USETYPE_ITEM,
			USETYPE_DOOR,
			USETYPE_COUNT
		} ItemUseType;

		Item(const char *Name, const char *Description);
		virtual ~Item();
		virtual const char *GetName();
		virtual int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		virtual int CanAccept(Item *NewItem);
		virtual CString *Inspect();
		virtual int AddItem(Item *NewItem);
		virtual int RemoveItem(Person *CurPerson, const char *Name);
		virtual int Use(void *Object, ItemUseType ItemType, Room *CurRoom, Person *CurPerson);
		virtual int HasUse();
		virtual CString *Describe();

	protected:
		const char *_Description;
		const char *_Name;
} Item;

#endif
