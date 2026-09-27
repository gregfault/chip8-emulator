#chip8-emulator

Emulator written in C, built for me to learn simple CPU work, bit manipulation etc.

CHIP-8 is a virtual machine from 70s made to make games for early microcomputers, this emulator laods a program made for CHIP-8 and runs it.

## CURRENT STATUS

I made it to the point where the emulator can print out simple IBM logo from instructions.

Implemented so far:
- Loading ROMs into memory at `0x200`
- Fetch-decode-execute loop
- Instructions: `1NNN` (jump), `6XNN` (set register), `7XNN` (add to register), `ANNN` (set index), `DXYN` (draw sprite with XOR and collision detection)
- Display rendered as text in the terminal

## Roadmap
- [ ] remaining instructions
- [ ] graphics window
- [ ] keyboard input
- [ ] timers and sound
- [ ] running games like pong

## How to run

Build with CMake (im using Clion) then run the program with the path to a ROM file:
- chip8 path/to/rom.ch8


