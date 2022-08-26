#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <seccomp.h>
#include <linux/filter.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "vm.h"
#include "cpu.h"
#include "vis.h"

void create_process(VM* vm, PROCESS* processes, PROCESS* parent, uint32_t new_location);

typedef struct area_t {
    size_t start;
    size_t end;
} area;

int load_program(VM* vm, int total_player, int player_id, uint8_t* program, size_t program_size,
        size_t random_offset, area* player_area)
{
    if (program_size > MAX_PROGRAM_SIZE * sizeof(INSTR)) {
        fprintf(stderr, "The program for player %d is too large!\n", player_id);
        return -1;
    }
    // Calculate start position
    int start_pos = (CORE_SIZE / total_player) * player_id;
    // the area between the current start pos and next start pos belongs to this player
    int next_start_pos = (CORE_SIZE / total_player) * (player_id + 1);
    player_area->start = start_pos;
    player_area->end = next_start_pos;
    if (player_id != 0) {
        start_pos += random_offset % (CORE_SIZE / total_player);
        start_pos %= CORE_SIZE;
    }
    memcpy(vm->arena + start_pos, program, program_size);
    return start_pos;
}


int execute_instruction(VM* vm, PROCESS* processes, PROCESS* proc)
{
    uint32_t pc = proc->registers[REG_PC];
    INSTR* instr = &vm->arena[pc];
    switch (instr->opcode) {
        case DAT:
            // Process dies
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: DAT\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            proc->state = DEAD;
            break;
        case MOV:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: MOV\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_mov(vm, proc, instr);
            break;
        case ADD:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: ADD\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_add(vm, proc, instr);
            break;
        case SUB:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: SUB\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_sub(vm, proc, instr);
            break;
        case MUL:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: MUL\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_mul(vm, proc, instr);
            break;
        case DIV:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: DIV\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_div(vm, proc, instr);
            break;
        case MOD:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: MOD\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_mod(vm, proc, instr);
            break;
        case JMP:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: JMP\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_jmp(vm, proc, instr);
            break;
        case JMZ:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: JMZ\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_jmz(vm, proc, instr);
            break;
        case JMN:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: JMN\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_jmn(vm, proc, instr);
            break;
        case DJN:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: DJN\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_djn(vm, proc, instr);
            break;
        case SPL:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: SPL\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            if (proc->state == ACTIVE) {
                int new_location = exec_spl(vm, proc, instr);
                create_process(vm, processes, proc, new_location);
            }
            break;
        case CMP:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: CMP\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_cmp(vm, proc, instr);
            break;
        case SEQ:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: SEQ\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_seq(vm, proc, instr);
            break;
        case SNE:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: SNE\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_sne(vm, proc, instr);
            break;
        case SLT:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: SLT\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_slt(vm, proc, instr);
            break;
        case LDP:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: LDP\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_ldp(vm, proc, instr);
            break;
        case STP:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: STP\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_stp(vm, proc, instr);
            break;
        case NOP:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: NOP\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC]);
#endif
            exec_nop(vm, proc, instr);
            break;
        default:
#ifdef DEBUG
            fprintf(stderr, "[%d:%d] %06x: ILLEGAL (%02x)\n", proc->player->player_id, proc->proc_id, proc->registers[REG_PC], instr->opcode);
#endif
            // Illegal instruction
            proc->state = DEAD;
            break;
    }
    return 0;
}


void create_process(VM* vm, PROCESS* processes, PROCESS* parent, uint32_t new_location)
{
    PLAYER* player = parent->player;
    // ensure the new location is within boundaries
    if (new_location < player->arena_start || new_location >= player->arena_end) {
#ifdef DEBUG
        fprintf(stderr, "You cannot fork outside of your area.\n");
#endif
        return;
    }
    // see if there are more slots in processes that are available
    int new_proc_id = -1;
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        PROCESS* proc = &processes[i];
        if (proc->player == NULL) {
            // available!
            new_proc_id = i;
            break;
        }
    }
    if (new_proc_id < 0) {
#ifdef DEBUG
        fprintf(stderr, "No more process slots.\n");
#endif
        return;
    }
    // Create the process
    PROCESS* proc = &processes[new_proc_id];
    proc->player = player;
    proc->proc_id = new_proc_id;
    proc->state = ACTIVE;
    for (int j = 0; j < MAX_REGS; ++j) {
        proc->registers[j] = 0;
    }
    proc->registers[REG_PC] = new_location;
    // Update player
    player->proc_ids[player->current_procs] = new_proc_id;
    player->current_procs += 1;
#ifdef DEBUG
    fprintf(stderr, "Player %d forks a new process at location %d. It has %d active processes.\n",
            player->player_id, new_location, player->current_procs);
#endif
}

