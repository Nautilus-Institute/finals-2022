#include "perplexity.h"

//////////
///Fireplace
//////////

ItemFireplace::ItemFireplace() : Item("fireplace", "")
{
	_Item = 0;
	_IsLit = 0;
	_HasWood = 0;
}

int ItemFireplace::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//go through possible commands and handle them

	//turn the fireplace on and off
	if(strcmp(Commands[0], "light") == 0)
	{
		//see if we need to be turned on
		if(_IsLit)
		{
			WriteString("The fireplace is already lit\n");
			return 1;
		}

		//light the fireplace if possible
		if(!_Item && !_HasWood)
		{
			WriteString("The fireplace has nothing to burn\n");
			return 1;
		}

		//if we have wood then the fireplace will continue to burn
		//otherwise it will go out as soon as the item is burned up
		if(_HasWood)
		{
			_IsLit = 1;
			WriteString("You strike a match and light the wood on fire\n");
			delete(_HasWood);
			_HasWood = 0;
			if(_Item)
				Write_PrintF("%s catches fire and burns up\n", _Item->GetName());
		}
		else
			Write_PrintF("You strike a match and light the %s on fire, watching it burn up\n", _Item->GetName());

		//if we burned an item up, destroy it
		if(_Item)
		{
			delete(_Item);
			_Item = 0;
		}

		return 1;
	}
	else if(strcmp(Commands[0], "extinguish") == 0)
	{
		if(!_IsLit)
			WriteString("The fireplace is not currently lit\n");
		else
		{
			_IsLit = 0;
			WriteString("You poke at the wood logs until the fire goes out\n");
		}
		return 1;
	}

	return 0;
}

CString *ItemFireplace::Describe()
{
	CString *NewString;

	if(_IsLit)
		return new CString("The fireplace is lit and providing heat into the room");
	else if(_HasWood)
	{
		NewString = new CString("There is wood in the unlit fireplace");
		if(_Item)
		{
			*NewString += " with ";
			*NewString += _Item->GetName();
			*NewString += " on top of the wood";
		}
	}
	else if(_Item)
	{
		NewString = new CString("There is a ");
		*NewString += _Item->GetName();
		*NewString += " in the unlit fireplace";
	}
	else
		NewString = new CString("an empty fireplace in the corner");
	return NewString;
}

int ItemFireplace::CanAccept(Item *NewItem)
{
	//we will accept anything they can pick up if we don't have an item
	if(_Item)
		return 0;
	return 1;
}

int ItemFireplace::AddItem(Item *NewItem)
{
	//if the item is wood then indicate we have wood otherwise just add the item
	if(strcmp(NewItem->GetName(), "wood") == 0)
	{
		_HasWood = NewItem;
		Write_PrintF("You add %s to the fireplace\n", NewItem->GetName());
	}
	else if(_IsLit)
	{
		//fireplace is burning, burn the item up
		Write_PrintF("You add %s to the fireplace, watching it burn up\n", NewItem->GetName());
		delete(NewItem);
	}
	else
	{
		_Item = NewItem;
		Write_PrintF("You add %s to the fireplace\n", NewItem->GetName());
	}

	return 1;
}

int ItemFireplace::RemoveItem(Person *CurPerson, const char *ItemName)
{
	//if the item doesn't match then fail
	if(!_Item || strcmp(_Item->GetName(), ItemName) != 0)
		return 0;

	//BUG: we fail to remove it from ourselves
	//causing a duplication. This can be abused via destruction in fire
	CurPerson->AddItem(_Item);
	Write_PrintF("You added %s to yourself\n", _Item->GetName());
	return 1;
}