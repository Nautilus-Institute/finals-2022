#ifndef PERPLEXITY_PERSON
#define PERPLEXITY_PERSON

typedef class List List;
typedef class Item Item;

typedef class Person
{
	public:
		Person();
		~Person();
		void AddItem(Item *NewItem);
		void RemoveItem(Item *RemItem);
		List *GetItems();
		void SetName(const char *Name);
		CString *GetName();

	private:
		List _items;
		CString _Name;
} Person;

#endif
