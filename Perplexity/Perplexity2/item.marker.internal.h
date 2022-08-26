#ifndef PERPLEXITY_ITEM_MARKER_INTERNAL
#define PERPLEXITY_ITEM_MARKER_INTERNAL

#include "item.marker.h"

typedef class ItemMarkerInternal : Item
{
	public:
		ItemMarkerInternal(char *Description, void *WrapItem);
		~ItemMarkerInternal();

		CString *Describe();

	private:
		void *_Object;
} ItemMarkerInternal;

#endif
