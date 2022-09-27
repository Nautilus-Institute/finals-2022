#ifndef PERPLEXITY_ITEM_MARKER
#define PERPLEXITY_ITEM_MARKER

#include "item.h"

typedef class ItemMarker : public Item
{
	public:
		ItemMarker(const char *Name, const char *Description);
		int Use(void *Object, ItemUseType ItemType, Room *CurRoom, Person *CurPerson);
		int HasUse();

} ItemMarker;

#endif
