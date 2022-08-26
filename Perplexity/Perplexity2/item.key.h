#ifndef PERPLEXITY_ITEM_KEY
#define PERPLEXITY_ITEM_KEY

#include "item.h"
#include <stddef.h>

typedef class ItemKey : public Item
{
	public:
		ItemKey(uint16_t KeyValue);
		int Use(void *Object, ItemUseType ItemType, Room *CurRoom, Person *CurPerson);
		int HasUse();

	private:
		//key data
		uint16_t _KeyValue;
} ItemKey;

#endif
