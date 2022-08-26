#include <string.h>
#include "vm.h"
#include "vis.h"

// WRAP AROUND 
#define W(pos, proc)  (((pos - proc->player->arena_start) % proc->player->arena_size) + proc->player->arena_start)
// Check: if the address is at the upper boundary, make it the lower boundary instead
#define CH(pos, proc) (pos == proc->player->arena_end? proc->player->arena_start: pos)
#define CH2(pos, proc) (pos == proc->player->arena_end + 1? proc->player->arena_start + 1: pos)

#define RESOLVE_A \
    switch (instr->a_mode) { \
        case OPMODE_MEM: \
            a += proc->registers[REG_PC]; \
            { \
                /* recalculate sub-arena start and end */ \
                /* this allows programs to keep functioning after crossing boundaries */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            /* Load an instruction */ \
            ins = vm->arena[a]; \
            vis_read(proc->proc_id, a); \
            break; \
        case OPMODE_CONST: \
            /* a = a */; \
            break; \
        case OPMODE_RELATIVE: \
            a += proc->registers[REG_PC]; \
            { \
                /* recalculate sub-arena start and end */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            INSTR relative_instr = vm->arena[a]; \
            vis_read(proc->proc_id, a); \
            a = relative_instr.a + a; \
            { \
                /* recalculate sub-arena start and end */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            break; \
        default: \
            proc->state = DEAD; \
            vis_proc_dead(proc->proc_id); \
            return; \
            break; \
    }

#define RESOLVE_B \
    switch (instr->b_mode) { \
        case OPMODE_MEM: \
            b += proc->registers[REG_PC]; \
            { \
                /* recalculate sub-arena start and end */ \
                /* this allows programs to keep functioning after crossing boundaries */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            break; \
        case OPMODE_CONST: \
            /* b = b */ \
            break; \
        case OPMODE_RELATIVE: \
            b += proc->registers[REG_PC]; \
            { \
                /* recalculate sub-arena start and end */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            INSTR relative_instr = vm->arena[b]; \
            vis_read(proc->proc_id, b); \
            b = relative_instr.b + b; \
            { \
                /* recalculate sub-arena start and end */ \
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size; \
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start; \
            } \
            break; \
        default: \
            proc->state = DEAD; \
            vis_proc_dead(proc->proc_id); \
            return; \
            break; \
    }

#define RESOLVE_B_INSB \
    switch (instr->b_mode) { \
        case OPMODE_MEM: \
            b += proc->registers[REG_PC]; \
            b = W(b, proc); \
            ins_b = vm->arena[b]; \
            vis_read(proc->proc_id, b); \
            break; \
        case OPMODE_CONST: \
            /* b = b */ \
            break; \
        case OPMODE_RELATIVE: \
            proc->state = DEAD; \
            vis_proc_dead(proc->proc_id); \
            return; \
            break; \
        default: \
            proc->state = DEAD; \
            vis_proc_dead(proc->proc_id); \
            return; \
            break; \
    }


void exec_mov(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;

    // Special: We allow writing across boundaries by 10 instrutions
    // This way the warrior can "throw" at most 10 instructions across the boundary and execute them
    switch (instr->b_mode) {
        case OPMODE_MEM:
            b += proc->registers[REG_PC];
            b = b % CORE_SIZE;
            if ((b - proc->player->arena_end) <= 10) {
                // do nothing
                ;
            } else { 
                /* recalculate sub-arena start and end */
                /* this allows programs to keep functioning after crossing boundaries */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            break;
        case OPMODE_CONST:
            /* b = b */
            break;
        case OPMODE_RELATIVE:
            b += proc->registers[REG_PC];
            {
                /* recalculate sub-arena start and end */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            INSTR relative_instr = vm->arena[b];
            vis_read(proc->proc_id, b);
            b = relative_instr.b + b;
            if ((b - proc->player->arena_end) <= 10) {
                // do nothing
                ;
            } else {
                /* recalculate sub-arena start and end */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                b = ((b - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            break;
        default:
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
            break;
    }

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    vm->arena[b] = ins;
    vis_write(proc->proc_id, b);
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_add(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        return;
    }

    switch (instr->a_mode) {
        case OPMODE_MEM:
            // Add an entire instruction
            vm->arena[b].opcode += ins.opcode;
            vm->arena[b].a += ins.a;
            vm->arena[b].a_mode += ins.a_mode;
            vm->arena[b].b += ins.b;
            vm->arena[b].b_mode += ins.b_mode;
            vis_write(proc->proc_id, b);
            break;
        case OPMODE_CONST:
            // Add the field
            vm->arena[b].b += a;
            vis_write(proc->proc_id, b);
            break;
        default:
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
            break;
    }
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_sub(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        return;
    }

    switch (instr->a_mode) {
        case OPMODE_MEM:
            // Subtract an entire instruction
            vm->arena[b].opcode -= ins.opcode;
            vm->arena[b].a -= ins.a;
            vm->arena[b].a_mode -= ins.a_mode;
            vm->arena[b].b -= ins.b;
            vm->arena[b].b_mode -= ins.b_mode;
            vis_write(proc->proc_id, b);
            break;
        case OPMODE_CONST:
            // Subtract the field
            vm->arena[b].b -= a;
            vis_write(proc->proc_id, b);
            break;
        default:
            proc->state = DEAD;
            return;
            break;
    }
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_mul(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        return;
    }

    switch (instr->a_mode) {
        case OPMODE_MEM:
            // Multiply an entire instruction
            vm->arena[b].opcode *= ins.opcode;
            vm->arena[b].a *= ins.a;
            vm->arena[b].a_mode *= ins.a_mode;
            vm->arena[b].b *= ins.b;
            vm->arena[b].b_mode *= ins.b_mode;
            break;
        case OPMODE_CONST:
            // Multiply the field
            vm->arena[b].b *= a;
            break;
        default:
            proc->state = DEAD;
            return;
            break;
    }
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_div(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        return;
    }

    switch (instr->a_mode) {
        case OPMODE_MEM:
            // Divide by an entire instruction
            if (ins.opcode == 0 ||
                    ins.a == 0 ||
                    ins.a_mode == 0 ||
                    ins.b == 0 ||
                    ins.b_mode == 0) {
                proc->state = DEAD;
                return;
            }
            vm->arena[b].opcode /= ins.opcode;
            vm->arena[b].a /= ins.a;
            vm->arena[b].a_mode /= ins.a_mode;
            vm->arena[b].b /= ins.b;
            vm->arena[b].b_mode /= ins.b_mode;
            vis_write(proc->proc_id, b);
            break;
        case OPMODE_CONST:
            // Divide by the field
            if (a == 0) {
                proc->state = DEAD;
                return;
            }
            vm->arena[b].b /= a;
            vis_write(proc->proc_id, b);
            break;
        default:
            proc->state = DEAD;
            return;
            break;
    }
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_mod(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->b_mode == OPMODE_CONST) {
        // You cannot use a const as the destination
        return;
    }

    switch (instr->a_mode) {
        case OPMODE_MEM:
            // Modulo an entire instruction
            if (ins.opcode == 0 ||
                    ins.a == 0 ||
                    ins.a_mode == 0 ||
                    ins.b == 0 ||
                    ins.b_mode == 0) {
                proc->state = DEAD;
                return;
            }
            vm->arena[b].opcode %= ins.opcode;
            vm->arena[b].a %= ins.a;
            vm->arena[b].a_mode %= ins.a_mode;
            vm->arena[b].b %= ins.b;
            vm->arena[b].b_mode %= ins.b_mode;
            vis_write(proc->proc_id, b);
            break;
        case OPMODE_CONST:
            // Divide by the field
            if (a == 0) {
                proc->state = DEAD;
                return;
            }
            vm->arena[b].b %= a;
            vis_write(proc->proc_id, b);
            break;
        default:
            proc->state = DEAD;
            return;
            break;
    }
    // Increment the PC
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

void exec_jmp(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a;

    INSTR ins = {0};

    RESOLVE_A;

    if (instr->a_mode == OPMODE_CONST) {
        // do not support absolute jumps
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    {
        /* recalculate sub-arena start and end */
        size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
        a = ((a - subarena_start) % proc->player->arena_size) + subarena_start;
    }
    proc->registers[REG_PC] = a;
}

void exec_jmz(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->a_mode == OPMODE_CONST) {
        // do not support absolute jumps
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (b == 0) {
        proc->registers[REG_PC] = a;
        proc->registers[REG_PC] = W(proc->registers[REG_PC], proc);
    } else {
        // Vuln: Failed jmz may jump across the arena boundary!
        proc->registers[REG_PC] += 1;
        proc->registers[REG_PC] %= CORE_SIZE;
    }
}

void exec_jmn(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};

    RESOLVE_A;
    RESOLVE_B;

    if (instr->a_mode == OPMODE_CONST) {
        // do not support absolute jumps
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (b != 0) {
        proc->registers[REG_PC] = a;
        proc->registers[REG_PC] = W(proc->registers[REG_PC], proc);
    } else {
        proc->registers[REG_PC] += 1;
        proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    }
}

void exec_djn(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b;

    INSTR ins = {0};

    RESOLVE_A;
    switch (instr->b_mode) {
        case OPMODE_MEM:
            instr->b--;
            b = instr->b;
            b += proc->registers[REG_PC];
            b = W(b, proc);
            break;
        case OPMODE_CONST:
            instr->b --;
            b = instr->b;
            break;
        case OPMODE_RELATIVE:
            instr->b --;
            b = instr->b;
            b += proc->registers[REG_PC];
            b = W(b, proc);
            INSTR relative_instr = vm->arena[b];
            vis_read(proc->proc_id, b);
            b = relative_instr.b + b;
            b = W(b, proc);
            break;
        default:
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
            break;
    }

    // Vuln: You can use djn to do direct jumps!

    if (b == 0) {
        proc->registers[REG_PC] = a;
        proc->registers[REG_PC] = W(proc->registers[REG_PC], proc);
    } else {
        proc->registers[REG_PC] += 1;
        proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    }
}

uint32_t exec_spl(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a;
    INSTR ins;

    switch (instr->a_mode) {
        case OPMODE_MEM:
            a += proc->registers[REG_PC];
            {
                /* recalculate sub-arena start and end */
                /* this allows programs to keep functioning after crossing boundaries */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            /* Load an instruction */
            ins = vm->arena[a];
            vis_read(proc->proc_id, a);
            break;
        case OPMODE_CONST:
            /* a = a */;
            break;
        case OPMODE_RELATIVE:
            /* TODO: What does this mean? */ \
            a += proc->registers[REG_PC];
            {
                /* recalculate sub-arena start and end */
                /* this allows programs to keep functioning after crossing boundaries */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            INSTR relative_instr = vm->arena[a];
            vis_read(proc->proc_id, a);
            a = relative_instr.a + a;
            {
                /* recalculate sub-arena start and end */
                /* this allows programs to keep functioning after crossing boundaries */
                size_t subarena_start = (proc->registers[REG_PC] / proc->player->arena_size) * proc->player->arena_size;
                a = ((a - subarena_start) % proc->player->arena_size) + subarena_start;
            }
            break;
        default:
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return -1;
            break;
    }

    return a;
}

void exec_cmp(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};
    INSTR ins_b = {0};

    RESOLVE_A;
    RESOLVE_B_INSB;

    int cmp;
    if (instr->a_mode == OPMODE_CONST) {
        if (instr->b_mode == OPMODE_MEM) {
            cmp = a == ins_b.b;
        } else if (instr->b_mode == OPMODE_CONST) {
            cmp = a == b;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else if (instr->a_mode == OPMODE_MEM) {
        if (instr->b_mode == OPMODE_CONST) {
            cmp = ins.b == a;
        } else if (instr->b_mode == OPMODE_MEM) {
            // Compare two instructions
            cmp = memcmp(&ins, &ins_b, sizeof(ins)) == 0;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else {
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (cmp) {
        proc->registers[REG_PC] += 2;
    } else {
        proc->registers[REG_PC] += 1;
    }
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    proc->registers[REG_PC] = CH2(proc->registers[REG_PC], proc);
}

void exec_seq(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};
    INSTR ins_b = {0};

    RESOLVE_A;
    RESOLVE_B_INSB;

    int cmp;
    if (instr->a_mode == OPMODE_CONST) {
        if (instr->b_mode == OPMODE_MEM) {
            cmp = a == ins_b.b;
        } else if (instr->b_mode == OPMODE_CONST) {
            cmp = a == b;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else if (instr->a_mode == OPMODE_MEM) {
        if (instr->b_mode == OPMODE_CONST) {
            cmp = ins.b == a;
        } else if (instr->b_mode == OPMODE_MEM) {
            // Compare two instructions
            cmp = memcmp(&ins, &ins_b, sizeof(ins)) == 0;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else {
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (cmp) {
        // Vuln: seq (although it should have been the same as cmp) may cause jumping across the arena
        proc->registers[REG_PC] += 2;
        proc->registers[REG_PC] %= CORE_SIZE;
    } else {
        proc->registers[REG_PC] += 1;
        proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    }
}

void exec_sne(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};
    INSTR ins_b = {0};

    RESOLVE_A;
    RESOLVE_B_INSB;

    int cmp;
    if (instr->a_mode == OPMODE_CONST) {
        if (instr->b_mode == OPMODE_MEM) {
            cmp = a == ins_b.b;
        } else if (instr->b_mode == OPMODE_CONST) {
            cmp = a == b;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else if (instr->a_mode == OPMODE_MEM) {
        if (instr->b_mode == OPMODE_CONST) {
            cmp = ins.b == a;
        } else if (instr->b_mode == OPMODE_MEM) {
            // Compare two instructions
            cmp = memcmp(&ins, &ins_b, sizeof(ins)) == 0;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else {
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (!cmp) {
        proc->registers[REG_PC] += 2;
    } else {
        proc->registers[REG_PC] += 1;
    }
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    proc->registers[REG_PC] = CH2(proc->registers[REG_PC], proc);
}

void exec_slt(VM* vm, PROCESS* proc, INSTR* instr)
{
    uint32_t a = instr->a, b = instr->b;

    INSTR ins = {0};
    INSTR ins_b = {0};

    RESOLVE_A;
    RESOLVE_B_INSB;

    int lt;
    if (instr->a_mode == OPMODE_CONST) {
        if (instr->b_mode == OPMODE_MEM) {
            lt = a < ins_b.b;
        } else if (instr->b_mode == OPMODE_CONST) {
            lt = a < b;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else if (instr->a_mode == OPMODE_MEM) {
        if (instr->b_mode == OPMODE_CONST) {
            lt = ins.b < a;
        } else if (instr->b_mode == OPMODE_MEM) {
            // Compare two instructions
            // Unsupported
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        } else {
            proc->state = DEAD;
            vis_proc_dead(proc->proc_id);
            return;
        }
    } else {
        proc->state = DEAD;
        vis_proc_dead(proc->proc_id);
        return;
    }

    if (lt) {
        proc->registers[REG_PC] += 2;
    } else {
        proc->registers[REG_PC] += 1;
    }
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
    proc->registers[REG_PC] = CH2(proc->registers[REG_PC], proc);
}

void exec_ldp(VM* vm, PROCESS* proc, INSTR* instr)
{
    // TODO:
}

void exec_stp(VM* vm, PROCESS* proc, INSTR* instr)
{
    // TODO:
}

void exec_nop(VM* vm, PROCESS* proc, INSTR* instr)
{
    proc->registers[REG_PC] += 1;
    proc->registers[REG_PC] = CH(proc->registers[REG_PC], proc);
}

