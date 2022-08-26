#include "perplexity.h"

//////////
//Replicator
//////////

ItemReplicator::ItemReplicator() : Item("replicator", "a star trek like replicator in the wall")
{
	_Broken = 0;
}

int ItemReplicator::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	char **UserInput;
	int EntryNum;
	Item *NewItem;

	//go through possible commands and handle them
	if(_Broken)
	{
		WriteString("The replicator is inoperable. It appears to have been fried by a liquid\n");
		return 1;
	}

	//use the replicator
	if(strcmp(Commands[0], "use") == 0)
	{
		WriteString("What item would you like to replicate:\n");
		WriteString(" 1. Desk     2. Chair     3. Cup\n");
		WriteString(" 4. Key      5. Lockpick  6. Towel\n");
		WriteString(" 7. Phone    8. Food      9. Water\n");
		WriteString("10. Marker  11. Wood     12. Wheel\n");

		//get what the user wants replicated
		UserInput = GetUserInput();
		if(!UserInput[0] || UserInput[1] || !IsNumber(UserInput[0]))
		{
			Write_PrintF("Invalid entry '%s'\n", UserInput[0]);
			FreeUserInput(UserInput);
			return 0;
		}

		//get the entry
		EntryNum = atoi(UserInput[0]);
		FreeUserInput(UserInput);

		NewItem = 0;
		switch(EntryNum)
		{
			case 1:
				//a desk
				NewItem = new ItemStorage("desk","a simple wooden desk");
				break;

			case 2:
				//a Chair
				NewItem = new Item("chair", "a wooden chair");
				break;

			case 3:
				//a Cup
				NewItem = new Item("cup", "a plastic cup");
				break;

			case 4:
				//a Key
				NewItem = new ItemKey(NautilusGetRandVal() & 0xffff);
				break;

			case 5:
				//Lockpick
				WriteString("This replicator does not condone bypassing security\n");
				return 0;

			case 6:
				//Towel
				NewItem = new Item("towel", "white, soft, fluffy towel");
				WriteString("Never leave home without your towel\n");
				break;

			case 7:
				//Phone
				NewItem = new Item("phone", "a dead cellphone");
				break;

			case 8:
				//Food
				NewItem = new Item("food", "a moldy block of unknown substance");
				break;

			case 9:
				//Water
				_Broken = 1;
				WriteString("Water leaks down the replicator as white smoke rises from the bottom of it.\n");
				return 1;

			case 10:
				//marker
				NewItem = new ItemMarker("marker", "a black marker");
				break;

			case 11:
				//Wood
				NewItem = new Item("wood", "a small bundle of wood");
				break;

			case 12:
				//a wheel
				NewItem = new ItemShapeWheel((NautilusGetRandVal() & 3) + 3);
				break;

			default:
				WriteString("Invalid entry\n");
				return 0;
		}

		//if we have an item, add it to the person
		if(NewItem)
		{
			if(EntryNum == 1)
			{
				CurRoom->AddItem(NewItem);
				Write_PrintF("A %s appears in front of the replicator. You shove it out of the way.\n", NewItem->GetName());
			}
			else
			{
				CurPerson->AddItem(NewItem);
				Write_PrintF("A %s appears in the replicator. You place it in your pocket\n", NewItem->GetName());
			}
		}
		return 1;
	}

	Write_PrintF("Unknown command %s for item %s\n", Commands[0], _Name);
	return 0;
}

