#include "sha512.h"
#include "WELL512a.h"
#include "NautilusRand.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <errno.h>

void NautilusInitRandFile(char *Filename)
{
	int fd;
	off_t FileSize;
	uint8_t *Data;

	//open up the file and process it into the rand data function
	fd = open(Filename, O_RDONLY);
	if(fd < 0) {
		printf("Error opening %s for random initialization\n", Filename);
		_exit(-1);
	}

	FileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	if(FileSize <= 0) {
		close(fd);
		printf("Error reading file for random initialization\n");
		_exit(-1);
	}

	Data = (uint8_t *)malloc(FileSize);
	if(read(fd, Data, FileSize) != FileSize) {
		close(fd);
		printf("Error reading file for random initialization\n");
		_exit(-1);
	}

	close(fd);

	//initialize our random data
	NautilusInitRandData(Data, FileSize);

	//before returning we need to securely wipe out the data from memory
	explicit_bzero(Data, FileSize);
	free(Data);
	return;
}

void NautilusInitRandData(void *Data, uint64_t Len)
{
	uint8_t SHAData[512 / 8];

	//first generate a sha512 of the data
	sha512Hash(Data, Len, SHAData);

	//now repeat a few times over
	for(int i = 0; i < 1000; i++) {
		sha512Hash(SHAData, sizeof(SHAData), SHAData);
	}

	//initialize WELL512a
	InitWELLRNG512a(SHAData);
}

inline uint64_t NautilusGetRandVal()
{
	union {
		uint32_t Val[2];
		uint64_t Val64;
	} v;
	v.Val[0] = WELLRNG512a();
	v.Val[1] = WELLRNG512a();
	return v.Val64;
}

void NautilusGetRandData(void *Data, uint64_t Len)
{
	uint64_t CurPos;
	uint64_t CurValue;
	uint8_t *Buffer = (uint8_t *)Data;

	CurPos = 0;
	for(CurPos = 0; CurPos < (Len & ~7); CurPos += 8) {
		*(uint64_t *)&Buffer[CurPos] = NautilusGetRandVal();
	}

	//see if we have any odd amounts left to get
	Len -= CurPos;
	if(Len) {
		CurValue = NautilusGetRandVal();
		if(Len >= 4) {
			*(uint32_t *)&Buffer[CurPos] = (uint32_t)CurValue;
			CurValue >>= 32;
			Len -= 4;
			CurPos += 4;
		}
		if(Len >= 2) {
			*(uint16_t *)&Buffer[CurPos] = (uint16_t)CurValue;
			CurValue >>= 2;
			Len -= 2;
			CurPos += 2;
		}
		if(Len) {
			*(uint8_t *)&Buffer[CurPos] = (uint8_t)CurValue;
		}
	}
}

void __attribute__ ((constructor)) InitNautilusRand()
{
	struct stat statbuf;
	if(stat("flag", &statbuf) == 0) {
		NautilusInitRandFile("flag");
	}
	else {
		uint8_t RandData[512 / 8];
		getrandom(RandData, sizeof(RandData), 0);
		NautilusInitRandData(RandData, sizeof(RandData));
	}
}
