#include "perplexity.h"

///////////////////
//DirectionWheelLockDoor
///////////////////
DirectionWheelLockDoor::DirectionWheelLockDoor(Room *Room, List *Directions, List *Wheels) : Direction(Room, Directions)
{
	size_t i;
	int RandSymbol;
	List *SymbolList;

	//setup our symbol list to match
	for(i = 0; i < Wheels->Count(); i++) {
		SymbolList = ((ItemShapeWheel *)(*Wheels)[i])->GetShapeList();
		RandSymbol = NautilusGetRandVal() % SymbolList->Count();
		_Symbols += (void *)(*SymbolList)[RandSymbol];
	}

	_IsOpen = false;
}

Room *DirectionWheelLockDoor::MoveDirection()
{
	size_t ExactMatchCount, MatchOutOfPlace;

	if(_IsOpen)
		return _Room;

	if(!ValidateCombo(&ExactMatchCount, &MatchOutOfPlace))
		WriteString("Door is currently closed and locked\n");
	else
		WriteString("Door is currently closed\n");

	return 0;
}

CString *DirectionWheelLockDoor::Description()
{
	CString *Ret;
	ItemShapeWheel *CurWheel;
	size_t ExactMatchCount, MatchOutOfPlace;
	bool Valid;

	if(_IsOpen) {
		Ret = new CString("an open door leading to ");
		*Ret += _Room->GetShortDescription();
		return Ret;
	}

	Valid = ValidateCombo(&ExactMatchCount, &MatchOutOfPlace);

	//generate the string to return
	Ret = new CString("a closed door");
	if(_Wheels.Count()) {
		*Ret += " with symbols ";
		for(size_t i = 0; i < _Wheels.Count(); i++) {
			CurWheel = (ItemShapeWheel *)_Wheels[i];
			*Ret += ShapeWheelNames[CurWheel->GetShape()];
			*Ret += " ";
		}

		*Ret += "displayed";
		if(_Symbols.Count() - _Wheels.Count())
			*Ret += " and ";
		else
			*Ret += ".";
	}
	else {
		*Ret += " with ";
	}

	if(_Symbols.Count() - _Wheels.Count()) {
		*Ret += _Symbols.Count() - _Wheels.Count();
		*Ret += " empty slots.";
	}
	else {
		if(!Valid) {
			*Ret += " The numbers ";
			*Ret += ExactMatchCount;
			*Ret += " ";
			*Ret += MatchOutOfPlace;
			*Ret += " glow below the wheels.";
		}
		else
			*Ret += " The wheels are lit green.";
	}

	return Ret;
}

Direction::DoorType DirectionWheelLockDoor::IsDoor()
{
	//we are a door, return the type of door
	return DoorType_WheelLock;
}

int DirectionWheelLockDoor::Action(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	//interact with the door as needed
	CString *Description;

	//if open requested then see if it is already open
	if(strcmp(Commands[0], "open") == 0)
	{
		if(_IsOpen)
		{
			WriteString("The door is already open\n");
			return 0;
		}
		else if(!ValidateCombo(0, 0))
		{
			WriteString("The door is currently locked\n");
			return 0;
		}

		//door not open, open it
		_IsOpen = true;

		Description = _Room->GetShortDescription();
		Write_PrintF("You see %s beyond the door\n", Description->Get());
		return 1;
	}
	else if(strcmp(Commands[0], "close") == 0)
	{
		if(!_IsOpen)
		{
			WriteString("The door is already closed\n");
			return 0;
		}

		//door not closed, close it
		_IsOpen = false;
		WriteString("You close the door\n");
		return 1;
	}
	else if(strcmp(Commands[0], "from") == 0) {
		//from x door take wheel y
		if(Commands[3] && Commands[4] && Commands[5]) {
			strtolower(Commands[3]);
			strtolower(Commands[4]);
			if(strcmp(Commands[3], "take") == 0) {
				//matches the layout, find our command
				CString WheelName(Commands[4]);
				WheelName += " ";
				WheelName += Commands[5];
				for(size_t i = 0; i < _Wheels.Count(); i++) {
					ItemShapeWheel *CurWheel = (ItemShapeWheel *)_Wheels[i];
					if(strcmp(CurWheel->GetName(), WheelName.Get()) == 0) {
						Write_PrintF("Removed %s from %s door\n", WheelName.Get(), (const char *)(*_directions)[0]);
						CurWheel->RemoveDoor();
						_Wheels.RemoveItemByID(i);
						CurPerson->AddItem(CurWheel);
						return 1;
					}
				}
			}
		}
	}

	Write_PrintF("Unknown command %s for door\n", Commands[0]);
	return 0;
}

int DirectionWheelLockDoor::AddWheel(ItemShapeWheel *Wheel)
{
	if(_Wheels.Count() == _Symbols.Count())
		return 0;

	_Wheels += Wheel;
	return 1;
}

bool DirectionWheelLockDoor::ValidateCombo(size_t *ExactMatchCount, size_t *MatchOutOfPlace)
{
	size_t CurMatchCount = 0;
	size_t i, x;
	ItemShapeWheel::ShapeEnum CurWheelShape;
	List UnMatchedSymbols;
	List UnMatchedWheels;

	//check each of the wheels to see if they match what we need

	if(ExactMatchCount)
		*ExactMatchCount = 0;
	if(MatchOutOfPlace)
		*MatchOutOfPlace = 0;

	//if they don't have enough wheels then don't let them guess anything
	if(_Wheels.Count() != _Symbols.Count()) {
		return false;
	}

	//check the wheels against our symbols to see how many line up
	for(i = 0; i < _Symbols.Count(); i++) {
		CurWheelShape = ((ItemShapeWheel *)_Wheels[i])->GetShape();
		if((size_t)_Symbols[i] == CurWheelShape) {
			if(ExactMatchCount)
				*ExactMatchCount += 1;
			CurMatchCount++;
		}
		else if(MatchOutOfPlace) {
			//add the symbol and wheel to our unmatch list
			UnMatchedSymbols += _Symbols[i];
			UnMatchedWheels += (void *)CurWheelShape;
		}
	}

	//walk our unmatched list and see if we can find matches
	if(MatchOutOfPlace) {
		for(i = 0; i < UnMatchedSymbols.Count(); i++) {
			for(x = 0; x < UnMatchedWheels.Count(); x++) {
				if(UnMatchedSymbols[i] == UnMatchedWheels[x]) {
					*MatchOutOfPlace += 1;
					UnMatchedWheels.RemoveItemByID(x);
					break;
				}
			}
		}
	}

	//return if we found an exact match
	return CurMatchCount == _Symbols.Count();
}

List *DirectionWheelLockDoor::GetWheels()
{
	return &_Wheels;
}
