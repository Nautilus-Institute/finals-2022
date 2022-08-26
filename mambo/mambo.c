/*
 * There should be three seperate bugs in this thing
 * There is an out of bound memory read that can be used to read the flag on the heap
 * There is an out of bound write in case 2 of the register parsing which will allow you to overwrite a pointer in abort and trigger for codex
 * There is heap corruption with arb read and write, where you can use the push and pop commands in order to arb read/write heap data and most likely steal teh got
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "mambo.h"
/*
	Two sets of command packets, one if high bit is set
	If high bit is not set, it will have up to two things, one arithmatic and one that is a load store
	If high bit is set it will do a a vectorized instruction applying to a range of things
*/




struct regStruct regs;
int cyclesToKill = (1<<24);
uint8_t * fakeHeap;
int baseFD;
uint8_t * dataBuffer;
size_t dataOffset;
size_t dataRead;
uint8_t * oldFlagData;
int numConnections = 0;
int shouldRekey = 1;
int shouldWrite = 1;
struct cryptoStateStruct clientCryptoState;
struct cryptoStateStruct serverCryptoState;



inline __attribute__((always_inline))
uint8_t PRGA(struct cryptoStateStruct * cryptoState)
{
	uint8_t ret = cryptoState->stateBuf[cryptoState->ctr];
	uint8_t temp;

	ret += cryptoState->stateBuf[cryptoState->last];
	ret ^= cryptoState->stateBuf[cryptoState->last ^ cryptoState->ctr];

	cryptoState->ctr++;
	cryptoState->last = cryptoState->stateBuf[ret];

	temp = cryptoState->stateBuf[cryptoState->ctr];
	cryptoState->stateBuf[cryptoState->ctr] = cryptoState->stateBuf[cryptoState->last];
	cryptoState->stateBuf[cryptoState->last] = temp;

	return ret;
}

inline __attribute__((always_inline))
void writeBuffer(void * buffer, size_t numBytes)
{
	LOGI("Writing buffer of size %ld\n", numBytes);
	uint8_t * buf = buffer;
	uint8_t * buffy;
	int bytesWritten = 0;

	if(shouldWrite == 0)
	{
		return;
	}
	
	buffy = malloc(numBytes);

	if(write(baseFD, &numBytes, 4) != 4)
	{
		exit(200);
	}

	for(int i = 0 ; i < numBytes; i++)
	{
		uint8_t modVal = PRGA(&serverCryptoState);
		buffy[i] = buf[i] ^ modVal;
	}

	while(numBytes != bytesWritten)
	{
		int tempWritten = write(baseFD, buffy, numBytes - bytesWritten);
		if(tempWritten < 1)
		{
			exit(201);
		}
		bytesWritten += tempWritten;
	}

	free(buffy);
}

int fillBuffer(void)
{
	errno = 0;
	uint32_t bytesToRead;
	uint32_t bytesRead;
	ssize_t numRead;

	bytesRead =read(baseFD, &bytesToRead, 4);
	if(errno != 0 || bytesRead != 4)
	{
		exit(111);
	}
	
	bytesRead = 0;
	
	if(bytesToRead > BUF_LENGTH)
	{
		exit(9);
	}

	LOGI("Reading buffer of %03d %03d\n", bytesRead, bytesToRead);
	while(bytesRead != bytesToRead)
	{
		numRead = read(baseFD, dataBuffer+bytesRead, bytesToRead - bytesRead);
		if(numRead < 1)
		{
			exit(13);
		}
		bytesRead += numRead;
		LOGI("Read %d of %d \n", bytesRead, bytesToRead);
	}

	for(int i = 0; i < bytesRead; i++)
	{
		uint8_t modVal = PRGA(&clientCryptoState);
		dataBuffer[i] ^= modVal;
	}

	dataRead = bytesRead;
	dataOffset = 0;
	return bytesRead;
}

void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}



inline __attribute__((always_inline))
uint64_t getRegisters(uint8_t * bufs, int length, struct regSet * rs)
{
	uint64_t ret;

	if(length == 0)
	{
		rs->r1 = bufs[0] >> 3;
		rs->r2 = bufs[1] >> 3;
		if((bufs[0] & 7) || (bufs[1] & 7))
		{
			exit(19);
		}
	}
	else if (length == 1)
	{
		rs->r1 = bufs[0];
		rs->r2 = bufs[1];
		rs->r3 = bufs[2];
	}
	else
	{
		LOGI("INVALID LENGTH=%d \n", length);
		exit(21);
	}
	ret = (rs->r1 << 16) | (rs->r2 << 8) | rs->r3;

	return ret;
}

