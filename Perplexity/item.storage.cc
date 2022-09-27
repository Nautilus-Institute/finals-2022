#include "perplexity.h"

//////////
///Storage
//////////

ItemStorage::ItemStorage(const char *Name, const char *Description) : Item(Name, Description), _Items(100)
{
}

ItemStorage::~ItemStorage()
{
}

CString *ItemStorage::Describe()
{
	CString *NewString;

	if(_Items.Count())
	{
		//we have entries
		NewString = new CString("There are ");
		*NewString += _Items.Count();
		*NewString += " items in the ";
		*NewString += _Name;
		return NewString;
	}
	else
	{
		NewString = new CString("The ");
		*NewString += _Name;
		*NewString += " has nothing in it";
	}

	return NewString;
}

int ItemStorage::CanAccept(Item *NewItem)
{
	//we will accept anything they can pick up if we don't have an item
	return 1;
}

int ItemStorage::AddItem(Item *NewItem)
{
	//add the item to our list
	_Items += NewItem;
	Write_PrintF("You added %s to the %s\n", NewItem->GetName(), _Name);
	return 1;
}

int ItemStorage::RemoveItem(Person *CurPerson, const char *ItemName)
{
	Item *CurItem;
	size_t i;

	//remove the item from our list and put it into the person

	//first, find the item
	for(i = 0; i < _Items.Count(); i++)
	{
		CurItem = (Item *)_Items[i];
		if(strcmp(CurItem->GetName(), ItemName) == 0)
		{
			//found it, add it to the person
			//BUG: we fail to remove it from ourselves
			//causing a duplication. This can be abused via destruction in fire
			CurPerson->AddItem(CurItem);
			Write_PrintF("You added %s to yourself\n", CurItem->GetName());
			return 1;
		}
	}

	Write_PrintF("Unable to locate %s in %s\n", ItemName, _Name);
	return 0;
}

CString *ItemStorage::Inspect()
{
	CString *NewString;
	Item *CurItem;
	int i;
	int ItemCount;

	//generate a list of things this object is holding
	NewString = new CString("You look over the ");
	*NewString += _Name;

	ItemCount = _Items.Count();

	//if no items then modify our output
	if(!ItemCount)
	{
		*NewString += "and do not see anything extra\n";
		return NewString;
	}

	//we have items, add them to the output
	*NewString += " and see the following items:\n";

	for(i = 0; i < ItemCount; i++)
	{
		CurItem = (Item *)_Items[i];
		*NewString += "\t";
		*NewString += CurItem->GetName();
		*NewString += "\n";
	}

	return NewString;
}
