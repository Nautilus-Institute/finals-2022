#include "perplexity.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/////////////////////////////
//DirectionTeleporter
/////////////////////////////

typedef struct MemoryMapStruct {
	uint64_t StartAddress;
	uint64_t EndAddress;
} MemoryMapStruct;

MemoryMapStruct *TeleportMemoryMap;

typedef struct RoomVTable {
	unsigned long VTable;
} RoomVTable;

void Teleporter_GetMemoryMap()
{
	FILE *fd;
	char Filename[256];
	char Buffer[256];
	int EntryCount;
	uint32_t StartPos, EndPos;
	char Flags[5];

	//open up the /maps file and find all RW areas, one of them is the heap
	if(TeleportMemoryMap) {
		free(TeleportMemoryMap);
		TeleportMemoryMap = 0;
	}

	sprintf(Filename, "/proc/%d/maps", getpid());
	fd = fopen(Filename, "r");
	if(fd < 0)
		return;

	//get the lines and parse
	EntryCount = 1;
	TeleportMemoryMap = (MemoryMapStruct *)malloc(sizeof(MemoryMapStruct));
	while(fgets(Buffer, sizeof(Buffer), fd)) {
		if(sscanf(Buffer, "%08x-%08x %4s", &StartPos, &EndPos, Flags) == 3) {
			if(memcmp(Flags, "rw", 2) == 0) {
				TeleportMemoryMap = (MemoryMapStruct *)realloc(TeleportMemoryMap, sizeof(MemoryMapStruct) * (EntryCount+1));
				TeleportMemoryMap[EntryCount-1].StartAddress = StartPos;
				TeleportMemoryMap[EntryCount-1].EndAddress = EndPos;
				EntryCount++;
			}
		}
	}

	fclose(fd);

	//set the last entry to null
	memset(&TeleportMemoryMap[EntryCount-1], 0, sizeof(MemoryMapStruct));
}

DirectionTeleporter::DirectionTeleporter(List *Directions) : Direction((Room *)-1, Directions)
{
	_IsPowered = 0;
	_IsOn = 0;
	DummyRoom = new Room();
	_Room = (Room *)-1;
}

