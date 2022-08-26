#include "list.h"
#include <malloc.h>
#include <string.h>

List::List()
{
	//allocate space for 10 entries by default
	_items = (const void **)malloc(sizeof(void *) * 10);
	_maxcount = 10;
	_count = 0;
	_max_set_count = 0;
}

List::List(unsigned int MaxCount)
{
	//allocate space for 10 entries by default
	_items = (const void **)malloc(sizeof(void *) * MaxCount);
	_maxcount = MaxCount;
	_count = 0;
	_max_set_count = MaxCount;
}

List::~List()
{
	free(_items);
}

void List::Clear()
{
	_count = 0;
}

const void * List::operator[](int Index)
{
	//get an item from the list of items
	if(Index >= (int)_count)
		return 0;

	return _items[Index];
}

List& List::operator+=(const void *ptr)
{
	const void **NewList;

	//add an item, see if we need to expand our array or not
	//BUG: If a count is fixed then we will never increase but we will keep adding
	if(!_max_set_count && (_count == _maxcount))
	{
		_maxcount += 10;
		NewList = (const void **)malloc(sizeof(void *) * _maxcount);
		memcpy(NewList, _items, _count * sizeof(void *));
		free(_items);
		_items = NewList;
	}

	//add the item
	_items[_count] = ptr;
	_count++;
	return *this;
}

size_t List::ExtendList(const List& rhs)
{
	const void **NewList;
	unsigned int NewCount;

	//copy a whole list into our array, first see if there is room
	//BUG: If a count is fixed then we will never increase but we will keep adding
	NewCount = _count + rhs._count;
	if(!_max_set_count && (NewCount >= _maxcount))
	{
		//no room, expand
		_maxcount = NewCount + 10;
		NewList = (const void **)malloc(sizeof(void *) * _maxcount);
		memcpy(NewList, _items, _count * sizeof(void *));
		free(_items);
		_items = NewList;
	}

	//copy the list over
	memcpy(&_items[_count], rhs._items, rhs._count * sizeof(void *));

	//update our count
	_count = NewCount;
	return _count;
}

const void *List::RemoveItemByID(unsigned int Index)
{
	const void *Entry;

	if(Index >= _count)
		return 0;

	//shift all data down 1 entry
	Entry = _items[Index];
	memcpy(&_items[Index], &_items[Index+1], (_count - Index - 1) * sizeof(void *));
	_count--;

	//return the entry removed
	return Entry;
}

const void *List::RemoveItem(const void *Ptr)
{
	unsigned int Index;

	for(Index = 0; Index < _count; Index++)
	{
		if(_items[Index] == Ptr)
			break;
	}

	if(Index >= _count)
		return 0;

	//shift all data down 1 entry
	memcpy(&_items[Index], &_items[Index+1], (_count - Index - 1) * sizeof(void *));
	_count--;

	//return the entry removed
	return Ptr;
}

unsigned int List::Find(const void *Ptr)
{
	unsigned int Index;

	for(Index = 0; Index < _count; Index++)
	{
		if(_items[Index] == Ptr)
			break;
	}

	if(Index >= _count)
		return NO_MATCH;

	//return the entry found
	return Index;
}

void List::InsertItem(const void *ptr, unsigned int Index)
{
	unsigned int i;
	const void **NewList;

	//insert an item into a specific index value, shifting everything after it up a slot
	if(Index >= _count)
	{
		//index is above so we may need to expand entries
		if(!_max_set_count && (Index >= _maxcount))
		{
			_maxcount += (((Index - _maxcount) / 10) + 1) * 10;
			NewList = (const void **)malloc(sizeof(void *) * _maxcount);

			//copy over the lower part of the list
			memcpy(NewList, _items, _count * sizeof(void *));
			free(_items);
			_items = NewList;
		}

		//make sure entries are 0 between the count and us
		memset(&_items[_count], 0, sizeof(void *) * (Index - _count));
		_items[Index] = ptr;
		_count = Index + 1;
		return;
	}

	//add an item, see if we need to expand our array or not
	if(!_max_set_count && (_count == _maxcount))
	{
		_maxcount += 10;
		NewList = (const void **)malloc(sizeof(void *) * _maxcount);

		//copy over the lower part of the list
		memcpy(NewList, _items, Index * sizeof(void *));

		//set our entry
		NewList[Index] = ptr;

		//copy the upper part
		memcpy(&NewList[Index+1], &_items[Index], (_count - Index) * sizeof(void *));
		free(_items);
		_items = NewList;
		return;
	}

	//copy the upper part over
	for(i = _count; i > Index; i--)
		_items[i] = _items[i - 1];
	_items[Index] = ptr;
	_count++;
	return;
}

unsigned int List::Count()
{
	return _count;
}