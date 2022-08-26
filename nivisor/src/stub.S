#include <sys/syscall.h>
#define SIGSTOP 19
        
        .section .text
        .globl _stubStart
        .type _stubStart, @function
_stubStart:
	// get our own pid for the upcoming kill
        ;; mov $SYS_getpid, %rax
        ;; syscall

	// attempting 0 as target pid here to simplify
        // any global seccomp filter application
	mov $0, %rdi
	mov $SIGSTOP, %rsi
	mov $SYS_kill, %rax
	syscall

trap:
	int3
        jmp trap
	
        ret

        .section .text
        .globl _stubEnd
        .type _stubEnd, @function
_stubEnd:
        ret
