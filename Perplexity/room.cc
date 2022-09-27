#include "perplexity.h"

List DiscoveredRooms;

Room::Room()
{
}

Room::~Room()
{
}

void Room::Describe()
{
	size_t i;
	Direction *CurDirection;
	List *DirectionNames;
	CString *CurDesc;

	//check if this room is in the discovered list, if not then add it
	if(DiscoveredRooms.Find(this) == List::NO_MATCH)
		DiscoveredRooms += this;

	//build a description to send to the user
	Write_PrintF("%s\n", _Description.Get());

	//handle all of the directions
	for(i = 0; i < _directions.Count(); i++)
	{
		CurDirection = (Direction *)_directions[i];
		DirectionNames = CurDirection->GetDirections();
		CurDesc = CurDirection->Description();

		Write_PrintF("To the %s is %s\n", (const char *)(*DirectionNames)[0], CurDesc->Get());
		delete(CurDesc);
	}
}

List *Room::GetDirections()
{
	return &_directions;
}

List *Room::GetItems()
{
	return &_items;
}

CString *Room::GetDescription()
{
	return &_Description;
}

CString *Room::GetShortDescription()
{
	return &_ShortDescription;
}

void Room::SetDirections(List *directions)
{
	_directions.Clear();
	_directions.ExtendList(*directions);
}

void Room::AddItem(Item *NewItem)
{
	_items += NewItem;
}

void Room::RemoveItem(Item *RmItem)
{
	//remove if possible
	_items.RemoveItem(RmItem);
}

void Room::SetShortDescription(const char *desc)
{
	SetShortDescription(new CString(desc));
}

void Room::SetShortDescription(CString *desc)
{
	_ShortDescription.Clear();
	_ShortDescription += desc;
	_ShortDescription += " room";

	_Description.Clear();
	_Description += "You are in ";
	_Description += desc;
	_Description += " room";

	if(_directions.Count() == 0) {
		_Description += " with no exits, you found hell";
	}
	else if(_directions.Count() == 1) {
		_Description += " with one exit";
	}
	else {
		_Description += " with multiple exits";
	}
}
