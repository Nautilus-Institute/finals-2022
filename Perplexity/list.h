#ifndef PERPLEXITY_LIST
#define PERPLEXITY_LIST

#include <stddef.h>

typedef class List
{
	public:
		static const unsigned int NO_MATCH = (unsigned int)-1;

		List();
		List(unsigned int MaxItems);
		virtual ~List();
		virtual const void *operator[](int Index);			//force a vtable for List so it can be abused via a fake room teleport setup
		size_t ExtendList(const List& rhs);
		const void *RemoveItemByID(unsigned int Index);
		const void *RemoveItem(const void *Data);
		void InsertItem(const void *Data, unsigned int Index);
		unsigned int Count();
		unsigned int Find(const void *Data);
		List& operator+=(const void *Data);
		void Clear();

	private:
		const void **_items;
		unsigned int _count;
		unsigned int _maxcount;
		unsigned int _max_set_count;
} List;

#endif
