# Pop It 2600

A simple, barely functioning game written in pure C for the beloved Stella as part of [Global Game Jam 2025](https://globalgamejam.org/games/2025/pop-it-4-1).

## About

Popping bubbles is fun! This is a simple game for the Atari 2600 where you simply do that, trying to avoid lurking dangers. Simply move the target with the joystick and press the fire button at the right time to pop the cyan bubble while avoiding the enemy with the same colour as the target.

The game was developed and debugged on a real console, but can also be played in [Stella](https://stella-emu.github.io/) by flashing/loading the pop_it.bin file.
The emulator must be set in SECAM mode (Ctrl F once in Stella), as that was the region of the console it was developed against, however PAL colours have been tested and seem okay as well.

In order to control the player, the joystick must be plugged in to the Right controller port due to an unresolved bug.
There are two difficulty settings which can be controlled by setting the right player difficulty switch. (F7/F8 in Stella.)

The challenge behind this was to write a game for the Atari 2600 VCS entirely using the C programming language, without directly writing any 6502 Assembly. Unfortunately the compiled binary is still not fast enough, as rendering the player (target) results in more scanlines being used than intended, thus stretching everything around it vertically.

# How to play

Load the pop_it.bin file in your favourite flash cartridge and play it on a real console (no mappers needed), or open the file in the Stella emulator.

# How to build

Make sure you have cc65 set up, cd into the source dir and run make. The build.sh script is to automate programming a flash cart with the minipro, since I didn't have time to do it properly in the Makefile.