int parseInstruction(uint8_t * buf, size_t bytesRemaining)
{
	uint8_t instrLength = buf[0] >> INSTR_SIZE_OFFSET;
	uint8_t instrType = buf[0] & INSTR_MASK;

	int bytesConsumed;
	uint64_t temp;
	struct regSet rs;

	memset(&rs, 0, sizeof(struct regSet));

	if(bytesRemaining == 0)
	{
		return -1;
	}

	LOGV("Instruction length = %02x type = %02x encoded = %02x\n", instrLength, instrType, buf[0]);
	bytesRemaining -= 1;

	while (instrType > 0)
	{
		cyclesToKill--;
		size_t offsetToBuffer;
		switch (instrType & 0xf8)
		{
			case ADD_CMD:
				instrType -= ADD_CMD;
				
				temp = getRegisters(buf+1, 0, &rs);

				LOGV("Going to add\n");

				temp = regs.base[rs.r1];
				regs.base[rs.r1] += regs.base[rs.r2];

				break;
			case SUB_CMD:
				LOGV("Going to sub\n");
				instrType -= SUB_CMD;
				temp = getRegisters(buf+1, 0, &rs);
				regs.base[rs.r1] -= regs.base[rs.r2];
				break;
			case MUL_CMD:
				LOGV("Going to MUL\n");
				instrType -= MUL_CMD;

				temp = getRegisters(buf+1, 0, &rs);

				regs.base[rs.r1] *= regs.base[rs.r2];

				break;

			case DIV_CMD:
				LOGV("Going to DIV\n");
				instrType -= DIV_CMD;

				temp = getRegisters(buf+1, 0, &rs);

				if(regs.base[rs.r2] != 0){
					regs.base[rs.r1] /= regs.base[rs.r2];
				}
				else
				{
					regs.base[rs.r1] = 0;
				}


				break;
			case XOR_CMD:
				LOGV("Going to XOR\n");
				instrType -= XOR_CMD;

				temp = getRegisters(buf+1, 0, &rs);

				temp = regs.base[rs.r1];
				regs.base[rs.r1] ^= regs.base[rs.r2];

				break;

			case ROR_CMD:
				LOGV("Going to ROR\n");
				instrType -= ROR_CMD;

				temp = getRegisters(buf+1, 0, &rs);

				temp = regs.base[rs.r1] >> rs.r2;
				regs.base[rs.r1] =  (regs.base[rs.r1] << rs.r2) | temp;

				break;
			case ROL_CMD:
				LOGV("Going to ROL\n");
				instrType -= ROL_CMD;

				temp = getRegisters(buf+1, 0, &rs);

				temp = regs.base[rs.r1] << rs.r2;
				regs.base[rs.r1] =  (regs.base[rs.r1] >> rs.r2) | temp;
				break;

			case RD_REG:
				LOGV("Reading a value\n");

				instrType -= RD_REG;
				temp = getRegisters(buf+1, instrLength, &rs);

				regs.base[rs.r1] = rs.r2 | (rs.r3 << 8);

				break;

			case WR_REG:
				instrType -= WR_REG;
				temp = getRegisters(buf+1, instrLength, &rs);

				LOGV("Writing a value %ld %d\n", regs.base[rs.r1], rs.r1);
				writeBuffer(&regs.base[rs.r1], sizeof(uint64_t));

				break;

			case WR_BUF:
				instrType -= WR_BUF;
				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = ((regs.base[rs.r1] + rs.r2) % FAKE_HEAP_SIZE );
				
				writeBuffer(offsetToBuffer + fakeHeap, regs.base[rs.r3]);
				break;

			case RD_BUF:
				instrType -= RD_BUF;

				LOGI("Reading to offset buffer\n");

				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = ((regs.base[rs.r1] + rs.r2) % FAKE_HEAP_SIZE );
				for(int i = 0 ; i < rs.r3; i++)
				{
					*(uint8_t*)(fakeHeap + offsetToBuffer + i) = (buf+4)[i];
				}
				instrLength = rs.r3 + 4;
				LOGI("Read %d bytes into buffer\n", rs.r3);
				break;

			case BYE_CMD:
				LOGI("EXIT\n");
				exit(0);
				break;
			case 0:
				break;				

			default:
				LOGI("Illegal instruction\n");
				exit(1);
				break;
		}
		switch(instrType & 0x7)
		{
			case LD4_CMD:
				
				instrType -= LD4_CMD;
				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = regs.base[rs.r1] % (FAKE_HEAP_SIZE-8) ;
				regs.base[rs.r2] = *(uint32_t*)(fakeHeap + offsetToBuffer);
				LOGV("LD4 %02d\n", rs.r2);
				break;

			case LD8_CMD:
	
				instrType -= LD8_CMD;
				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = regs.base[rs.r1] %  (FAKE_HEAP_SIZE-8) ;
				regs.base[rs.r2] = *(uint64_t*)(fakeHeap + offsetToBuffer);
				LOGV("LD8 %02d\n", rs.r2);
				break;

			case ST4_CMD:
				LOGV("ST4\n");

				instrType -= ST4_CMD;
				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = regs.base[rs.r1] %  (FAKE_HEAP_SIZE-8) ;
				*(uint32_t*)(fakeHeap + offsetToBuffer) = regs.base[rs.r2];
				break;
				
			case ST8_CMD:
	
				instrType -= ST8_CMD;
				temp = getRegisters(buf+1, instrLength, &rs);

				offsetToBuffer = regs.base[rs.r1] %  (FAKE_HEAP_SIZE-8) ;
				*(uint64_t*)(fakeHeap + offsetToBuffer) = regs.base[rs.r2];
				break;

			case PUSH_CMD:
	
				instrType -= PUSH_CMD;
				temp = getRegisters(buf+1, 1, &rs);
				*(uint64_t*)(fakeHeap + regs.sp) = regs.base[rs.r1];
				regs.sp += 8;
				LOGV("PUSH R%02d\n", rs.r1);
				break;

			case POP_CMD:

				instrType -= POP_CMD;
				temp = getRegisters(buf+1, 1, &rs);
				regs.base[rs.r1] = *(uint64_t*)(fakeHeap + (regs.sp-8));
				regs.sp -= 8;
				LOGV("POP R%02d\n", rs.r1);
				break;

			case 0:
				break;				
			default:
				LOGI("Illegal instruction 2\n");
				exit(1);
				break;
		}
	}
	regs.base[0] = 0;
	if(instrLength == 0)
	{
		bytesConsumed = 3;
	}
	else if(instrLength == 1)
	{
		bytesConsumed = 4;
	}
	else
	{
		bytesConsumed = instrLength;
	}

	return bytesConsumed;
}



