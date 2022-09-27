#ifndef PERPLEXITY_PERPLEXITY
#define PERPLEXITY_PERPLEXITY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define array_count(x) (sizeof(x) / sizeof(x[0]))

#include "list.h"
#include "cstring.h"
#include "person.h"
#include "direction.h"
#include "direction.powerdoor.h"
#include "direction.teleporter.h"
#include "direction.door.h"
#include "direction.keydoor.h"
#include "room.h"
#include "room.exit.h"
#include "item.h"
#include "item.fireplace.h"
#include "item.marker.h"
#include "item.roomdisplay.h"
#include "item.key.h"
#include "item.storage.h"
#include "item.replicator.h"
#include "item.powerswitch.h"
#include "item.page.h"
#include "item.journal.h"

#include "direction.wheellock.h"
#include "item.shapewheel.h"

extern "C" {
#include "../../NautilusRand/NautilusRand.h"
};

extern char UserInputBuffer[200];
extern char UserInput[200];
extern int UserInputSize;
extern Person* UserData;
extern time_t StartTime;
extern bool FoundEnd;

char **GetUserInput();
void FreeUserInput(char **Input);
void WriteString(const char *Msg);
void Write_PrintF(const char *Msg, ...);
void strtolower(char *Str);
int IsNumber(char *Str);
Room *GenerateGame();

extern List DiscoveredRooms;

#define rotl(x, y) ((x << y) | (x >> (64-y)))
#define rotr(x, y) ((x >> y) | (x << (64-y)))

#endif