int run_game(VM* vm, PLAYER* players, PROCESS* processes)
{
    for (int i = 0; i < MAX_CYCLE; ++i) {
        int alive_players = 0;
        for (int player_id = 0; player_id < vm->players; ++player_id) {
            PLAYER* player = &players[player_id];
            if (player->current_procs > 0) {
                alive_players += 1;

                uint16_t proc_offset = player->next_proc_offset;
                PROCESS* proc = &processes[player->proc_ids[proc_offset]];
                if (proc->state == ACTIVE) {
                    execute_instruction(vm, processes, proc);
                }
                // if the process dies, clean up
                if (proc->state == DEAD) {
#ifdef DEBUG
                    fprintf(stderr, "%d dead...\n", proc->proc_id);
#endif
                    for (int j = proc_offset; j < player->current_procs - 1; ++j) {
                        player->proc_ids[j] = player->proc_ids[j + 1];
                    }
                    player->current_procs--;
                    if (player->current_procs == 0) {
                        alive_players -= 1;
                    }
                }
                // Update next_proc_offset
                if (player->current_procs != 0) {
                    player->next_proc_offset = (player->next_proc_offset + 1) % player->current_procs;
                }
            }
        }

        if (alive_players == 0) {
            // tie 
            fprintf(stderr, "RESULT: Tie.\n");
            break;
        } else if (alive_players == 1) {
            // winner
            int last_alive_player = -1;
            for (int player_id = 0; player_id < vm->players; ++player_id) {
                if (players[player_id].current_procs > 0) {
                    if (last_alive_player != -1) {
                        fprintf(stderr, "Unexpected: More than one players is alive.\n");
                        _exit(1);
                    }
                    last_alive_player = player_id;
                }
            }
            fprintf(stderr, "RESULT: Player %d won.\n", last_alive_player);
            break;
        } else if (i == MAX_CYCLE - 1) {
            // last iteration but players are still alive
            fprintf(stderr, "RESULT: Tie.\n");
        }
    }
    return 0;
}


void apply_seccomp()
{
#ifndef VIS
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_load(ctx);
    seccomp_release(ctx);
#endif
}


int main(int argc, char** argv)
{
    VM* vm = malloc(sizeof(VM));
    PLAYER* players = malloc(sizeof(PLAYER) * 256);
    PROCESS* processes = malloc(sizeof(PROCESS) * MAX_PROCESSES);
    int player_count = argc - 2;
    const int program_start_argc = 2;

    if (argc < 4) {
        fprintf(stderr, "Insufficient number of arguments.\n");
        return 1;
    }

    memset(vm, 0, sizeof(VM));
    memset(players, 0, sizeof(PLAYER) * 256);
    memset(processes, 0, sizeof(PROCESS) * MAX_PROCESSES);

    // Load the random offset
    if (argv[1] == NULL || strlen(argv[1]) <= 0) {
        return 1;
    }
    for (int i = 0; i < strlen(argv[1]); ++i) {
        if (!isdigit(argv[1][i])) {
            return 1;
        }
    }
    int random_offset = atoi(argv[1]);

    vis_init();
    // Initialize the program
    for (int i = program_start_argc; i < argc; ++i) {
        int player_id = i - program_start_argc;
        FILE *fp = fopen(argv[i], "rb");
        if (fp == NULL) {
            perror("fopen");
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        uint8_t buf[file_size];
        if (fread(buf, 1, file_size, fp) != file_size) {
            perror("fread");
            return 1;
        }
        fclose(fp);
        area player_area;
        int start_pos = load_program(vm, player_count, player_id, buf, file_size, random_offset,
                &player_area);
        if (start_pos < 0) {
            fprintf(stderr, "Failed to load program for player %d\n", player_id);
            return 2;
        }
        // Initialize player
        uint16_t proc_id = player_id;
        players[player_id].player_id = player_id;
        players[player_id].proc_ids[0] = proc_id;
        players[player_id].current_procs = 1;
        players[player_id].arena_start = player_area.start;
        players[player_id].arena_end = player_area.end;
        players[player_id].arena_size = player_area.end - player_area.start;
#ifdef DEBUG
        fprintf(stderr, "Player %d is loaded at %d [arena %u-%u, size %u].\n",
                player_id, start_pos,
                players[player_id].arena_start,
                players[player_id].arena_end,
                players[player_id].arena_size);
#endif
        memset(&players[player_id].player_name, 0, sizeof(players[player_id].player_name));
        strncpy(players[player_id].player_name, argv[i], sizeof(players[player_id].player_name) - 1);
        // Initialize process
        processes[proc_id].player = &players[player_id];
        processes[proc_id].proc_id = proc_id;
        processes[proc_id].state = ACTIVE;
        for (int j = 0; j < MAX_REGS; ++j) {
            processes[proc_id].registers[j] = 0;
        }
        processes[proc_id].registers[REG_PC] = start_pos;
        vis_proc_init(player_id, proc_id, start_pos, file_size);
        vm->players += 1;
    }

#ifndef VIS
    apply_seccomp();
#endif
    run_game(vm, players, processes);
    vis_finalize();
    fflush(stdout);
    fflush(stderr);
}
