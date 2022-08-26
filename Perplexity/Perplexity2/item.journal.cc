#include "perplexity.h"

//////////
//Journal
//////////

ItemJournal::ItemJournal() : Item("journal", "")
{
}

ItemJournal::~ItemJournal()
{
	ItemPage *CurPage;

	//delete each of the page entries
	while(_Pages.Count()) {
		CurPage = (ItemPage *)_Pages[0];
		_Pages.RemoveItem(CurPage);
		delete CurPage;
	}
}

int ItemJournal::HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson)
{
	CString NameToMatch;
	ItemPage *CurPage;
	size_t i;

	//in journal on page x write
	//in journal create page
	//in journal read page x
	//in journal erase page x
	if((strcmp(Commands[0], "in") == 0) && Commands[2] && Commands[3]) {
		//see what the sub command is
		strtolower(Commands[2]);
		strtolower(Commands[3]);
		if((strcmp(Commands[2], "create") == 0) && (strcmp(Commands[3], "page") == 0)) {
			CurPage = new ItemPage();
			_Pages += CurPage;
			Write_PrintF("You whisper at the journal and %s was added to the journal\n", CurPage->Describe()->Get());
			return 1;
		}
		else if(Commands[4] && ((strcmp(Commands[2], "read") == 0) || (strcmp(Commands[2], "erase") == 0) || (strcmp(Commands[2], "on") == 0))) {
			NameToMatch += "page ";
			NameToMatch += Commands[4];
			for(i = 0; i < _Pages.Count(); i++) {
				CurPage = (ItemPage *)_Pages[i];
				if(strcmp(NameToMatch.Get(), CurPage->GetName()) == 0) {
					return CurPage->HandleAction(&Commands[2], CurRoom, CurPerson, OnPerson);
				}
			}

			Write_PrintF("Unable to locate that page number\n");
			return 0;
		}
	}

	//handle any common commands
	//from journal take page x
	return Item::HandleAction(Commands, CurRoom, CurPerson, OnPerson);
}

CString *ItemJournal::Describe()
{
	CString *Ret;

	Ret = new CString("a journal with ");
	(*Ret) += _Pages.Count();
	*Ret += " pages";
	return Ret;
}

CString *ItemJournal::Inspect()
{
	CString *Ret;
	size_t i;
	size_t TotalPages;
	ItemPage *CurPage;

	TotalPages = _Pages.Count();
	Ret = new CString("looking closely at the journal you see ");
	*Ret += TotalPages;
	*Ret += " pages";

	if(TotalPages) {
		*Ret += " numbered ";
		for(i = 0; i < TotalPages; i++) {
			CurPage = (ItemPage *)_Pages[i];
			*Ret += CurPage->GetPageNumber();
			if((TotalPages > 1) && (i <= (TotalPages - 2))) {
				*Ret += ", ";
				if(i == (TotalPages - 2)) {
					*Ret += "and ";
				}
			}
		}
	}

	return Ret;
}

int ItemJournal::CanAccept(Item *NewItem)
{
	ItemPage *Paper;

	//if this is a piece of paper see if it is crumpled, if not then accept it
	if(strncmp(NewItem->GetName(), "page ", 5) == 0) {
		Paper = (ItemPage *)NewItem;
		return !Paper->IsCrumpled();
	}

	return 0;
}

int ItemJournal::AddItem(Item *NewItem)
{
	_Pages += NewItem;
	return 1;
}

int ItemJournal::RemoveItem(Person *CurPerson, const char *Name)
{
	size_t i;
	ItemPage *CurPage;

	//cycle through the pages and try to find the entry that matches the name
	for(i = 0; i < _Pages.Count(); i++) {
		CurPage = (ItemPage *)_Pages[i];
		if(strcmp(Name, CurPage->GetName()) == 0) {
			_Pages.RemoveItem(CurPage);
			CurPerson->AddItem(CurPage);
			return 1;
		}
	}

	return 0;
}