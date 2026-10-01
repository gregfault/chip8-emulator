# chip8-emulator

Emulator written in C built for me to learn how a CPU works, registers, memory, the stack, bit manipulation etc

CHIP-8 is a virtual machine from 70s made to make games for early microcomputers, this emulator loads a program made for CHIP-8 and runs it.


## Current status

**Finished. All CHIP-8 instructions are implemented**, except `0NNN` which ran native machine code on the original hardware and is ignored by modern emulators. The emulator passes:
- the classic **IBM logo** test
- all 22 checks of the **Timendus corax+ opcode test**
- the **Timendus flags test**
- the **Timendus keypad test** (`EX9E`, `EXA1`, `FX0A`)
- the **Timendus beep test**
- 4 of 6 checks of the **Timendus quirks test** (see possible improvements)

It also runs real games, like Super Pong from the [chip8Archive](https://github.com/JohnEarnest/chip8Archive).

The first program it ever ran, back when the output was printed into the terminal in form of #:

```
            ######## #########   #####         #####
                                                    
            ######## ########### ######       ######
                                                    
              ####     ###   ###   #####     ##### 
                                                    
              ####     #######     ####### ####### 
                                                    
              ####     #######     ### ####### ### 
                                                    
              ####     ###   ###   ###  #####  ### 
                                                    
            ######## ########### #####   ###   #####
                                                    
            ######## #########   #####    #    #####
```

### Implemented instructions
- **Flow control:** `00E0` (clear screen), `1NNN` (jump), `BNNN` (jump with offset), `2NNN` (call subroutine), `00EE` (return)
- **Conditional skips:** `3XNN`, `4XNN`, `5XY0`, `9XY0`
- **Registers:** `6XNN` (set), `7XNN` (add), `CXNN` (random)
- **Arithmetic and logic:** `8XY0`–`8XY7`, `8XYE`, with carry/borrow flags in VF and the original shift behavior (shifting VY)
- **Memory:** `ANNN`, `FX1E`, `FX33` (BCD), `FX55`, `FX65` (with the original I increment), `FX29` (built-in font)
- **Display:** `DXYN` (sprites drawn with XOR and collision detection)
- **Timers:** `FX07`, `FX15`, `FX18`
- **Keyboard:** `EX9E`, `EXA1`, `FX0A` (waits for the key to be pressed and released, like the original)

### Features
- SDL2 window, scaled 20×
- Real-time execution at 60 FPS (11 instructions per frame by default)
- Delay and sound timers counting down at 60 Hz
- Beep (440 Hz square wave) while the sound timer is active
- Keyboard mapped to the original 4×4 hex keypad

## Roadmap
- [x] Core instruction set
- [x] Pass the corax+ opcode test
- [x] Remaining instructions: `BNNN`, `CXNN`, `EX9E`, `EXA1`, `FX07`, `FX0A`, `FX15`, `FX18`, `FX29`
- [x] Pass the flags test
- [x] Graphics window
- [x] Keyboard input
- [x] Timers and sound
- [x] Run real games like Pong
- [x] Split `main` into separate functions

### Future things to improve
- [ ] VF reset quirk (`8XY1`–`8XY3` reset VF on the original hardware)
- [ ] Display wait quirk (at most one sprite drawn per frame)
- [ ] Speed as a command-line option

## Controls

The CHIP-8 keypad is mapped onto the left side of the keyboard:

```
CHIP-8 keypad      Keyboard
1  2  3  C         1  2  3  4
4  5  6  D         Q  W  E  R
7  8  9  E         A  S  D  F
A  0  B  F         Z  X  C  V
```

## How to run

Build with CMake (I use clion). SDL2 is downloaded and built automatically the first time the project is configured, which takes a few minutes. Then run the program with the path to a ROM file


Games are tuned for different speeds. If a game runs too fast or too slow, change `INSTRUCTIONS_PER_FRAME` in `main.c`. The chip8Archive lists the intended speed for each game as `tickrate`.

## Test ROMs

ROMs are not included in this repository. Test ROMs can be downloaded from [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite), and games from [JohnEarnest/chip8Archive](https://github.com/JohnEarnest/chip8Archive).