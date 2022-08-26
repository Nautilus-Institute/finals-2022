#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include <openssl/md5.h>

#include "player.hpp"
#include "vm.hpp"
#include "cpu.hpp"
#include "parser.hpp"

#define FLAG_PATH "./flag"


VM::VM()
    : curr_proc_id(0), players(0)
{
    memset(arena, 0xcc, CORE_SIZE);
}


int load_program(VM* vm, int total_player, int player_id, uint8_t* program, size_t program_size,
        size_t random_offset)
{
    if (program_size > MAX_PROGRAM_SIZE) {
        return -1;
    }
    // Calculate start position
    int start_pos = (CORE_SIZE / total_player) * player_id;
    if (player_id != 0) {
        start_pos += random_offset;
    }
    start_pos %= CORE_SIZE;
#ifdef DEBUG
    fprintf(stderr, "start_pos: %#x, size: %d\n", start_pos, program_size);
#endif
    memcpy(vm->arena + start_pos, program, program_size);
    return start_pos;
}


int run_game(VM* vm, std::vector<Player*> players, std::map<int,Process*> & processes)
{
    std::map<int,CPU*> cpus;
    Parser parser;

    // Initialize CPUs
    for (int i = 0; i < processes.size(); ++i) {
        CPU* cpu = new CPU(vm);
        cpus[processes[i]->proc_id] = cpu;
        for (int j = 0; j < REGS_COUNT; ++j) {
            cpu->set_reg((enum reg32_t)j, processes[i]->registers[j]);
        }
    }

    // Load and copy the flag to the bottom of player 0's stack
    std::ifstream flag_file;
    char flag[128] = "FLAG{this_is_the_default_flag_for_corewar_nii}";
    flag_file.open(FLAG_PATH);
    if (flag_file.is_open()) {
        std::string line;
        if (std::getline(flag_file, line)) {
            strncpy(flag, line.c_str(), sizeof(flag) - 1);
        }
        flag_file.close();
    }
    else {
        std::cerr << "Failed to open the flag file. Use the default flag instead." << std::endl;
    }
    CPU* cpu0 = cpus[0];
    if (cpu0 != NULL) {
        for (int j = 0; j < 8; ++j) {
            for (int i = 0; i < sizeof(flag); i += sizeof(uint32_t)) {
                cpu0->push32(*(uint32_t*)(flag + i));
            }
        }
    }

    for (int i = 0; i < MAX_CYCLE; ++i) {
        int alive_players = 0;
        for (int player_id = 0; player_id < vm->players; ++player_id) {
            Player* player = players[player_id];
            if (player->current_procs > 0) {
                alive_players += 1;

                uint16_t proc_offset = player->next_proc_offset;
                Process *proc = processes[player->proc_ids[proc_offset]];
                if (proc->state == ACTIVE) {
                    CPU* cpu = cpus[proc->proc_id];
#ifdef DEBUG
                    fprintf(stderr, "[Proc %d:%#08x (%d)]\n", player->proc_ids[proc_offset], cpu->get_eip(), i);
#endif
                    InstrData instr;
                    parser.parse(vm, cpu, &instr);
                    cpu->exec(&instr);
                    if (cpu->halted()) {
                        // The process is dead
                        proc->state = DEAD;
                    }
                }
                // if the process dies, clean up
                if (proc->state == DEAD) {
                    for (int j = proc_offset; j < player->current_procs - 1; ++j) {
                        player->proc_ids[j] = player->proc_ids[j + 1];
                    }
                    player->current_procs--;

                    // Free CPUs
                    auto cpu_it = cpus.find(proc->proc_id);
                    if (cpu_it != cpus.end()) {
                        CPU* cpu = cpu_it->second;

                        for (int i = 0; i < REGS_COUNT; ++i) {
                            std::cout << "R" << i << ": " << std::hex << cpu->get_reg((enum reg32_t)i) << std::endl;
                        }
                        std::cout << std::endl;

                        delete cpu;
                        cpus.erase(cpu_it);
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
            std::cout << "RESULT: Tie." << std::endl;
            break;
        } else if (alive_players == 1) {
            int last_alive_player = -1;
            for (int player_id = 0; player_id < vm->players; ++player_id) {
                Player* player = players[player_id];
                if (player->current_procs > 0) {
                    if (last_alive_player == -1) {
                        last_alive_player = player_id;
                    }
                    else {
                        std::cerr << "Unexpected: More than one alive players in the end." << std::endl;
                        _exit(1);
                    }
                }
            }
            // winner
            std::cout << "RESULT: Player " << last_alive_player << " won." << std::endl;;
#ifdef AD
            if (last_alive_player == 1) {
                // the away team wins
                // Print the flag!
                std::cout << flag << std::endl;
            }
#endif
            break;
        } else if (i == MAX_CYCLE - 1) {
            // last iteration but players are still alive
            std::cout << "RESULT: Tie." << std::endl;
        }
    }
#ifdef AD
    std::cout << "Maybe this means something to you..." << std::endl;
    unsigned char digest[16] = {0};
    MD5((uint8_t*)flag, strlen(flag), digest);
    for (int i = 0; i < 16; ++i) {
        printf("%02x", digest[i]);
    }
    std::cout << std::endl;
#endif
    return 0;
}


int main(int argc, char** argv)
{
    VM* vm = new VM;;
    std::vector<Player*> players;
    std::map<int,Process*> processes;
    int player_count = argc - 2;
    const int program_start_argc = 2;

    if (argc < 4) {
        std::cerr << "Insufficient number of arguments" << std::endl;
        return 1;
    }

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

#ifdef AD
        // stupid "decryption"
        for (int i = 0; i < file_size; ++i) {
            buf[i] = buf[i] + 9;
            if (i % 2 == 0) {
                buf[i] = buf[i] ^ 0xca;
            } else {
                buf[i] = buf[i] ^ 0x77;
            }
        }
#endif

        int start_pos = load_program(vm, player_count, player_id, buf, file_size, random_offset);
        if (start_pos < 0) {
            std::cerr << "Failed to load program for player " << player_id << std::endl;
            return 2;
        }
        // Initialize player
        uint16_t proc_id = player_id;
        Player* player = new Player;
        players.push_back(player);
        player->player_id = player_id;
        player->proc_ids[0] = proc_id;
        player->current_procs = 1;
        memset(player->player_name, 0, sizeof(player->player_name));
        strncpy(player->player_name, argv[i], sizeof(player->player_name) - 1);
        // Initialize process
        Process* process = new Process;
        processes[proc_id] = process;
        process->proc_id = proc_id;
        process->state = ACTIVE;
        for (int j = 0; j < MAX_REGS; ++j) {
            process->registers[j] = 0;
        }
        process->registers[EIP] = start_pos;
        vm->players += 1;
    }

    if (vm->players > 1) {
        run_game(vm, players, processes);
    }
}