int DirectionTeleporter::Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//interact with the door as needed
	char **UserInput;
	unsigned long KeyValue;
	size_t i, x, FirstCharIndex;
	unsigned char RoomCode[32];
	char OrigRoomChars[17] = {'G','Q','0','W','B','L','J','2','X','R','T','5','4','H','8','N', '3'};
	char RoomChars[17];
	char TempRoomChars[17];

	//if open requested then see if it is already open
	if(strcmp(Commands[0], "activate") == 0)
	{
		if(!_IsPowered)
		{
			WriteString("The teleporter appears lifeless\n");
			return 0;
		}

		//if power then see if already on
		if(_IsOn)
		{
			WriteString("The teleporter is already active\n");
			return 0;
		}

		//teleporter is not on, attempt to activate it
		if(_Room != (Room *)-1)
		{
			//see if the room looks valid by having a proper vtable
			for(i = 0; TeleportMemoryMap[i].StartAddress; i++) {
				//if the value provided is in this memory map then we are good
				if((TeleportMemoryMap[i].StartAddress <= (uint64_t)_Room) && (TeleportMemoryMap[i].EndAddress > (uint64_t)_Room)) {
					//we found the memory map, see if the vtable looks right
					if(((RoomVTable *)_Room)->VTable == ((RoomVTable *)DummyRoom)->VTable)
						_IsOn = 1;
					else {
						//allow an info leak of valid memory areas, this can be used to determine libc, heap, etc
						_Room = (Room *)-1;
						WriteString("The teleporter refuses to activate\n");
						return 0;
					}
					break;
				}
			}

			if(_IsOn)
			{
				WriteString("You see the teleporter glow brightly with a bright yellow fluxuating liquid\n");
				return 1;
			}
		}

		_Room = (Room *)-1;
		WriteString("The teleporter fails to activate\n");
		return 0;
	}
	else if(strcmp(Commands[0], "deactivate") == 0)
	{
		//if on then turn off
		if(_IsOn)
		{
			_IsOn = 0;
			_Room = (Room *)-1;
			WriteString("The teleporter liquid pulls into the sides\n");
			return 1;
		}

		//if they have power then tell them
		if(_IsPowered)
		{
			WriteString("The teleporter appears lifeless except for the faint glow coming from the keypad\n");
			return 1;
		}

		//no power
		WriteString("The teleporter appears lifeless\n");
		return 0;
	}
	else if((strcmp(Commands[0], "touch") == 0) && Commands[3] && (strcmp(Commands[3], "keypad") == 0))
	{
		if(!_IsPowered)
		{
			WriteString("The teleporter keypad is blank\n");
			return 0;
		}

		//make sure it isn't active and room is invalid
		_IsOn = 0;
		_Room = (Room *)-1;

		WriteString("You see a keypad with various symbols glowing under the glass\nWhat do you wish to type?");

		//get the input
		UserInput = GetUserInput();

		//if we have a 2nd entry then fail
		if(!UserInput[0] || UserInput[1] || (strlen(UserInput[0]) > sizeof(RoomCode)))
		{
			WriteString("Failed to enter proper information to the teleporter\n");
			FreeUserInput(UserInput);
			return 0;
		}

		//parse up the user input and create the proper memory location

		//rotate the keys based on first entry
		for(i = 0; i < sizeof(OrigRoomChars); i++)
		{
			if(OrigRoomChars[i] == UserInput[0][0])
				break;
		}

		//if we failed to find it then bail
		if(i >= sizeof(OrigRoomChars))
		{
			WriteString("Failed to enter proper information to the teleporter\n");
			FreeUserInput(UserInput);
			return 0;
		}

		//setup the key
		FirstCharIndex = i;
		memcpy(RoomChars, &OrigRoomChars[i], sizeof(OrigRoomChars) - i);
		memcpy(&RoomChars[sizeof(OrigRoomChars) - i], &OrigRoomChars[0], i);

		RoomCode[0] = FirstCharIndex;
		
		//start creating the key value
		for(x = 1; x < strlen(UserInput[0]); x++)
		{
			//find the index
			for(i = 0; i < sizeof(RoomChars); i++)
			{
				if(RoomChars[i] == UserInput[0][x])
				{
					RoomCode[x] = i;
					break;
				}
			}

			if(i >= sizeof(RoomChars))
			{
				WriteString("Failed to enter proper information to the teleporter\n");
				FreeUserInput(UserInput);
				return 0;
			}

			//rotate
			memcpy(TempRoomChars, RoomChars, sizeof(RoomChars));
			i = (i + 1) % sizeof(RoomChars);
			memcpy(RoomChars, &TempRoomChars[i], sizeof(RoomChars) - i);
			memcpy(&RoomChars[sizeof(OrigRoomChars) - i], TempRoomChars, i);
		}

		//calculate
		KeyValue = 0;
		x--;
		for(; x > 0; x--) {
			KeyValue *= sizeof(RoomChars);
			KeyValue += RoomCode[x];
		}
		KeyValue ^= rotl(0xb236fdfa7ea1e492, FirstCharIndex);

		//assign it, activate will validate it
		_Room = (Room *)KeyValue;
		WriteString("The keypad glow turns blue then back to yellow\n");

		//free our data
		FreeUserInput(UserInput);
		return 1;
	}

	Write_PrintF("Unknown command %s for teleporter\n", Commands[0]);
	return 0;
}

Room *DirectionTeleporter::MoveDirection()
{
	//if we are powered, have a room, and are on then return the room after deactivating
	if(_IsPowered && _IsOn && (_Room != (Room *)-1))
		return _Room;

	WriteString("The teleporter is not active\n");
	return 0;
}

CString *DirectionTeleporter::Description()
{
	//generate the string to return
	if(_IsOn)
		return new CString("an active teleporter with a fluxuating yellow liquid covering the middle area");

	if(_IsPowered)
		return new CString("a lifeless teleporter with a glowing keypad to the left side");

	return new CString("a lifeless teleporter with a key pad to the left side");
}

Direction::DoorType DirectionTeleporter::IsDoor()
{
	return DoorType_Teleporter;
}

int DirectionTeleporter::PowerOn(int Powered)
{
	_IsPowered = Powered;

	//no power, reset
	if(!Powered)
	{
		_IsOn = 0;
		_Room = (Room *)-1;
	}

	return 0;
}