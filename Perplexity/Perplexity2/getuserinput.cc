#include "perplexity.h"
#include <unistd.h>

char UserInput[200];
int UserInputSize;

char **GetUserInput()
{
	int CurPos;
	int ReadSize;
	int MaxEntries;
	char *Newline;
	char **Lines;
	char **NewLines;
	char *BufCopy;

	//we assume that the result will be free'd

	//BUG - I dont check size of data in, could not only stack smash but also blow the global area

	//copy in the leftover user data
	memcpy(UserInputBuffer, UserInput, UserInputSize);
	CurPos = UserInputSize;

	while(1)
	{
		//scan the data for a newline, if found then parse on it
		Newline = strchr(UserInputBuffer, '\n');
		if(Newline)
		{
			//copy out any data left
			UserInputSize = &UserInputBuffer[CurPos - 1] - Newline;
			memcpy(UserInput, (Newline + 1), UserInputSize);
			UserInput[UserInputSize] = 0;

			//now replace the newline with a 0 and parse it up
			*Newline = 0;
			ReadSize = 0;

			//copy our buffer off
			MaxEntries = strlen(UserInputBuffer);
			BufCopy = (char *)malloc(MaxEntries + 1);
			memcpy(BufCopy, UserInputBuffer, MaxEntries);
			BufCopy[MaxEntries] = 0;

			//allocate memory and snag the pointers
			MaxEntries = 5;
			Lines = (char **)malloc(sizeof(char *)*5);

			//count how many spaces we have
			Lines[0] = BufCopy;
			Newline = strchr(BufCopy, ' ');
			while(Newline)
			{
				//add 1 to counter, find non space and then find next space
				*Newline = 0;
				Newline++;
				ReadSize++;
				while(*Newline == ' ')
					Newline++;

				Lines[ReadSize] = Newline;
				Newline = strchr(Newline, ' ');

				//if no room for the empty slot at the end indicating the end then
				//allocate more space and copy over then free the original
				if((ReadSize+1) == MaxEntries)
				{
					MaxEntries += 5;
					NewLines = (char **)malloc(sizeof(char*)*MaxEntries);
					memcpy(NewLines, Lines, sizeof(char*)*(ReadSize+1));
					free(Lines);
					Lines = NewLines;
				}
			};

			//insert blank entry
			Lines[ReadSize+1] = 0;
			break;
		}

		//BUG I fail to limit user input data size properly
		ReadSize = read(0, &UserInputBuffer[CurPos], sizeof(UserInputBuffer) - 1);
		if(ReadSize <= 0)
			exit(0);
		CurPos += ReadSize;
		UserInputBuffer[CurPos] = 0;
	};

	return Lines;
}

void FreeUserInput(char **Input)
{
	if(Input)
	{
		//free the string itself
		free(Input[0]);

		//free the pointer array
		free(Input);
	}
}