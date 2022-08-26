# Corewar-ni

This is the traditional core war program that implements the original version of Redcode (without ICWS'86 extensions).
In each round, teams are supposed to submit their assembled programs to the central submission system.
At the end of a round, the submission system will run all core war programs against each other, and generate a large matrix of result.

## Challenge type

King of The Hill.
There are no intentional bugs in this challenge.
The organizers will host a central copy of this challenge.

## Components

`vm`: The Corewar VM.

`vis`: The visulizer that visualizes the trace that the Corewar VM generates.
It was not used during the finals.

`asm`: The assembler. It is *not* provided to teams. Teams are supposed to reverse engineer the VM and develop their own assemblers.
