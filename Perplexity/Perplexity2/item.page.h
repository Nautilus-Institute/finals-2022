#ifndef PERPLEXITY_ITEM_PAGE
#define PERPLEXITY_ITEM_PAGE

#include "item.h"
#include "list.h"
#include "cstring.h"

typedef class ItemPage : public Item
{
	public:
		ItemPage();
		~ItemPage();
		virtual int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		virtual CString *Describe();
		bool IsCrumpled();
		size_t GetPageNumber();

	private:
		char *_Data;
		size_t _DataLen;
		size_t _AllocSize;
		bool _Crumpled;
		size_t _PageNumber;
		CString _PageName;

		void WriteOnPage();

} ItemPage;

#endif
