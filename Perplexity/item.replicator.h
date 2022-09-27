#ifndef PERPLEXITY_ITEM_REPLICATOR
#define PERPLEXITY_ITEM_REPLICATOR

#include "item.h"

typedef class ItemReplicator : public Item
{
	public:
		ItemReplicator();
		int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);

	private:
	    int _Broken;
} ItemReplicator;

#endif
