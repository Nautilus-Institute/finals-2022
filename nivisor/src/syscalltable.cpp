#include "syscall.h"

#include <unistd.h>

SyscallInfo_t kDefaultSyscallTable[] = {
               // #0
               {
                .fn = sys_exit,
                .n_args = 1,
                .is_terminating = 1,
               },
               // #1
               {
                .fn = sys_split_proc,
                .n_args = 1,
                .is_terminating = 0,
               },
               // #2
               {
                .fn = sys_get_spid,
                .n_args = 0,
                .is_terminating = 0,
               },
               // #3
               {
                .fn = sys_write_console,
                .n_args = 0,
                .is_terminating = 0,
               },
               // #4
               {
                .fn = sys_read_console,
                .n_args = 0,
                .is_terminating = 0,
               },
               // #5
               {
                .fn = sys_new_anon_region,
                .n_args = 3,
                .is_terminating = 0,
               },
#ifndef NIVISOR_REMOVE_MPROTECT
               // #6
               {
                .fn = sys_change_region_prot,
                .n_args = 3,
                .is_terminating = 0,
               },
#else
               // #6
               {
                .fn = NULL,
                .n_args = 0,
                .is_terminating = 0,
               },
#endif
               // #7
               {
                .fn = sys_exec_proc,
                .n_args = 2,
                .is_terminating = 0,
               },
               // #8
               {
                .fn = sys_open_file,
                .n_args = 2,
                .is_terminating = 0,
               },
               // #9
               {
                .fn = sys_read_file,
                .n_args = 3,
                .is_terminating = 0,
               },
               // #10
               {
                .fn = sys_write_file,
                .n_args = 3,
                .is_terminating = 0,
               },
               // #11
               {
                .fn = sys_close_file,
                .n_args = 1,
                .is_terminating = 0,
               },
#ifdef CFS
               // #14
               {
                .fn = sys_setsid,
                .n_args = 0,
                .is_terminating = 1,
               },
#endif
};

uint64_t kDefaultSyscallTableLength = sizeof(kDefaultSyscallTable) / \
                                    sizeof(SyscallInfo_t);
