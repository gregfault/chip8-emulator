#chip8-emulator

Emulator written in C, built for me to learn simple CPU work, bit manipulation etc.

CHIP-8 is a virtual machine from 70s made to make games for early microcomputers, this emulator laods a program made for CHIP-8 and runs it.

## Current status

**25 of 35 instructions implemented.** The emulator passes:
- the classic **IBM logo** test
- all 22 checks of the **Timendus corax+ opcode test**

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
- **Flow control:** `00E0` (clear screen), `1NNN` (jump), `2NNN` (call subroutine), `00EE` (return)
- **Conditional skips:** `3XNN`, `4XNN`, `5XY0`, `9XY0`
- **Registers:** `6XNN` (set), `7XNN` (add)
- **Arithmetic and logic:** `8XY0`–`8XY7`, `8XYE`, with carry/borrow flags in VF and the original shift behavior (shifting VY)
- **Memory:** `ANNN`, `FX1E`, `FX33` (BCD), `FX55`, `FX65` (with the original I increment)
- **Display:** `DXYN` (sprites drawn with XOR and collision detection)

The display is currently rendered as text in the terminal.

## Roadmap
- [x] Core instruction set (25 of 35)
- [x] Pass the corax+ opcode test
- [ ] Remaining instructions: `BNNN`, `CXNN`, `EX9E`, `EXA1`, `FX07`, `FX0A`, `FX15`, `FX18`, `FX29`
- [ ] Pass the flags and quirks tests
- [ ] Graphics window
- [ ] Keyboard input
- [ ] Timers and sound
- [ ] Run real games like Pong

## How to run

Build with CMake (for example in CLion), then run the program with the path to a ROM file

## test ROMs

ROMs are not included in this repository. Test ROMs can be downloaded from [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite).
