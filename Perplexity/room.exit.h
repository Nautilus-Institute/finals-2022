#ifndef PERPLEXITY_ROOM_EXIT
#define PERPLEXITY_ROOM_EXIT

#include "list.h"
#include "cstring.h"
#include "room.h"

typedef class RoomExit : public Room
{
	public:
		RoomExit();
		~RoomExit();
		void Describe();
		void SetShortDescription(const char *ShortDescription);
		void SetShortDescription(CString *ShortDescription);

	private:
} RoomExit;

#endif
