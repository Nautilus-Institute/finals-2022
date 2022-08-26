#pragma once

#include <stdint.h>

class ThreadContext;

typedef int (*sys_handle)(ThreadContext *, struct user_regs_struct *);


typedef struct SyscallInfo_t {
  sys_handle fn;
  uint32_t n_args;
  bool is_terminating;
} SyscallInfo_t;


int sys_exit(ThreadContext *ctx, struct user_regs_struct * regs);
int sys_is_computer_on(ThreadContext *ctx, struct user_regs_struct * regs);
int sys_get_real_pid(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_split_proc(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_get_spid(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_write_console(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_read_console(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_new_anon_region(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_exec_proc(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_open_file(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_read_file(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_write_file(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_close_file(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_change_region_prot(ThreadContext *ctx, struct user_regs_struct *regs);
int sys_setsid(ThreadContext *ctx, struct user_regs_struct *regs);
