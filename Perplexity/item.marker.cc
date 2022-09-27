#include "perplexity.h"
#include "item.marker.internal.h"

//////////
//Item marker
//////////


bool MarkerAlreadyUsedOn(void *Object);

ItemMarker::ItemMarker(const char *Name, const char *Description) : Item(Name, Description)
{
}

int ItemMarker::Use(void *Object, ItemUseType UseType, Room *CurRoom, Person *CurPerson)
{
	List *Dirs;
	char **UserInput;
	const char *Description;
	List *CurItems;
	ItemMarkerInternal *WriteObj;
	const char *DoorStr;
	int i;

	if(UseType == USETYPE_ITEM)
	{
		//get the description
		Description = ((Item *)Object)->GetName();
		DoorStr = "";
	}
	else if(UseType == USETYPE_DOOR)
	{
		//get a direction for the description
		Dirs = ((Direction *)Object)->GetDirections();
		Description = (const char *)(*Dirs)[0];
		DoorStr = " door";
	}
	else
		return 0;	//unknown type

	//see if we already wrote on it
	if(MarkerAlreadyUsedOn(Object))
	{
		Write_PrintF("Already wrote on %s\n", Description);
		return 0;
	}

	//we need to work some magic
	//here is where a type confusion comes up as we forget to check UseType for item or door

	Write_PrintF("What do you want to write on %s%s?\n", Description, DoorStr);

	//get the input
	UserInput = GetUserInput();

	//add in all the spaces that were removed
	for(i = 0; UserInput[i] && UserInput[i+1]; i++)
	{
		UserInput[i][strlen(UserInput[i])] = ' ';
	}

	//get the new object that will hold the writing, the object does assume it is an item and not a door
	WriteObj = new ItemMarkerInternal(UserInput[0], Object);

	//free the pointer list, we don't free the actual string from the user due to being in use
	free(UserInput);

	//remove the original object and replace with ours this allows us to encapsulate the object we modified
	//BUG: of course we assume it is an item and forget to validate with the door

	//if on the person then remove and add else it must be the room
	CurItems = CurPerson->GetItems();
	if(CurItems->Find(Object) != List::NO_MATCH)
	{
		CurItems->RemoveItem(Object);
		CurPerson->AddItem((Item *)WriteObj);
	}
	else
	{
		//must be in the room, remove the original and add in the new one
		CurRoom->RemoveItem((Item *)Object);
		CurRoom->AddItem((Item *)WriteObj);
	}

	return 1;
}

int ItemMarker::HasUse()
{
	return 1;
}