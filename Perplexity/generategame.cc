#include "perplexity.h"

const char *RoomTypes[] = {"round", "square", "rectangular", "oval", "hexgon", "dark", "library", "triangle", "octagon", "heptagon", "pentagon"};
const char *RoomSizes[] = {"small ", "normal ", "large ", "giant ", "grand ", "teeny ", "short ", "tall ", "huge ", "epic ", "big ", "measly "};
const char *StorageNames[] = {"desk", "box", "cabinet", "chest"};
const char *StorageDesc[] = {"an old desk", "a wooden box", "a metal filing cabinet", "a fancy game style chest"};

//maximum number of links between the two loops (one way)
#define MAX_ROOM_LINKS 20

//min/max number of rooms in a loop
#define MIN_ROOMS 5
#define MAX_ROOMS 15

//min/max number of forced links between loops
#define MIN_LOOP_LINKS 1
#define MAX_LOOP_LINKS 3

//min/max percentages of random items
#define MIN_RANDOM_ITEMS 30
#define MAX_RANDOM_ITEMS 80

//percentages of rooms that get X
#define FIREPLACE_PERCENT	20
#define WOOD_PERCENT		30
#define STORAGE_PERCENT		20
#define REPLICATOR_PERCENT	10

List RoomsWithSwitches;

CString *GenerateRoomDescription()
{
	CString *Description;
	size_t RandType, RandSize;

	RandType = NautilusGetRandVal() % array_count(RoomTypes);
	RandSize = NautilusGetRandVal() % array_count(RoomSizes);

	Description = new CString("a ");
	*Description += RoomSizes[RandSize];
	*Description += RoomTypes[RandType];

	return Description;
}

size_t GetUsedRoomDirections(Room *CurRoom, List *DirectionList)
{
	size_t Counter, ret;
	Direction *CurDirection;

	//generate a bit mask of the used directions
	ret = 0;
	for(Counter = 0; Counter < CurRoom->GetDirections()->Count(); Counter++) {
		CurDirection = (Direction *)(*CurRoom->GetDirections())[Counter];
		ret |= (1 << DirectionList->Find(CurDirection->GetDirections()));
	}

	return ret;
}

