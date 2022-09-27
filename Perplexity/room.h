#ifndef PERPLEXITY_ROOM
#define PERPLEXITY_ROOM

#include "list.h"
#include "cstring.h"

typedef class Room
{
	public:
		Room();
		~Room();
		virtual void Describe();
		virtual List *GetDirections();
		virtual List *GetItems();
		virtual CString *GetDescription();
		virtual CString *GetShortDescription();
		virtual void SetDirections(List *Directions);
		virtual void AddItem(Item *NewItem);
		virtual void RemoveItem(Item *RemItem);
		virtual void SetShortDescription(const char *ShortDescription);
		virtual void SetShortDescription(CString *ShortDescription);

	protected:
		List _directions;
		List _items;
		CString _Description;
		CString _ShortDescription;
} Room;

#endif
