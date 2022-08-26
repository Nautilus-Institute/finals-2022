#include "perplexity.h"

///////////
//Basic item
///////////

typedef struct ItemVTable
{
	unsigned long *VTable;
} ItemVTable;
#define USEFuncIndex 8

Item::Item(const char *Name, const char *Description)
{
	_Name = Name;
	_Description = Description;
}

Item::~Item()
{
}

CString *Item::Describe()
{
	return new CString(_Description);
}

const char *Item::GetName()
{
	return _Name;
}

int Item::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	unsigned int ID;
	List *Items;
	List *RoomItems;
	unsigned int ListCount, CurDirectionStrCount;
	Item *CurItem;
	List *Directions;
	Direction *CurDirection;
	List *CurDirectionStr;
	int Found;
	size_t i, x;
	ItemUseType UseType;
	void *UseItem;
	const char *DoorStr;
	CString *CStr;
	int SpaceOffset;

	//if there is a space in the name then it offsets some of the checks we do
	SpaceOffset = 0;
	if(strchr(_Name, ' ')) {
		SpaceOffset = 1;
	}

	//go through possible commands and handle them

	//pickup/take an item from a room
	if((strcmp(Commands[0], "take") == 0) || strcmp(Commands[0], "get") == 0)
	{
		//taking an item from a room and adding it to the person
		Items = CurRoom->GetItems();
		ID = Items->Find(this);
		if(ID == List::NO_MATCH)
		{
			Write_PrintF("%s is not in the room\n", _Name);
			return 0;
		}

		//found the item, remove it from the room, add it to the person
		Items->RemoveItemByID(ID);
		CurPerson->AddItem(this);
		Write_PrintF("You picked up %s\n", _Name);
		return 1;
	}

	//drop an item into a room
	else if(strcmp(Commands[0], "drop") == 0)
	{
		//drop an item into a room and remove it fromthe person
		Items = CurPerson->GetItems();
		ID = Items->Find(this);
		if(ID == List::NO_MATCH)
		{
			Write_PrintF("You do not have %s on you\n", _Name);
			return 0;
		}

		//found the item, remove it from the person, add it to the room
		Items->RemoveItemByID(ID);
		RoomItems = CurRoom->GetItems();
		*RoomItems += this;
		Write_PrintF("You dropped %s\n", _Name);
		return 1;
	}

	//put x in y assuming x is on the person
	else if((strcmp(Commands[0], "put") == 0) && Commands[2 + SpaceOffset] && Commands[3 + SpaceOffset] && (strcmp(Commands[2 + SpaceOffset], "in") == 0))
	{

		//if they are not holding it then stop
		if(!OnPerson)
		{
			Write_PrintF("You are not currently holding %s\n", _Name);
			return 0;
		}

		//find the item we are going to be put into, either in the room or on the person
		CString InItem;
		strtolower(Commands[3 + SpaceOffset]);
		InItem += Commands[3 + SpaceOffset];
		if(Commands[4 + SpaceOffset]) {
			InItem += " ";
			strtolower(Commands[4 + SpaceOffset]);
			InItem += Commands[4 + SpaceOffset];
		}

		//the entry should be an item, see if we can find it in the room
		Items = CurRoom->GetItems();
		ListCount = Items->Count();
		Found = 0;
		for(i = 0; i < ListCount; i++)
		{
			CurItem = (Item *)(*Items)[i];
			if(strcmp(CurItem->GetName(), InItem.Get()) == 0)
			{
				Found = 1;
				break;
			}
		}

		//if not found in the room then check the person
		if(!Found)
		{
			Items = CurPerson->GetItems();
			ListCount = Items->Count();
			for(i = 0; i < ListCount; i++)
			{
				CurItem = (Item *)(*Items)[i];
				if(strcmp(CurItem->GetName(), InItem.Get()) == 0)
				{
					Found = 1;
					break;
				}
			}
		}

#ifdef PERPLEXITY_V2
		//see if the item going into is a wheel lock door
		if(!Found && Commands[4 + SpaceOffset] && (strcmp(Commands[4 + SpaceOffset], "door") == 0))
		{
			Directions = CurRoom->GetDirections();
			if(Directions)
			{
				ListCount = Directions->Count();
				for(i = 0; i < ListCount; i++)
				{
					CurDirection = (Direction *)(*Directions)[i];
 
					//only check for it being a wheellock door
					if(CurDirection->IsDoor() != Direction::DoorType_WheelLock)
						continue;

					//check each direction string
					CurDirectionStr = CurDirection->GetDirections();
					CurDirectionStrCount = CurDirectionStr->Count();
					for(x = 0; x < CurDirectionStrCount; x++)
					{
						if(strcmp((const char *)(*CurDirectionStr)[x], Commands[3 + SpaceOffset]) == 0)
						{
							//add the "wheel" to the door although we failed to validate that the item is a wheel
							if(!((DirectionWheelLockDoor *)CurDirection)->AddWheel((ItemShapeWheel *)this))
							{
								Write_PrintF("Unable to insert %s into %s\n", _Name, Commands[3 + SpaceOffset]);
								return 0;
							}

							//remove ourselves from the person
							CurPerson->RemoveItem(this);
							return 1;
						}
					}
				}
			}
		}
#endif

		//if not found then fail
		if(!Found)
		{
			Write_PrintF("Unable to find %s to put %s into\n", InItem.Get(), _Name);
			return 0;
		}

		//found the object, see if we can be put into it
		if(!CurItem->CanAccept(this))
		{
			Write_PrintF("You are unable to put %s into %s\n", _Name, CurItem->GetName());
			return 0;
		}

		//put the item in
		CurItem->AddItem(this);

		//remove ourselves from the person
		CurPerson->RemoveItem(this);
		return 1;
	}

	//from x take y
	else if((strcmp(Commands[0], "from") == 0) && Commands[2 + SpaceOffset] && Commands[3 + SpaceOffset] && (strcmp(Commands[2], "take") == 0))
	{
		//lowercase the item we are going to take out
		strtolower(Commands[3 + SpaceOffset]);
		CString TakeItem;
		TakeItem += Commands[3 + SpaceOffset];
		if(Commands[4 + SpaceOffset]) {
			TakeItem += " ";
			TakeItem += Commands[4 + SpaceOffset];
		}

		//if we don't have the ability to remove then fail
		if(!RemoveItem(CurPerson, TakeItem.Get()))
		{
			Write_PrintF("You are unable to remove %s from %s\n", TakeItem.Get(), _Name);
			return 0;
		}

		//remove the item, items always go onto the person when removed
		return 1;
	}

	//inspect an item
	else if(strcmp(Commands[0], "inspect") == 0)
	{
		//inspect the object if we can otherwise just describe it
		CStr = Inspect();
		if(!CStr)
			CStr = Describe();

		WriteString(CStr->Get());
		delete(CStr);
		return 1;
	}

	//use x on y
	else if((strcmp(Commands[0], "use") == 0) && Commands[2 + SpaceOffset] && Commands[3 + SpaceOffset] && (strcmp(Commands[2], "on") == 0))
	{
		//if our item has no use ability then fail
		if(!HasUse())
		{
			Write_PrintF("%s can not be used on other objects\n", _Name);
			return 0;
		}

		UseType = USETYPE_ITEM;

		//if they are not holding it then stop
		if(!OnPerson)
		{
			Write_PrintF("You are not currently holding %s\n", _Name);
			return 0;
		}

		//find the item we are going to be used on, either in the room or on the person
		CString UsedOnItem;
		strtolower(Commands[3 + SpaceOffset]);
		UsedOnItem += Commands[3 + SpaceOffset];
		if(Commands[4 + SpaceOffset]) {
			UsedOnItem += " ";
			UsedOnItem += Commands[4 + SpaceOffset];
		}

		//the entry should be an item, see if we can find it in the room
		UseItem = 0;
		Items = CurRoom->GetItems();
		if(Items)
		{
			ListCount = Items->Count();
			for(i = 0; i < ListCount; i++)
			{
				CurItem = (Item *)(*Items)[i];
				if(strcmp(CurItem->GetName(), UsedOnItem.Get()) == 0)
				{
					UseItem = CurItem;
					break;
				}
			}
		}

		//if not found in the room then check the person
		if(!UseItem)
		{
			Items = CurPerson->GetItems();
			if(Items)
			{
				ListCount = Items->Count();
				for(i = 0; i < ListCount; i++)
				{
					CurItem = (Item *)(*Items)[i];
					if(strcmp(CurItem->GetName(), UsedOnItem.Get()) == 0)
					{
						UseItem = CurItem;
						break;
					}
				}
			}
		}

		//if not found then check directions as it could be a door, use key on door
		if(!UseItem && Commands[4 + SpaceOffset] && (strcmp(Commands[4], "door") == 0))
		{
			Directions = CurRoom->GetDirections();
			if(Directions)
			{
				ListCount = Directions->Count();
				for(i = 0; i < ListCount; i++)
				{
					CurDirection = (Direction *)(*Directions)[i];

					//only check known doors
					if(!CurDirection->IsDoor())
						continue;

					//check each direction string
					CurDirectionStr = CurDirection->GetDirections();
					CurDirectionStrCount = CurDirectionStr->Count();
					for(x = 0; x < CurDirectionStrCount; x++)
					{
						if(strcmp((const char *)(*CurDirectionStr)[x], Commands[3]) == 0)
						{
							UseItem = CurDirection;
							UseType = USETYPE_DOOR;
							break;
						}
					}
				}
			}
		}

		//if not found then fail
		if(!UseItem)
		{
			DoorStr = "";
			if(Commands[4 + SpaceOffset] && (strcmp(Commands[4 + SpaceOffset], "door") == 0))
				DoorStr = " door";
			Write_PrintF("Unable to find %s%s to use %s on\n", UsedOnItem.Get(), DoorStr, Commands[1]);
			return 0;
		}

		//go use the item
		if(!Use(UseItem, UseType, CurRoom, CurPerson))
		{
			DoorStr = "";
			if(Commands[4 + SpaceOffset] && (strcmp(Commands[4 + SpaceOffset], "door") == 0))
				DoorStr = " door";
			Write_PrintF("Failed to use %s on %s%s\n", _Name, UsedOnItem.Get(), DoorStr);
			return 0;
		}

		return 1;
	}

	Write_PrintF("Unknown command %s for item %s\n", Commands[0], _Name);
	return 0;
}

CString *Item::Inspect()
{
	return 0;
}

int Item::AddItem(Item *NewItem)
{
	return 0;
}

int Item::RemoveItem(Person *CurPerson, const char *Name)
{
	return 0;
}

int Item::Use(void *Object, ItemUseType ItemType, Room *CurRoom, Person *CurPerson)
{
	return 0;
}

int Item::HasUse()
{
	return 0;
}

int Item::CanAccept(Item *AcceptItem)
{
	//generic items can not accept anything
	return 0;
}