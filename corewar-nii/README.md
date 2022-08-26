# Corewar-nii

This is the Corewar-x86 for the second day and the third day.

## Challenge type

King of The Hill and Attack/Defense.
Only the Attack/Defense component was released during the finals, with a fixed seed of 1337.

The vm will be running on each team's own box as a service.
While we have a central submission system, teams are allowed and expected to submit their own programs to other teams' services, and leak flags if they want.

Only the central submission system can request teams' services to evaluate other teams' programs to get the King-of-The-Hill result.
All King-of-The-Hill match results will be re-evaluated by the central component.

## Intended Vulnerabilities or Strategies to Acquire the Flag

- One of the instructions incorrectly calculates the memory address that it supposed to be accessing, leading to an arbitrary memory leak.

- Only the host can read their flag from the bottom of their stack.
The attacker can hijack the host's execution flow, read from the bottom of the stack, and then the flag out to a remote endpoint.

- syscalls...

## The X86 emulator

The x86 emulator is a stripped down version of https://github.com/shift-crops/x86emu.
Corewar-nii supports a few more instructions.
The poller will poll these instructions *sometimes* to ensure teams did not just replace the emulation engine with an official one.

## Syscalls

- eax = 0: read(fd, addr, size)

- eax = 1: write(fd, addr, size)

- eax = 2: send_data(ip, addr, size, enc_key)