List *GenerateRoomLoop(List *DirLists)
{
	size_t i;
	size_t NewDirectionID, PrevDirectionID, FirstDirectionID;
	size_t RoomCount = (NautilusGetRandVal() % (MAX_ROOMS - MIN_ROOMS)) + MIN_ROOMS;
	List *Rooms;
	Room *CurRoom;
	Direction *NewDirection;
	List *CurRoomDirections;
	size_t PrevRoomLinks;

	Rooms = new List();

	//generate a bunch of rooms
	for(i = 0; i < RoomCount; i++) {
		CurRoom = new Room();
		CurRoom->SetDirections(new List());
		*Rooms += CurRoom;
	}

	//now setup a circle between the rooms
	PrevRoomLinks = 0;
	PrevDirectionID = NautilusGetRandVal() % DirLists->Count();
	FirstDirectionID = PrevDirectionID ^ 1;
	NewDirectionID = 0;
	for(i = 0; i < RoomCount; i++) {
		CurRoom = (Room *)(*Rooms)[i];
		CurRoomDirections = CurRoom->GetDirections();

		//if the last room we have some extra validations to do
		if(i == (RoomCount - 1)) {
			//if previous doesn't link and the direction doesn't match what we need
			//for entering the first room then mark it as having linked
			if(!PrevRoomLinks && (FirstDirectionID != PrevDirectionID)) {
				PrevRoomLinks = 1;
			}
			else if(PrevRoomLinks && (PrevDirectionID == FirstDirectionID)) {
				//we do link but it collides with the direction needed for the first, don't link
				PrevRoomLinks = 0;
			}
		}

		//we can walk back into the previous room, set it up
		if(PrevRoomLinks) {
			PrevDirectionID = NewDirectionID ^ 1;

			//get a new direction that isn't used
			if(i == (RoomCount - 1)) {
				//last entry, force us to move into the first room
				NewDirectionID = FirstDirectionID ^ 1;

				//make sure we don't have double entries)
				if(PrevDirectionID != NewDirectionID) {
					NewDirection = new Direction((Room *)(*Rooms)[i-1], (List *)(*DirLists)[PrevDirectionID]);
					*CurRoomDirections += NewDirection;
				}
			}
			else {
				NewDirection = new Direction((Room *)(*Rooms)[i-1], (List *)(*DirLists)[PrevDirectionID]);
				*CurRoomDirections += NewDirection;

				do {
					//we can walk back into the previous room, make sure we don't use it's location
					NewDirectionID = NautilusGetRandVal() % DirLists->Count();
				}
				while(NewDirectionID == PrevDirectionID);
			}
		}
		else {
			//use the direction to go back to the previous as our entry
			NewDirectionID = PrevDirectionID ^ 1;
		}

		//1 in 4 chance of being a door
		size_t WasDoor = (NautilusGetRandVal() % 3) == 0;
		if(WasDoor) {
			//it is a door, 1 in 3 chance of it not linking back unless last room
			if(i != RoomCount - 1) {
				PrevRoomLinks = NautilusGetRandVal() % 3;
			}
			else
				PrevRoomLinks = 0;
			NewDirection = new DirectionDoor((Room *)(*Rooms)[(i+1) % RoomCount], (List *)(*DirLists)[NewDirectionID]);
		}
		else {
			PrevRoomLinks = 1;
			NewDirection = new Direction((Room *)(*Rooms)[(i+1) % RoomCount], (List *)(*DirLists)[NewDirectionID]);
		}

		//add the direction to the list
		*CurRoomDirections += NewDirection;
		PrevDirectionID = NewDirectionID;
	}

	//pick a random room to drop a teleporter into
	i = NautilusGetRandVal() % Rooms->Count();
	NewDirectionID = NautilusGetRandVal() % DirLists->Count();
	PrevDirectionID = GetUsedRoomDirections((Room *)(*Rooms)[i], DirLists);
	while(PrevDirectionID & (1 << NewDirectionID)) {
		NewDirectionID = (NewDirectionID + 1) % DirLists->Count();
	}

	//found a room and direction, add the teleporter
	NewDirection = new DirectionTeleporter((List *)((*DirLists)[NewDirectionID]));
	((DirectionTeleporter *)NewDirection)->PowerOn(1);
	(*((Room *)((*Rooms)[i]))->GetDirections()) += NewDirection;

	//add the room display box
	i = NautilusGetRandVal() % Rooms->Count();
	((Room *)((*Rooms)[i]))->AddItem(new ItemRoomDisplay());

	//return the rooms
	return Rooms;
}

void RandomLinkRooms(List *LinkToList, List *LinkFromList, List *DirectionList)
{
	size_t RandLinks;
	size_t i;
	Room *ToRoom;
	Room *FromRoom;
	size_t FromRoomNewDir, FromRoomDirUsed;
	size_t Counter;
	Direction *CurDirection;


	//randomly link from one list to the other, not allowed to go backwards
	RandLinks = NautilusGetRandVal() % MAX_ROOM_LINKS;
	for(i = 0; i < RandLinks; i++) {
		//pick 2 random rooms to attempt to link
		ToRoom = (Room *)(*LinkToList)[NautilusGetRandVal() % LinkToList->Count()];
		FromRoom = (Room *)(*LinkFromList)[NautilusGetRandVal() % LinkFromList->Count()];

		//figure out each direction already used
		FromRoomDirUsed = GetUsedRoomDirections(FromRoom, DirectionList);

		//find a random direction that isn't already used
		FromRoomNewDir = NautilusGetRandVal() % DirectionList->Count();
		for(Counter = 0; Counter < DirectionList->Count(); Counter++) {
			//keep incrementing through directions until we find one that isn't used
			if(!(FromRoomDirUsed & (1 << FromRoomNewDir))) {
				//found an entry, create a door to the new area
				CurDirection = new DirectionDoor(ToRoom, (List *)(*DirectionList)[FromRoomNewDir]);
				(*FromRoom->GetDirections()) += CurDirection;
				break;
			}
			FromRoomNewDir = (FromRoomNewDir + 1) % DirectionList->Count();
		}
	}
}

