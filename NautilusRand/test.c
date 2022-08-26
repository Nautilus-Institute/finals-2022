#include "sha512.h"
#include "NautilusRand.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	//confirm SHA512 is accurate
	char ShaTestStr[] = "testing";
	uint64_t ShaResult[512 / 8 / 8];
	uint64_t ShaExpectedResult[] = {0xd114cdfbce9c1b52,0x87527787bba1e779,0x668ab23809626d0a,0x9d5b80e6c6ea07a1,0x805030575bf48909,0x39d44708715eaa41,0xf155932c31cd74ea,0x501de4408de0daf2};
	uint64_t RandExpectedResult[] = {0xf773127ce8f5a2e1,0xeb42129a140b4d18,0x3819bdbf3a840691,0xbfb5b3754d337b1d,0x3f71a5c9c9dd9ce8,0xb33092ed5197efcc,0xb8f8d7c1d7ac01db,0x7faed22e938379de};
	uint64_t RandExpectedResult2[] = {0x39978df7749403fd,0x4d521311536f0227,0xe4d9d8ad0ba0d9ae,0x19a9e732b1201625,0x0ad5d28cc91e36c4,0x673b48523df37201,0xea16ecbd241301a8,0x000000213793b313};

	int i;

	sha512Hash((uint8_t *)ShaTestStr, strlen(ShaTestStr), (uint8_t *)ShaResult);
	if(memcmp(ShaResult, ShaExpectedResult, sizeof(ShaExpectedResult))) {
		printf("SHA256 failed:\nExpected: ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(ShaExpectedResult[i]));
		}
		printf("\nGot:      ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(ShaResult[i]));
		}
		printf("\n");
	}
	else {
		printf("SHA256 valid\n");
	}

	NautilusInitRandData(ShaTestStr, strlen(ShaTestStr));
	NautilusGetRandData(ShaResult, sizeof(ShaResult));
	if(memcmp(ShaResult, RandExpectedResult, sizeof(ShaExpectedResult))) {
		printf("Rand failed:\nExpected: ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(RandExpectedResult[i]));
		}
		printf("\nGot:      ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(ShaResult[i]));
		}
		printf("\n");
	}
	else {
		printf("Rand valid\n");
	}

	memset(ShaResult, 0, sizeof(ShaResult));
	NautilusGetRandData(ShaResult, sizeof(ShaResult) - 3);
	if(memcmp(ShaResult, RandExpectedResult2, sizeof(ShaExpectedResult))) {
		printf("Rand partial failed:\nExpected: ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(RandExpectedResult2[i]));
		}
		printf("\nGot:      ");
		for(i = 0; i < 512 / 8 / 8; i++) {
			if(i) printf("-");
			printf("%016lx", __builtin_bswap64(ShaResult[i]));
		}
		printf("\n");
	}
	else {
		printf("Partial Rand valid\n");
	}

	return 0;
}
