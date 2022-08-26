#include "perplexity.h"
#include "direction.wheellock.h"

//////////
//Shape Wheel
//////////

size_t ShapeWheelID = 0;
const char *ShapeWheelNames[] = {"square", "circle", "triangle", "star", "hexagon", "octagon", "plus", "minus", "infinity"};

ItemShapeWheel::ItemShapeWheel(int ShapeCount) : Item(0, 0)
{
	_ShapeList = new List();
	CString *Description;
	char ValueBuf[11];
	char *NewName;
	size_t ShapeRandID[ShapeEnum::COUNT];
	size_t i;
	size_t TempID, TempVal;

	//get the value to indicate on the door and in the name
	ShapeWheelID++;
	sprintf(ValueBuf, "%ld", ShapeWheelID);

	//allocate a new Name
	NewName = (char *)malloc(5 + strlen(ValueBuf) + 1);
	strcpy(NewName, "wheel ");
	strcat(NewName, ValueBuf);

	//set everything to default
	_Name = NewName;
	_ShapeCount = ShapeCount;
	_CurrentShape = 0;

	//BUG: We never set _Door allowing for misuse and shapes can be generated via the replicator
	//we use _Door->ValidateCombo() upon rotation if we are not on the person which can be done in a room
	//_Door = 0;

	//generate the description
	Description = new CString("A small round wheel with the number ");
	*Description += ShapeWheelID;
	*Description += " engraved with ";
	*Description += ShapeCount;
	*Description += " pictures on it: ";

	//generate our list
	for(i = 0; i < ShapeEnum::COUNT; i++)
		ShapeRandID[i] = i;

	for(int i = 0; i < ShapeCount; i++) {
		TempID = i + (NautilusGetRandVal() % ((uint64_t)ItemShapeWheel::ShapeEnum::COUNT - i));
		TempVal = ShapeRandID[TempID];
		ShapeRandID[TempID] = ShapeRandID[i];
		ShapeRandID[i] = TempVal;

		*_ShapeList += (void *)ShapeRandID[i];
		*Description += ShapeWheelNames[ShapeRandID[i]];
		if(i+2 < ShapeCount) {
			*Description += ", ";
		}
		else if(i+1 < ShapeCount) {
			*Description += " and ";
		}
	}

	_Description = Description->Get();
}

int ItemShapeWheel::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	size_t i, x;

	//action wasn't handled, see if they are trying to rotate
	if(!Commands[3] && strcmp(Commands[0], "rotate") == 0) {
		if(OnPerson) {
			Write_PrintF("You spin the wheel in your hand\n");
			return 1;
		}
		else if(!_Door) {
			Write_PrintF("You spin the wheel\n");
			return 1;
		}

		//rotate our wheel
		_CurrentShape = (_CurrentShape + 1) % _ShapeCount;
		Write_PrintF("%s rotated to display %s\n", _Name, ShapeWheelNames[(uint64_t)(*_ShapeList)[_CurrentShape]]);

		//check if we are proper
		if(((DirectionWheelLockDoor *)_Door)->ValidateCombo(0, 0)) {
			Write_PrintF("You hear a click on the door\n");
		}

		return 1;
	}
	else if(Item::HandleAction(Commands, CurRoom, CurPerson, OnPerson)) {
		//if inserting a wheel then random rotate to a new location
		if(strcmp(Commands[0], "put") == 0) {
			_CurrentShape = NautilusGetRandVal() % _ShapeCount;

			//find the door as it must have been valid
			List *Directions = CurRoom->GetDirections();
			if(Directions)
			{
				size_t ListCount = Directions->Count();
				for(i = 0; i < ListCount; i++)
				{
					Direction *CurDirection = (Direction *)(*Directions)[i];

					//only check for it being a wheellock door
					if(CurDirection->IsDoor() != Direction::DoorType_WheelLock)
						continue;

					//check each direction string
					List *CurDirectionStr = CurDirection->GetDirections();
					size_t CurDirectionStrCount = CurDirectionStr->Count();
					for(x = 0; x < CurDirectionStrCount; x++)
					{
						if(strcmp((const char *)(*CurDirectionStr)[x], Commands[4]) == 0)
						{
							//found it, make sure there is room
							_Door = CurDirection;
							return 1;
						}
					}
				}
			}
		}

		return 1;
	}

	return 0;
}

int ItemShapeWheel::HasUse()
{
	return 0;
}

ItemShapeWheel::ShapeEnum ItemShapeWheel::GetShape()
{
	return (ItemShapeWheel::ShapeEnum)((uint64_t)(*_ShapeList)[_CurrentShape]);
}

List *ItemShapeWheel::GetShapeList()
{
	return _ShapeList;
}

void ItemShapeWheel::RemoveDoor()
{
	_Door = 0;
}