List *GenerateDirectionLists()
{
	List *DirListOfLists;
	List *DirList;

	//generate all of the directions you can move
	//return a list of lists

	DirListOfLists = new List();

	DirList = new List();
	*DirList += "north";
	*DirList += "n";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "northeast";
	*DirList += "ne";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "northwest";
	*DirList += "nw";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "south";
	*DirList += "s";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "southeast";
	*DirList += "se";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "southwest";
	*DirList += "sw";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "east";
	*DirList += "e";
	*DirListOfLists += DirList;

	DirList = new List();
	*DirList += "west";
	*DirList += "w";
	*DirListOfLists += DirList;

	return DirListOfLists;
}

void SortRoomDirections(List *Rooms, List *Directions)
{
	Room *CurRoom;
	List RoomDirections;
	List NewRoomDirections;
	size_t UsedDirections;
	size_t RoomID, DirID, RoomDirID;

	//go through each room and sort the directions
	for(RoomID = 0; RoomID < Rooms->Count(); RoomID++) {
		CurRoom = (Room *)((*Rooms)[RoomID]);

		RoomDirections.Clear();
		NewRoomDirections.Clear();

		RoomDirections.ExtendList(*(CurRoom->GetDirections()));
		UsedDirections = GetUsedRoomDirections(CurRoom, Directions);

		//walk the used directions
		DirID = 0;
		while(UsedDirections) {
			if(UsedDirections & 1) {
				//find the entry that matches this direction
				for(RoomDirID = 0; RoomDirID < RoomDirections.Count(); RoomDirID++) {
					//if this matches then add to the new list and remove it from the list we search
					if(((Direction *)RoomDirections[RoomDirID])->GetDirections() == (*Directions)[DirID]) {
						NewRoomDirections += RoomDirections[RoomDirID];
						RoomDirections.RemoveItemByID(RoomDirID);
						break;
					}
				}
			}

			UsedDirections >>= 1;
			DirID += 1;
		};

		//set the room directions properly
		CurRoom->SetDirections(&NewRoomDirections);
	}
}

void GenerateLoopLinks(List *MainLoop, List *NewLoop, List *Directions)
{
	//generate a link or two between loops with a forced door
	DirectionDoor *NewDoor;
	Room *Rooms[2];
	size_t i, x, LinkCount;
	size_t UsedRoomDirs[2];
	size_t RoomDir;
	bool Found;
	List NewItems;
	ItemPowerSwitch *Switch = 0;

	LinkCount = (NautilusGetRandVal() % (MAX_LOOP_LINKS - MIN_LOOP_LINKS)) + MIN_LOOP_LINKS;
	for(i = 0; i < LinkCount; i++) {
		Found = false;
		do {
			//pick a room from each link
			Rooms[0] = (Room *)((*MainLoop)[NautilusGetRandVal() % MainLoop->Count()]);
			Rooms[1] = (Room *)((*NewLoop)[NautilusGetRandVal() % NewLoop->Count()]);

			//find a direction that isn't used in the room
			UsedRoomDirs[0] = GetUsedRoomDirections(Rooms[0], Directions);
			UsedRoomDirs[1] = GetUsedRoomDirections(Rooms[1], Directions);

			//pick a random direction
			RoomDir = NautilusGetRandVal() % Directions->Count();

			//make sure the direction isn't used
			for(x = 0; x < Directions->Count(); x++) {
				//if the direction isn't used, check the other room
				if(!(UsedRoomDirs[0] & (1 << RoomDir))) {
					if(!(UsedRoomDirs[1] & (1 << (RoomDir ^ 1)))) {
						Found = true;
						break;
					}
				}

				//failed to find an entry, try again
				RoomDir = (RoomDir + 1) % Directions->Count();
			}
		} while(!Found);

		//found an entry, setup the door and the path back through
		switch(NautilusGetRandVal() % 3) {
			case 0:
			{
				NewDoor = (DirectionDoor *)new DirectionPowerDoor(Rooms[1], (List *)(*Directions)[RoomDir]);

				//only initialize the switch once
				if(!Switch) {
					Switch = new ItemPowerSwitch();
					NewItems += Switch;
				}
				Switch->AddDoor((DirectionPowerDoor *)NewDoor);
				break;
			}
			case 1:
			{
				List Wheels;
				size_t WheelCount = (NautilusGetRandVal() % 4) + 2;
				while(WheelCount) {
					Wheels += new ItemShapeWheel((NautilusGetRandVal() % 5) + 2);
					WheelCount--;
				};
				NewDoor = (DirectionDoor *)new DirectionWheelLockDoor(Rooms[1], (List *)(*Directions)[RoomDir], &Wheels);
				NewItems.ExtendList(Wheels);
				break;
			}
			default:
			{
				size_t KeyValue = NautilusGetRandVal() & 0xffff;
				NewDoor = (DirectionDoor *)new DirectionKeyDoor(Rooms[1], (List *)(*Directions)[RoomDir], KeyValue);
				NewItems += new ItemKey(KeyValue);
				break;
			}
		}

		//add the door to the first room
		(*Rooms[0]->GetDirections()) += NewDoor;

		//setup a walk through from the other side
		(*Rooms[1]->GetDirections()) += new Direction(Rooms[0], (List *)(*Directions)[RoomDir ^ 1]);

		//now random scatter the item(s) into the first block of rooms
		while(NewItems.Count()) {
			Rooms[0] = (Room *)((*MainLoop)[NautilusGetRandVal() % MainLoop->Count()]);

			//if this is a switch then make sure we don't add it to an already existing room with a switch
			if(NewItems[0] == Switch) {
				//if we found this room then try again
				if(RoomsWithSwitches.Find(Rooms[0]) != List::NO_MATCH)
					continue;

				//not in the list, add it
				RoomsWithSwitches += Rooms[0];
			}
			Rooms[0]->AddItem((Item *)NewItems[0]);
			NewItems.RemoveItemByID(0);
		};
	}
}

