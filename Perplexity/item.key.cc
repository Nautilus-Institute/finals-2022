#include "perplexity.h"

//////////
//Key
//////////

ItemKey::ItemKey(uint16_t KeyValue) : Item(0, 0)
{
	CString *Description;
	CString *Name;

	//set everything to default
	Name = new CString("key ");
	*Name += KeyValue;
	_Name = Name->Get();
	_KeyValue = KeyValue;

	//generate the description
	Description = new CString();

	*Description += "key with the number '";
	*Description += KeyValue;
	*Description += "' engraved on it";
	_Description = Description->Get();
}

int ItemKey::Use(void *Object, ItemUseType UseType, Room *CurRoom, Person *CurPerson)
{
	DirectionKeyDoor *ModDirection;
	List *Directions;
	int NewState;
	int ReqKey;

	//if not a door then fail
	if(UseType != USETYPE_DOOR)
		return 0;

	//cast it and check that it is truely a key door
	ModDirection = (DirectionKeyDoor *)Object;

	//check it's type
	if(ModDirection->IsDoor() != Direction::DoorType_KeyLock)
		return 0;

	//get the type of key required then see if we match, we & to allow skeleton keys
	ReqKey = ModDirection->RequiredKey();
	if((ReqKey & _KeyValue) == ReqKey)
	{
		//if we fail due to the door being opened then fail
		NewState = ModDirection->LockState(1);
		if(NewState == -1)
			return 0;

		//get a direction to report
		Directions = ModDirection->GetDirections();

		if(NewState)
			Write_PrintF("You unlocked the %s door\n", (const char *)(*Directions)[0]);
		else
			Write_PrintF("You locked the %s door\n", (const char *)(*Directions)[0]);

		//success
		return 1;
	}

	//fail
	return 0;
}

int ItemKey::HasUse()
{
	return 1;
}