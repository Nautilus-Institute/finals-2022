#include "perplexity.h"
#include "item.marker.internal.h"

extern "C" {
	extern void ItemMarkerInternalFakeFunc00();
	extern void ItemMarkerInternalFakeFunc01();
	extern void ItemMarkerInternalFakeFunc02();
	extern void ItemMarkerInternalFakeFunc03();
	extern void ItemMarkerInternalFakeFunc04();
	extern void ItemMarkerInternalFakeFunc05();
	extern void ItemMarkerInternalFakeFunc06();
	extern void ItemMarkerInternalFakeFunc07();
	extern void ItemMarkerInternalFakeFunc08();
	extern void ItemMarkerInternalFakeFunc09();
	extern void ItemMarkerInternalFakeFunc10();
	extern void ItemMarkerInternalFakeFunc11();
	extern void ItemMarkerInternalFakeFunc12();
	extern void ItemMarkerInternalFakeFunc13();
	extern void ItemMarkerInternalFakeFunc14();
	extern void ItemMarkerInternalFakeFunc15();
	extern void ItemMarkerInternalFakeFunc16();
	extern void ItemMarkerInternalFakeFunc17();
	extern void ItemMarkerInternalFakeFunc18();
	extern void ItemMarkerInternalFakeFunc19();
	extern void _ZN18ItemMarkerInternal8DescribeEv();

	typedef void(*vtable_func)();

	vtable_func Item_Marker_Fake_VTable[20] = {
		ItemMarkerInternalFakeFunc00,
		ItemMarkerInternalFakeFunc01,
		ItemMarkerInternalFakeFunc02,
		ItemMarkerInternalFakeFunc03,
		ItemMarkerInternalFakeFunc04,
		ItemMarkerInternalFakeFunc05,
		ItemMarkerInternalFakeFunc06,
		ItemMarkerInternalFakeFunc07,
		ItemMarkerInternalFakeFunc08,
		ItemMarkerInternalFakeFunc09,
		_ZN18ItemMarkerInternal8DescribeEv,
		ItemMarkerInternalFakeFunc11,
		ItemMarkerInternalFakeFunc12,
		ItemMarkerInternalFakeFunc13,
		ItemMarkerInternalFakeFunc14,
		ItemMarkerInternalFakeFunc15,
		ItemMarkerInternalFakeFunc16,
		ItemMarkerInternalFakeFunc17,
		ItemMarkerInternalFakeFunc18,
		ItemMarkerInternalFakeFunc19
	};
}

typedef struct InternalMarkerVTable
{
	void **VTable;
} InternalMarkerVTable;

bool MarkerAlreadyUsedOn(void *Object)
{
	InternalMarkerVTable *VTable;

	//see if the vtable of the item already matches our fake one, if so we already wrote on it
	VTable = (InternalMarkerVTable *)Object;

	return (VTable->VTable == (void **)&Item_Marker_Fake_VTable);

}

ItemMarkerInternal::ItemMarkerInternal(char *Description, void *WrapItem) : Item(0, Description)
{
	InternalMarkerVTable *VTable;

	_Object = WrapItem;

	//rewrite our own vtable so that we overwrite describe and pass everything else through
	VTable = (InternalMarkerVTable *)this;
	VTable->VTable = (void **)&Item_Marker_Fake_VTable;
}

ItemMarkerInternal::~ItemMarkerInternal()
{
	//free the string then delete the attached item
	free((void *)_Description);
	delete (Item *)_Object;
}

CString *ItemMarkerInternal::Describe()
{
	CString *Description;
	Item *Object;

	//go get the original description, we assume it is an item and not a door BUG
	Object = (Item*)_Object;
	Description = Object->Describe();

	//now modify it
	*Description += " with \"";
	*Description += _Description;
	*Description += "\" scrawled across it";
	return Description;
}