void AddExitRoom(List *Rooms, List *Directions)
{
	Room *CurRoom;
	RoomExit *Exit;
	size_t CurDirection;
	size_t UsedDirections;
	DirectionKeyDoor *ExitKeyDoor;

	//find a room to use
	do
	{
		//get random room and it's used directions
		CurRoom = (Room *)(*Rooms)[NautilusGetRandVal() % Rooms->Count()];
		UsedDirections = GetUsedRoomDirections(CurRoom, Directions);

		//if all directions aren't used then select one of the unused ones
		if(UsedDirections != (size_t)((1 << Directions->Count()) - 1)) {
			//find an unset bit
			CurDirection = 0;
			while(UsedDirections & (1 << CurDirection))
				CurDirection++;
			break;
		}
	} while(1);

	//add the exit
	Exit = new RoomExit();

	//add the direction
	ExitKeyDoor = new DirectionKeyDoor((Room *)new RoomExit(), (List *)((*Directions)[CurDirection]), 0x1337);
	(*CurRoom->GetDirections()) += ExitKeyDoor;

	//add it to the list
	(*Rooms) += Exit;

	//pick a random room to throw a key into
	CurRoom = (Room *)(*Rooms)[NautilusGetRandVal() % Rooms->Count()];
	CurRoom->AddItem(new ItemKey(0x1337));
}

