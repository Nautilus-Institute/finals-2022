#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>


#include "mambo.h"

struct cryptoStateStruct randomGenerator;


int baseConnect(char * target, int port)
{
	int socketFD;
	struct sockaddr_in server_addr;
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval timeout;

	if(socketFD < 0)
	{
		printf("Failed to create socket\n");
		return -1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(target);

	if(connect(socketFD,(const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		return -1;
	}
	timeout.tv_sec  = 5;
	timeout.tv_usec = 0;

	if(setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
	{
		printf("[-] Failed to set recv timeout\n");
	}
	if(setsockopt(socketFD, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)))
	{
		printf("[-] Failed to set send timeout\n");
	}

	return socketFD;
}

int encodeInstruction(uint8_t instr, uint8_t r1, uint8_t r2, uint8_t r3, 
	uint8_t instrType, uint8_t * buffer)
{
	r1 = r1 & 0xff;
	r2 = r2 & 0xff; 
	r3 = r3 & 0xff;
	assert(instrType < 2);
	if(instrType == 0)
	{
		buffer[0] = (instr) & 0x7f;
		buffer[1] = r1 << 3 ;
		buffer[2] = r2 << 3;
		return 3;
	}
	if(instrType == 1)
	{
		buffer[0] = (instr) | 0x80;
		buffer[1] = r1;
		buffer[2] = r2;
		buffer[3] = r3;
		return 4;
	}
	return -1;

}

int getData(uint8_t * buffer, int length)
{
	int bytesToRead;
	int numBytes = read(baseFD, &bytesToRead, 4);
	if(numBytes != 4)
	{
		LOGI("Read %d of %d bytes\n", numBytes, 4);
		exit(245);
	}
	if(bytesToRead > length)
	{
		exit(244);
	}
	numBytes = 0;
	while (numBytes < bytesToRead)
	{
		int bytesRead = read(baseFD, buffer, length);
		if(bytesRead < 1)
		{
			exit(240);
		}
		numBytes += bytesRead;
	}

	for(int i =0; i < bytesToRead; i++)
	{
		buffer[i] = buffer[i] ^ PRGA(&clientCryptoState);
	}

	return bytesToRead;
}

int doExploit(int targetFD)
{
	uint8_t keyBuf[CRYPTO_KEY_LEN];
	baseFD = targetFD;
	uint8_t buf[256];
	uint8_t recvBuf[4096*8];

	size_t offset = 0;
	if(read(targetFD, &keyBuf, CRYPTO_KEY_LEN) != 16)
	{
		return FAILED_TO_SET_KEYS;
	}

	initCryptoState(keyBuf, &serverCryptoState, 9);
	initCryptoState(keyBuf, &clientCryptoState, 3);

	printf("INSTR1\n");
	offset += encodeInstruction(RD_REG,  1, 255, 0, 1, buf );
	printf("INSTR2\n");
	offset += encodeInstruction(RD_REG,  2, 16, 0,  1, buf + offset);
	printf("INSTR3\n");
	offset += encodeInstruction(RD_REG,  3, 64, 20, 1, buf + offset);
	printf("INSTR4 %ld\n", offset);
	offset += encodeInstruction(MUL_CMD, 1, 2, 64,  0, buf + offset);
	printf("INSTR4 %ld\n", offset);
	offset += encodeInstruction(WR_BUF,  1, 0, 3,   1, buf + offset);
	printf("INSTR4 %ld\n", offset);
	writeBuffer(&buf,offset);
	sleep(1);

	int nb = getData(recvBuf, 4096*8);
	for(int i = 0; i < nb; i++)
	{
		printf("%02X",recvBuf[i]);
	}
	printf("\n");

	return 0;
}

uint64_t getRegister(uint8_t reg)
{
	uint8_t buf[128];
	int numBytes;
	numBytes = encodeInstruction(WR_REG, reg, 0, 0, 1, buf);
	writeBuffer(buf, numBytes);
	while(getData(buf, 8) != 8)
	{
		sleep(1);
	}
	return *(uint64_t*)buf;
}

size_t setRegister(uint8_t *buf , uint8_t reg, uint8_t valLow, uint8_t valHigh)
{
	size_t numBytes = encodeInstruction(RD_REG, reg, valLow, valHigh, 1, buf);
	return numBytes;
}

size_t sayGoodbye(uint8_t *buf )
{
	size_t numBytes = encodeInstruction(BYE_CMD, 0, 0, 0, 0, buf);
	return numBytes;
}


int parseData(uint8_t * data, size_t length)
{
	size_t offset = 0;
	while(1)
	{
		int parsedBytes = parseInstruction(data + offset, length - offset);
		
		if(parsedBytes <= 0)
		{
			abort();
		}
		offset += parsedBytes;
		LOGV("Parsed %d:%ld of %ld bytes\n", parsedBytes, offset, offset);
		
		if(offset >= length)
		{
			return 0;
		}
	}
}

int doPoll(int targetFD)
{
	uint8_t keyBuf[CRYPTO_KEY_LEN];
	baseFD = targetFD;

	uint8_t recvBuf[4096*3];
	uint8_t sndBuf[4096*8];

	struct regStruct remoteRS;

	uint8_t mathCommands[]   = { ADD_CMD, SUB_CMD, MUL_CMD, DIV_CMD, XOR_CMD, ROL_CMD, ROR_CMD };
	uint8_t memoryCommands[] = { LD4_CMD, LD8_CMD, ST4_CMD, ST8_CMD, PUSH_CMD, POP_CMD };

	uint8_t numRegisters;
	uint8_t registersToUse[40] = { 0 };
	size_t offset = 0;

	int numMathCommands   = PRGA(&randomGenerator) * 2;
	int numIOCommands     = PRGA(&randomGenerator);
	int numMemoryCommands = PRGA(&randomGenerator);

	if(read(targetFD, &keyBuf, CRYPTO_KEY_LEN) != 16)
	{
		return FAILED_TO_SET_KEYS;
	}
	

	for(int i = 0; i < NUM_BASE_REG; i++)
	{
		regs.base[i] = 0 ;		
	}
	regs.sp = FAKE_HEAP_SIZE/2 ;
	
	LOGI("Got Crypto Key ");
	for(int i = 0; i < CRYPTO_KEY_LEN; i++)
	{
		LOGI("%02X ", keyBuf[i]);
	}
	LOGI("\n");

	initCryptoState(keyBuf, &serverCryptoState, 9);
	initCryptoState(keyBuf, &clientCryptoState, 3);
	memset(&remoteRS, 0, sizeof(struct regStruct));

	numRegisters = (PRGA(&randomGenerator) % NUM_BASE_REG) + 6;
	LOGI("Setting %d registers\n", numRegisters);


	for(int i = 0; i < numRegisters; i++)
	{
		registersToUse[i] = PRGA(&randomGenerator) % NUM_BASE_REG;
		LOGI("Setting R%02d\n", registersToUse[i]);
		offset += setRegister(sndBuf + offset, registersToUse[i], PRGA(&randomGenerator), PRGA(&randomGenerator));
	}
	LOGI("Doing %02d math commands\n", numMathCommands);

	for(int i = 0; i < numMathCommands; i++)
	{
		uint8_t r1,r2, command;
		r1=PRGA(&randomGenerator) % numRegisters;
		r2=PRGA(&randomGenerator) % numRegisters;
		command = mathCommands[PRGA(&randomGenerator) % sizeof(mathCommands)];
		offset += encodeInstruction(command, r1, r2, 0, 0, sndBuf + offset);
	}
	
	writeBuffer(sndBuf, offset);

	parseData(sndBuf, offset);

	for(int i = 0; i < NUM_BASE_REG; i++)
	{
		remoteRS.base[i] = getRegister(i);
		LOGI("Reg %02d: %016lX %016lx\n",i, remoteRS.base[i], regs.base[i] );
		if(remoteRS.base[i] != regs.base[i]){
			exit(253);
		}
	}

	numIOCommands = numIOCommands & 1;
	
	LOGI("NIC = %d %d\n", numIOCommands, numIOCommands&1);

	if((numIOCommands & 2) == 2)
	{
		LOGI("Writing out a buffer\n");
		size_t bytesToWrite;
		uint8_t r1, r2, r3;
		uint8_t lenLow, lenHigh;
		
		r1 = PRGA(&randomGenerator) % NUM_BASE_REG;
		r2 = PRGA(&randomGenerator) % NUM_BASE_REG;
		r3 = PRGA(&randomGenerator) % NUM_BASE_REG;

		lenLow = PRGA(&randomGenerator);
		lenHigh = PRGA(&randomGenerator) % 4;
		bytesToWrite = (lenLow | (lenHigh<<8));

		offset += setRegister(sndBuf + offset, r1, PRGA(&randomGenerator), PRGA(&randomGenerator) % 25);
		offset += setRegister(sndBuf + offset, r2, PRGA(&randomGenerator) % 60, 0);
		offset += setRegister(sndBuf + offset, r3, lenLow, lenHigh);
		offset += encodeInstruction(RD_BUF, r1, r2, r3, 1, sndBuf + offset);
		offset += bytesToWrite;
		for(int i = 0; i < bytesToWrite; i++)
		{
			sndBuf[offset++] = PRGA(&randomGenerator);
		}
		writeBuffer(sndBuf, offset);
		shouldWrite = 0;
		LOGI("Before parse %03d\n",__LINE__);
		parseData(sndBuf, offset);
		LOGI("AFTER parse  %03d\n", __LINE__);
		shouldWrite = 1;
		offset = 0;
	}

	if((numIOCommands & 1) == 1)
	{
		LOGI("Reading out a buffer\n");
		size_t bytesToRead = PRGA(&randomGenerator) + PRGA(&randomGenerator);
		uint8_t r1, r2, r3;
		size_t bytesRead;
		uint8_t lenLow, lenHigh;

		r1 = PRGA(&randomGenerator) % NUM_BASE_REG;
		r2 = PRGA(&randomGenerator) % NUM_BASE_REG;
		r3 = PRGA(&randomGenerator) % NUM_BASE_REG;

		lenLow = PRGA(&randomGenerator);
		lenHigh = PRGA(&randomGenerator) % 2;
		bytesToRead = (lenLow | (lenHigh<<8));
		offset += setRegister(sndBuf + offset, r1, PRGA(&randomGenerator), PRGA(&randomGenerator) % 25);
		offset += setRegister(sndBuf + offset, r2, PRGA(&randomGenerator) % 60, 0);
		offset += setRegister(sndBuf + offset, r3, lenLow, lenHigh);
		offset += encodeInstruction(WR_BUF, r1, r2, r3, 1, sndBuf + offset);

		writeBuffer(&sndBuf,offset);
		sleep(1);

		bytesRead = getData(recvBuf, bytesToRead);
		LOGI("BR= %04ld %04ld\n", bytesRead, bytesToRead);

		shouldWrite = 0;
		LOGI("Before parse %03d\n",__LINE__);
		parseData(sndBuf, offset);
		LOGI("AFTER parse  %03d\n", __LINE__);
		shouldWrite = 1;

	}

	offset = 0;

	for(int i = 0; i < numMemoryCommands; i++)
	{
		uint8_t whichCmd = memoryCommands[PRGA(&randomGenerator) % sizeof(memoryCommands)];
		uint8_t r1, r2;
		r1 = PRGA(&randomGenerator) % NUM_BASE_REG;
		r2 = PRGA(&randomGenerator) % NUM_BASE_REG;

		offset += encodeInstruction(whichCmd, r1, r2, 0, 1,  sndBuf + offset);
	}

	writeBuffer(sndBuf, offset);

	LOGI("Before parse\n");
	shouldWrite = 0;
	parseData(sndBuf, offset);
	shouldWrite = 1;
	LOGI("AFTER parse\n");


	for(int i = 0; i < NUM_BASE_REG; i++)
	{
		remoteRS.base[i] = getRegister(i);
		LOGI("Reg %02d: %016lX %016lx\n",i, remoteRS.base[i], regs.base[i] );
		if(remoteRS.base[i] != regs.base[i]){
			LOGI("FAILED\n");
			exit(254);
		}
	}

	LOGI("[+] Success\n");

	return 0;
}

int main(int argc, char *argv[])
{
    
    int targetFD;
	uint8_t cryptoKeyBuf[CRYPTO_KEY_LEN];
	char * target;
	char * seed;
	int port;
	char * portString;

	fakeHeap = malloc(FAKE_HEAP_SIZE);
    dataBuffer = malloc(8192*4);
	memset(fakeHeap, 0, FAKE_HEAP_SIZE);
	memset(cryptoKeyBuf, 0, CRYPTO_KEY_LEN);

	target = getenv("HOST");
	portString = getenv("PORT");
	seed = getenv("SEED");


	if(target == NULL)
	{
		LOGI("Failed to get target\n");
		exit(1);
	}
	if(portString == NULL)
	{
		LOGI("Failed to get port\n");
		exit(2);
	}
	if(seed == NULL)
	{
		LOGI("Failed to get seed\n");
		exit(3);
	}
	port = atoi(portString);
	LOGI("Attempting poll on %s:%d with seed %s\n", target, port, seed);

    targetFD = baseConnect(target,port );

    if(targetFD < 0)
    {
    	printf("[!] Failed to connect to %s\n", target);
    	exit(250);
    }
    
    memset(cryptoKeyBuf, 0, CRYPTO_KEY_LEN);
    

    strncpy((char*)cryptoKeyBuf, seed , CRYPTO_KEY_LEN);

	initCryptoState(cryptoKeyBuf, &randomGenerator, 11);

    return doPoll(targetFD);
}

