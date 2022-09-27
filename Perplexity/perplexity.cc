#include "perplexity.h"
#include <unistd.h>
#include <time.h>

Person *UserData;
time_t StartTime;
bool FoundEnd;

int main()
{
	char **Commands;
	Room *CurRoom;
	Room *NewRoom;
	List *Directions;
	List *Items;
	Direction *CurDirection;
	Item *CurItem;
	Person *CurPerson;
	int ListCount;
	int i;
	List *CurDirectionStrs;
	size_t DirectionStrCount;
	size_t x;
	int Found;
	CString *ItemStr;
	char UsernameInput[200];

	FoundEnd = false;
	UserInputSize = 0;
	CurRoom = GenerateGame();
	CurPerson = new Person();
	UserData = CurPerson;
	CurPerson->AddItem(new ItemJournal());

	StartTime = time(0);
	WriteString("Welcome to Perplexity: an entangled state\n\n");
	WriteString("Please provide your name: ");

	//BUG: THIS IS BAD, Fuck it!
	memset(UsernameInput, 0, sizeof(UsernameInput));
	gets(UsernameInput);
	Write_PrintF("Welcome %s\n", UsernameInput);
	CurPerson->SetName(UsernameInput);

	//loop and handle each move
	Commands = 0;
	DiscoveredRooms += CurRoom;

	CurRoom->Describe();
	while(!FoundEnd)
	{
		WriteString("> ");

		//get an array of user input strings
		if(Commands)
		{
			FreeUserInput(Commands);
		}
		Commands = GetUserInput();

		//process each command
		if(!Commands || !Commands[0])
			break;

		//lowercase and compare
		strtolower(Commands[0]);

		if((strcmp(Commands[0], "q") == 0) || (strcmp(Commands[0], "quit") == 0))
		{
			WriteString("Good-bye\n");
			break;
		}

		//if they ask where they are tell them
		Found = 0;
		if(Commands[0] && Commands[1] && Commands[2])
		{
			//compare lowercase strings
			strtolower(Commands[1]);
			strtolower(Commands[2]);

			if((strcmp(Commands[0], "where") == 0) &&
			(strcmp(Commands[1], "am") == 0) &&
			(strcmp(Commands[2], "i") == 0))
			{
				CurRoom->Describe();
				Found = 1;
			}
		}

		//check directions
		if(!Found)
		{
			if((strcmp(Commands[0], "describe") == 0) || (strcmp(Commands[0], "d") == 0))
			{
				//redescribe the current room
				CurRoom->Describe();
				Found = 1;
			}
			else if((strcmp(Commands[0], "look") == 0) || (strcmp(Commands[0], "l") == 0))
			{
				//go through all items in the room and report them
				Items = CurRoom->GetItems();
				if(Items)
					ListCount = Items->Count();
				else
					ListCount = 0;

				if(!Items || !ListCount)
					WriteString("You do not see anything in this room\n");
				else
				{
					WriteString("You see:\n");
					for(i = 0; i < ListCount; i++)
					{
						CurItem = (Item *)(*Items)[i];
						ItemStr = CurItem->Describe();
						Write_PrintF("\t%s\n", ItemStr->Get());
						delete(ItemStr);
					}
				}
				Found = 1;
			}
			else
			{
				//get all directions from the current room
				Directions = CurRoom->GetDirections();
				if(Directions)
				{
					ListCount = Directions->Count();
					for(i = 0; i < ListCount; i++)
					{
						//for each direction, get it's string list that works
						CurDirection = (Direction *)(*Directions)[i];
						CurDirectionStrs = CurDirection->GetDirections();
						DirectionStrCount = CurDirectionStrs->Count();
						for(x = 0; x < DirectionStrCount; x++)
						{
							//see if the string matches, if so indicate it and tell it to move
							if(strcmp(Commands[0], (const char *)(*CurDirectionStrs)[x]) == 0)
							{
								//the direction will print an appropriate error if the move fails, just see if
								//we got a new direction
								NewRoom = CurDirection->MoveDirection();
								if(NewRoom)
								{
									//add it to the discovered list
									if(DiscoveredRooms.Find(NewRoom) == List::NO_MATCH)
										DiscoveredRooms += NewRoom;

									//describe the room
									CurRoom = NewRoom;
									CurRoom->Describe();
								}
								Found = 1;
								break;
							}
							else if(
								(
									(CurDirection->IsDoor() != Direction::DoorType_Teleporter) &&
									Commands[1] && Commands[2] && (strcmp(Commands[2], "door") == 0) &&
									(strcmp(Commands[1], (const char *)(*CurDirectionStrs)[x]) == 0)
								) || (
									(CurDirection->IsDoor() == Direction::DoorType_Teleporter) &&
									Commands[1] && Commands[2] && (strcmp(Commands[2], "teleporter") == 0) &&
									(strcmp(Commands[1], (const char *)(*CurDirectionStrs)[x]) == 0)
								)
							)
							{
								//we are giving an action to a door, handle it
								CurDirection->Action(Commands, CurRoom, CurPerson, 0);
								Found = 1;
							}
						}
						if(Found)
							break;
					}
				}
			}
		}

		//see if they want to see what is on their own person
		if(!Found && Commands[0] && Commands[1])
		{
			//compare lowercase strings
			strtolower(Commands[1]);

			if((strcmp(Commands[0], "view") == 0) && (strcmp(Commands[1], "self") == 0))
			{
				//grab a list of items and print them out
				Items = CurPerson->GetItems();
				ListCount = Items->Count();
				if(!ListCount)
					WriteString("You do not have anything on you\n");
				else
				{
					WriteString("You have the following things on you:\n");
					for(i = 0; i < ListCount; i++)
					{
						CurItem = (Item *)(*Items)[i];
						ItemStr = CurItem->Describe();
						Write_PrintF("\t%s\n", ItemStr->Get());
						delete(ItemStr);
					}
				}

				Found = 1;
			}
		}

		//see if we need to use an item
		if(!Found && Commands[1])
		{
			CString AltName;
			strtolower(Commands[1]);

			if(Commands[2]) {
				AltName += Commands[1];
				AltName += " ";
				AltName += Commands[2];
			}

			//the entry should be an item, see if we can find it in the room or on the person
			Items = CurRoom->GetItems();
			if(Items)
			{
				ListCount = Items->Count();
				for(i = 0; i < ListCount; i++)
				{
					CurItem = (Item *)(*Items)[i];
					if((strcmp(CurItem->GetName(), Commands[1]) == 0) || (strcmp(CurItem->GetName(), AltName.Get()) == 0))
					{
						CurItem->HandleAction(Commands, CurRoom, CurPerson, 0);
						Found = 1;
						break;
					}
				}
			}

			//if not found in the room then check the person
			if(!Found)
			{
				Items = CurPerson->GetItems();
				if(Items)
				{
					ListCount = Items->Count();
					for(i = 0; i < ListCount; i++)
					{
						CurItem = (Item *)(*Items)[i];
						if((strcmp(CurItem->GetName(), Commands[1]) == 0) || (strcmp(CurItem->GetName(), AltName.Get()) == 0))
						{
							CurItem->HandleAction(Commands, CurRoom, CurPerson, 1);
							Found = 1;
							break;
						}
					}
				}
			}

			//see if this is a match for a gear on a door
			if(!Found)
			{
				Directions = CurRoom->GetDirections();
				if(Directions)
				{
					ListCount = Directions->Count();
					for(i = 0; i < ListCount; i++)
					{
						CurDirection = (Direction *)(*Directions)[i];

						//only check for it being a wheellock door
						if(CurDirection->IsDoor() == Direction::DoorType_WheelLock) {
							//get any wheels in the door and check them
							List *Wheels = ((DirectionWheelLockDoor *)CurDirection)->GetWheels();
							for(x = 0; x < Wheels->Count(); x++) {
								CurItem = (Item *)(*Wheels)[x];
								if((strcmp(CurItem->GetName(), Commands[1]) == 0) || (strcmp(CurItem->GetName(), AltName.Get()) == 0)) {
									CurItem->HandleAction(Commands, CurRoom, CurPerson, 0);
									Found = 1;
									break;
								}
							}

							if(Found)
								break;
						}
					}
				}
			}
		}

		//if no direction found then error
		if(!Found)
			WriteString("I did not understand that command\n");

	};

	return 0;
}