void setRekey(int signum)
{
	shouldRekey = 1;
}

void initCryptoState(uint8_t * origCryptoKey, struct cryptoStateStruct * cryptoState, int server)
{
	uint8_t cryptoKey[CRYPTO_KEY_LEN];	
	memcpy(cryptoKey, origCryptoKey, CRYPTO_KEY_LEN);

	for(int i = 0; i < CRYPTO_KEY_LEN; i++)
	{
		cryptoKey[i] = cryptoKey[i] + (i * 101) + server;
	}
	
	for(int i = 0; i < CRYPTO_STATE_LEN; i++)
	{
		cryptoState->stateBuf[i] = i;
	}
	
	for(int i = 0; i < (CRYPTO_STATE_LEN * 20); i++)
	{
		uint8_t temp = cryptoState->stateBuf[i % CRYPTO_STATE_LEN];;
		uint8_t jIdx = (i+cryptoKey[i % CRYPTO_KEY_LEN]) & 0xff;
		
		cryptoState->stateBuf[i % CRYPTO_STATE_LEN] = cryptoState->stateBuf[ jIdx ];
		cryptoState->stateBuf[ jIdx ] = temp;
	}
	cryptoState->modifier = server;
	cryptoState->last     = cryptoState->stateBuf[cryptoState->stateBuf[cryptoState->stateBuf[1]]] ;
}

