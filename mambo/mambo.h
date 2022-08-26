#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INSTR_SIZE_MASK (0x7f)
#define INSTR_SIZE_OFFSET (7)
#define INSTR_MASK (INSTR_SIZE_MASK-1)
#define BUF_LENGTH (4096)

#define FAKE_HEAP_SIZE (8192)
#define CRYPTO_KEY_LEN (16)
#define CRYPTO_STATE_LEN (256)

#define NUM_BASE_REG (32)
#define TCP_MAX_LISTEN_COUNT (20)

#define FLAG_PATH "./flag"

#define ADD_CMD  (0x08)
#define SUB_CMD  (0x10)
#define MUL_CMD  (0x18)
#define DIV_CMD  (0x20)
#define XOR_CMD  (0x28)
#define ROL_CMD  (0x30)
#define ROR_CMD  (0x38)

#define BYE_CMD  (0x40)
#define DSA_CMD  (0x48)

#define WR_REG   (0x50)
#define WR_BUF   (0x58)
#define RD_REG   (0x60)
#define RD_BUF   (0x78)


#define LD4_CMD  (0x01)
#define LD8_CMD  (0x02)
#define ST4_CMD  (0x03)
#define ST8_CMD  (0x04)
#define PUSH_CMD (0x05)
#define POP_CMD  (0x06)




#define MAMBO_PORT (18071)

#define FAILED_TO_SET_KEYS 3

struct regStruct {
	uint64_t base[NUM_BASE_REG];
	uint32_t sp;
}; 
struct regSet {
	uint8_t r1;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
} ;

struct cryptoStateStruct
{
	uint8_t stateBuf[CRYPTO_STATE_LEN];
	uint8_t ctr;
	uint8_t last;
	uint8_t modifier;
}; 

int do_tcp_listen(const char *server_ip, uint16_t port);
int do_tcp_accept(int lfd);
void  doChallenge(void);
void initCryptoState(uint8_t * origCryptoKey, struct cryptoStateStruct * cryptoState, int server);
void writeBuffer(void * buffer, size_t numBytes);
uint8_t PRGA(struct cryptoStateStruct * cryptoState);
int fillBuffer(void);
int parseInstruction(uint8_t * buf, size_t bytesRemaining);

extern  struct regStruct regs;
extern  int cyclesToKill ;
extern  uint8_t * fakeHeap;
extern  int baseFD;
extern  uint8_t * dataBuffer;
extern  size_t dataOffset;
extern  size_t dataRead;
extern  uint8_t * oldFlagData;
extern  int shouldRekey ;
extern struct cryptoStateStruct clientCryptoState;
extern struct cryptoStateStruct serverCryptoState;
extern int shouldWrite;

#ifdef CLIENT
#define LOGI(format, args ...) dprintf(2, format, ## args );
#else
#define LOGI(...)           do { } while(0)
#endif

#define LOGV(...)           do { } while(0)
//#define LOGV printf
