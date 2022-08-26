#include "perplexity.h"

Person::Person() : _items(50)
{
}

Person::~Person()
{
}

void Person::AddItem(Item *NewItem)
{
	//add an item to the person
	_items += NewItem;
}

void Person::RemoveItem(Item *RemItem)
{
	//remove an item from the person
	_items.RemoveItem(RemItem);
}

List *Person::GetItems()
{
	//return all items they hold
	return &_items;
}

void Person::SetName(const char *Name)
{
	_Name += Name;
}

CString *Person::GetName()
{
	return &_Name;
}