void GenerateRandomItems(List *Rooms)
{
	List UsedRooms;
	size_t i, counter;
	Room *RandRoom;
	Item *NewItem;

	//generate a random number of fireplaces across the rooms, we aim for a portion of the rooms
	counter = Rooms->Count() * FIREPLACE_PERCENT / 100;
	for(; counter > 0; counter--) {

		//get a random room we haven't selected before
		do
		{
			i = NautilusGetRandVal() % Rooms->Count();
			RandRoom = (Room *)(*Rooms)[i];
		} while(UsedRooms.Find(RandRoom) != List::NO_MATCH);

		//add a fireplace
		RandRoom->AddItem(new ItemFireplace());
		UsedRooms += RandRoom;
	}
	UsedRooms.Clear();

	//now add wood to random rooms
	counter = Rooms->Count() * WOOD_PERCENT / 100;
	for(; counter > 0; counter--) {

		//get a random room we haven't selected before
		do
		{
			i = NautilusGetRandVal() % Rooms->Count();
			RandRoom = (Room *)(*Rooms)[i];
		} while(UsedRooms.Find(RandRoom) != List::NO_MATCH);

		//add wood
		RandRoom->AddItem(new Item("wood", "a small bundle of wood"));
		UsedRooms += RandRoom;
	}
	UsedRooms.Clear();

	//now add storage to random rooms
	counter = Rooms->Count() * STORAGE_PERCENT / 100;
	for(; counter > 0; counter--) {
		//get a random room we haven't selected before
		do
		{
			i = NautilusGetRandVal() % Rooms->Count();
			RandRoom = (Room *)(*Rooms)[i];
		} while(UsedRooms.Find(RandRoom) != List::NO_MATCH);

		//add storage
		i = NautilusGetRandVal() % array_count(StorageNames);
		RandRoom->AddItem(new ItemStorage(StorageNames[i], StorageDesc[i]));
		UsedRooms += RandRoom;
	}
	UsedRooms.Clear();

	//add replicators to random rooms
	counter = Rooms->Count() * REPLICATOR_PERCENT / 100;
	for(; counter > 0; counter--) {
		//get a random room we haven't selected before
		do
		{
			i = NautilusGetRandVal() % Rooms->Count();
			RandRoom = (Room *)(*Rooms)[i];
		} while(UsedRooms.Find(RandRoom) != List::NO_MATCH);

		//add storage
		i = NautilusGetRandVal() % array_count(StorageNames);
		RandRoom->AddItem(new ItemReplicator());
		UsedRooms += RandRoom;
	}
	UsedRooms.Clear();

	//now pick random rooms and add random items
	counter = (NautilusGetRandVal() % (MAX_RANDOM_ITEMS - MIN_RANDOM_ITEMS)) + MIN_RANDOM_ITEMS;

	//got a random percent of rooms to fill, now get that count
	counter = (Rooms->Count() * counter) / 100;
	for(; counter > 0; counter--) {
		RandRoom = (Room *)(*Rooms)[NautilusGetRandVal() % Rooms->Count()];

		//determine what we are placing
#define RAND_ITEM_COUNT 8
		switch(NautilusGetRandVal() % RAND_ITEM_COUNT) {
			case 0:
				//a Chair
				NewItem = new Item("chair", "a wooden chair");
				break;

			case 1:
				//a Cup
				NewItem = new Item("cup", "a plastic cup");
				break;

			case 2:
				//a Key
				NewItem = new ItemKey(NautilusGetRandVal() & 0xffff);
				break;

			case 3:
				//Towel
				NewItem = new Item("towel", "white, soft, fluffy towel");
				break;

			case 4:
				//Phone
				NewItem = new Item("phone", "a dead cellphone");
				break;

			case 5:
				//Food
				NewItem = new Item("food", "a moldy block of unknown substance");
				break;

			case 6:
				//marker
				NewItem = new ItemMarker("marker", "a black marker");
				break;

			case 7:
				//a wheel
				NewItem = new ItemShapeWheel((NautilusGetRandVal() & 3) + 3);
				break;
			};

			RandRoom->AddItem(NewItem);
	}
}

Room *GenerateGame()
{
	List *RoomList;
	List *NewRoomLoop;
	List *Directions = GenerateDirectionLists();
	size_t i;

	RoomList = GenerateRoomLoop(Directions);
	RandomLinkRooms(RoomList, RoomList, Directions);

	for(i = 0; i < 6; i++) {
		//generate a new loop
		NewRoomLoop = GenerateRoomLoop(Directions);

		//add links to the new loop
		GenerateLoopLinks(RoomList, NewRoomLoop, Directions);

		//loop on itself
		RandomLinkRooms(NewRoomLoop, NewRoomLoop, Directions);

		//loop back into the first block of rooms
		RandomLinkRooms(RoomList, NewRoomLoop, Directions);

		//extend our room list
		RoomList->ExtendList(*NewRoomLoop);
	}

	//generate random descriptions for all rooms
	for(i = 0; i < RoomList->Count(); i++) {
		((Room *)(*RoomList)[i])->SetShortDescription(GenerateRoomDescription());
	}

	//add the exit
	AddExitRoom(RoomList, Directions);

	//scatter random items around
	GenerateRandomItems(RoomList);

	//sort the room directions
	SortRoomDirections(RoomList, Directions);

	//update the teleporter memory list
	Teleporter_GetMemoryMap();

	//get rid of the list we made
	RoomsWithSwitches.Clear();

	return (Room *)(*RoomList)[0];
}