void doRekey(void)
{
	LOGI("[-] Rekeying\n");
	struct stat st;
	uint8_t * flagData = malloc(FAKE_HEAP_SIZE);
	int fd;
	size_t fileSize;
	uint8_t * basePtr = (uint8_t*) &regs;
	
	fd = stat(FLAG_PATH, &st);
	if(fd == -1)
	{
		if(errno == ENOENT)
		{
			exit(0);
		}
	}

	memset(flagData, 0, FAKE_HEAP_SIZE);

	fileSize = st.st_size;
	fd = open(FLAG_PATH, O_RDONLY);
	read(fd, flagData, fileSize);
	close(fd);

	if(oldFlagData != NULL){
		free(oldFlagData);
	}
	oldFlagData = flagData;

	shouldRekey = 0;
	
	SHA512(flagData, fileSize, basePtr);

	regs.base[0] = regs.base[0] - numConnections;

	initCryptoState((void*)&regs, &serverCryptoState, 3);
	initCryptoState((void*)&regs, &clientCryptoState, 9);


	alarm(4);
}

void goAway(int sig)
{
	exit(0);
}

#ifndef CLIENT
int main()
{
	int serverFD = do_tcp_listen("0.0.0.0", MAMBO_PORT);
	signal(SIGALRM, setRekey);
struct sigaction sa;
sa.sa_handler = &handle_sigchld;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
if (sigaction(SIGCHLD, &sa, 0) == -1) {
  perror(0);
  exit(1);
}


	LOGI("[-] Signal set\n");
	fakeHeap = malloc(FAKE_HEAP_SIZE);
	memset(fakeHeap, 0, FAKE_HEAP_SIZE);

	while(1) {

		if(shouldRekey == 1)
		{
			doRekey();
		}

		int child;
		LOGI("[-] Accepting client\n");
		baseFD = do_tcp_accept(serverFD);

		child = fork();
		if(child == -1)
		{
			exit(1);
		}
		if(child == 0)
		{
			break;
		}
		numConnections++;
		close(baseFD);
	}
	signal(SIGALRM, goAway);
	alarm(15);
	doChallenge();
	
}
#endif //CLIENT

void  doChallenge(void)
{
	dataBuffer = malloc(BUF_LENGTH);
	memset(dataBuffer, 0, BUF_LENGTH);

	write(baseFD, &regs, CRYPTO_KEY_LEN);
	for(int i = 0; i < NUM_BASE_REG; i++)
	{
		regs.base[i] = 0 ;		
	}
	regs.sp = FAKE_HEAP_SIZE/2 ;
	
	fillBuffer();

	while(1)
	{
		int parsedBytes = parseInstruction(dataBuffer + dataOffset, dataRead - dataOffset);

		if(parsedBytes <= 0)
		{
			LOGI("Failed to parse bytes\n");
			exit(1);
		}
		dataOffset += parsedBytes;
		
		if(dataOffset >= dataRead)
		{
			LOGI("Filling\n");
			fillBuffer();
			dataOffset = 0;
			LOGI("Filled buffer doing %zu for %zu\n",dataOffset, dataRead);
			if(dataRead == 0)
			{
				exit(0);
			}
		}
	}

	LOGI("MyPID=%d\n",getpid());

	exit(0);
}

int do_tcp_listen(const char *server_ip, uint16_t port)
{
    struct sockaddr_in addr;
    int optval = 1;
    int lfd;
    int ret;

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        LOGI("Socket creation failed\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    if (inet_aton(server_ip, &addr.sin_addr) == 0) {
        LOGI("inet_aton failed\n");
        goto err_handler;
    }
    addr.sin_port = htons(port);

    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
        LOGI("set sock reuseaddr failed\n");
    }
    ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret) {
        LOGI("bind failed %s:%d\n", server_ip, port);
        goto err_handler;
    }

    LOGI("TCP listening on %s:%d...\n", server_ip, port);
    ret = listen(lfd, TCP_MAX_LISTEN_COUNT);
    if (ret) {
        LOGI("listen failed\n");
        goto err_handler;
    }
    LOGI("TCP listen fd=%d\n", lfd);
    return lfd;
err_handler:
    close(lfd);
    return -1;
}




int do_tcp_accept(int lfd)
{
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int cfd;

    LOGI("\n\n###Waiting for TCP connection from client...\n");
    cfd = accept(lfd, (struct sockaddr *)&peeraddr, &peerlen);
    if (cfd < 0) {
        LOGI("accept failed, errno=%d\n", errno);
        return -1;
    }

    LOGI("TCP connection accepted fd=%d\n", cfd);
    return cfd;
}
